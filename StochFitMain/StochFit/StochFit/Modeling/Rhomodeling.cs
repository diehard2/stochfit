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
using iTextSharp.text.pdf;
using iTextSharp.text;
using System.Drawing.Imaging;
using StochasticModeling;
using System.Runtime.InteropServices;
using System.Globalization;
using StochasticModeling.Settings;


namespace StochasticModeling
{
    /// <summary>
    /// Fits a model dependent electron density profile to the model independent electron density profile
    /// </summary>
    public partial class Rhomodeling : BoxReflFitBase
    {
        #region Variables

        
    
        private Graphing m_gRhoGraphing;

        #endregion

        /// <summary>
        /// Graphs and fits an Electron Density Profile against a user defined Electron Density Profile
        /// </summary>
        /// <param name="Z">Array of thickness data points</param>
        /// <param name="ERho">Electron density of the profile to be fit</param>
        /// <param name="roughness">The overall smoothing parameter found by the model independent fit</param>
        /// <param name="leftoffset">The offset in Z for the first box</param>
        /// <param name="subsld">The subphase SLD</param>
        /// <param name="supsld">The superphase SLD</param>
        public Rhomodeling(double[] Z, double[] ERho, double roughness, string leftoffset, string subsld, string supsld):base(Z, ERho,roughness, subsld, supsld)
           // base(roughness, 
        {
            InitializeComponent();
            
            
            this.Text = "Profile Modeling";

            //Setup variables
            if (roughness == 0) roughness = 3.0;

            m_dRoughness = roughness;
            SubRough.Text = roughness.ToString();
            Zoffset.Text = leftoffset.ToString();
          
            SubphaseSLD.Text = subsld;
            SupSLDTB.Text = supsld;

            //Set default array values
            if (m_bUseSLD == false)
                Rho2.Text = Rho1.Text = ((double)1.1).ToString();
            else
                Rho2.Text = Rho1.Text = ((double)1.3*double.Parse(subsld)).ToString();

            Sigma2.Text = Sigma1.Text = ((double)3.25).ToString();
            LLength1.Text = LLength2.Text = ((double)15.1).ToString();
            

            //Initialize the arrays
            MakeArrays();

            if(Z == null)
                 Report_btn.Enabled = UndoFit.Enabled = LevenbergFit.Enabled = false;
            
             //Setup the Graph
             m_gRhoGraphing = new Graphing(string.Empty);
             m_gRhoGraphing.SubSLD = double.Parse(subsld);
             m_gRhoGraphing.IsNeutron = m_bUseSLD;

             if (m_bUseSLD == false)
             {
                 m_gRhoGraphing.CreateGraph(RhoGraph, "Electron Density Profile", "Z", "Normalized Electron Density", AxisType.Linear);
                 RhoLabel.Text = "Normalized Rho";
             }
             else
             {
                 m_gRhoGraphing.CreateGraph(RhoGraph, "SLD Profile", "Z", "SLD", AxisType.Linear);
                 RhoLabel.Text = "SLD";
             }

             m_gRhoGraphing.SetGraphType(false, false);

             if (Z != null)
                 m_gRhoGraphing.LoadfromArray("Model Independent Fit", Z, RealRho, System.Drawing.Color.Black, SymbolType.None, 0, true, string.Empty);

             

             MakeArrays();
             ChangeRoughnessArray(roughness);
             GreyFields(); 
             //Create the electron density graph
             UpdateProfile();
        }

        protected override void MakeArrays()
        {
            SubphaseSLDTB = SubphaseSLD;
            BoxCountTB = BoxCount;
            SubRoughTB = SubRough;
            ZOffsetTB = Zoffset;
            SuperSLDTB = SupSLDTB;
            HoldsigmaCB = Holdsigma;
            SubphaseRoughness = SubRoughTB;

            BoxSigmaArray.Add(Sigma1);
            BoxSigmaArray.Add(Sigma2);
            BoxSigmaArray.Add(Sigma3);
            BoxSigmaArray.Add(Sigma4);
            BoxSigmaArray.Add(Sigma5);
            BoxSigmaArray.Add(Sigma6);

            BoxLengthArray.Add(LLength1);
            BoxLengthArray.Add(LLength2);
            BoxLengthArray.Add(LLength3);
            BoxLengthArray.Add(LLength4);
            BoxLengthArray.Add(LLength5);
            BoxLengthArray.Add(LLength6);

            BoxRhoArray.Add(Rho1);
            BoxRhoArray.Add(Rho2);
            BoxRhoArray.Add(Rho3);
            BoxRhoArray.Add(Rho4);
            BoxRhoArray.Add(Rho5);
            BoxRhoArray.Add(Rho6);
        }

        private void ChangeRoughnessArray(double rough)
        {
            SubRough.Text = rough.ToString();
            BoxSigmaArray.ForEach( p => p.Text = rough.ToString());
        }


        private void UpdateProfile()
        {
            //Fill Rho array
            try
            {
                BackupArrays();

                if (Holdsigma.Checked)
                    ChangeRoughnessArray(SubRough.ToDouble());

                //Blank our Rho data from the previous iteration
                RhoArray.Clear();
                BoxRhoArray.ForEach(p => RhoArray.Add(p.ToDouble()));

                //Fill Length array
                LengthArray.Clear();
                BoxLengthArray.ForEach(p => LengthArray.Add(p.ToDouble()));
           
                //Fill Sigma array
                SigmaArray.Clear();
                BoxSigmaArray.ForEach(p => SigmaArray.Add(p.ToDouble()));

                m_dZ_offset = Zoffset.ToDouble();

                if (Z != null)
                {
                    double[] parameters = null;
                    InfoStruct = new BoxModelSettings();
                                 
                    MakeParameters(ref parameters, true, true, BoxCount.ToInt(), 0, SubRough.ToDouble());
                    SetInitStruct(ref InfoStruct, null, null, null);
                    if (ElectronDensityArray != null)
                    {
                        RhoGenerate(InfoStruct, parameters, parameters.Length, ElectronDensityArray, BoxElectronDensityArray);
                    }

                    InfoStruct.Dispose();

                    m_gRhoGraphing.LoadfromArray("Model Dependent Fit", Z, ElectronDensityArray, System.Drawing.Color.Turquoise, SymbolType.None, 0, true, string.Empty);
                    m_gRhoGraphing.LoadfromArray("Model Dependent Box Model", Z, BoxElectronDensityArray, System.Drawing.Color.Red, SymbolType.None, 0, false, string.Empty);
                }
            }
            catch (Exception ex)
            {
               MessageBox.Show(ex.Message.ToString());
            }
        }

        protected override void SetInitStruct(ref StochasticModeling.Settings.BoxModelSettings InitStruct, double[] parampercs, double[] UL, double[] LL)
        {
            base.SetInitStruct(ref InitStruct, parampercs, UL, LL);
            InitStruct.SetZ(Z, RealRho);
        }

    

     
        private void Field_Validated (object sender, EventArgs e)
        {
            GreyFields();
            UpdateProfile();
        }

       

        private void LevenbergFit_Click(object sender, EventArgs e)
        {
            BackupArrays();
          
            double[] info = new double[9];
            double[] parameters = null;
            int arrayconst;
           
            base.MakeParameters(ref parameters, true, Holdsigma.Checked, BoxCount.ToInt(), 0, SubRough.ToDouble());
            
            m_dCovarArray = new double[parameters.Length];
            
            InfoStruct = new BoxModelSettings();
            SetInitStruct(ref InfoStruct, null, null, null);
            

            chisquaretb.Text = Rhofit(InfoStruct, parameters, m_dCovarArray, parameters.Length, info, info.Length).ToString("##.### E-0");

            InfoStruct.Dispose();

            SubRough.Text = parameters[0].ToString();
            Zoffset.Text = parameters[1].ToString();

            if (Holdsigma.Checked == true)
                arrayconst = 2;
            else
                arrayconst = 3;

            //Update paramters
            for (int i = 0; i < BoxCount.ToInt(); i++)
            {
                BoxLengthArray[i].Text = parameters[arrayconst * i + 2].ToString();

                if(m_bUseSLD == false)
                    BoxRhoArray[i].Text = parameters[arrayconst * i + 3].ToString();
                else
                    BoxRhoArray[i].Text = (parameters[arrayconst * i + 3]*SubphaseSLD.ToDouble()).ToString();

                if(Holdsigma.Checked)
                    BoxSigmaArray[i].Text = parameters[0].ToString();
                else
                    BoxSigmaArray[i].Text = parameters[3 * i + 4].ToString();
            }

            //Display the fit
            UpdateProfile();
          
            //Write graph to the master graph
            GraphCollection.Instance.RhoGraph = m_gRhoGraphing;

            SaveParamsforReport();
        }

        private void SaveParamsforReport()
        {
            ReportGenerator g = ReportGenerator.Instance;
            g.ClearRhoModelInfo();

            List<string> info = new List<string>();

            if (Holdsigma.Checked)
            {
                if(m_bUseSLD == false)
                    info.Add("The electron density profile was fit with a single roughness parameter\n");
                else
                    info.Add("The SLD profile was fit with a single roughness parameter\n");
            }
            else
            {
                if(m_bUseSLD == false)
                    info.Add(String.Format("The electron density profile was fit with {0} roughness parameters\n", (BoxCount.ToInt() + 1)));
                else
                    info.Add(String.Format("The SLD profile was fit with {0} roughness parameters\n", (BoxCount.ToInt() + 1)));
            }

            info.Add(string.Format("Chi Square: " + chisquaretb.Text + "\n"));
            info.Add(string.Format("The subphase roughness was: {0:#.### E-0} " + (char)0x00B1 + " {1:#.### E-0}\n", SubRough.ToDouble(), m_dCovarArray[0]));

            int offset = 0;

            if (!Holdsigma.Checked)
                offset = 1;
            
                for (int i = 0; i < BoxCount.ToInt(); i++)
                {
                    info.Add((i + 1).ToString());
                    info.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[(2+offset) * i + 1].ToString("#.### E-0"));
                    
                    if(m_bUseSLD == false)
                        info.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[(2+offset) * i + 2].ToString("#.### E-0"));
                    else
                        info.Add((RhoArray[i]).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (m_dCovarArray[(2+offset) * i + 2]).ToString("#.### E-0"));
                    
                    if(Holdsigma.Checked)
                        info.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[0].ToString("#.### E-0"));
                    else
                        info.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[3 * i + 3].ToString("#.### E-0"));
                }
            g.SetRhoModelInfo = info;
        }

        private void UndoFit_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < 6; i++)
            {
                BoxRhoArray[i].Text = PreviousRhoArray[i].ToString();
                BoxLengthArray[i].Text = PreviousLengthArray[i].ToString() ;
                BoxSigmaArray[i].Text = PreviousSigmaArray[i].ToString();
            }

            SubRough.Text = m_dPrevioussigma.ToString();
            Zoffset.Text = m_dPreviouszoffset.ToString();

            UpdateProfile();
        }

        //Show errors associated with the fit
        private void Report_btn_Click(object sender, EventArgs e)
        {
            LevmarOutput lo = new LevmarOutput();
            lo.DisplayOutput(ErrorReport());
            lo.ShowDialog();
        }

        //Format error reporting
        private string ErrorReport()
        {
            StringBuilder output = new StringBuilder();
            int offset = 0;

            if (!Holdsigma.Checked)
                offset = 1;
            
            if (m_dCovarArray != null)
            {
               
                    output.Append("\u03C3 = " + string.Format("{0:#.### E-0} ", SubRough.ToDouble()) + " " +
                        (char)0x00B1 + " " + m_dCovarArray[0].ToString("#.### E-0") + Environment.NewLine + Environment.NewLine);

                    for (int i = 0; i < BoxCount.ToInt(); i++)
                    {
                        output.Append("Layer " + (i + 1).ToString() + Environment.NewLine);

                        if(m_bUseSLD == false)
                            output.Append("\t" + " \u03C1 = " + RhoArray[i].ToString("#.### E-0") + " " +
                                (char)0x00B1 + " " + m_dCovarArray[(2+offset) * i + 1].ToString("#.### E-0") + Environment.NewLine);
                        else
                            output.Append("\t" + " SLD = " + RhoArray[i].ToString("#.### E-0") + " " +
                                (char)0x00B1 + " " + m_dCovarArray[(2+offset) * i + 1].ToString("#.### E-0") + Environment.NewLine);

                        output.Append("\t" + " Length = " + LengthArray[i].ToString("#.### E-0") + " " +
                        (char)0x00B1 + " " + m_dCovarArray[(2+offset) * i + 2].ToString("#.### E-0") + Environment.NewLine);

                        if(!Holdsigma.Checked)
                           output.Append("\t" + " \u03C3 = " + SigmaArray[i].ToString("#.### E-0") + " " +
                             (char)0x00B1 + " " + m_dCovarArray[3 * i + 3].ToString("#.### E-0") + Environment.NewLine);
                    }
                return output.ToString();
            }
            else
            {
                return "No fitting has been performed";
            }

        }

        private void button1_Click(object sender, EventArgs e)
        {

            if(MessageBox.Show("Do you wish to fit with constraints?", "Constrain", MessageBoxButtons.YesNo) == DialogResult.Yes)
            {
                ConstrainedReflmodeling Refl = new ConstrainedReflmodeling(SubRough.ToDouble(), LengthArray.ToArray(), RhoArray.ToArray(), SigmaArray.ToArray(), BoxCount.ToInt(), Holdsigma.Checked, SubphaseSLD.Text, SupSLDTB.Text);
                Refl.ShowDialog(this);
            }
            else
            {
                Reflmodeling Refl = new Reflmodeling(SubRough.ToDouble(), LengthArray.ToArray(), RhoArray.ToArray(), SigmaArray.ToArray(), BoxCount.ToInt(), Holdsigma.Checked, SubphaseSLD.Text, SupSLDTB.Text);
                Refl.ShowDialog(this);
            }
        }

        private void OnFormClosing(object sender, FormClosingEventArgs e)
        {
            if (m_gRhoGraphing != null && Report_btn.Enabled == true)
                GraphCollection.Instance.RhoGraph = m_gRhoGraphing;
        }

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry 
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to an double or false if not</param>
        protected override void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            base.ValidateNumericalInput(sender, e);
        }

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry 
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to an integer or false if not</param>
        protected override void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            base.ValidateIntegerInput(sender, e);

            if (BoxCount.ToInt()  > 6)
            {
                MessageBox.Show("Six is the maximum number of boxes");
                e.Cancel = true;
            }
        }

        private double GetSubSLD
        {
            get { return double.Parse(SubphaseSLD.Text); }
        }
    }
}