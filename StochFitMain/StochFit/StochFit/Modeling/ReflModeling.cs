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


namespace StochasticModeling
{
    /// <summary>
    /// ReflModeling provides the GUI for a purely model dependent fit of the data
    /// </summary>
    public partial class Reflmodeling : StochFormBase
    {
        #region Variables

        double m_roughness = 3;
        bool m_bvalidfit = false;
        bool m_bUseSLD = false;
        Graphing ReflGraphing;
        Graphing RhoGraphing;
        //Arrays
        double[] RhoArray;
        double[] LengthArray;
        double[] SigmaArray;
        double[] PreviousRhoArray;
        double[] PreviousLengthArray;
        double[] PreviousSigmaArray;
        double[] covar;
        double[] info;
        double oldnormfactor;
        double previoussigma;
        bool initialized = false;
        TextBox[] BoxSigmaArray;
        TextBox[] BoxRhoArray;
        TextBox[] BoxLengthArray;
        double[] Qincrement;
        double[] QErrors;
        double[] ReflectivityMap;
        double[] Z;
        double[] ElectronDensityArray;
        double[] BoxElectronDensityArray;
        double[] RealReflErrors;
        double[] RealRefl;
        bool m_bmodelreset = false;
        Thread Stochthread;

        public delegate void UpdateGUI(bool onoff);
        public delegate void UpdateProfilefromThread();

        #endregion

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="roughness">Overall roughness value</param>
        /// <param name="inLength">Length array with element count boxnumber</param>
        /// <param name="inRho">Electron density array with element count boxnumber</param>
        /// <param name="inSigma">Roughness array with element count boxnumber</param>
        /// <param name="boxnumber">The number of boxes in the model</param>
        /// <param name="holdsigma">True if the film is to be treated as an elastic sheet, false otherwise</param>
        /// <param name="subphase">Substrate SLD</param>
        /// <param name="superphase">Superphase SLD</param>
        /// <param name="UseSLD">True if using SLD instead of ED</param>
        public Reflmodeling(double roughness, double[] inLength, double[] inRho, double[] inSigma, int boxnumber, bool holdsigma, string subphase, string superphase)
        {
            InitializeComponent();
            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession; 
            //Setup variables
            m_roughness = roughness;
            SubRough.Text = roughness.ToString();

            //Initialize the arrays
            MakeArrays();
            RhoArray = new double[6];
            LengthArray = new double[6];
            SigmaArray = new double[6];
            RhoArray = (double[])inRho.Clone();
            LengthArray = (double[])inLength.Clone();
            SigmaArray = (double[])inSigma.Clone();
            info = new double[9];
            //Copy over the values
            BoxCount.Text = boxnumber.ToString();
            SubRough.Text = roughness.ToString();
            SubphaseSLD.Text = subphase;
            SupSLDTB.Text = superphase;

            if (m_bUseSLD)
                tabControl1.TabPages[1].Text = "SLD";
            else
                tabControl1.TabPages[1].Text = "Electron Density";

            if (m_bUseSLD == false)
                RhoLabel.Text = "Normalized Rho";
            else
                RhoLabel.Text = "SLD";

            for (int i = 0; i < 6; i++)
            {
                BoxRhoArray[i].Text = RhoArray[i].ToString();
                BoxLengthArray[i].Text = LengthArray[i].ToString();
                BoxSigmaArray[i].Text = SigmaArray[i].ToString();
            }

            WavelengthTB.Text = ((double)(1.24)).ToString();

            //Setup arrays to hold the old values
            PreviousRhoArray = new double[6];
            PreviousLengthArray = new double[6];
            PreviousSigmaArray = new double[6];
         
            Holdsigma.Checked = holdsigma;
            //Setup the Graph
            ReflGraphing = new Graphing(string.Empty);
            ReflGraphing.SetGraphType(Properties.Settings.Default.ForceRQ4, DBFCB.Checked);
            ReflGraphing.SubSLD = Double.Parse(SubphaseSLD.Text);
            ReflGraphing.SupSLD = Double.Parse(SupSLDTB.Text);
            ReflGraphing.Wavelength = Double.Parse(WavelengthTB.Text);
            ReflGraphing.CreateGraph(RhoGraph, "Reflectivity", "Q/Qc", "Intensity / Fresnel", AxisType.Log);
            ReflGraphing.LoadDataFiletoGraph("Reflectivity Data", System.Drawing.Color.Black, SymbolType.Circle, 5);
            ReflGraphing.SetAllFonts("Garamond", 22, 18);

            RhoGraphing = new Graphing(string.Empty);
            RhoGraphing.SubSLD = double.Parse(subphase);
            RhoGraphing.IsNeutron = m_bUseSLD;
            RhoGraphing.SetGraphType(false, false);

            if(m_bUseSLD == false)
                RhoGraphing.CreateGraph(EDzedGraphControl1, "Electron Density Profile", "Z", "Normalized Electron Density",
                  AxisType.Linear);
            else
                 RhoGraphing.CreateGraph(EDzedGraphControl1, "SLD Profile", "Z", "SLD", AxisType.Linear);

            RhoGraphing.SetAllFonts("Garamond", 20, 18);
            //Get our Q data into a useable form
            Qincrement = ReflData.Instance.GetQData;
            RealRefl = ReflData.Instance.GetReflData;
            RealReflErrors = ReflData.Instance.GetRErrors;
            QErrors = ReflData.Instance.GetQErrors;

            ReflectivityMap = new double[ReflData.Instance.GetNumberDataPoints];

            //Create Z
            Z = new double[500];
            ElectronDensityArray = new double[500];
            BoxElectronDensityArray = new double[500];

            //Make the Z Arrays
            double length = 0;

            for (int k = 0; k < boxnumber; k++)
            {
                length += LengthArray[k];
            }
            for (int i = 0; i < 500; i++)
            {
                Z[i] = i * (50 + length) / 499.0;
            }

            RhoGraphing.Pane.XAxis.Scale.Min = 0;
            RhoGraphing.Pane.XAxis.Scale.Max = Z[Z.Length - 1];

            //Create the reflectivity density graph
            initialized = true;
            UpdateProfile();

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

            //Setup the callback if the graph updates the bounds
            ReflGraphing.ChangedBounds += new Graphing.ChangedEventHandler(PointChanged);
            BackupArrays();
        }

        void MakeArrays()
        {
            BoxSigmaArray = new TextBox[6];

            BoxSigmaArray[0] = Sigma1;
            BoxSigmaArray[1] = Sigma2;
            BoxSigmaArray[2] = Sigma3;
            BoxSigmaArray[3] = Sigma4;
            BoxSigmaArray[4] = Sigma5;
            BoxSigmaArray[5] = Sigma6;

            BoxLengthArray = new TextBox[6];

            BoxLengthArray[0] = LLength1;
            BoxLengthArray[1] = LLength2;
            BoxLengthArray[2] = LLength3;
            BoxLengthArray[3] = LLength4;
            BoxLengthArray[4] = LLength5;
            BoxLengthArray[5] = LLength6;

            BoxRhoArray = new TextBox[6];

            BoxRhoArray[0] = Rho1;
            BoxRhoArray[1] = Rho2;
            BoxRhoArray[2] = Rho3;
            BoxRhoArray[3] = Rho4;
            BoxRhoArray[4] = Rho5;
            BoxRhoArray[5] = Rho6;

        }
        void ChangeRoughnessArray(double rough)
        {
            for (int i = 0; i < BoxSigmaArray.Length; i++)
                BoxSigmaArray[i].Text = rough.ToString();
        }

       
        void UpdateProfile()
        {
            m_bvalidfit = false;
      

            if (initialized == false)
                return;

            //Blank our Rho data from the previous iteration

            if (m_bUseSLD == false)
            {
                RhoArray[0] = Double.Parse(Rho1.Text);
                RhoArray[1] = Double.Parse(Rho2.Text);
                RhoArray[2] = Double.Parse(Rho3.Text);
                RhoArray[3] = Double.Parse(Rho4.Text);
                RhoArray[4] = Double.Parse(Rho5.Text);
                RhoArray[5] = Double.Parse(Rho6.Text);
            }
            else
            {
                RhoArray[0] = Double.Parse(Rho1.Text)/double.Parse(SubphaseSLD.Text);
                RhoArray[1] = Double.Parse(Rho2.Text) / double.Parse(SubphaseSLD.Text);
                RhoArray[2] = Double.Parse(Rho3.Text) / double.Parse(SubphaseSLD.Text);
                RhoArray[3] = Double.Parse(Rho4.Text) / double.Parse(SubphaseSLD.Text);
                RhoArray[4] = Double.Parse(Rho5.Text) / double.Parse(SubphaseSLD.Text);
                RhoArray[5] = Double.Parse(Rho6.Text) / double.Parse(SubphaseSLD.Text);
            }

            //Fill Length array
            LengthArray[0] = Double.Parse(LLength1.Text);
            LengthArray[1] = Double.Parse(LLength2.Text);
            LengthArray[2] = Double.Parse(LLength3.Text);
            LengthArray[3] = Double.Parse(LLength4.Text);
            LengthArray[4] = Double.Parse(LLength5.Text);
            LengthArray[5] = Double.Parse(LLength6.Text);

            //Fill Sigma array
            SigmaArray[0] = Double.Parse(Sigma1.Text);
            SigmaArray[1] = Double.Parse(Sigma2.Text);
            SigmaArray[2] = Double.Parse(Sigma3.Text);
            SigmaArray[3] = Double.Parse(Sigma4.Text);
            SigmaArray[4] = Double.Parse(Sigma5.Text);
            SigmaArray[5] = Double.Parse(Sigma6.Text);

            oldnormfactor = Double.Parse(NormCorrectTB.Text);
            CheckZLength();

            double[] parameters = new double[3 * int.Parse(BoxCount.Text) + 2];
            double[] eparameters = new double[3*int.Parse(BoxCount.Text)+2];
            parameters[0] = double.Parse(SubRough.Text);
            eparameters[0] = double.Parse(SubRough.Text);
            eparameters[1] = 25;
            for (int i = 0; i < int.Parse(BoxCount.Text); i++)
            {
                parameters[3 * i + 1] = LengthArray[i];
                parameters[3 * i + 2] = RhoArray[i];
                parameters[3 * i + 3] = SigmaArray[i];

                eparameters[3 * i + 2] = LengthArray[i];
                eparameters[3 * i + 3] = RhoArray[i];
                eparameters[3 * i + 4] = SigmaArray[i];
            }

            parameters[3 * int.Parse(BoxCount.Text) + 1] = Double.Parse(NormCorrectTB.Text);

            FastReflGenerate(int.Parse(BoxCount.Text), double.Parse(SubphaseSLD.Text), double.Parse(SupSLDTB.Text), double.Parse(WavelengthTB.Text), parameters, parameters.Length,
                  Qincrement, QErrors, Qincrement.Length, ReflectivityMap, ReflectivityMap.Length, Double.Parse(QSpreadTB.Text),NormCorrectCB.Checked);
            RhoGenerate(int.Parse(BoxCount.Text), double.Parse(SubphaseSLD.Text), double.Parse(SupSLDTB.Text), eparameters,
                    eparameters.Length, Z, Z.Length, ElectronDensityArray, BoxElectronDensityArray, ElectronDensityArray.Length);
          

            if (m_bmodelreset == true)
            {
                ReflGraphing.Clear();
                ReflGraphing.LoadDataFiletoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5);
                m_bmodelreset = false;
            }

            //Setup the graphs
            ReflGraphing.LoadfromArray("modelrefl", Qincrement, ReflectivityMap, System.Drawing.Color.Black, SymbolType.XCross, 4, true, string.Empty);
            RhoGraphing.LoadfromArray("Model Dependent Fit", Z, ElectronDensityArray, System.Drawing.Color.Turquoise, SymbolType.None, 0, true, string.Empty);
            RhoGraphing.LoadfromArray("Model Dependent Box Fit", Z, BoxElectronDensityArray, System.Drawing.Color.Red, SymbolType.None, 0, false, string.Empty);
        }

       
        private void CheckZLength()
        {
            double length = 50;

            for (int k = 0; k < int.Parse(BoxCount.Text); k++)
                length += LengthArray[k];

            if (length != Z[Z.Length - 1])
                for (int i = 0; i < 500; i++)
                    Z[i] = i * length / 499.0;

            RhoGraphing.Pane.XAxis.Scale.Min = 0;
            RhoGraphing.Pane.XAxis.Scale.Max = Z[Z.Length - 1];
        }

        #region Fitting Routines

        private void LevenbergFit_Click(object sender, EventArgs e)
        {
            BackupArrays();

            double[] parameters;
            double chisquare = 0;
            int boxes = int.Parse(BoxCount.Text);

            //Setup our ranges
            double[] qrange = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];
            double[] reflectivity = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];
            double[] reflectivityerrors = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];
            double[] qerrors = null;

            if (ReflData.Instance.HaveErrorinQ)
                qerrors = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];

            for (int i = 0; i < qrange.Length; i++)
            {
                qrange[i] = (double)Qincrement[int.Parse(CritOffset.Text) + i];
                reflectivity[i] = (double)RealRefl[int.Parse(CritOffset.Text) + i];
                reflectivityerrors[i] = (double)RealReflErrors[int.Parse(CritOffset.Text) + i];

                if (ReflData.Instance.HaveErrorinQ)
                    qerrors[i] = (double)ReflData.Instance.GetQErrorPt(int.Parse(CritOffset.Text) + i);
            }

            if (Holdsigma.Checked == true)
            {
                //consts - # of boxes, subphase SLD,
                //variable - Subphase sigma (just one to start), Rho1, length1, Rho2, length2, Z-offset

                //Let's set up our parameter order for a system with only one roughness (elastic sheet)
                // 0 - Subphase roughness, 2 - 3 Layer 1 Values (Length, Rho), 4 - 5 Layer 2 Values, 6 - 7 Layer 3 Values
                // 8 - 9 Layer 4 Values, 10 - 11 Layer 5 Values, 11 - 13 Layer 6 Values. The last box will be our imperfect
                // normalization factor

                //We have to hold the # of boxes and the subphase SLD constant, or else 
                //the fit will wildly diverge

                parameters = new double[2 + boxes * 2];
                parameters[0] = Double.Parse(SubRough.Text);

                for (int i = 0; i < boxes; i++)
                {
                    parameters[2 * i + 1] = LengthArray[i];
                    parameters[2 * i + 2] = RhoArray[i];
                }

                parameters[1 + boxes * 2] = Double.Parse(NormCorrectTB.Text);
                covar = new double[parameters.Length];
            }
            else
            {
                //Let's set up our parameter order for a system with only one roughness (elastic sheet)
                // 0 - # of boxes 1 - Subphase SLD, 2 - Subphase roughness, 3 - Reflectivity scaling factor (not implemented), 
                // 4 - 6 Layer 1 Values (Length, Rho, Sigma), 7 - 9 Layer 2 Values, 10 - 12 Layer 3 Values
                // 13 - 15 Layer 4 Values, 16 - 18 Layer 5 Values, 19 - 21 Layer 6 Values. The last box will be our imperfect
                // normalization factor
                //
                //
                //We have to hold the # of boxes and the subphase SLD constant, or else 
                //the fit will wildly diverge

                parameters = new double[2 + boxes * 3];
                parameters[0] = Double.Parse(SubRough.Text);

                for (int i = 0; i < boxes; i++)
                {
                    parameters[3 * i + 1] = LengthArray[i];
                    parameters[3 * i + 2] = RhoArray[i];
                    parameters[3 * i + 3] = SigmaArray[i];
                }

                parameters[1 + boxes * 3] = Double.Parse(NormCorrectTB.Text);
                covar = new double[parameters.Length];
            }

            //Do the fit
            chisquare = FastReflfit(ReflData.Instance.GetWorkingDirectory, boxes, Double.Parse(SubphaseSLD.Text), Double.Parse(SupSLDTB.Text), Double.Parse(WavelengthTB.Text), 
                parameters, parameters.Length, qrange, qerrors, qrange.Length,  reflectivity, reflectivity.Length, reflectivityerrors, covar, covar.Length, info, info.Length, 
                Holdsigma.Checked, true, Double.Parse(QSpreadTB.Text),NormCorrectCB.Checked);

            //Update the ChiSquare
            chisquaretb.Text = chisquare.ToString("#.### E-000");

            //Make sure parameters are reasonable
            for (int i = 0; i < parameters.Length; i++)
            {
                if (parameters[i] * 0.3 < covar[i])
                {
                    MessageBox.Show("The error in a fitting parameter is greater than 30% of the parameter value. Check the 'Fit details' button for more information");
                    break;
                }
            }

            //Update paramters
            if (Holdsigma.Checked == true)
            {
                SubRough.Text = parameters[0].ToString();
                for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                {
                    BoxLengthArray[i].Text = parameters[2 * i + 1].ToString();

                    if(m_bUseSLD == false)
                        BoxRhoArray[i].Text = parameters[2 * i + 2].ToString();
                    else
                        BoxRhoArray[i].Text = (string)(parameters[2 * i + 2] * double.Parse(SubphaseSLD.Text)).ToString();

                    BoxSigmaArray[i].Text = parameters[0].ToString();
                }
                NormCorrectTB.Text = parameters[1 + 2 * int.Parse(BoxCount.Text)].ToString();
            }
            else
            {
                SubRough.Text = parameters[0].ToString();

                for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                {
                    BoxLengthArray[i].Text = parameters[3 * i + 1].ToString();

                    if(m_bUseSLD == false)
                        BoxRhoArray[i].Text = parameters[3 * i + 2].ToString();
                    else
                        BoxRhoArray[i].Text = (string)(parameters[3 * i + 2] * double.Parse(SubphaseSLD.Text)).ToString();

                    BoxSigmaArray[i].Text = parameters[3 * i + 3].ToString();
                }
                NormCorrectTB.Text = parameters[1 + 3 * int.Parse(BoxCount.Text)].ToString();
            }

            //Display the fit
            UpdateProfile();

            //Update report parameters
            SaveParamsforReport();
            m_bvalidfit = true;
            WriteFullfitFiles();

            //Add the graph to the master graph
            GraphCollection.Instance.ReflGraph = ReflGraphing;
            GraphCollection.Instance.ReflEGraph = RhoGraphing;

        }

        void Stoch()
        {
            double[] parameters;
            double[] parampercs = new double[7];
            bool bfitting = false;
            int boxes = int.Parse(BoxCount.Text);

            //Setup our ranges
            double[] qrange = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];
            double[] reflectivity = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];
            double[] reflectivityerrors = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];
            double[] qerrors = null;

            if (ReflData.Instance.HaveErrorinQ)
                qerrors = new double[Qincrement.Length - int.Parse(CritOffset.Text) - int.Parse(Rightoffset.Text)];

            for (int i = 0; i < qrange.Length; i++)
            {
                qrange[i] = (double)Qincrement[int.Parse(CritOffset.Text) + i];
                reflectivity[i] = (double)RealRefl[int.Parse(CritOffset.Text) + i];
                reflectivityerrors[i] = (double)RealReflErrors[int.Parse(CritOffset.Text) + i];

                if (ReflData.Instance.HaveErrorinQ)
                    qerrors[i] = (double)ReflData.Instance.GetQErrorPt(int.Parse(CritOffset.Text) + i);

            }

            if (Holdsigma.Checked == true)
            {
                //consts - # of boxes, subphase SLD,
                //variable - Subphase sigma (just one to start), Rho1, length1, Rho2, length2, Z-offset

                //Let's set up our parameter order for a system with only one roughness (elastic sheet)
                // 0 - Subphase roughness, 2 - 3 Layer 1 Values (Length, Rho), 4 - 5 Layer 2 Values, 6 - 7 Layer 3 Values
                // 8 - 9 Layer 4 Values, 10 - 11 Layer 5 Values, 11 - 13 Layer 6 Values
                //

                //We have to hold the # of boxes and the subphase SLD constant, or else 
                //the fit will wildly diverge

                parameters = new double[2 + boxes * 2];


                parameters[0] = Double.Parse(SubRough.Text);

                for (int i = 0; i < boxes; i++)
                {
                    parameters[2 * i + 1] = LengthArray[i];
                    parameters[2 * i + 2] = RhoArray[i];
                }
                parameters[1 + boxes * 2] = Double.Parse(NormCorrectTB.Text);

                covar = new double[parameters.Length];

            }
            else
            {
                //Let's set up our parameter order for a system with only one roughness (elastic sheet)
                // 0 - # of boxes 1 - Subphase SLD, 2 - Subphase roughness, 3 - Reflectivity scaling factor (not implemented), 
                // 4 - 6 Layer 1 Values (Length, Rho, Sigma), 7 - 9 Layer 2 Values, 10 - 12 Layer 3 Values
                // 13 - 15 Layer 4 Values, 16 - 18 Layer 5 Values, 19 - 21 Layer 6 Values
                //
                // Hold = false, Change = true
                //
                //We have to hold the # of boxes and the subphase SLD constant, or else 
                //the fit will wildly diverge

                parameters = new double[2 + boxes * 3];
                parameters[0] = Double.Parse(SubRough.Text);

                for (int i = 0; i < boxes; i++)
                {
                    parameters[3 * i + 1] = LengthArray[i];
                    parameters[3 * i + 2] = RhoArray[i];
                    parameters[3 * i + 3] = SigmaArray[i];
                }
                parameters[1 + boxes * 3] = Double.Parse(NormCorrectTB.Text);

                covar = new double[parameters.Length];
            }


            //Do the fit

            StochFitUI UI = new StochFitUI();

            if (UI.ShowDialog() == DialogResult.OK)
            {
                UI.GetParamPercs(ref parampercs);
                StochOutputWindow outwin;

                double[] ParamArray = new double[1000 * parameters.Length];
                double[] ChiSquareArray = new double[1000];
                double[] CovarArray = new double[1000 * parameters.Length];
                int size = 0;

                StochFit(boxes, Double.Parse(SubphaseSLD.Text), Double.Parse(SupSLDTB.Text), Double.Parse(WavelengthTB.Text), parameters, parameters.Length, qrange, qerrors, qrange.Length, reflectivity,
                      reflectivity.Length, reflectivityerrors, covar, covar.Length, info, info.Length, Holdsigma.Checked, false, UI.IterationCount, ParamArray, out size, parampercs, ChiSquareArray, CovarArray,
                      Double.Parse(QSpreadTB.Text), NormCorrectCB.Checked);
                outwin = new StochOutputWindow(ParamArray, size, parameters.Length, ChiSquareArray, CovarArray, Holdsigma.Checked, boxes, Double.Parse(SubphaseSLD.Text), Double.Parse(SupSLDTB.Text),
                       Double.Parse(WavelengthTB.Text), Double.Parse(QSpreadTB.Text), NormCorrectCB.Checked);

                if (outwin.ShowDialog() != DialogResult.Cancel)
                {
                    bfitting = true;
                    chisquaretb.Text = outwin.GetParameters(out parameters, out covar);
                }
            }

            //Make sure parameters are reasonable
            for (int i = 0; i < parameters.Length; i++)
            {
                if (parameters[i] * 0.3 < covar[i])
                {
                    MessageBox.Show("The error in a fitting parameter is greater than 30% of the parameter value. Check the 'Fit details' button for more information");
                    break;
                }
            }

            //Update paramters
            if (bfitting == true)
            {
                 //Update paramters
                //Update paramters
                if (Holdsigma.Checked == true)
                {
                    SubRough.Text = parameters[0].ToString();
                    for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                    {
                        BoxLengthArray[i].Text = parameters[2 * i + 1].ToString();

                        if (m_bUseSLD == false)
                            BoxRhoArray[i].Text = parameters[2 * i + 2].ToString();
                        else
                            BoxRhoArray[i].Text = (string)(parameters[2 * i + 2] * double.Parse(SubphaseSLD.Text)).ToString();

                        BoxSigmaArray[i].Text = parameters[0].ToString();
                    }
                    NormCorrectTB.Text = parameters[1 + 2 * int.Parse(BoxCount.Text)].ToString();
                }
                else
                {
                    SubRough.Text = parameters[0].ToString();

                    for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                    {
                        BoxLengthArray[i].Text = parameters[3 * i + 1].ToString();

                        if (m_bUseSLD == false)
                            BoxRhoArray[i].Text = parameters[3 * i + 2].ToString();
                        else
                            BoxRhoArray[i].Text = (string)(parameters[3 * i + 2] * double.Parse(SubphaseSLD.Text)).ToString();

                        BoxSigmaArray[i].Text = parameters[3 * i + 3].ToString();
                    }
                    NormCorrectTB.Text = parameters[1 + 3 * int.Parse(BoxCount.Text)].ToString();
                }

                this.Invoke(new UpdateProfilefromThread(this.UpdateProfile));
                LevenbergFit_Click(null, null);
            }
            //Restore window
            this.Invoke(new UpdateGUI(this.DisablePanel), new object[]{false});
        }

        private void button2_Click(object sender, EventArgs e)
        {
            BackupArrays();
            Stochthread = new Thread(new ThreadStart(Stoch));
            DisablePanel(true);
            Stochthread.Start();
        }

        #endregion

        private void WriteFullfitFiles()
        {
            using (StreamWriter sw = new StreamWriter("fullreffit.dat"))
            {
                string outputstring;
                double Qc = Graphing.CalcQc(double.Parse(SubphaseSLD.Text), double.Parse(SupSLDTB.Text), Double.Parse(WavelengthTB.Text));

                for (int i = 0; i < RealRefl.Length; i++)
                {
                    double QQc = Qincrement[i] / Qc;
                    double DBF = ReflectivityMap[i] / Graphing.CalcFresnelPoint((double)Qincrement[i], Qc);
                    outputstring = (Qincrement[i]).ToString() + " " + (RealRefl[i]).ToString() + " " +
                        (RealReflErrors[i]).ToString() + " " + ReflectivityMap[i].ToString() + " " + QQc.ToString() + " " + DBF.ToString() + "\n";
                    sw.Write(outputstring);
                }
            }
        }

        private void BackupArrays()
        {
            for (int i = 0; i < 6; i++)
            {
                if(m_bUseSLD == true)
                    PreviousRhoArray[i] = RhoArray[i]*double.Parse(SubphaseSLD.Text);
                else
                    PreviousRhoArray[i] = RhoArray[i];

                PreviousSigmaArray[i] = SigmaArray[i];
                PreviousLengthArray[i] = LengthArray[i];
            }
            previoussigma = double.Parse(SubRough.Text);
            oldnormfactor = Double.Parse(NormCorrectTB.Text);
        }

        private void UndoFit_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < 6; i++)
            {
                BoxRhoArray[i].Text = PreviousRhoArray[i].ToString();
                BoxLengthArray[i].Text = PreviousLengthArray[i].ToString();
                BoxSigmaArray[i].Text = PreviousSigmaArray[i].ToString();
            }
            SubRough.Text = previoussigma.ToString();
            NormCorrectTB.Text = oldnormfactor.ToString();
           
            UpdateProfile();
        }

        #region Reporting Functions
        /// <summary>
        /// Write a pdf report for documentation
        /// </summary>
        private void SaveParamsforReport()
        {
            ReportGenerator g = ReportGenerator.Instance;
            g.ClearReflModelInfo();

            ArrayList ginfo = new ArrayList();

            if (Holdsigma.Checked)
                ginfo.Add("The reflectivity curve was fit with a single roughness parameter\n");
            else
                ginfo.Add(String.Format("The reflectivity curve was fit with {0} roughness parameters\n", (int.Parse(BoxCount.Text) + 1)));

            ginfo.Add(String.Format("Percent Error in Q: " + QSpreadTB.Text + "\n"));
            ginfo.Add(String.Format("Normalization Constant: " + NormCorrectTB.Text + "\n"));
            ginfo.Add(String.Format("Critical Edge Offset: " + CritOffset.Text + "\n"));
            ginfo.Add(String.Format("High Q Offset: " + Rightoffset.Text + "\n"));
            ginfo.Add(String.Format("Superphase SLD: " + SupSLDTB.Text + "\n"));
            ginfo.Add(String.Format("Subphase SLD: " + SubphaseSLD.Text + "\n"));
            ginfo.Add(String.Format("Wavelength: " + WavelengthTB.Text + "\n"));
            ginfo.Add(String.Format("Chi Square for reflectivity fit: " + chisquaretb.Text + "\n"));
            ginfo.Add(String.Format("The subphase roughness was: {0:#.### E-0} " +(char)0x00B1 + " {1:#.### E-0}\n", double.Parse(SubRough.Text), covar[0]));
      
            if (Holdsigma.Checked == true)
            {
                for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                {
                    ginfo.Add((i + 1).ToString());
                    ginfo.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[2 * i + 1].ToString("#.### E-0"));

                    if(m_bUseSLD == false)
                        ginfo.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[2 * i + 2].ToString("#.### E-0"));
                    else
                        ginfo.Add((RhoArray[i]*GetSubSLD).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (covar[2 * i + 2]*GetSubSLD).ToString("#.### E-0"));

                    ginfo.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[0].ToString("#.### E-0"));
                }
            }
            else
            {
                for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                {
                    ginfo.Add((i + 1).ToString());
                    ginfo.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[3 * i + 1].ToString("#.### E-0"));

                    if(m_bUseSLD == false)
                        ginfo.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[3 * i + 2].ToString("#.### E-0"));
                    else
                        ginfo.Add((RhoArray[i]*GetSubSLD).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (covar[3 * i + 2]*GetSubSLD).ToString("#.### E-0"));

                    ginfo.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[3 * i + 3].ToString("#.### E-0"));
                }
            }
            g.SetReflModelInfo = ginfo;
        }

        //Format error reporting
        private string ErrorReport()
        {
            StringBuilder output = new StringBuilder();

            if (m_bvalidfit == true && covar != null)
            {
                output.Append("\u03C3 = " + string.Format("{0:#.### E-0} ", double.Parse(SubRough.Text)) + " " +
                    (char)0x00B1 + " " + covar[0].ToString("#.### E-0") + Environment.NewLine + Environment.NewLine);

                for (int i = 0; i < int.Parse(BoxCount.Text); i++)
                {
                    if (Holdsigma.Checked == true)
                    {
                        output.Append("Layer " + (i + 1).ToString() + Environment.NewLine);

                        if(m_bUseSLD == false)
                            output.Append("\t" + " \u03C1 = " + RhoArray[i].ToString("#.### E-0") + " " +
                                (char)0x00B1 + " " + covar[2 * i + 2].ToString("#.### E-0") + Environment.NewLine);
                        else
                            output.Append("\t" + " SLD = " + (RhoArray[i] * GetSubSLD ).ToString("#.### E-0") + " " +
                                (char)0x00B1 + " " + (covar[2 * i + 2] * GetSubSLD).ToString("#.### E-0") + Environment.NewLine);

                        output.Append("\t" + " Length = " + LengthArray[i].ToString("#.### E-0") + " " +
                        (char)0x00B1 + " " + covar[2 * i + 1].ToString("#.### E-0") + Environment.NewLine);
                    }
                    else
                    {
                        output.Append("Layer " + (i + 1).ToString() + Environment.NewLine);

                        if(m_bUseSLD == false)
                            output.Append("\t" + " \u03C1 = " + RhoArray[i].ToString("#.### E-0") + " " +
                                (char)0x00B1 + " " + covar[3 * i + 2].ToString("#.### E-0") + Environment.NewLine);
                        else
                            output.Append("\t" + " SLD = " + (RhoArray[i] * GetSubSLD).ToString("#.### E-0") + " " +
                               (char)0x00B1 + " " + (covar[3 * i + 2] * GetSubSLD).ToString("#.### E-0") + Environment.NewLine);

                        output.Append("\t" + " Length = " + LengthArray[i].ToString("#.### E-0") + " " +
                            (char)0x00B1 + " " + covar[3 * i + 1].ToString("#.### E-0") + Environment.NewLine);
                        output.Append("\t" + " \u03C3 = " + SigmaArray[i].ToString("#.### E-0") + " " +
                            (char)0x00B1 + " " + covar[3 * i + 3].ToString("#.### E-0") + Environment.NewLine);
                    }
                }

                if (NormCorrectCB.Checked)
                    output.Append(Environment.NewLine + "Normalization factor = " + Double.Parse(NormCorrectTB.Text).ToString("#.###") + " " +
                       (char)0x00B1 + " " + covar[covar.Length - 1].ToString("#.### E-0") + Environment.NewLine);

                output.Append(Environment.NewLine + "Levenberg-Marquadt output" + Environment.NewLine + "\tNumber of iterations : " + info[5].ToString() + Environment.NewLine);
                output.Append("Reason for termination: " + termreason((int)info[6]));

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

        #endregion


        private void OnFormClosing(object sender, FormClosingEventArgs e)
        {
            GraphCollection.Instance.ReflGraph = ReflGraphing;
            GraphCollection.Instance.ReflEGraph = RhoGraphing;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            LevmarOutput lo = new LevmarOutput();
            lo.DisplayOutput(ErrorReport());
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
                ReflGraphing.SupSLD = double.Parse(SupSLDTB.Text);
                ReflGraphing.SubSLD = double.Parse(SubphaseSLD.Text);
                ReflGraphing.Wavelength = double.Parse(WavelengthTB.Text);
                ReflGraphing.SetGraphType(Properties.Settings.Default.ForceRQ4, DBFCB.Checked);
                UpdateProfile();
            }
            catch { }
        }

        private void MajorVariable_Changed(object sender, EventArgs e)
        {
            try
            {
                m_bmodelreset = true;
                Variable_Changed(sender, e);
            }
            catch { }
        }

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to a double or false if not</param>
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

            if(((TextBox)sender).Name == "BoxCount")
            {
                if (int.Parse(BoxCount.Text) > 6)
                {
                    MessageBox.Show("Six is the maximum number of boxes");
                    e.Cancel = true;
                }
            }
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

        private double GetSubSLD
        {
            get
            {
                return double.Parse(SubphaseSLD.Text);
            }
        }

    }
}