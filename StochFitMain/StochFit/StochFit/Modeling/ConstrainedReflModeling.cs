/* 
 *	Copyright (C) 2008 Stephen Danauskas
 *	
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using ZedGraph;
using System.Threading;
using StochasticModeling.Modeling;
using StochasticModeling.Settings;
using StochasticModeling.Core;
using System.IO;

#pragma warning disable 1591

namespace StochasticModeling
{
    /// <summary>
    /// Form for the Constrained Reflectivity Modeling Routines
    /// </summary>
    public partial class ConstrainedReflmodeling : Form
    {
        #region Variables

        ReflFit ReflCalc;
        Graphing ReflGraphing;
        Graphing RhoGraphing;
       
        List<TextBox> SigmaArray;
        List<TextBox> RhoArray;
        List<TextBox> LengthArray;
        List<CheckBox> HoldSigmaArray;
        List<CheckBox> HoldRhoArray;
        List<CheckBox> HoldLengthArray;
        
        Constraints ConstrForm;

        bool m_bmodelreset = false;
        bool m_bUseSLD = false;
        bool m_isupdating = false;
        bool m_bfitperformed = false;
        bool m_bfitsaved = false;
      
        Thread Stochthread;
        StochOutputWindow ErrorWindow;

        #endregion

        public ConstrainedReflmodeling(RhoFit RhoCalc)
        {
            InitializeComponent();

            ReflCalc = new ReflFit(RhoCalc as BoxReflFitBase);
            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession;

            SigmaArray = new List<TextBox>(6);
            LengthArray = new List<TextBox>(6);
            RhoArray = new List<TextBox>(6);
            HoldSigmaArray = new List<CheckBox>(6);
            HoldLengthArray = new List<CheckBox>(6);
            HoldRhoArray = new List<CheckBox>(6);

            if (m_bUseSLD)
            {
                tabControl1.TabPages[1].Text = "SLD";
                RhoLabel.Text = "SLD";
            }
            else
                tabControl1.TabPages[1].Text = "Electron Density";

            WavelengthTB.Text = ((double)(1.24)).ToString();

            //Initialize the arrays
            MakeArrays();
           
            //Copy over the values
            BoxCount.Text = RhoCalc.BoxCount.ToString();
            SubRough.Text = RhoCalc.GetSubRoughness.ToString();
            SubphaseSLD.Text = RhoCalc.SubphaseSLD.ToString();
            SupSLDTB.Text = RhoCalc.SuperphaseSLD.ToString();
            Holdsigma.Checked = RhoCalc.IsOneSigma;

           
            //Setup the Graph
            ReflGraphing = new Graphing(string.Empty);
            ReflGraphing.SetGraphType(Properties.Settings.Default.ForceRQ4, DBFCB.Checked);
            ReflGraphing.SubSLD = SubphaseSLD.ToDouble();
            ReflGraphing.SupSLD = SupSLDTB.ToDouble();
            ReflGraphing.CreateGraph(RhoGraph, "Reflectivity", "Q/Qc", "Intensity / Fresnel", AxisType.Log);
            ReflGraphing.LoadDatawithErrorstoGraph("Reflectivity Data", System.Drawing.Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
            
            //Set up ED Graph
            RhoGraphing = new Graphing(string.Empty);
            RhoGraphing.SubSLD = SubphaseSLD.ToDouble();
            RhoGraphing.UseSLD = m_bUseSLD;
            RhoGraphing.SetGraphType(false, false);
            
            if (!m_bUseSLD)
                RhoGraphing.CreateGraph(EDzedGraphControl1, "Electron Density Profile", "Z", "Normalized Electron Density", AxisType.Linear);
            else
                RhoGraphing.CreateGraph(EDzedGraphControl1, "SLD Profile", "Z", "SLD", AxisType.Linear);

           
            RhoGraphing.Pane.XAxis.Scale.Min = 0;
            RhoGraphing.Pane.XAxis.Scale.Max = ReflCalc.Z[ReflCalc.Z.Length - 1];

            loadingCircle2.Visible = loadingCircle1.Visible = false;
            loadingCircle2.NumberSpoke = loadingCircle1.NumberSpoke = 25;
            loadingCircle2.InnerCircleRadius = loadingCircle1.InnerCircleRadius = 60;
            loadingCircle2.OuterCircleRadius = loadingCircle1.OuterCircleRadius = 120;
            loadingCircle2.RotationSpeed = loadingCircle1.RotationSpeed = 150;
            loadingCircle2.SpokeThickness = loadingCircle1.SpokeThickness = 3;

            //Initialize constrain form
            ConstrForm = new Constraints(6);
            GreyFields();

            //Setup the callback if the graph updates the bounds
            ReflGraphing.ChangedBounds += new Graphing.ChangedEventHandler(PointChanged);
           
            //Setup the callback to update the frontend with new information
            ReflCalc.Update += new BoxReflFitBase.UpdateProfileHandler(ReflCalc_Update);
            ReflCalc_Update(null, null);
            MakeReflectivity();
        }

        /// <summary>
        /// This routine updates our frontend. All updates are performed in a threadsafe manner, as the
        /// stochastic methods can take place on a separate thread
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void ReflCalc_Update(object sender, EventArgs e)
        {
           //Necessary due to the possibility of updating from a separate thread (such as stochastic fitting). Otherwise, events are fired which reset some of the values
           //below. A lock isn't necessary this isn't necessarilly a race condition
           m_isupdating = true;
           
           SubRough.ThreadSafeSetText(ReflCalc.GetSubRoughness.ToString()) ;
           NormCorrectTB.ThreadSafeSetText(ReflCalc.NormalizationFactor.ToString());
           Holdsigma.ThreadSafeChecked(ReflCalc.IsOneSigma);
           BoxCount.ThreadSafeSetText(ReflCalc.BoxCount.ToString());
           ImpNormCB.ThreadSafeChecked(ReflCalc.ImpNormCB);
            
            //Blank our Rho data from the previous iteration
            for (int i = 0; i < RhoArray.Count; i++)
            {
                if(!m_bUseSLD)
                    RhoArray[i].ThreadSafeSetText(ReflCalc.RhoArray[i].ToString());
                else
                    RhoArray[i].ThreadSafeSetText((ReflCalc.RhoArray[i]/ReflCalc.SubphaseSLD).ToString());

                LengthArray[i].ThreadSafeSetText(ReflCalc.LengthArray[i].ToString());
                SigmaArray[i].ThreadSafeSetText(ReflCalc.SigmaArray[i].ToString());
            }

            if (Holdsigma.Checked)
                ChangeRoughnessArray();

            if (m_bmodelreset)
            {
                ReflGraphing.Clear();
                ReflGraphing.LoadDatawithErrorstoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
                m_bmodelreset = false;
            }

            GreyFields();
            //Update graphs
            RhoGraphing.Pane.XAxis.Scale.Min = 0;
            RhoGraphing.Pane.XAxis.Scale.Max = ReflCalc.Z[ReflCalc.Z.Length - 1];

            ReflGraphing.LoadfromArray("modelrefl", ReflData.Instance.GetQData, ReflCalc.ReflectivityMap, System.Drawing.Color.Black, SymbolType.XCross, 4, true, string.Empty);
            RhoGraphing.LoadfromArray("Model Dependent Fit", ReflCalc.Z, ReflCalc.ElectronDensityArray, System.Drawing.Color.Turquoise, SymbolType.None, 0, true, string.Empty);
            RhoGraphing.LoadfromArray("Model Dependent Box Fit", ReflCalc.Z, ReflCalc.BoxElectronDensityArray, System.Drawing.Color.Red, SymbolType.None, 0, false, string.Empty);

            chisquaretb.ThreadSafeSetText(ReflCalc.MakeChiSquare().ToString());
            FitnessScoreTB.ThreadSafeSetText(ReflCalc.MakeFitnessScore().ToString());

            m_isupdating = false;
        }
        
        private void ChangeRoughnessArray()
        {
            if (Holdsigma.Checked)
            {
                SubRough.ThreadSafeSetText(ReflCalc.GetSubRoughness.ToString());
                SigmaArray.ForEach(p => p.ThreadSafeSetText(ReflCalc.GetSubRoughness.ToString()));
            }
        }

        private void MakeReflectivity()
        {
            //Fill Rho array
            try
            {
                ReflCalc.IsOneSigma = Holdsigma.Checked;
                ReflCalc.ZOffset = 25;
                ReflCalc.GetSubRoughness = SubRough.ToDouble();
                ReflCalc.BoxCount = BoxCount.ToInt();
                ReflCalc.SubphaseSLD = SubphaseSLD.ToDouble();
                ReflCalc.SuperphaseSLD = SupSLDTB.ToDouble();
                ReflCalc.HighQOffset = Rightoffset.ToInt();
                ReflCalc.LowQOffset = CritOffset.ToInt();
                ReflCalc.NormalizationFactor = NormCorrectTB.ToDouble();
                ReflCalc.ImpNormCB = ImpNormCB.Checked;
                ReflCalc.QSpreadTB = QSpreadTB.ToDouble();
                //Blank our Rho data from the previous iteration
                ReflCalc.RhoArray.Clear();
                ReflCalc.LengthArray.Clear();
                ReflCalc.SigmaArray.Clear();

                RhoArray.ForEach(p => ReflCalc.RhoArray.Add(p.ToDouble()));
                LengthArray.ForEach(p => ReflCalc.LengthArray.Add(p.ToDouble()));
                SigmaArray.ForEach(p => ReflCalc.SigmaArray.Add(p.ToDouble()));

                if (Holdsigma.Checked)
                    ChangeRoughnessArray();

                ReflCalc.UpdateProfile();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message.ToString());
            }
        }

        void MakeArrays()
        {

            SigmaArray.Add(Sigma1);
            SigmaArray.Add(Sigma2);
            SigmaArray.Add(Sigma3);
            SigmaArray.Add(Sigma4);
            SigmaArray.Add(Sigma5);
            SigmaArray.Add(Sigma6);

            LengthArray.Add(LLength1);
            LengthArray.Add(LLength2);
            LengthArray.Add(LLength3);
            LengthArray.Add(LLength4);
            LengthArray.Add(LLength5);
            LengthArray.Add(LLength6);

            RhoArray.Add(Rho1);
            RhoArray.Add(Rho2);
            RhoArray.Add(Rho3);
            RhoArray.Add(Rho4);
            RhoArray.Add(Rho5);
            RhoArray.Add(Rho6);

            //Arrays for determining if a value should be held constant during the fitting
            HoldSigmaArray.Add(SigCB1);
            HoldSigmaArray.Add(SigCB2);
            HoldSigmaArray.Add(SigCB3);
            HoldSigmaArray.Add(SigCB4);
            HoldSigmaArray.Add(SigCB5);
            HoldSigmaArray.Add(SigCB6);

            HoldLengthArray.Add(LLCB1);
            HoldLengthArray.Add(LLCB2);
            HoldLengthArray.Add(LLCB3);
            HoldLengthArray.Add(LLCB4);
            HoldLengthArray.Add(LLCB5);
            HoldLengthArray.Add(LLCB6);

            
            HoldRhoArray.Add(RhoCB1);
            HoldRhoArray.Add(RhoCB2);
            HoldRhoArray.Add(RhoCB3);
            HoldRhoArray.Add(RhoCB4);
            HoldRhoArray.Add(RhoCB5);
            HoldRhoArray.Add(RhoCB6);
        }

        void ChangeRoughnessArray(double rough)
        {
            for (int i = 0; i < SigmaArray.Count; i++)
                SigmaArray[i].Text = rough.ToString();
        }
     
        //Update our variables after validation
        private void Variable_Changed(object sender, EventArgs e)
        {
            try
            {
                if (!m_isupdating)
                {
                    RhoGraphing.SupSLD = ReflGraphing.SupSLD = SupSLDTB.ToDouble();
                    RhoGraphing.SubSLD = ReflGraphing.SubSLD = SubphaseSLD.ToDouble();
                    ReflGraphing.SetGraphType(Properties.Settings.Default.ForceRQ4, DBFCB.Checked);
                    ReflGraphing.SetGraphType(Properties.Settings.Default.ForceRQ4, DBFCB.Checked);

                    MakeReflectivity();
                }
            }
            catch { }
        }

        private void GreyFields()
        {
            for (int i = 0; i < 6; i++)
            {
                if (i < int.Parse(BoxCount.Text))
                {
                    RhoArray[i].Enabled = LengthArray[i].Enabled = true;
                   
                    if (Holdsigma.Checked)
                        SigmaArray[i].Enabled = false;
                    else
                        SigmaArray[i].Enabled = true;
                }
                else
                {
                    SigmaArray[i].Enabled = RhoArray[i].Enabled = LengthArray[i].Enabled = false;
                }
            }

            NormCorrectTB.Enabled = ImpNormCB.Checked;
        }

        private void MajorVariable_Changed(object sender, EventArgs e)
        {
            try
            {
                if (!m_isupdating)
                {
                    m_bmodelreset = true;
                    Variable_Changed(sender, e);
                }
            }
            catch { }
        }

        private void LevenbergFit_Click(object sender, EventArgs e)
        {
            GetConstraints();
            
            if (ErrorWindow == null)
            {
                ErrorWindow = new StochOutputWindow();
            }

            ReflCalc.DataFit();

            ErrorWindow.AddModel(ReflCalc);

            m_bfitperformed = true;
          
        }
       

        private void UndoFit_Click(object sender, EventArgs e)
        {
            ReflCalc.UndoFit();
        }

        private void OnFormClosing(object sender, FormClosingEventArgs e)
        {
            if (!Properties.Settings.Default.DisableSavePrompt)
            {
                if (m_bfitperformed && !m_bfitsaved)
                {
                    if (MessageBox.Show("A fit has been performed but not saved, would you like to save the fit?", "Save", MessageBoxButtons.YesNo) == DialogResult.Yes)
                        SaveBT_Click(null, null);

                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            LevmarOutput lo = new LevmarOutput();
            lo.DisplayOutput(ReflCalc.ErrorReport());
            lo.ShowDialog();
        }

        private bool ModelChooserUI(double[] ParamArray, double[] ChiSquareArray, double[] CovarArray, double[] info, int size, int paramcount, BoxModelSettings InfoStruct)
        {
            StochOutputWindow outwin = new StochOutputWindow(ParamArray, size, paramcount, ChiSquareArray, CovarArray, info, ReflCalc);


            if (outwin.ShowDialog() != DialogResult.Cancel)
            {
                outwin.GetParameters(ReflCalc);
                return true;
            }
            else
                return false;
        
        }

        void Stoch()
        {
              double[] parampercs = new double[7];
              StochFitUI UI = new StochFitUI();

              ReflCalc.ModelChooser = new ReflFit.StochasticModel(ModelChooserUI);
              if (UI.ShowDialog() == DialogResult.OK)
              {
                  UI.GetParamPercs(ref parampercs);
                  chisquaretb.ThreadSafeSetText(ReflCalc.StochFit(parampercs, UI.IterationCount));
                  m_bfitperformed = true;
              }
        
              //Restore window
              this.Invoke((ThreadStart)delegate() { DisablePanel(false); }); 
        }


       
        private void button2_Click(object sender, EventArgs e)
        {
            Stochthread = new Thread(delegate() { Stoch(); });
            Stochthread.Start();
            DisablePanel(true);
        }

        private void DisablePanel(bool onoff)
        {
            tabControl1.SelectedIndex = 0;
            controlbox.Enabled = button1.Enabled = UndoFit.Enabled = button2.Enabled = LevenbergFit.Enabled = ClearReportBT.Enabled =
            PrevFitTB.Enabled = SaveBT.Enabled = ConstraintsBT.Enabled = groupBox1.Enabled = tabControl1.Enabled = !onoff;
            loadingCircle1.Active = loadingCircle1.Visible = onoff;
        }
        
        //Set up the constraints
        private void button3_Click(object sender, EventArgs e)
        {
            ConstrForm.ShowDialog(this);
            GetConstraints();
        }

        private void GetConstraints()
        {
            int arrayconst = 2;
            int holdcount = 0;

            if (SubRoughCB.Checked || SubRoughCB.Checked)
                holdcount++;

            //Check if any holds have been set
            Action<CheckBox> f = p => { if (p.Checked) holdcount++; };

            HoldRhoArray.ForEach(f);
            HoldSigmaArray.ForEach(f);
            HoldLengthArray.ForEach(f);

            //Setup all of our constraints if true, if it has been uninitialized, 
            if (ConstrForm.Initialized || holdcount > 0)
            {
                if (!ReflCalc.IsOneSigma) arrayconst = 3;

                ReflCalc.UL = new double[2 + arrayconst * ReflCalc.BoxCount];
                ReflCalc.LL = new double[2 + arrayconst * ReflCalc.BoxCount];
                int ULLength = ReflCalc.UL.Length;

                //Set the subphase constraints
                if (SubRoughCB.Checked)
                {
                    ReflCalc.UL[0] = SubRough.ToDouble();
                    ReflCalc.LL[0] = SubRough.ToDouble();
                }
                else if (!ConstrForm.Initialized)
                {
                    ReflCalc.UL[0] = 10000;
                    ReflCalc.LL[0] = -10000;
                }
                else
                {
                    ReflCalc.UL[0] = ConstrForm.SubRoughMax;
                    ReflCalc.LL[0] = ConstrForm.SubRoughMin;
                }

                //Set the Normalization const
                if (NormCB.Checked)
                {
                    ReflCalc.UL[ULLength - 1] = NormCorrectTB.ToDouble();
                    ReflCalc.LL[ULLength - 1] = NormCorrectTB.ToDouble();
                }
                else if (!ConstrForm.Initialized)
                {
                    ReflCalc.UL[ULLength - 1] = 10000;
                    ReflCalc.LL[ULLength - 1] = -10000;
                }
                else
                {
                    ReflCalc.UL[ULLength - 1] = ConstrForm.NormMax;
                    ReflCalc.LL[ULLength - 1] = ConstrForm.NormMin;
                }

                for (int i = 0; i < ReflCalc.BoxCount; i++)
                {
                    //Check for constraints
                    if (ConstrForm.Initialized)
                    {
                        ReflCalc.UL[arrayconst * i + 1] = ConstrForm.ThickHighArray[i];
                        ReflCalc.LL[arrayconst * i + 1] = ConstrForm.ThickLowArray[i];

                        if (!m_bUseSLD)
                        {
                            ReflCalc.UL[arrayconst * i + 2] = ConstrForm.RhoHighArray[i];
                            ReflCalc.LL[arrayconst * i + 2] = ConstrForm.RhoLowArray[i];
                        }
                        else
                        {
                            if (ReflCalc.SubphaseSLD > 0)
                            {
                                ReflCalc.UL[arrayconst * i + 2] = ConstrForm.RhoHighArray[i] / ReflCalc.SubphaseSLD;
                                ReflCalc.LL[arrayconst * i + 2] = ConstrForm.RhoLowArray[i] / ReflCalc.SubphaseSLD;
                            }
                            else
                            {
                                ReflCalc.LL[arrayconst * i + 2] = ConstrForm.RhoHighArray[i] / ReflCalc.SubphaseSLD;
                                ReflCalc.UL[arrayconst * i + 2] = ConstrForm.RhoLowArray[i] / ReflCalc.SubphaseSLD;
                            }
                        }
                        if (!Holdsigma.Checked)
                        {
                            ReflCalc.UL[arrayconst * i + 3] = ConstrForm.SigmaHighArray[i];
                            ReflCalc.LL[arrayconst * i + 3] = ConstrForm.SigmaLowArray[i];
                        }
                    }
                    else
                    {
                        ReflCalc.UL[arrayconst * i + 2] = ReflCalc.UL[arrayconst * i + 1] = 10000;
                        ReflCalc.LL[arrayconst * i + 2] = ReflCalc.LL[arrayconst * i + 1] = -10000;

                        if (!Holdsigma.Checked)
                        {
                            ReflCalc.UL[arrayconst * i + 3] = 10000;
                            ReflCalc.LL[arrayconst * i + 3] = -10000;
                        }
                    }
                    //Check for holds - holds take precedence over constraints
                    if (HoldLengthArray[i].Checked)
                    {
                        ReflCalc.LL[arrayconst * i + 1] = ReflCalc.UL[arrayconst * i + 1] = LengthArray[i].ToDouble();
                    }
                    if (HoldRhoArray[i].Checked)
                    {
                        ReflCalc.LL[arrayconst * i + 2] = ReflCalc.UL[arrayconst * i + 2] = RhoArray[i].ToDouble();
                    }
                    if (HoldSigmaArray[i].Checked && !Holdsigma.Checked)
                    {
                        ReflCalc.LL[arrayconst * i + 3] = ReflCalc.UL[arrayconst * i + 3] = SigmaArray[i].ToDouble();
                    }
                }
            }
            else
            {
                ReflCalc.UL = null;
                ReflCalc.LL = null;
            }
        }

      
        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to a double or false if not</param>
        protected void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            base.OnValidating(e);
            HelperFunctions.ValidateNumericalInput(sender, e);
        }


        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry. 
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to an integer or false if not</param>
        protected  void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
                base.OnValidating(e);

                HelperFunctions.ValidateIntegerInput(sender, e);

                if (((TextBox)sender).Name == "BoxCount")
                {
                    if (BoxCount.ToInt() > 6)
                    {
                        MessageBox.Show("Six is the maximum number of boxes");
                        e.Cancel = true;
                    }
                }
            }
            catch { }
        }

        #region Offset functions

        private void PointChanged(object sender, EventArgs e)
        {
            CritOffset.Text = ReflGraphing.GetLowQOffset.ToString();
            Rightoffset.Text = (ReflData.Instance.GetNumberDataPoints - ReflGraphing.GetHighQOffset).ToString();
        }

        private void HQoffsetTB_Validated(object sender, EventArgs e)
        {
            ReflGraphing.GetHighQOffset = Rightoffset.ToInt();
            ReflGraphing.SetBounds();
        }


        private void LowQ_TextChanged(object sender, EventArgs e)
        {
            ReflGraphing.GetLowQOffset = CritOffset.ToInt();
            ReflGraphing.SetBounds();
        }

        private void HQ_TextChanged(object sender, EventArgs e)
        {
            ReflGraphing.GetHighQOffset = ReflData.Instance.GetNumberDataPoints - Rightoffset.ToInt();
            ReflGraphing.SetBounds();
        }

        #endregion

        private void PreviousFitListTB_Click(object sender, EventArgs e)
        {
            if (ErrorWindow != null)
            {
                if (ErrorWindow.ShowDialog() == DialogResult.OK)
                {
                    ErrorWindow.GetParameters(ReflCalc);
                }

                ReflCalc.UpdateProfile();
               
            }
            else
            {
                MessageBox.Show("No fitting has been performed");
            }
                
        }

        private void SaveBT_Click(object sender, EventArgs e)
        {
            if (m_bfitperformed && !m_bfitsaved)
            {
                saveFileDialog1.Filter = "Data File | .dat";
                saveFileDialog1.AddExtension = true;
               
                if (saveFileDialog1.ShowDialog() == DialogResult.OK)
                {
                    ReflCalc.WriteFiles(new FileInfo(saveFileDialog1.FileName));
                    //Add the graph to the master graph

                    GraphCollection.Instance.ReflGraph = ReflGraphing;
                    GraphCollection.Instance.ReflEGraph = RhoGraphing;
                    ReflCalc.SaveParamsForReport();

                    m_bfitsaved = true;
                }
            }
           
        }

        private void button3_Click_1(object sender, EventArgs e)
        {
            ReflCalc.ClearReports();
            GraphCollection.Instance.ReflGraph.Hide = true;
            GraphCollection.Instance.ReflEGraph.Hide = true;

            m_bfitsaved = false;
            m_bfitperformed = false;

        }

    }
}