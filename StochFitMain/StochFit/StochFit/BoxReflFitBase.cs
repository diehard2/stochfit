using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using StochasticModeling.Settings;
using System.Globalization;
using System.Threading;
using System.Drawing;
using ZedGraph;

namespace StochasticModeling
{
    public class BoxReflFitBase : Form
    {
                /// <summary>
        /// Generates a reflectivity using the Nevot-Croce correction to the Parratt recursion
        /// </summary>
        /// <param name="boxes">Number of boxes in the model</param>
        /// <param name="SLD">Subphase SLD</param>
        /// <param name="SupSLD">Superphase SLD</param>
        /// <param name="wavelength">X-ray wavelength</param>
        /// <param name="parameters">Parameter array. See code for setup</param>
        /// <param name="paramsize">Element count of parameters</param>
        /// <param name="QRange">Array of Q values to calculate reflectivity points for</param>
        /// <param name="QError">Array of error in Q. Can be NULL</param>
        /// <param name="QSize">QRange element count</param>
        /// <param name="Reflectivity">Allocated array of QSize elements that recieves the </param>
        /// <param name="reflectivitysize">Element count of the reflectivity array</param>
        /// <param name="QSpread">Percent error in Q</param>
        /// <param name="ImpNorm">True if the curve is believed to be imperfectly normalized, false otherwise</param>
        /// <returns></returns>
        [DllImport("LevMardll.dll", EntryPoint = "FastReflGenerate", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void FastReflGenerate(BoxModelSettings InitStruct, double[] parameters, int parametersize, double[] ReflectivityMap);

        /// <summary>
        /// Performs a Levenberg-Marquadt least squares fit of the data
        /// </summary>
        /// <param name="directory">The working directory</param>
        /// <param name="boxes">Number of boxes in the model</param>
        /// <param name="SLD">Subphase SLD</param>
        /// <param name="SupSLD">Superphase SLD</param>
        /// <param name="wavelength">Wavelength in Angstroms</param>
        /// <param name="parameters">The parameter array</param>
        /// <param name="paramsize">The size of the parameter array</param>
        /// <param name="QRange">Q array</param>
        /// <param name="QErrors">Error in Q array if it exists. NULL otherwise</param>
        /// <param name="QSize">Size of QRange</param>
        /// <param name="Reflectivity">Array that will recieve the reflectivity</param>
        /// <param name="reflectivitysize">Size of Reflectivity</param>
        /// <param name="Errors">Array containing the error for Reflectivity in standard deviation</param>
        /// <param name="covar">Array which recieves the covariance matrix</param>
        /// <param name="covarsize">Size of covar</param>
        /// <param name="info">Array which recieves information regarding the terminating conditions of the fit</param>
        /// <param name="infosize">Size of info</param>
        /// <param name="onesigma">True if the system can be treated as an elastic sheet</param>
        /// <param name="writefiles">Write output files to the specified directory</param>
        /// <param name="Qspread">Error in Q as a fixed percentage</param>
        /// <param name="ImpNorm">Choose whether to correct the reflectivity for imperfect normalization</param>
        /// <returns>The Chi square value for the fit</returns>
        [DllImport("LevMardll.dll", EntryPoint = "FastReflfit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double FastReflfit([MarshalAs(UnmanagedType.LPStruct)] BoxModelSettings InitStruct, double[] parameters, double[] covar, int covarsize, double[] info, int infosize);

        /// <summary>
        /// Generates a smoothed and box electron density profile
        /// </summary>
        /// <param name="boxes"></param>
        /// <param name="SLD"></param>
        /// <param name="SupSLD"></param>
        /// <param name="parameters"></param>
        /// <param name="paramsize"></param>
        /// <param name="ZRange"></param>
        /// <param name="ZSize"></param>
        /// <param name="ED"></param>
        /// <param name="BoxED"></param>
        /// <param name="EDSize"></param>
        [DllImport("LevMardll.dll", EntryPoint = "RhoGenerate", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void RhoGenerate(int boxes, double SLD, double SupSLD, double[] parameters, int paramsize,
           double[] ZRange, int ZSize, double[] ED, double[] BoxED, int EDSize, bool onesigma);

        /// <summary>
        /// Fits an electron density profile to the electron density profile generated by the model independent fit
        /// </summary>
        /// <param name="directory"></param>
        /// <param name="boxes"></param>
        /// <param name="SLD"></param>
        /// <param name="SupSLD"></param>
        /// <param name="parameters"></param>
        /// <param name="paramsize"></param>
        /// <param name="ZRange"></param>
        /// <param name="ZSize"></param>
        /// <param name="ED"></param>
        /// <param name="EDsize"></param>
        /// <param name="covariance"></param>
        /// <param name="covarsize"></param>
        /// <param name="info"></param>
        /// <param name="infosize"></param>
        /// <param name="onesigma"></param>
        /// <returns></returns>
        [DllImport("LevMardll.dll", EntryPoint = "Rhofit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double Rhofit(string directory, int boxes, double SLD, double SupSLD, double[] parameters, int paramsize,
            double[] ZRange, int ZSize, double[] ED, int EDsize, double[] covariance,
            int covarsize, double[] info, int infosize, bool onesigma);


        /// <summary>
        /// Performs a constrained Levenberg-Marquadt least squares fit of the data
        /// </summary>
        /// <param name="boxes">Number of boxes in the model</param>
        /// <param name="SLD">Subphase SLD</param>
        /// <param name="SupSLD">Superphase SLD</param>
        /// <param name="wavelength">X-ray wavelength</param>
        /// <param name="parameters">Parameter array. See code for setup</param>
        /// <param name="paramsize">Element count of parameters</param>
        /// <param name="QRange">Array of Q values to calculate reflectivity points for</param>
        /// <param name="QError">Array of error in Q. Can be NULL</param>
        /// <param name="QSize">QRange element count</param>
        /// <param name="Reflectivity">Allocated array of QSize elements that recieves the </param>
        /// <param name="reflectivitysize">Element count of the reflectivity array</param>
        /// <param name="Errors">Array of errors in the reflectivity data</param>
        /// <param name="covar">The original covariance array to fill</param>
        /// <param name="covarsize">Element count of covar</param>
        /// <param name="info">Array of size 6, returns information pertaining to the fit</param>
        /// <param name="infosize">Element count of info</param>
        /// <param name="onesigma">If the film is to be treated as an elastic sheet, set to true</param>
        /// <param name="writefiles">Set to true to write files</param>
        /// <param name="iterations">Total number of iterations for the stochastic fitting</param>
        /// <param name="ParamArray">Returns the array of all fits. Allocated to a size of 1000 * paramsize</param>
        /// <param name="paramarraysize">Element count of ParamArray</param>
        /// <param name="parampercs">The percentage by which each parameter can vary, with an element count of 6. 1 and 2 correspond to the thickness
        /// of the file, 2 and 3 correspond to the electron density, and 4 and 5 correspond to the roughness</param>
        /// <param name="chisquarearray">Array of Chi square values for each model found</param>
        /// <param name="Covararray">Covariance array for each model found</param>
        /// <param name="UL">Array of upper limits of paramsize elements for each parameter in ParamArray</param>
        /// <param name="LL">Array of lower limits of paramsize elements for each parameter in ParamArray</param>
        /// <param name="QSpread">Percent error in Q</param>
        /// <param name="ImpNorm">True if the curve is believed to be imperfectly normalized, false otherwise</param>   
        [DllImport("LevMardll.dll", EntryPoint = "ConstrainedStochFit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void ConstrainedStochFit(int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
            double[] QRange, double[] QError, int QSize, double[] Reflectivity, int reflectivitysize, double[] Errors, double[] covar, int covarsize, double[] info, int infosize, bool onesigma, bool writefiles, int iterations,
            double[] ParamArray, out int paramarraysize, double[] parampercs, double[] chisquarearray, double[] Covararray, double[] UL, double[] LL, double QSpread, bool ImpNorm);

        /// <summary>
        /// The culture is set to US for the purposes of inputting data to the numerical routines
        /// </summary>
        protected CultureInfo m_CI = new CultureInfo("en-US");


        #region Variables
        protected double m_roughness = 3;
        protected bool m_bvalidfit = false;
        protected bool m_bUseSLD = false;
        protected Graphing ReflGraphing;
        protected Graphing RhoGraphing;
        //Arrays
        protected double[] RhoArray;
        protected double[] LengthArray;
        protected double[] SigmaArray;
        protected double[] PreviousRhoArray;
        protected double[] PreviousLengthArray;
        protected double[] PreviousSigmaArray;
        protected double[] covar;
        protected double[] info;
        protected double oldnormfactor;
        protected double previoussigma;
        protected bool initialized = false;
        protected TextBox[] BoxSigmaArray;
        protected TextBox[] BoxRhoArray;
        protected TextBox[] BoxLengthArray;
        protected TextBox[] SubphaseRoughness;
        protected TextBox[] NormalizationFactor;
        protected double[] Qincrement;
        protected double[] QErrors;
        protected double[] ReflectivityMap;
        protected double[] Z;
        protected double[] ElectronDensityArray;
        protected double[] BoxElectronDensityArray;
        protected double[] RealReflErrors;
        protected double[] RealRefl;
        protected bool m_bmodelreset = false;
        protected BoxModelSettings ReflStruct;
        protected Thread Stochthread;

    #endregion


        public BoxReflFitBase(double roughness, double[] inLength, double[] inRho, double[] inSigma, int boxnumber, bool holdsigma, string subphase, string superphase)
        {
            ReflStruct = new BoxModelSettings();
            m_roughness = roughness;
            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession;
            
            MakeArrays();
            RhoArray = new double[6];
            LengthArray = new double[6];
            SigmaArray = new double[6];
            RhoArray = (double[])inRho.Clone();
            LengthArray = (double[])inLength.Clone();
            SigmaArray = (double[])inSigma.Clone();
            info = new double[9];

            //Setup arrays to hold the old values
            PreviousRhoArray = new double[6];
            PreviousLengthArray = new double[6];
            PreviousSigmaArray = new double[6];

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

        protected virtual void MakeArrays()
        {

        }


        protected virtual void ChangeRoughnessArray(double rough, ref TextBox t)
        {
            t.Text = rough.ToString();

            foreach (TextBox b in BoxSigmaArray)
                b.Text = rough.ToString();
        }

        protected virtual void UpdateProfile()
        {
            m_bvalidfit = false;



            double[] parameters = null;


           // MakeParameters(ref parameters, false);
            //Generare Reflectivity and ED          
            FastReflGenerate(ReflStruct, parameters, parameters.Length, ReflectivityMap);

            if (m_bmodelreset == true)
            {
                ReflGraphing.Clear();
                ReflGraphing.LoadDatawithErrorstoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
                m_bmodelreset = false;
            }

            //Setup the graphs
            ReflGraphing.LoadfromArray("modelrefl", Qincrement, ReflectivityMap, System.Drawing.Color.Black, SymbolType.XCross, 4, true, string.Empty);
            RhoGraphing.LoadfromArray("Model Dependent Fit", Z, ElectronDensityArray, System.Drawing.Color.Turquoise, SymbolType.None, 0, true, string.Empty);
            RhoGraphing.LoadfromArray("Model Dependent Box Fit", Z, BoxElectronDensityArray, System.Drawing.Color.Red, SymbolType.None, 0, false, string.Empty);
        }

        protected void MakeParameters(ref double[] parameters, bool IsED, bool onesigma, int boxcount,
          double normcorrection, double subrough)
        {
            int arrayconst = 0;
            int EDconst = 0;

            if (onesigma)
                arrayconst = 2;
            else
                arrayconst = 3;


            //consts - # of boxes, subphase SLD,
            //variable - Subphase sigma (just one to start), Rho1, length1, Rho2, length2, Z-offset

            //Let's set up our parameter order for a system with only one roughness (elastic sheet)
            // 0 - Subphase roughness, 2 - 3 Layer 1 Values (Length, Rho), 4 - 5 Layer 2 Values, 6 - 7 Layer 3 Values
            // 8 - 9 Layer 4 Values, 10 - 11 Layer 5 Values, 11 - 13 Layer 6 Values. The last box will be our imperfect
            // normalization factor

            //We have to hold the # of boxes and the subphase SLD constant, or else 
            //the fit will wildly diverge
            if (IsED)
            {
                parameters = new double[arrayconst + 1 + boxcount * 2];
                parameters[1] = 25;
                EDconst++;
            }
            else
            {
                parameters = new double[arrayconst + boxcount * 2];
            }

            parameters[0] = subrough ;


            for (int i = 0; i < boxcount; i++)
            {
                parameters[arrayconst * i + 1 + EDconst] = LengthArray[i];
                parameters[arrayconst * i + 2 + EDconst] = RhoArray[i];

                if (!onesigma)
                    parameters[3 * i + 3 + EDconst] = SigmaArray[i];
            }

            if (!IsED)
                parameters[1 + boxcount * arrayconst] = normcorrection;


            SetInitStruct(ref ReflStruct, null, null, null);
        }

        protected virtual void SetInitStruct(ref BoxModelSettings InitStruct, double[] parampercs, double[] UL, double[] LL)
        {

        }
    }
}

