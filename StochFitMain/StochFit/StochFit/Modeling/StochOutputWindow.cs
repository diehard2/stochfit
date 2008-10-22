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
using System.Text;
using System.Windows.Forms;
using ZedGraph;

namespace StochasticModeling.Modeling
{
    /// <summary>
    /// Displays the fits collected from the stochastic parameter space search
    /// </summary>
    public partial class StochOutputWindow : Form
    {
        double[][] ParameterArray;
        double[][] CovarArray;
        double[][] NewRParameter;
        double[][] NewEParameter;
        double[][] Z;
        double[][] ElectronDensityArray;
        double[][] BoxElectronDensityArray;
        double[] ChiSquareArray;
        string[] ModelInfoStringArray;
        double[][] RhoArray;
        double[][] LengthArray;
        double[][] SigmaArray;
        double[] SubRoughArray;
        double[] Qincrement, QErrors;
        double[][] ReflectivityMap;
        double[] RealReflErrors;
        double[] RealRefl;
        double m_dSubSLD;
        double m_dSupSLD;
        bool m_bonesigma;
        bool m_bimpnorm;
        int m_iboxes;
        private double[] selectedmodel;
        private double[] selectedcovar;
        private double selectedchisquare;
        private bool m_bUseSLD = false;
        Graphing ReflGraphing;
        Graphing RhoGraphing;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="FullParameterArray">Contains all of the parameters for all of the fits. This is parsed
        /// internally</param>
        /// <param name="ParameterArraysize">The total number of fits contained in FullParameterArray</param>
        /// <param name="paramsize">The element count of each parameter set contained in FullParameterArray </param>
        /// <param name="FullChisquareArray">The array of all chisquare valued for each fit</param>
        /// <param name="FullCovariance">The covariance matrix for each fit</param>
        /// <param name="OneSigma">True if the fits were performed treating the system as an elastic sheet, false otherwise</param>
        /// <param name="boxes">The number of boxes for the fit</param>
        /// <param name="SubSLD">The substrate SLD</param>
        /// <param name="SupSLD">The superphase SLD</param>
        /// <param name="wavelength">The x-ray wavelength used</param>
        /// <param name="QSpread">The percent error in Q used (if applicable)</param>
        /// <param name="Impnorm">True if the fit was performed assuming an imperfect normalization, false otherwise</param>
        public StochOutputWindow(double[] FullParameterArray, int ParameterArraysize, int paramsize, double[] FullChisquareArray, double[] FullCovariance, bool OneSigma, int boxes, double SubSLD, double SupSLD, double wavelength, double QSpread, bool Impnorm)
        {
            InitializeComponent();
            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession; 
            ParameterArray = new double[ParameterArraysize][];
            CovarArray = new double[ParameterArraysize][];
            RhoArray = new double[ParameterArraysize][];
            LengthArray = new double[ParameterArraysize][];
            SigmaArray = new double[ParameterArraysize][];
            SubRoughArray = new double[ParameterArraysize];
            ChiSquareArray = new double[ParameterArraysize];
            ModelInfoStringArray = new string[ParameterArraysize];
            NewRParameter = new double[ParameterArraysize][];
            NewEParameter = new double[ParameterArraysize][];
            ReflectivityMap = new double[ParameterArraysize][];
            ChiSquareArray = (double[])FullChisquareArray.Clone();
            m_dSubSLD = SubSLD;
            m_dSupSLD = SupSLD;
            m_bonesigma = OneSigma;
            m_iboxes = boxes;
            m_bimpnorm = Impnorm;
            selectedmodel = new double[paramsize];
            selectedcovar = new double[paramsize];

            //Fill the ParameterArray
            for (int i = 0; i < ParameterArraysize; i++)
            {
                ParameterArray[i] = new double[paramsize];
                CovarArray[i] = new double[paramsize];
                for (int j = 0; j < paramsize; j++)
                {
                    ParameterArray[i][j] = FullParameterArray[i * paramsize + j];
                    CovarArray[i][j] = FullCovariance[i * paramsize + j];
                }
               
            }

            //Fill the list
            string itemname;
            for (int i = 0; i < ParameterArraysize; i++)
            {
                itemname = "Model " + (i+1).ToString() + " - Fit Score = " + ChiSquareArray[i].ToString("#.### E-000");
                ModelLB.Items.Add(itemname);
            }

            //Get information for each model
            for (int i = 0; i < ParameterArraysize; i++)
            {
                ModelInfoStringArray[i] = ModelParameterString(ParameterArray[i], CovarArray[i]);
            }
     
            //Setup the graphs

            ReflGraphing = new Graphing(string.Empty);
            ReflGraphing.SetGraphType(Properties.Settings.Default.ForceRQ4, true);
            ReflGraphing.SubSLD = SubSLD;
            ReflGraphing.SupSLD = SupSLD;
            ReflGraphing.CreateGraph(ReflGraph, "Reflectivity", "Q/Qc", "Intensity / Fresnel", AxisType.Log);
            ReflGraphing.LoadDatawithErrorstoGraph("Reflectivity Data", System.Drawing.Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
            ReflGraphing.SetAllFonts("Garamond", 22, 18);


            RhoGraphing = new Graphing(string.Empty);
            RhoGraphing.SubSLD = m_dSubSLD;
            RhoGraphing.IsNeutron = m_bUseSLD;
            RhoGraphing.SetGraphType(false, false);

            if (m_bUseSLD == false)
                RhoGraphing.CreateGraph(RhoGraph, "Electron Density Profile", "Z", "Normalized Electron Density",
                  AxisType.Linear);
            else
                RhoGraphing.CreateGraph(RhoGraph, "SLD Profile", "Z", "SLD", AxisType.Linear);

            RhoGraphing.SetAllFonts("Garamond", 20, 18);
            //Get our Q data into a useable form
            FillRealData();

            //Create Z
            Z = new double[ParameterArraysize][];
            ElectronDensityArray = new double[ParameterArraysize][];
            BoxElectronDensityArray = new double[ParameterArraysize][];
            //Move the models into arrays
            MakeParamArrays();

            //Make the Z Arrays
            for(int j = 0; j < ParameterArraysize; j++)
            {
                Z[j] = new double[500];
                ElectronDensityArray[j] = new double[500];
                BoxElectronDensityArray[j] = new double[500];

                double length = 0;

                for(int k = 0; k < LengthArray[j].Length; k++)
                {
                    length += LengthArray[j][k];
                }
                for(int i = 0; i < 500; i++)
                {
                    Z[j][i] =  i*(75 + length)/500.0;

                }
            }

            //Make the reflectivities and electron densities
            for (int i = 0; i < ParameterArraysize; i++)
            {
                //RhoGenerate(m_iboxes, m_dSubSLD, m_dSupSLD, NewEParameter[i],
                //    NewEParameter[i].Length, Z[i], Z[i].Length, ElectronDensityArray[i], BoxElectronDensityArray[i], ElectronDensityArray.Length);

                //FastReflGenerate(m_iboxes, m_dSubSLD, m_dSupSLD, wavelength, NewRParameter[i], NewRParameter[i].Length,
                //  Qincrement, QErrors, Qincrement.Length, ReflectivityMap[i], ReflectivityMap[i].Length, QSpread, Impnorm);
            }

            ModelLB.SelectedIndex = 0;
            ModelLB.Focus();

            //Play a chime when the stochastic methods are complete
            System.Media.SystemSounds.Exclamation.Play();
        }



        private void MakeParamArrays()
        {
            if (m_bonesigma == true)
            {
                for(int j = 0; j < ParameterArray.Length; j++)
                {
                    NewRParameter[j] = new double[m_iboxes * 3 + 2];
                    NewEParameter[j] = new double[m_iboxes * 3 + 2];
                    LengthArray[j] = new double[m_iboxes];
                    NewRParameter[j][0] = Math.Abs(ParameterArray[j][0]);
                    NewEParameter[j][0] = Math.Abs(ParameterArray[j][0]);
                    NewEParameter[j][1] = 25;

                    for (int i = 0; i < m_iboxes; i++)
                    {
                        NewRParameter[j][3*i+1] = ParameterArray[j][2 * i + 1];
                        NewRParameter[j][3*i+2] = ParameterArray[j][2 * i + 2];
                        LengthArray[j][i] = Math.Abs(ParameterArray[j][2 * i + 1]);
                        NewRParameter[j][3*i+3] = ParameterArray[j][0];
                        NewEParameter[j][3 * i + 2] = ParameterArray[j][2 * i + 1];
                        NewEParameter[j][3 * i + 3] = ParameterArray[j][2 * i + 2];
                        NewEParameter[j][3 * i + 4] = Math.Abs(ParameterArray[j][0]);
                    }
                    NewRParameter[j][3*m_iboxes+1] = ParameterArray[j][2 * m_iboxes + 1];
                }
            }
            else
            {
                for (int j = 0; j < ParameterArray.Length; j++)
                {

                    NewRParameter[j] = new double[m_iboxes * 3 + 2];
                    NewEParameter[j] = new double[m_iboxes * 3 + 2];
                    LengthArray[j] = new double[m_iboxes];
                    NewRParameter[j][0] = Math.Abs(ParameterArray[j][0]);
                    NewEParameter[j][0] = Math.Abs(ParameterArray[j][0]);
                    NewEParameter[j][1] = 25;

                    for (int i = 0; i < m_iboxes; i++)
                    {
                        NewRParameter[j][3*i+1] = ParameterArray[j][3 * i + 1];
                        NewRParameter[j][3*i+2] = ParameterArray[j][3 * i + 2];
                        LengthArray[j][i] = ParameterArray[j][3 * i + 1];
                        NewRParameter[j][3*i+3] = Math.Abs(ParameterArray[j][3 * i + 3]);
                        NewEParameter[j][3 * i + 2] = ParameterArray[j][3 * i + 1];
                        NewEParameter[j][3 * i + 3] = ParameterArray[j][3 * i + 2];
                        NewEParameter[j][3 * i + 4] = Math.Abs(ParameterArray[j][3 * i + 3]);
                    }
                    NewRParameter[j][3 * m_iboxes + 1] = ParameterArray[j][3 * m_iboxes + 1];
                }
               }


         }
        
         private void OnModelIndexChange(object sender, EventArgs e)
        {
            if (ModelLB.SelectedIndex != -1)
            {
                ParametersTB.Text = ModelInfoStringArray[ModelLB.SelectedIndex];
                UpdateGraphs(ModelLB.SelectedIndex);
            }
        }

        private void UpdateGraphs(int index)
        {
            RhoGraphing.LoadfromArray("Model Dependent Fit", Z[index], ElectronDensityArray[index], System.Drawing.Color.Turquoise, SymbolType.None, 0, true, string.Empty);
            RhoGraphing.LoadfromArray("Model Dependent Box Fit", Z[index], BoxElectronDensityArray[index], System.Drawing.Color.Red, SymbolType.None, 0, false, string.Empty);
            ReflGraphing.LoadfromArray("Model Dependent Fit", Qincrement, ReflectivityMap[index], System.Drawing.Color.Black, SymbolType.XCross, 4, true, string.Empty);
        }

        private string ModelParameterString(double[] parameters, double[] covar)
        {
            StringBuilder output = new StringBuilder();

            if (covar != null)
            {
                if (m_bonesigma == true)
                {
                    output.Append("\u03C3 = " + string.Format("{0:#.### E-0} ", parameters[0]) + " " +
                        (char)0x00B1 + " " + covar[0].ToString("#.### E-0") + Environment.NewLine + Environment.NewLine);

                    for (int i = 0; i < m_iboxes ; i++)
                    {
                        output.Append("Layer " + (i + 1).ToString() + Environment.NewLine);
                        
                        if(m_bUseSLD == false)
                            output.Append("\t" + " \u03C1 = " + parameters[2*i+2].ToString("#.### E-0") + " " +
                                (char)0x00B1 + " " + covar[2 * i + 2].ToString("#.### E-0") + Environment.NewLine);
                        else
                            output.Append("\t" + "SLD = " + (parameters[2 * i + 2]*m_dSubSLD).ToString("#.### E-0") + " " +
                                (char)0x00B1 + " " + (covar[2 * i + 2] * m_dSubSLD).ToString("#.### E-0") + Environment.NewLine);

                        output.Append("\t" + " Length = " + parameters[2*i+1].ToString("#.### E-0") + " " +
                        (char)0x00B1 + " " + covar[2 * i + 1].ToString("#.### E-0") + Environment.NewLine);
                    }
                }
                else
                {
                    output.Append("Subphase " + "\u03C3 = " + string.Format("{0:#.### E-0} ", parameters[0]) + " " +
                        (char)0x00B1 + " " + covar[0].ToString("#.### E-0") + Environment.NewLine + Environment.NewLine);

                    for (int i = 0; i < m_iboxes; i++)
                    {
                        output.Append("Layer " + (i + 1).ToString() + Environment.NewLine);

                        if(m_bUseSLD == false)
                            output.Append("\t" + " \u03C1 = " + parameters[3*i+2].ToString("#.### E-0") + " " +
                                (char)0x00B1 + " " + covar[3 * i + 2].ToString("#.### E-0") + Environment.NewLine);
                        else
                            output.Append("\t" + "SLD = " + (parameters[3*i+2]*m_dSubSLD).ToString("#.### E-0") + " " +
                                (char)0x00B1 + " " + (covar[3 * i + 2]*m_dSubSLD).ToString("#.### E-0") + Environment.NewLine);

                        output.Append("\t" + " Length = " + parameters[3*i+1].ToString("#.### E-0") + " " +
                            (char)0x00B1 + " " + covar[3 * i + 1].ToString("#.### E-0") + Environment.NewLine);
                        output.Append("\t" + " \u03C3 = " + parameters[3*i+3].ToString("#.### E-0") + " " +
                            (char)0x00B1 + " " + covar[3 * i + 3].ToString("#.### E-0") + Environment.NewLine);

                    }
                }

                if (m_bimpnorm)
                    output.Append(Environment.NewLine + "Normalization factor = " + parameters[parameters.Length - 1].ToString("#.###") + " " +
                            (char)0x00B1 + " " + covar[covar.Length-1].ToString("#.### E-0") + Environment.NewLine);

                return output.ToString();
            }
            else
            {
                return "No fitting has been performed";
            }


        }

        private string termreason(int reason)
        {
            switch (reason)
            {
                case 1:
                    return "Stopped by small gradient J^T e - OK";
                case 2:
                    return "Stopped by small Dp - OK";
                case 3:
                    return "Stopped by itmax - Likely Failure";
                case 4:
                    return "Singular matrix. Restart from current p with increased \u03BC - Failure";
                case 5:
                    return "No further error reduction is possible. Restart with increased \u03BC - Failure";
                case 6:
                    return "Stopped by small error - OK";
                default:
                    return "?";
            }
        }

       private void FillRealData()
        {
            Qincrement = ReflData.Instance.GetQData;
            RealRefl = ReflData.Instance.GetReflData;
            RealReflErrors = ReflData.Instance.GetRErrors;
            QErrors = ReflData.Instance.GetQErrors;

            //Allocate room for the rest of the arrays
            for (int i = 0; i < ParameterArray.Length; i++)
            {
                ReflectivityMap[i] = new double[ReflData.Instance.GetNumberDataPoints];
            }
        }

        private void SelModelBT_Click(object sender, EventArgs e)
        {
            if (ModelLB.SelectedIndex != -1)
            {
                selectedmodel = ParameterArray[ModelLB.SelectedIndex];
                selectedchisquare = ChiSquareArray[ModelLB.SelectedIndex];
                selectedcovar = CovarArray[ModelLB.SelectedIndex];
                this.DialogResult = DialogResult.OK;
            }
            
                this.Close();
            

            
        }

        internal string GetParameters(out double[] parameters,out  double[] covar)
        {
            parameters = (double[])selectedmodel.Clone();
            covar = (double[])selectedcovar.Clone();
            return selectedchisquare.ToString("#.### E-0");
        }
    }
}