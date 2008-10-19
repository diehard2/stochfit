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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using ZedGraph;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using System.Threading;
using System.Drawing.Imaging;
using StochasticModeling;
using System.Runtime.InteropServices;
using StochasticModeling.Modeling;
using System.Globalization;
using StochasticModeling.Settings;


namespace StochasticModeling
{
    /// <summary>
    /// ReflModeling provides the GUI for a purely model dependent fit of the data
    /// </summary>
    public partial class Reflmodeling :Form
    {
        #region Variables

        public delegate void UpdateGUI(bool onoff);
        public delegate void UpdateProfilefromThread();
        List<TextBox> SigmaArray;
        List<TextBox> LengthArray;
        List<TextBox> RhoArray;
        private ReflFit ReflCalc;
        protected Graphing ReflGraphing;
        protected Graphing RhoGraphing;
        bool m_bUseSLD;
        #endregion

        public Reflmodeling(RhoFit FitBase)
        {
            InitializeComponent();

            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession;
           
            ReflCalc = new ReflFit(FitBase as BoxReflFitBase);

            SigmaArray = new List<TextBox>(6);
            LengthArray = new List<TextBox>(6);
            RhoArray = new List<TextBox>(6);

            if (m_bUseSLD)
            {
                tabControl1.TabPages[1].Text = "SLD";
                RhoLabel.Text = "SLD";
            }
            else
            {
                tabControl1.TabPages[1].Text = "Electron Density";
                RhoLabel.Text = "Normalized Rho";
            }

            ReflCalc.IsOneSigma = ReflCalc.IsOneSigma;
            WavelengthTB.Text = ((double)(1.24)).ToString();




            //Setup the Graph
            ReflGraphing = new Graphing(string.Empty);
            ReflGraphing.SetGraphType(Properties.Settings.Default.ForceRQ4, DBFCB.Checked);
            ReflGraphing.SubSLD = ReflCalc.SubphaseSLD;
            ReflGraphing.SupSLD = ReflCalc.SuperphaseSLD;
            ReflGraphing.CreateGraph(RhoGraph, "Reflectivity", "Q/Qc", "Intensity / Fresnel", AxisType.Log);
            ReflGraphing.LoadDatawithErrorstoGraph("Reflectivity Data", System.Drawing.Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
            ReflGraphing.SetAllFonts("Garamond", 22, 18);

            RhoGraphing = new Graphing(string.Empty);
            RhoGraphing.SubSLD = ReflCalc.SubphaseSLD;
            RhoGraphing.IsNeutron = m_bUseSLD;
            RhoGraphing.SetGraphType(false, false);

            if (m_bUseSLD == false)
                RhoGraphing.CreateGraph(EDzedGraphControl1, "Electron Density Profile", "Z", "Normalized Electron Density",
                  AxisType.Linear);
            else
                RhoGraphing.CreateGraph(EDzedGraphControl1, "SLD Profile", "Z", "SLD", AxisType.Linear);

            RhoGraphing.SetAllFonts("Garamond", 20, 18);

            RhoGraphing.Pane.XAxis.Scale.Min = 0;
            RhoGraphing.Pane.XAxis.Scale.Max = ReflCalc.Get_Z[ReflCalc.Get_Z.Length - 1];

            ////Create the reflectivity density graph
            //initialized = true;
            //UpdateProfile();

            loadingCircle1.Visible = false;
            loadingCircle1.NumberSpoke = 25;
            loadingCircle1.InnerCircleRadius = 60;
            loadingCircle1.OuterCircleRadius = 120;
            loadingCircle1.RotationSpeed = 150;
            loadingCircle1.SpokeThickness = 3;

            loadingCircle2.Visible = false;
            loadingCircle2.NumberSpoke = 25;
            loadingCircle2.InnerCircleRadius = 60;
            loadingCircle2.OuterCircleRadius = 120;
            loadingCircle2.RotationSpeed = 150;
            loadingCircle2.SpokeThickness = 3;

            GreyFields();
            ////Setup the callback if the graph updates the bounds
            //ReflGraphing.ChangedBounds += new Graphing.ChangedEventHandler(PointChanged);
            //BackupArrays();
        }

        private void MakeArrays()
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
        }

       
        void UpdateProfile()
        {
           
            //if (Holdsigma.Checked)
            //    ChangeRoughnessArray(double.Parse(SubRough.Text),ref SubRough);

            //if (initialized == false)
            //    return;

            ////Blank our Rho data from the previous iteration

          //  if (m_bUseSLD == false)
          //  {
          //      RhoArray[0] = Double.Parse(Rho1.Text);
          //      RhoArray[1] = Double.Parse(Rho2.Text);
          //      RhoArray[2] = Double.Parse(Rho3.Text);
          //      RhoArray[3] = Double.Parse(Rho4.Text);
          //      RhoArray[4] = Double.Parse(Rho5.Text);
          //      RhoArray[5] = Double.Parse(Rho6.Text);
          //  }
          //  else
          //  {
          //      RhoArray[0] = Double.Parse(Rho1.Text) / double.Parse(SubphaseSLD.Text);
          //      RhoArray[1] = Double.Parse(Rho2.Text) / double.Parse(SubphaseSLD.Text);
          //      RhoArray[2] = Double.Parse(Rho3.Text) / double.Parse(SubphaseSLD.Text);
          //      RhoArray[3] = Double.Parse(Rho4.Text) / double.Parse(SubphaseSLD.Text);
          //      RhoArray[4] = Double.Parse(Rho5.Text) / double.Parse(SubphaseSLD.Text);
          //      RhoArray[5] = Double.Parse(Rho6.Text) / double.Parse(SubphaseSLD.Text);
          //  }

          //  //Fill Length array
          //  LengthArray[0] = Double.Parse(LLength1.Text);
          //  LengthArray[1] = Double.Parse(LLength2.Text);
          //  LengthArray[2] = Double.Parse(LLength3.Text);
          //  LengthArray[3] = Double.Parse(LLength4.Text);
          //  LengthArray[4] = Double.Parse(LLength5.Text);
          //  LengthArray[5] = Double.Parse(LLength6.Text);

          //  //Fill Sigma array
          //  SigmaArray[0] = Double.Parse(Sigma1.Text);
          //  SigmaArray[1] = Double.Parse(Sigma2.Text);
          //  SigmaArray[2] = Double.Parse(Sigma3.Text);
          //  SigmaArray[3] = Double.Parse(Sigma4.Text);
          //  SigmaArray[4] = Double.Parse(Sigma5.Text);
          //  SigmaArray[5] = Double.Parse(Sigma6.Text);

          //  oldnormfactor = Double.Parse(NormCorrectTB.Text);

          //  double[] eparameters = null;
          ////  MakeParameters(ref eparameters, true);
           
           
            
          //  CheckZLength();

            //RhoGenerate(int.Parse(BoxCount.Text), double.Parse(SubphaseSLD.Text), double.Parse(SupSLDTB.Text), eparameters,
            //        eparameters.Length, Z, Z.Length, ElectronDensityArray, BoxElectronDensityArray, ElectronDensityArray.Length, Holdsigma.Checked);

        }

      

       
        private void CheckZLength()
        {
            //double length = 50;

            //for (int k = 0; k < int.Parse(BoxCount.Text); k++)
            //    length += LengthArray[k];

            //if (length != Z[Z.Length - 1])
            //    for (int i = 0; i < 500; i++)
            //        Z[i] = i * length / 499.0;

            //RhoGraphing.Pane.XAxis.Scale.Min = 0;
            //RhoGraphing.Pane.XAxis.Scale.Max = Z[Z.Length - 1];
        }

        #region Fitting Routines

        private void LevenbergFit_Click(object sender, EventArgs e)
        {
           // double[] parameters = null;
           // double chisquare = 0;

           // BackupArrays();
           //// MakeParameters(ref parameters, false);
           // covar = new double[parameters.Length];
           // chisquare = Calculations.FastReflfit(InfoStruct, parameters, covar, parameters.Length, info, info.Length);
           // UpdateFromFit(parameters, chisquare);
        }

        //Update the pertinent arrays based on the fit
        private void UpdateFromFit(double[] parameters, double chisquare)
        {

            ////Update the ChiSquare
            //chisquaretb.Text = chisquare.ToString("#.### E-000");

            //int arrayconst;

            //if (Holdsigma.Checked)
            //    arrayconst = 2;
            //else
            //    arrayconst = 3;

            ////Make sure parameters are reasonable
            //for (int i = 0; i < parameters.Length; i++)
            //{
            //    if (parameters[i] * 0.3 < covar[i])
            //    {
            //        MessageBox.Show("The error in a fitting parameter is greater than 30% of the parameter value. Check the 'Fit details' button for more information");
            //        break;
            //    }
            //}

            //Update paramters
            //SubRough.Text = parameters[0].ToString();

            //for (int i = 0; i < int.Parse(BoxCount.Text); i++)
            //{
            //    BoxLengthArray[i].Text = parameters[arrayconst * i + 1].ToString();

            //    if (m_bUseSLD == false)
            //        BoxRhoArray[i].Text = parameters[arrayconst * i + 2].ToString();
            //    else
            //        BoxRhoArray[i].Text = (parameters[arrayconst * i + 2] * double.Parse(SubphaseSLD.Text)).ToString();

            //    if (Holdsigma.Checked)
            //        BoxSigmaArray[i].Text = parameters[0].ToString();
            //    else
            //        BoxSigmaArray[i].Text = parameters[3 * i + 3].ToString();
            //}

            //NormCorrectTB.Text = parameters[1 + arrayconst * int.Parse(BoxCount.Text)].ToString();

            ////Display the fit
            //UpdateProfile();

            ////Update report parameters
            //SaveParamsforReport();
            //m_bvalidfit = true;

            //Add the graph to the master graph
            GraphCollection.Instance.ReflGraph = ReflGraphing;
            GraphCollection.Instance.ReflEGraph = RhoGraphing;
        }

   
        void Stoch()
        {
            //double[] parameters;
            //double[] parampercs = new double[7];
            //bool bfitting = false;
            //int boxes = int.Parse(BoxCount.Text);

            ////Setup our ranges
            //double[] qrange = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];
            //double[] reflectivity = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];
            //double[] reflectivityerrors = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];
            //double[] qerrors = null;

            //if (ReflData.Instance.HaveErrorinQ)
            //    qerrors = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];

            //for (int i = 0; i < qrange.Length; i++)
            //{
            //    qrange[i] = (double)Qincrement[int.Parse(CritOffset.Text) + i];
            //    reflectivity[i] = (double)RealRefl[int.Parse(CritOffset.Text) + i];
            //    reflectivityerrors[i] = (double)RealReflErrors[int.Parse(CritOffset.Text) + i];

            //    if (ReflData.Instance.HaveErrorinQ)
            //        qerrors[i] = (double)ReflData.Instance.GetQErrorPt(int.Parse(CritOffset.Text) + i);

            //}

            //if (Holdsigma.Checked == true)
            //{
            //    //consts - # of boxes, subphase SLD,
            //    //variable - Subphase sigma (just one to start), Rho1, length1, Rho2, length2, Z-offset

            //    //Let's set up our parameter order for a system with only one roughness (elastic sheet)
            //    // 0 - Subphase roughness, 2 - 3 Layer 1 Values (Length, Rho), 4 - 5 Layer 2 Values, 6 - 7 Layer 3 Values
            //    // 8 - 9 Layer 4 Values, 10 - 11 Layer 5 Values, 11 - 13 Layer 6 Values
            //    //

            //    //We have to hold the # of boxes and the subphase SLD constant, or else 
            //    //the fit will wildly diverge

            //    parameters = new double[2 + boxes * 2];


            //    parameters[0] = Double.Parse(SubRough.Text);

            //    for (int i = 0; i < boxes; i++)
            //    {
            //        parameters[2 * i + 1] = LengthArray[i];
            //        parameters[2 * i + 2] = RhoArray[i];
            //    }
            //    parameters[1 + boxes * 2] = Double.Parse(NormCorrectTB.Text);

            //    covar = new double[parameters.Length];

            //}
            //else
            //{
            //    //Let's set up our parameter order for a system with only one roughness (elastic sheet)
            //    // 0 - # of boxes 1 - Subphase SLD, 2 - Subphase roughness, 3 - Reflectivity scaling factor (not implemented), 
            //    // 4 - 6 Layer 1 Values (Length, Rho, Sigma), 7 - 9 Layer 2 Values, 10 - 12 Layer 3 Values
            //    // 13 - 15 Layer 4 Values, 16 - 18 Layer 5 Values, 19 - 21 Layer 6 Values
            //    //
            //    // Hold = false, Change = true
            //    //
            //    //We have to hold the # of boxes and the subphase SLD constant, or else 
            //    //the fit will wildly diverge

            //    parameters = new double[2 + boxes * 3];
            //    parameters[0] = Double.Parse(SubRough.Text);

            //    for (int i = 0; i < boxes; i++)
            //    {
            //        parameters[3 * i + 1] = LengthArray[i];
            //        parameters[3 * i + 2] = RhoArray[i];
            //        parameters[3 * i + 3] = SigmaArray[i];
            //    }
            //    parameters[1 + boxes * 3] = Double.Parse(NormCorrectTB.Text);

            //    covar = new double[parameters.Length];
            //}


            ////Do the fit

            //StochFitUI UI = new StochFitUI();

            //if (UI.ShowDialog() == DialogResult.OK)
            //{
            //    UI.GetParamPercs(ref parampercs);
            //    StochOutputWindow outwin;

            //    double[] ParamArray = new double[1000 * parameters.Length];
            //    double[] ChiSquareArray = new double[1000];
            //    double[] CovarArray = new double[1000 * parameters.Length];
            //    int size = 0;

            //    //StochFit(boxes, Double.Parse(SubphaseSLD.Text), Double.Parse(SupSLDTB.Text), Double.Parse(WavelengthTB.Text), parameters, parameters.Length, qrange, qerrors, qrange.Length, reflectivity,
            //    //      reflectivity.Length, reflectivityerrors, covar, covar.Length, info, info.Length, Holdsigma.Checked, false, UI.IterationCount, ParamArray, out size, parampercs, ChiSquareArray, CovarArray,
            //    //      Double.Parse(QSpreadTB.Text), ImpNormCB.Checked);
            //    //outwin = new StochOutputWindow(ParamArray, size, parameters.Length, ChiSquareArray, CovarArray, Holdsigma.Checked, boxes, Double.Parse(SubphaseSLD.Text), Double.Parse(SupSLDTB.Text),
            //    //       Double.Parse(WavelengthTB.Text), Double.Parse(QSpreadTB.Text), ImpNormCB.Checked);

            //    //if (outwin.ShowDialog() != DialogResult.Cancel)
            //    //{
            //    //    bfitting = true;
            //    //    chisquaretb.Text = outwin.GetParameters(out parameters, out covar);
            //    //}
            //}

            ////Make sure parameters are reasonable
            //for (int i = 0; i < parameters.Length; i++)
            //{
            //    if (parameters[i] * 0.3 < covar[i])
            //    {
            //        MessageBox.Show("The error in a fitting parameter is greater than 30% of the parameter value. Check the 'Fit details' button for more information");
            //        break;
            //    }
            //}

            ////Update paramters
            //if (bfitting == true)
            //{
            //     //Update paramters
            //    //Update paramters
            //    if (Holdsigma.Checked == true)
            //    {
            //        SubRough.Text = parameters[0].ToString();
            //        for (int i = 0; i < int.Parse(BoxCount.Text); i++)
            //        {
            //            BoxLengthArray[i].Text = parameters[2 * i + 1].ToString();

            //            if (m_bUseSLD == false)
            //                BoxRhoArray[i].Text = parameters[2 * i + 2].ToString();
            //            else
            //                BoxRhoArray[i].Text = (string)(parameters[2 * i + 2] * double.Parse(SubphaseSLD.Text)).ToString();

            //            BoxSigmaArray[i].Text = parameters[0].ToString();
            //        }
            //        NormCorrectTB.Text = parameters[1 + 2 * int.Parse(BoxCount.Text)].ToString();
            //    }
            //    else
            //    {
            //        SubRough.Text = parameters[0].ToString();

            //        for (int i = 0; i < int.Parse(BoxCount.Text); i++)
            //        {
            //            BoxLengthArray[i].Text = parameters[3 * i + 1].ToString();

            //            if (m_bUseSLD == false)
            //                BoxRhoArray[i].Text = parameters[3 * i + 2].ToString();
            //            else
            //                BoxRhoArray[i].Text = (string)(parameters[3 * i + 2] * double.Parse(SubphaseSLD.Text)).ToString();

            //            BoxSigmaArray[i].Text = parameters[3 * i + 3].ToString();
            //        }
            //        NormCorrectTB.Text = parameters[1 + 3 * int.Parse(BoxCount.Text)].ToString();
            //    }

            //    this.Invoke(new UpdateProfilefromThread(this.UpdateProfile));
            //    LevenbergFit_Click(null, null);
            //}
            ////Restore window
            //this.Invoke(new UpdateGUI(this.DisablePanel), new object[]{false});
        }

        private void button2_Click(object sender, EventArgs e)
        {
            //BackupArrays();
            //Stochthread = new Thread(new ThreadStart(Stoch));
            //DisablePanel(true);
            //Stochthread.Start();
        }

        #endregion

        //private void WriteFullfitFiles()
        //{
        //    using (StreamWriter sw = new StreamWriter("fullreflfit.dat"))
        //    {
        //        string outputstring;
        //        double Qc = Graphing.CalcQc(double.Parse(SubphaseSLD.Text), double.Parse(SupSLDTB.Text), Double.Parse(WavelengthTB.Text));

        //        for (int i = 0; i < RealRefl.Length; i++)
        //        {
        //            double QQc = Qincrement[i] / Qc;
        //            double DBF = ReflectivityMap[i] / Graphing.CalcFresnelPoint((double)Qincrement[i], Qc);
        //            outputstring = (Qincrement[i]).ToString() + " " + (RealRefl[i]).ToString() + " " +
        //                (RealReflErrors[i]).ToString() + " " + ReflectivityMap[i].ToString() + " " + QQc.ToString() + " " + DBF.ToString() + "\n";
        //            sw.Write(outputstring);
        //        }
        //    }
        //}


        private void UndoFit_Click(object sender, EventArgs e)
        {
            ReflCalc.UndoFit();
        }

        #region Reporting Functions
       
       

        #endregion


        private void OnFormClosing(object sender, FormClosingEventArgs e)
        {
            GraphCollection.Instance.ReflGraph = ReflGraphing;
            GraphCollection.Instance.ReflEGraph = RhoGraphing;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            LevmarOutput lo = new LevmarOutput();
            lo.DisplayOutput(ReflCalc.ErrorReport());
            lo.ShowDialog();
        }

        private void DisablePanel(bool onoff)
        {
                tabControl1.SelectedIndex = 0;
                tabControl1.Enabled = groupBox1.Enabled = LevenbergFit.Enabled = button2.Enabled = 
                   UndoFit.Enabled = button1.Enabled = controlbox.Enabled = !onoff;
                loadingCircle1.Visible = loadingCircle1.Active = onoff;
        }

        private void Variable_Changed(object sender, EventArgs e)
        {
            try
            {
                GreyFields();
                ReflGraphing.SupSLD = SupSLDTB.ToDouble();
                ReflGraphing.SubSLD = SubphaseSLD.ToDouble();
                RhoGraphing.SubSLD = SubphaseSLD.ToDouble();
                RhoGraphing.SupSLD = SupSLDTB.ToDouble();
                ReflGraphing.SetGraphType(Properties.Settings.Default.ForceRQ4, DBFCB.Checked);
                ReflCalc.UpdateProfile();
                UpdateProfile();
            }
            catch { }
        }


        private void GreyFields()
        {

            for (int i = 0; i < 6; i++)
            {
                if (i < BoxCount.ToInt())
                {
                    RhoArray[i].Enabled = LengthArray[i].Enabled = true;
                    SigmaArray[i].Enabled = !Holdsigma.Checked;
                }
                else
                {
                    SigmaArray[i].Enabled = RhoArray[i].Enabled = LengthArray[i].Enabled = false;
                }
            }
        }

        private void MajorVariable_Changed(object sender, EventArgs e)
        {
            //try
            //{
            //    m_bmodelreset = true;
            //    Variable_Changed(sender, e);
            //}
            //catch { }
        }

         /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to a double or false if not</param>
        protected virtual void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
                base.OnValidating(e);
                Double.Parse(((TextBox)sender).Text);
            }
            catch
            {
                MessageBox.Show("Error in input - A real number was expected");
                e.Cancel = true;
            }
        }

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry 
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to an integer or false if not</param>
        protected virtual void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
                base.OnValidating(e);
                Convert.ToInt32(((TextBox)sender).Text);
            }
            catch
            {
                MessageBox.Show("Error in input - An integer was expected");
                e.Cancel = true;
            }
        }
        private double GetSubSLD
        {
            get { return double.Parse(SubphaseSLD.Text); }
        }
    


        #region Offset functions

        private void PointChanged(object sender, EventArgs e)
        {
            CritOffset.Text = ReflGraphing.GetLowQOffset.ToString();
            Rightoffset.Text = (ReflData.Instance.GetNumberDataPoints - ReflGraphing.GetHighQOffset).ToString();
        }

        private void HQoffsetTB_Validated(object sender, EventArgs e)
        {
            ReflGraphing.GetHighQOffset = int.Parse(Rightoffset.Text);
            ReflGraphing.SetBounds();
        }


        private void LowQ_TextChanged(object sender, EventArgs e)
        {
            ReflGraphing.GetLowQOffset = int.Parse(CritOffset.Text);
            ReflGraphing.SetBounds();
        }

        private void HQ_TextChanged(object sender, EventArgs e)
        {
            ReflGraphing.GetHighQOffset = ReflData.Instance.GetNumberDataPoints - int.Parse(Rightoffset.Text);
            ReflGraphing.SetBounds();
        }

        #endregion

      
        
    }
}