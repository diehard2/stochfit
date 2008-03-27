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


namespace StochasticModeling
{
    public partial class Rhomodeling : StochFormBase
    {
        [DllImport("LevMardll.dll", EntryPoint = "Rhofit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern double Rhofit(string directory, int boxes, double SLD, double SupSLD, double[] parameters, int paramsize,
			double[] ZRange, int ZSize, double[] ED, int EDsize, double[] covariance,
			int covarsize, double[] info, int infosize, bool onesigma);

        [DllImport("LevMardll.dll", EntryPoint = "RhoGenerate", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern double RhoGenerate(int boxes, double SLD, double supSLD, double[] parameters, int paramsize,
           double[] ZRange, int ZSize, [Out] double[] ED, [Out] double[] BoxED, int EDSize);

        #region Variables

        private double m_dRoughness = 3;
        private double m_dZ_offset = 15;
        private Graphing m_gRhoGraphing;
        //Arrays
        private double[] m_dRhoArray;
        private double[] m_dLengthArray;
        private double[] m_dSigmaArray;
        private double[] m_dPreviousRhoArray;
        private double[] m_dPreviousLengthArray;
        private double[] m_dPreviousSigmaArray;
        private double[] m_dCovarArray;
        private double m_dPrevioussigma;
        private double m_dPreviouszoffset;

        private ArrayList m_alParameters;
        private TextBox[] m_tbBoxSigmaArray;
        private TextBox[] m_tbBoxRhoArray;
        private TextBox[] m_tbBoxLengthArray;
        private double[] m_dZincrement;
        private double[] m_dRho;
        private double[] m_dBoxRho;
        private double[] m_dRealRho;
        private ArrayList m_alHoldList;
        private ArrayList m_alHeldParameters;
        #endregion

        public Rhomodeling(double[] Z, double[] ERho, double roughness, string leftoffset, string subsld, string supsld)
        {
            InitializeComponent();

            //Setup variables
            m_dRoughness = roughness;
            SubRough.Text = roughness.ToString();
            Zoffset.Text = leftoffset.ToString();
            m_alParameters = new ArrayList();
            SubphaseSLD.Text = subsld;
            SupSLDTB.Text = supsld;

            //Initialize the arrays
            MakeArrays();
            m_dRhoArray = new double[6];
            m_dLengthArray = new double[6];
            m_dSigmaArray = new double[6];
            m_dPreviousRhoArray = new double[6];
            m_dPreviousLengthArray = new double[6];
            m_dPreviousSigmaArray = new double[6];
            
            //Set default array values
            for (int i = 0; i < 2; i++)
            {
                m_tbBoxRhoArray[i].Text = ((double)1.1).ToString();
                m_tbBoxSigmaArray[i].Text = ((double)3.25).ToString();
                m_tbBoxLengthArray[i].Text = ((double)15.1).ToString();
            }
            SubphaseSLD.Text = ((double)(9.38)).ToString();

            if (Z != null)
            {
                m_dRho = new double[ERho.Length];
                m_dRealRho = new double[ERho.Length];
                m_dBoxRho = new double[ERho.Length];
                m_dZincrement = new double[Z.Length];

                m_dZincrement = Z;
                m_dRealRho = ERho;
             }
             else
             {
                 LevenbergFit.Enabled = false;
                 UndoFit.Enabled = false;
                 Report_btn.Enabled = false;
             }

             m_alHoldList = new ArrayList();
             m_alHeldParameters = new ArrayList();
            
             //Setup the Graph
             m_gRhoGraphing = new Graphing(string.Empty);
             m_gRhoGraphing.CreateGraph(RhoGraph, "Electron Density Profile", "Z", "Normalized Electron Density",
                  AxisType.Linear);

             if (Z != null)
                 m_gRhoGraphing.LoadfromArray("Model Independent Fit", m_dZincrement, m_dRealRho, System.Drawing.Color.Black, SymbolType.None, 0, true);

             m_gRhoGraphing.SetAllFonts("Garamond", 20, 18);

             ChangeRoughnessArray(roughness);

             //Create the electron density graph
             UpdateProfile();
        }

        private void MakeArrays()
        {
            m_tbBoxSigmaArray = new TextBox[6];

            m_tbBoxSigmaArray[0] = Sigma1;
            m_tbBoxSigmaArray[1] = Sigma2;
            m_tbBoxSigmaArray[2] = Sigma3;
            m_tbBoxSigmaArray[3] = Sigma4;
            m_tbBoxSigmaArray[4] = Sigma5;
            m_tbBoxSigmaArray[5] = Sigma6;

            m_tbBoxLengthArray = new TextBox[6];

            m_tbBoxLengthArray[0] = LLength1;
            m_tbBoxLengthArray[1] = LLength2;
            m_tbBoxLengthArray[2] = LLength3;
            m_tbBoxLengthArray[3] = LLength4;
            m_tbBoxLengthArray[4] = LLength5;
            m_tbBoxLengthArray[5] = LLength6;

            m_tbBoxRhoArray = new TextBox[6];

            m_tbBoxRhoArray[0] = Rho1;
            m_tbBoxRhoArray[1] = Rho2;
            m_tbBoxRhoArray[2] = Rho3;
            m_tbBoxRhoArray[3] = Rho4;
            m_tbBoxRhoArray[4] = Rho5;
            m_tbBoxRhoArray[5] = Rho6;
        }

        private void ChangeRoughnessArray(double rough)
        {
            SubRough.Text = rough.ToString();

            for (int i = 0; i < m_tbBoxSigmaArray.Length; i++)
                m_tbBoxSigmaArray[i].Text = rough.ToString();
        }


        private void UpdateProfile()
        {
            //Fill Rho array
            try
            {
                //Blank our Rho data from the previous iteration
                m_dRhoArray[0] = Double.Parse(Rho1.Text);
                m_dRhoArray[1] = Double.Parse(Rho2.Text);
                m_dRhoArray[2] = Double.Parse(Rho3.Text);
                m_dRhoArray[3] = Double.Parse(Rho4.Text);
                m_dRhoArray[4] = Double.Parse(Rho5.Text);
                m_dRhoArray[5] = Double.Parse(Rho6.Text);

                //Fill Length array
                m_dLengthArray[0] = Double.Parse(LLength1.Text);
                m_dLengthArray[1] = Double.Parse(LLength2.Text);
                m_dLengthArray[2] = Double.Parse(LLength3.Text);
                m_dLengthArray[3] = Double.Parse(LLength4.Text);
                m_dLengthArray[4] = Double.Parse(LLength5.Text);
                m_dLengthArray[5] = Double.Parse(LLength6.Text);

                //Fill Sigma array
                m_dSigmaArray[0] = Double.Parse(Sigma1.Text);
                m_dSigmaArray[1] = Double.Parse(Sigma2.Text);
                m_dSigmaArray[2] = Double.Parse(Sigma3.Text);
                m_dSigmaArray[3] = Double.Parse(Sigma4.Text);
                m_dSigmaArray[4] = Double.Parse(Sigma5.Text);
                m_dSigmaArray[5] = Double.Parse(Sigma6.Text);

                m_dZ_offset = Double.Parse(Zoffset.Text);

                if (m_dZincrement != null)
                {
                    double[] parameters = new double[3 * int.Parse(BoxCount.Text) + 2];

                    parameters[0] = double.Parse(SubRough.Text);
                    parameters[1] = double.Parse(Zoffset.Text);

                    for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                    {
                        parameters[3 * i + 2] = m_dLengthArray[i];
                        parameters[3 * i + 3] = m_dRhoArray[i];
                        parameters[3 * i + 4] = m_dSigmaArray[i];
                    }

                    if (m_dRho != null)
                    {
                        RhoGenerate(int.Parse(BoxCount.Text), double.Parse(SubphaseSLD.Text), double.Parse(SupSLDTB.Text), parameters,
                            parameters.Length, m_dZincrement, m_dZincrement.Length, m_dRho, m_dBoxRho, m_dRho.Length);
                    }

                        m_gRhoGraphing.LoadfromArray("Model Dependent Fit", m_dZincrement, m_dRho, System.Drawing.Color.Turquoise, SymbolType.None, 0, true);
                        m_gRhoGraphing.LoadfromArray("Model Dependent Box Model", m_dZincrement, m_dBoxRho, System.Drawing.Color.Red, SymbolType.None, 0, false);
                }
            }
            catch (Exception ex)
            {
               MessageBox.Show(ex.Message.ToString());
            }
        }

        private void BackupArrays()
        {
            for (int i = 0; i < 6; i++)
            {
                m_dPreviousRhoArray[i] = m_dRhoArray[i];
                m_dPreviousSigmaArray[i] = m_dSigmaArray[i];
                m_dPreviousLengthArray[i] = m_dLengthArray[i];
            }
            m_dPrevioussigma = double.Parse(SubRough.Text);
            m_dPreviouszoffset = double.Parse(Zoffset.Text);
        }

     
        private void Field_Validated (object sender, EventArgs e)
        {
            UpdateProfile();
        }
   

        private void LevenbergFit_Click(object sender, EventArgs e)
        {
            BackupArrays();
          
            double[] info = new double[9];
            double[] parameters;
            int boxnumber = int.Parse(BoxCount.Text);
           
            if (Holdsigma.Checked == true)
            {
                //consts - # of boxes, subphase SLD,
                //variable - Subphase sigma (just one to start), Rho1, length1, Rho2, length2, Z-offset

                //Let's set up our parameter order for a system with only one roughness (elastic sheet)
                // 0 - Subphase roughness, 1 - Z-Offset, 
                // 2 - 3 Layer 1 Values (Length, Rho), 4 - 5 Layer 2 Values, 6 - 7 Layer 3 Values
                // 8 - 9 Layer 4 Values, 10 - 11 Layer 5 Values, 12 - 13 Layer 6 Values
                //
                
                //We have to hold the # of boxes and the subphase SLD constant, or else 
                //the fit will wildly diverge

                parameters = new double[2+2*boxnumber];
               
                //subphase roughness (1 sigma), z-offset, layer 1 length, layer 1 rho, layer2 length, layer 2 rho
                 parameters[0] = double.Parse(SubRough.Text);
                 parameters[1] = double.Parse(Zoffset.Text);

                 for (int i = 0; i < boxnumber; i++)
                 {
                     parameters[2 * i + 2] = m_dLengthArray[i];
                     parameters[2 * i + 3] = m_dRhoArray[i];
                 }
                
                 m_dCovarArray = new double[parameters.Length];

                 chisquaretb.Text = Rhofit(ReflData.Instance.GetWorkingDirectory, boxnumber, double.Parse(SubphaseSLD.Text),double.Parse(SupSLDTB.Text), parameters, parameters.Length,
                        m_dZincrement, m_dZincrement.Length, m_dRealRho, m_dZincrement.Length, m_dCovarArray, m_dCovarArray.Length, info, info.Length, true).ToString("##.### E-0");
            }
            else
            {
                //consts - # of boxes, subphase SLD,
                //variable - Subphase sigma, Z-offset , Rho1, length1, sigma1, Rho2, length2, sigma2, etc

                //Let's set up our parameter order for a system with only one roughness (elastic sheet)
                // 0 - Subphase roughness, 1 - Z-Offset, 
                // 2 - 5 Layer 1 Values (Length, Rho, Sigma), 6 - 8 Layer 2 Values, 9 - 11 Layer 3 Values
                // 12 - 14 Layer 4 Values, 15 - 17 Layer 5 Values, 18 - 20 Layer 6 Values
                //
                //We have to hold the # of boxes and the subphase SLD constant, or else 
                //the fit will wildly diverge
                parameters = new double[2 + 3 * boxnumber];
               
                //subphase roughness (1 sigma), z-offset, layer 1 length, layer 1 rho, layer2 length, layer 2 rho
                parameters[0] = double.Parse(SubRough.Text);
                parameters[1] = double.Parse(Zoffset.Text);

                for (int i = 0; i < boxnumber; i++)
                {
                    parameters[3 * i + 2] = m_dLengthArray[i];
                    parameters[3 * i + 3] = m_dRhoArray[i];
                    parameters[3 * i + 4] = m_dSigmaArray[i];
                }

                m_dCovarArray = new double[parameters.Length];

                chisquaretb.Text = Rhofit(ReflData.Instance.GetWorkingDirectory, boxnumber, double.Parse(SubphaseSLD.Text), double.Parse(SupSLDTB.Text), parameters, parameters.Length,
                   m_dZincrement, m_dZincrement.Length, m_dRealRho, m_dZincrement.Length, m_dCovarArray, m_dCovarArray.Length, info, info.Length, false).ToString("##.### E-0");
            }

            //Update paramters
            if (Holdsigma.Checked == true)
            {
                SubRough.Text = parameters[0].ToString();
                Zoffset.Text = parameters[1].ToString();

                for (int i = 0; i < boxnumber; i++)
                {
                    m_tbBoxLengthArray[i].Text = parameters[2 * i + 2].ToString();
                    m_tbBoxRhoArray[i].Text = parameters[2 * i + 3].ToString();
                    m_tbBoxSigmaArray[i].Text = parameters[0].ToString();
                }
            }
            else
            {
                SubRough.Text = parameters[0].ToString();
                Zoffset.Text = parameters[1].ToString();

                for (int i = 0; i < boxnumber; i++)
                {
                    m_tbBoxLengthArray[i].Text = parameters[3 * i + 2].ToString();
                    m_tbBoxRhoArray[i].Text = parameters[3 * i + 3].ToString();
                    m_tbBoxSigmaArray[i].Text = parameters[3 * i + 4].ToString();
                }
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

            ArrayList info = new ArrayList();

            if (Holdsigma.Checked)
            {
               info.Add("The electron density profile was fit with a single roughness parameter\n");
            }
            else
            {
                info.Add(String.Format("The electron density profile was fit with {0} roughness parameters\n", (int.Parse(BoxCount.Text) + 1)));
            }

            info.Add(String.Format("Chi Square for reflectivity fit: " + chisquaretb.Text + "\n"));
            info.Add(String.Format("The subphase roughness was: {0:#.### E-0} " + (char)0x00B1 + " {1:#.### E-0}\n", double.Parse(SubRough.Text), m_dCovarArray[0]));
           
            if (Holdsigma.Checked == true)
            {
                for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                {
                    info.Add((i + 1).ToString());
                    info.Add(m_dLengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[2 * i + 1].ToString("#.### E-0"));
                    info.Add(m_dRhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[2 * i + 2].ToString("#.### E-0"));
                    info.Add(m_dSigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[0].ToString("#.### E-0"));
                }
            }
            else
            {
                for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                {
                    info.Add((i + 1).ToString());
                    info.Add(m_dLengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[3 * i + 1].ToString("#.### E-0"));
                    info.Add(m_dRhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[3 * i + 2].ToString("#.### E-0"));
                    info.Add(m_dSigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[3 * i + 3].ToString("#.### E-0"));
                }
            }

            g.SetRhoModelInfo = info;
        }

        private void UndoFit_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < 6; i++)
            {
                m_tbBoxRhoArray[i].Text = m_dPreviousRhoArray[i].ToString();
                m_tbBoxLengthArray[i].Text = m_dPreviousLengthArray[i].ToString() ;
                m_tbBoxSigmaArray[i].Text = m_dPreviousSigmaArray[i].ToString();
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
        
            if (m_dCovarArray != null)
            {
                if (Holdsigma.Checked == true)
                {
                    output.Append("\u03C3 = " + string.Format("{0:#.### E-0} ", double.Parse(SubRough.Text)) + " " +
                        (char)0x00B1 + " " + m_dCovarArray[0].ToString("#.### E-0") + Environment.NewLine+Environment.NewLine);

                    for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                    {
                        output.Append("Layer " + (i + 1).ToString() + Environment.NewLine);
                        output.Append("\t" + " \u03C1 = " + m_dRhoArray[i].ToString("#.### E-0") + " " +
                        (char)0x00B1 + " " + m_dCovarArray[2 * i + 1].ToString("#.### E-0") + Environment.NewLine);
                        output.Append("\t" + " Length = " + m_dLengthArray[i].ToString("#.### E-0") + " " +
                        (char)0x00B1 + " " + m_dCovarArray[2 * i + 2].ToString("#.### E-0") + Environment.NewLine);
                    }
                }
                else
                {
                    output.Append("Subphase " + "\u03C3 = " + string.Format("{0:#.### E-0} ", double.Parse(SubRough.Text)) + " " +
                        (char)0x00B1 + " " + m_dCovarArray[0].ToString("#.### E-0") + Environment.NewLine+Environment.NewLine);
                   
                    for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                    {
                        output.Append("Layer " + (i + 1).ToString()+Environment.NewLine);
                        output.Append("\t" + " \u03C1 = " + m_dRhoArray[i].ToString("#.### E-0") + " " +
                            (char)0x00B1 + " " + m_dCovarArray[3 * i + 1].ToString("#.### E-0") + Environment.NewLine);
                        output.Append("\t" + " Length = " + m_dLengthArray[i].ToString("#.### E-0") + " " +
                            (char)0x00B1 + " " + m_dCovarArray[3 * i + 2].ToString("#.### E-0") + Environment.NewLine);
                        output.Append("\t" + " \u03C3 = " + m_dSigmaArray[i].ToString("#.### E-0") + " " +
                            (char)0x00B1 + " " + m_dCovarArray[3 * i + 3].ToString("#.### E-0") + Environment.NewLine);
                    }
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
                ConstrainedReflmodeling Refl = new ConstrainedReflmodeling(double.Parse(SubRough.Text), m_dLengthArray, m_dRhoArray, m_dSigmaArray, int.Parse(BoxCount.Text), Holdsigma.Checked, SubphaseSLD.Text, SupSLDTB.Text);
                Refl.ShowDialog(this);
                Refl.Dispose();
            }
            else
            {
                Reflmodeling Refl = new Reflmodeling(double.Parse(SubRough.Text), m_dLengthArray, m_dRhoArray, m_dSigmaArray,int.Parse(BoxCount.Text),Holdsigma.Checked, SubphaseSLD.Text, SupSLDTB.Text);
                Refl.ShowDialog(this);
                Refl.Dispose();
            }
        }

        private void OnFormClosing(object sender, FormClosingEventArgs e)
        {
            if (m_gRhoGraphing != null && Report_btn.Enabled == true)
            {
                GraphCollection.Instance.RhoGraph = m_gRhoGraphing;
            }
        }


        protected override void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            base.ValidateNumericalInput(sender, e);
        }

        protected override void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            base.ValidateIntegerInput(sender, e);

            if (int.Parse(BoxCount.Text) > 6)
            {
                MessageBox.Show("Six is the maximum number of boxes");
                e.Cancel = true;
            }
        }
    }
}