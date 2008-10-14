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

#pragma warning disable 1591

namespace StochasticModeling
{
    public class BoxReflFitBase 
    {
        
        /// <summary>
        /// The culture is set to US for the purposes of inputting data to the numerical routines
        /// </summary>
        protected CultureInfo m_CI = new CultureInfo("en-US");


        #region Variables
        protected double m_roughness = 3;
        protected bool m_bvalidfit = false;
        protected bool m_bUseSLD = false;
        
        //Arrays
        protected List<double> RhoArray;
        protected List<double> LengthArray;
        protected List<double> SigmaArray;
        protected List<double> PreviousRhoArray;
        protected List<double> PreviousLengthArray;
        protected List<double> PreviousSigmaArray;
        protected double[] covar;
        protected double[] info;
        protected double oldnormfactor;
        protected double previoussigma;
        protected bool initialized = false;
        protected double SubphaseRoughness;
        protected double LeftOffset;
        protected double NormalizationFactor;
        protected double SubphaseSLDTB;
        protected double SuperSLDTB;
        protected double WavelengthTB;
        protected double QSpreadTB;
        protected double SubRoughTB;
        protected double ZOffsetTB;
        protected bool ImpNormCB;
        protected double[] Qincrement;
        protected double[] QErrors;
        protected double[] ReflectivityMap;
        protected double[] Z;
        protected double[] ElectronDensityArray;
        protected double[] BoxElectronDensityArray;
        protected double[] RealReflErrors;
        protected double[] RealRefl;
        protected double[] RealRho;
        protected bool m_bmodelreset = false;
        protected BoxModelSettings InfoStruct;
        protected Thread Stochthread;
        protected bool HoldsigmaCB;
        protected int BoxCountTB;
        protected double m_dZ_offset = 15;
        protected double m_dRoughness = 3;
        protected double[] m_dCovarArray;
        protected double m_dPrevioussigma;
        protected double m_dPreviouszoffset;

        public delegate void UpdateProfileHandler(object sender, EventArgs e);
        public event UpdateProfileHandler Update;

    #endregion

        public BoxReflFitBase()
        {

        }

        public BoxReflFitBase(double[] Z, double[] ERho)
        {
            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession;
            

           
            RhoArray = new List<double>(6);
            LengthArray = new List<double>(6);
            SigmaArray = new List<double>(6);
            info = new double[9];

            //Setup arrays to hold the old values
            PreviousRhoArray = new List<double>(6);
            PreviousLengthArray = new List<double>(6);
            PreviousSigmaArray = new List<double>(6);

            if (Z != null)
            {
                RealRho = (double[])ERho.Clone();
                ElectronDensityArray = new double[ERho.Length];
                BoxElectronDensityArray = new double[ERho.Length];
                this.Z = (double[])Z.Clone();
            }
        }


        public BoxReflFitBase(double roughness, double[] inLength, double[] inRho, double[] inSigma, int boxnumber, bool holdsigma, string subphase, string superphase)
        {
           
            m_roughness = roughness;
            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession;
            
          
            RhoArray = new List<double>(inRho);
            LengthArray = new List<double>(inLength);
            SigmaArray = new List<double>(inSigma);
            info = new double[9];

            //Setup arrays to hold the old values
            PreviousRhoArray = new List<double>(inSigma.Length);
            PreviousLengthArray = new List<double>(inSigma.Length);
            PreviousSigmaArray = new List<double>(inSigma.Length);

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



        public virtual void UpdateProfile()
        {
           m_bvalidfit = false;

           double[] parameters = null;
           double[] eparameters = null;
           
           MakeParameters(ref eparameters, true, HoldsigmaCB, BoxCountTB,NormalizationFactor, SubphaseRoughness);
           Calculations.RhoGenerate(InfoStruct, eparameters, eparameters.Length, ElectronDensityArray, BoxElectronDensityArray);

           //if(ReflGraphing != null)
           //{
           //     MakeParameters(ref parameters, false, HoldsigmaCB.Checked, BoxCountTB.ToInt(), NormalizationFactor.ToDouble(), SubphaseRoughness.ToDouble());
           //     Calculations.FastReflGenerate(InfoStruct, parameters, parameters.Length, ReflectivityMap);

           //     if (m_bmodelreset == true)
           //     {
           //         ReflGraphing.Clear();
           //         ReflGraphing.LoadDatawithErrorstoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
           //         m_bmodelreset = false;
           //     }

           //     //Setup the graphs
           //     ReflGraphing.LoadfromArray("modelrefl", Qincrement, ReflectivityMap, System.Drawing.Color.Black, SymbolType.XCross, 4, true, string.Empty);
           //}
           // RhoGraphing.LoadfromArray("Model Dependent Fit", Z, ElectronDensityArray, System.Drawing.Color.Turquoise, SymbolType.None, 0, true, string.Empty);
           // RhoGraphing.LoadfromArray("Model Dependent Box Fit", Z, BoxElectronDensityArray, System.Drawing.Color.Red, SymbolType.None, 0, false, string.Empty);
        }

        public void MakeParameters(ref double[] parameters, bool IsED, bool onesigma, int boxcount, double normcorrection, double subrough)
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

                if (ZOffsetTB == null)
                    parameters[1] = 25;
                else
                    parameters[1] = ZOffsetTB;
                EDconst++;
            }
            else
            {
                parameters = new double[arrayconst + boxcount * 2];
            }

            parameters[0] = subrough;

            for (int i = 0; i < boxcount; i++)
            {
                parameters[arrayconst * i + 1 + EDconst] = LengthArray[i];

                if (!m_bUseSLD)
                    parameters[arrayconst * i + 2 + EDconst] = RhoArray[i];
                else
                    parameters[arrayconst * i + 2 + EDconst] = RhoArray[i] / SubphaseSLDTB;

                if (!onesigma)
                    parameters[3 * i + 3 + EDconst] = SigmaArray[i];
            }

            if (!IsED)
                parameters[1 + boxcount * arrayconst] = normcorrection;
        }

        protected virtual void SetInitStruct(ref BoxModelSettings InitStruct, double[] parampercs, double[] UL, double[] LL)
        {
            if (InitStruct.Q == IntPtr.Zero)
            {
                InitStruct.SetArrays(ReflData.Instance.GetQData, ReflData.Instance.GetReflData, ReflData.Instance.GetRErrors,
                    ReflData.Instance.GetQErrors, parampercs, UL, LL);
            }
            InitStruct.Directory = ReflData.Instance.GetWorkingDirectory;
            InitStruct.Boxes = BoxCountTB;

            InitStruct.SubSLD = SubphaseSLDTB;
            InitStruct.SupSLD = SuperSLDTB;

            if (WavelengthTB != null)
                InitStruct.Wavelength = WavelengthTB;

            InitStruct.OneSigma = HoldsigmaCB;

            if (QSpreadTB != null)
                InitStruct.QSpread = QSpreadTB;

            if (ImpNormCB != null)
                InitStruct.ImpNorm = ImpNormCB;

            InitStruct.WriteFiles = true;

        }

      

        protected void BackupArrays()
        {
            PreviousRhoArray.Clear();
            PreviousSigmaArray.Clear();
            PreviousLengthArray.Clear();

            RhoArray.ForEach(p => PreviousRhoArray.Add(p));
            SigmaArray.ForEach(p => PreviousSigmaArray.Add(p));
            LengthArray.ForEach(p => PreviousLengthArray.Add(p));
            
            //m_dPrevioussigma = SubphaseRoughness.ToDouble();
            //m_dPreviouszoffset = ZOffsetTB.ToDouble();
        }

        public double GetSubRoughness
        {
            get
            {
                return SubphaseRoughness;
            }
        }

        public bool IsOneSigma
        {
            get
            {
                return HoldsigmaCB;
            }
        }
    }
}

