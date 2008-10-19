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
    public abstract class BoxReflFitBase 
    {
        
        /// <summary>
        /// The culture is set to US for the purposes of inputting data to the numerical routines
        /// </summary>
        protected CultureInfo m_CI = new CultureInfo("en-US");


        #region Variables
        protected bool m_bvalidfit = false;
        protected bool m_bUseSLD = false;
        
        //Arrays
        protected List<double> _RhoArray;
        protected List<double> _LengthArray;
        protected List<double> _SigmaArray;
        protected List<double> PreviousRhoArray;
        protected List<double> PreviousLengthArray;
        protected List<double> PreviousSigmaArray;
        protected double m_dPrevioussigma;
        protected double[] covar;
        protected double[] info;
        protected double oldnormfactor;
        protected bool initialized = false;
        protected double LeftOffset;
        protected double NormalizationFactor;
        protected double SubphaseSLDTB;
        protected double SuperSLDTB;
        protected double WavelengthTB;
        protected double QSpreadTB;
        protected double SubRoughTB;
        protected double ZOffsetTB;
        private double _HighQOffset;
        private double _LowQOffset;

        
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
        protected double[] UL;
        protected double[] LL;

        protected bool m_bmodelreset = false;
        protected BoxModelSettings InfoStruct;
        protected Thread Stochthread;
        protected bool HoldsigmaCB;
        protected int BoxCountTB;
        protected double m_dZ_offset = 15;
        protected double m_dRoughness = 3;
        protected double[] m_dCovarArray;
        //protected double m_dPrevioussigma;
        protected double m_dPreviouszoffset;
        protected double m_dChiSquare;
        private double m_dPreviousImpNorm;
        private double m_dImpNorm;

        public delegate void UpdateProfileHandler(object sender, EventArgs e);
        public event UpdateProfileHandler Update;

    #endregion

    
        public BoxReflFitBase(double[] Z, double[] ERho)
        {
            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession;
            

           
            _RhoArray = new List<double>(6);
            _LengthArray = new List<double>(6);
            _SigmaArray = new List<double>(6);
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


        public BoxReflFitBase(BoxReflFitBase previousFitBase)
        {
            _RhoArray = new List<double>(previousFitBase._RhoArray.ToArray());
            _LengthArray = new List<double>(previousFitBase._LengthArray.ToArray());
            _SigmaArray = new List<double>(previousFitBase._SigmaArray.ToArray());

            SubRoughTB = previousFitBase.SubRoughTB;
            SubphaseSLD = previousFitBase.SubphaseSLD;
            SuperphaseSLD = previousFitBase.SuperphaseSLD;

            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession;
            HoldsigmaCB = previousFitBase.HoldsigmaCB;
            ImpNormCB = previousFitBase.ImpNormCB;
            m_dImpNorm = previousFitBase.m_dImpNorm;
            WavelengthTB = previousFitBase.WavelengthTB;
          
            
            info = new double[9];

            //Setup arrays to hold the old values
            PreviousRhoArray = new List<double>(6);
            PreviousLengthArray = new List<double>(6);
            PreviousSigmaArray = new List<double>(6);

            //Get our Q data into a useable form
            Qincrement = ReflData.Instance.GetQData;
            RealRefl = ReflData.Instance.GetReflData;
            RealReflErrors = ReflData.Instance.GetRErrors;
            QErrors = ReflData.Instance.GetQErrors;

            ReflectivityMap = new double[ReflData.Instance.GetNumberDataPoints];

            //Create Z
            Z = new double[500];
            MakeZ();

            ElectronDensityArray = new double[500];
            BoxElectronDensityArray = new double[500];

            ZOffset = 25;
        }

        private void MakeZ()
        {
             //Make the Z Arrays
            double length = 0;

            for (int k = 0; k < BoxCountTB; k++)
            {
                length += _LengthArray[k];
            }
            for (int i = 0; i < 500; i++)
            {
                Z[i] = i * (50 + length) / 499.0;
            }

        }

        public abstract void StochFit();
               
        public virtual void UpdateProfile()
        {
           m_bvalidfit = false;

           double[] parameters = null;
           double[] eparameters = null;
           
           MakeParameters(ref eparameters, true);
          
           InfoStruct = new BoxModelSettings();
           SetInitStruct(ref InfoStruct, null, null, null);

           Calculations.RhoGenerate(InfoStruct, eparameters, eparameters.Length, ElectronDensityArray, BoxElectronDensityArray);

           InfoStruct.Dispose();
           
           
          
           if (Qincrement != null)
           {
               MakeParameters(ref parameters, false);
               Calculations.FastReflGenerate(InfoStruct, parameters, parameters.Length, ReflectivityMap);

               //if (m_bmodelreset == true)
               //{
               //    ReflGraphing.Clear();
               //    ReflGraphing.LoadDatawithErrorstoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
               //    m_bmodelreset = false;
               //}

               ////Setup the graphs
               //ReflGraphing.LoadfromArray("modelrefl", Qincrement, ReflectivityMap, System.Drawing.Color.Black, SymbolType.XCross, 4, true, string.Empty);
           }
           
            Update(this, null);
        }

       
        public void MakeParameters(ref double[] parameters, bool IsED)
        {
            int arrayconst = 0;
            int EDconst = 0;

            if (HoldsigmaCB)
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
                parameters = new double[arrayconst * BoxCountTB + 2];

                if (ZOffsetTB == 0)
                    parameters[1] = 25;
                else
                    parameters[1] = ZOffsetTB;
                EDconst++;
            }
            else
            {
                parameters = new double[arrayconst * BoxCountTB + 2];
            }

            parameters[0] = SubRoughTB;

            for (int i = 0; i < BoxCountTB; i++)
            {
                parameters[arrayconst * i + 1 + EDconst] = _LengthArray[i];

                if (!m_bUseSLD)
                    parameters[arrayconst * i + 2 + EDconst] = _RhoArray[i];
                else
                    parameters[arrayconst * i + 2 + EDconst] = _RhoArray[i] / SubphaseSLDTB;

                if (!HoldsigmaCB)
                    parameters[arrayconst * i + 3 + EDconst] = _SigmaArray[i];
            }

            if (!IsED)
                parameters[1 + BoxCountTB * arrayconst] = NormalizationFactor;
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

            if (WavelengthTB != 0)
                InitStruct.Wavelength = WavelengthTB;

            InitStruct.OneSigma = HoldsigmaCB;

            if (QSpreadTB != 0)
                InitStruct.QSpread = QSpreadTB;

            if (ImpNormCB == true)
                InitStruct.ImpNorm = ImpNormCB;
            
            if(Z != null)
                InitStruct.SetZ(Z, RealRho);

            InitStruct.WriteFiles = true;

        }

        protected abstract void SaveParamsForReport();
       

        public abstract string DataFit();
       
        public void UndoFit()
        {
            for (int i = 0; i < PreviousRhoArray.Count; i++)
            {
                RhoArray[i] = PreviousRhoArray[i];
                SigmaArray[i] = PreviousSigmaArray[i];
                LengthArray[i] = PreviousLengthArray[i];
            }
            SubRoughTB = m_dPrevioussigma;
            ZOffsetTB = m_dPreviouszoffset;
            m_dImpNorm = m_dPreviousImpNorm;

            m_dCovarArray = null;

            UpdateProfile();
        }

        protected void BackupArrays()
        {
            PreviousRhoArray.Clear();
            PreviousSigmaArray.Clear();
            PreviousLengthArray.Clear();

            _RhoArray.ForEach(p => PreviousRhoArray.Add(p));
            _SigmaArray.ForEach(p => PreviousSigmaArray.Add(p));
            _LengthArray.ForEach(p => PreviousLengthArray.Add(p));
            
            m_dPrevioussigma = SubRoughTB;
            m_dPreviouszoffset = ZOffsetTB;
            m_dPreviousImpNorm = m_dImpNorm;
        }

        public abstract void UpdateBoundsArrays(double[] UL, double[] LL);
       

        protected string ErrorReport(string OptionalString)
        {
            StringBuilder output = new StringBuilder();
            int offset = 0;

            if (!HoldsigmaCB)
                offset = 1;

            if (m_dCovarArray != null)
            {

                output.Append("\u03C3 = " + string.Format("{0:#.### E-0} ", SubRoughTB) + " " +
                    (char)0x00B1 + " " + m_dCovarArray[0].ToString("#.### E-0") + Environment.NewLine + Environment.NewLine);

                if(OptionalString != string.Empty)
                {
                    output.Append(OptionalString + Environment.NewLine);
                }

                for (int i = 0; i < BoxCountTB; i++)
                {
                    output.Append("Layer " + (i + 1).ToString() + Environment.NewLine);

                    if (m_bUseSLD == false)
                        output.Append("\t" + " \u03C1 = " + RhoArray[i].ToString("#.### E-0") + " " +
                            (char)0x00B1 + " " + m_dCovarArray[(2 + offset) * i + 1].ToString("#.### E-0") + Environment.NewLine);
                    else
                        output.Append("\t" + " SLD = " + RhoArray[i].ToString("#.### E-0") + " " +
                            (char)0x00B1 + " " + m_dCovarArray[(2 + offset) * i + 1].ToString("#.### E-0") + Environment.NewLine);

                    output.Append("\t" + " Length = " + LengthArray[i].ToString("#.### E-0") + " " +
                    (char)0x00B1 + " " + m_dCovarArray[(2 + offset) * i + 2].ToString("#.### E-0") + Environment.NewLine);

                    if (!HoldsigmaCB)
                        output.Append("\t" + " \u03C3 = " + SigmaArray[i].ToString("#.### E-0") + " " +
                          (char)0x00B1 + " " + m_dCovarArray[3 * i + 3].ToString("#.### E-0") + Environment.NewLine);
                }

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
                    return "Uknown reason for termination";
            }
        }

        public double WaveLength
        {
            get
            {
                return WavelengthTB;
            }
            set
            {
                WavelengthTB = value;
            }
        }

        public double GetSubRoughness
        {
            get
            {
                return SubRoughTB;
            }
            set
            {
                SubRoughTB = value;
            }
        }

        public bool IsOneSigma
        {
            get
            {
                return HoldsigmaCB;
            }
            set
            {
                HoldsigmaCB = value;
            }
        }

        public double ZOffset
        {
            get
            {
                return ZOffsetTB;
            }
            set
            {
                ZOffsetTB = value;
            }
        }

        public int BoxCount
        {
            get
            {
                return BoxCountTB;
            }
            set
            {
                BoxCountTB = value;
            }
        }

        public double SubphaseSLD
        {
            set
            {
                SubphaseSLDTB = value;
            }
            get
            {
                return SubphaseSLDTB;
            }
        }

        public double SuperphaseSLD
        {
            set
            {
                SuperSLDTB = value;
            }
            get
            {
                return SuperSLDTB;
            }
        }

        public List<double> RhoArray
        {
            get
            {
                return _RhoArray;
            }
        }

        public List<double> LengthArray
        {
            get
            {
                return _LengthArray;
            }
        }

        public List<double> SigmaArray
        {
            get
            {
                return _SigmaArray;
            }
        }

        public double[] Get_Z
        {
            get
            {
                return Z;
            }
        }

        public double[] Get_Rho
        {
            get
            {
                return ElectronDensityArray;
            }
        }

        public double[] Get_BoxRho
        {
            get
            {
                return BoxElectronDensityArray;
            }
        }

        public double HighQOffset
        {
            get { return _HighQOffset; }
            set { _HighQOffset = value; }
        }


        public double LowQOffset
        {
            get { return _LowQOffset; }
            set { _LowQOffset = value; }
        }
            
    }
}

