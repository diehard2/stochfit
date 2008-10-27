using System;
using System.Collections.Generic;
using System.Text;
using StochasticModeling.Settings;
using System.IO;


#pragma warning disable 1591

namespace StochasticModeling
{
    
    public abstract class BoxReflFitBase
    {
        #region Variables
        protected bool m_bUseSLD = false;
        private double m_zlength = 0;
        //Arrays
        protected List<double> _RhoArray;
        protected List<double> _LengthArray;
        protected List<double> _SigmaArray;
        protected List<double> PreviousRhoArray;
        protected List<double> PreviousLengthArray;
        protected List<double> PreviousSigmaArray;
        protected List<BoxReflFitBase> FitHolder;
        protected double m_dPrevioussigma;

        protected double[] _fitinfo;

        private double _FitnessScore;

        protected double FitnessScore
        {
            get { return _FitnessScore; }
            set { _FitnessScore = value; }
        }
        protected double oldnormfactor;
        protected bool initialized = false;
        protected double LeftOffset;
        private double _NormalizationFactor;
        protected double SubphaseSLDTB;
        protected double SuperSLDTB;
        protected double WavelengthTB;
        private double _QSpreadTB;
        protected double SubRoughTB;
        protected double ZOffsetTB;
        private int _HighQOffset;
        private int _LowQOffset;
        private bool _ImpNormCB;
        protected double[] _ReflectivityMap;
        protected double[] _Z;
        protected double[] _ElectronDensityArray;
        protected double[] _BoxElectronDensityArray;
        protected double[] RealRho;
        private double[] _UL;
        private double[] _LL;
        protected BoxModelSettings InfoStruct;
        protected bool HoldsigmaCB;
        protected int BoxCountTB;
        protected double[] m_dCovarArray;

        public double[] CovarArray
        {
            get { return (double[]) m_dCovarArray.Clone(); }
            set { m_dCovarArray = (double[])value.Clone(); }
        }
        protected double m_dPreviouszoffset;
        protected double m_dChiSquare;

        public double ChiSquare
        {
            get { return m_dChiSquare; }
            set { m_dChiSquare = value; }
        }
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
            _fitinfo = new double[9];

            //Setup arrays to hold the old values
            PreviousRhoArray = new List<double>(6);
            PreviousLengthArray = new List<double>(6);
            PreviousSigmaArray = new List<double>(6);
            FitHolder = new List<BoxReflFitBase>(100);
            if (ERho != null)
            {
                RealRho = (double[])ERho.Clone();
                _ElectronDensityArray = new double[ERho.Length];
                _BoxElectronDensityArray = new double[ERho.Length];
                this._Z = (double[])Z.Clone();
            }
            else
            {
                _ElectronDensityArray = new double[500];
                _BoxElectronDensityArray = new double[500];
                _Z = new double[500];
                MakeZ(true);
            }
        }

        public abstract void WriteFiles(FileInfo path);

        public BoxReflFitBase(BoxReflFitBase previousFitBase)
        {
            _RhoArray = new List<double>(previousFitBase._RhoArray.ToArray());
            _LengthArray = new List<double>(previousFitBase._LengthArray.ToArray());
            _SigmaArray = new List<double>(previousFitBase._SigmaArray.ToArray());
            BoxCountTB = previousFitBase.BoxCountTB;
            SubRoughTB = previousFitBase.SubRoughTB;
            SubphaseSLD = previousFitBase.SubphaseSLD;
            SuperphaseSLD = previousFitBase.SuperphaseSLD;

            m_bUseSLD = Properties.Settings.Default.UseSLDSingleSession;
            HoldsigmaCB = previousFitBase.HoldsigmaCB;
            _ImpNormCB = previousFitBase._ImpNormCB;
            m_dImpNorm = previousFitBase.m_dImpNorm;
            WavelengthTB = 1.24;
          
            
            _fitinfo = new double[9];

            //Setup arrays to hold the old values
            PreviousRhoArray = new List<double>(6);
            PreviousLengthArray = new List<double>(6);
            PreviousSigmaArray = new List<double>(6);
            _ReflectivityMap = new double[ReflData.Instance.GetNumberDataPoints];
            FitHolder = new List<BoxReflFitBase>(100);

            //Create Z
            _Z = new double[500];
            MakeZ(true);

            _ElectronDensityArray = new double[500];
            _BoxElectronDensityArray = new double[500];

            ZOffset = 25;
        }

        public abstract BoxReflFitBase CreateLightWeightClone();

        public abstract void LoadLightWeightClone(BoxReflFitBase b);

        private void MakeZ(bool initialize)
        {
            if (RealRho == null)
            {
                //Make the Z thickness array
                double templength = ZOffsetTB +35;

                for (int k = 0; k < BoxCountTB; k++)
                {
                    templength += _LengthArray[k];
                }

                if (templength != m_zlength || initialize)
                {
                    m_zlength = templength;

                    for (int i = 0; i < 500; i++)
                    {
                        _Z[i] = i * m_zlength / 499.0;
                    }
                }
            }
        }

        public abstract string StochFit(double[] parampercs, int iterations);
        public abstract string MakeChiSquare();
        public abstract string MakeFitnessScore();

               
        public virtual void UpdateProfile()
        {
           double[] parameters = null;
           double[] eparameters = null;
           
           MakeParameters(ref eparameters, true);
          
           InfoStruct = new BoxModelSettings();
           SetInitStruct(ref InfoStruct, null);

           Calculations.RhoGenerate(InfoStruct, eparameters, eparameters.Length, _ElectronDensityArray, _BoxElectronDensityArray);
          
           if (_ReflectivityMap != null)
           {
               MakeParameters(ref parameters, false);
               Calculations.FastReflGenerate(InfoStruct, parameters, parameters.Length, _ReflectivityMap);
           }

           InfoStruct.Dispose();

            if(Update != null)
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
                parameters[1 + BoxCountTB * arrayconst] = _NormalizationFactor;
        }

        protected virtual void SetInitStruct(ref BoxModelSettings InitStruct, double[] parampercs)
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

            if (_QSpreadTB != 0)
                InitStruct.QSpread = _QSpreadTB;

            if (_ImpNormCB == true)
                InitStruct.ImpNorm = _ImpNormCB;

            if (_Z != null)
            {
                MakeZ(false);
                InitStruct.SetZ(_Z, RealRho);
            }

            InitStruct.WriteFiles = true;

        }

        public abstract void SaveParamsForReport();
        public abstract void ClearReports();

        public abstract string DataFit();
       
        public void UndoFit()
        {
            if (PreviousRhoArray.Count > 0)
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
        }

        protected bool CheckforParameterChange
        {
            get
            {
                bool change = false;
                if (PreviousRhoArray.Count > 0)
                {
                    if(!((m_dPreviousImpNorm == m_dImpNorm) && (m_dPrevioussigma == SubRoughTB) && (m_dPreviouszoffset == ZOffsetTB)))
                        change = true;
                    for (int i = 0; i < PreviousRhoArray.Count; i++)
                    {
                        if (!((_RhoArray[i] == PreviousRhoArray[i]) && (_SigmaArray[i] == PreviousSigmaArray[i]) && (_LengthArray[i] == PreviousLengthArray[i])))
                            change = true;
                    }

                }
                return change;
            }
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

        public abstract string ErrorReport();

        public abstract void UpdatefromParameterArray(double[] paramarray);

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

                output.Append(Environment.NewLine + "Levenberg-Marquadt output" + Environment.NewLine + "\tNumber of iterations : " + _fitinfo[5].ToString() + Environment.NewLine);
                    output.Append("Reason for termination: " + termreason((int)_fitinfo[6]));
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
                    return "Uknown";
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
            get{return HoldsigmaCB;}
            set{HoldsigmaCB = value;}
        }

        public double ZOffset
        {
            get{return ZOffsetTB;}
            set{ZOffsetTB = value;}
        }

        public int BoxCount
        {
            get{return BoxCountTB;}
            set{BoxCountTB = value;}
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

        public bool ImpNormCB
        {
            get { return _ImpNormCB; }
            set { _ImpNormCB = value; }
        }

        public double[] ReflectivityMap
        {
            get { return (double[])_ReflectivityMap.Clone(); }
        }

        public double[] Z
        {
            get { return (double[])_Z.Clone(); }
        }

        public double[] ElectronDensityArray
        {
            get { return (double[])_ElectronDensityArray.Clone(); }
        }

        public double[] BoxElectronDensityArray
        {
            get { return (double[])_BoxElectronDensityArray.Clone(); }
        }

        public double[] UL
        {
            get { return _UL; }
            set { _UL = value; }
        }

        public double[] LL
        {
            get { return _LL; }
            set { _LL = value; }
        }

        public int HighQOffset
        {
            get { return _HighQOffset; }
            set { _HighQOffset = value; }
        }

        public int LowQOffset
        {
            get { return _LowQOffset; }
            set { _LowQOffset = value; }
        }

        public double NormalizationFactor
        {
            get { return _NormalizationFactor; }
            set { _NormalizationFactor = value; }
        }

        public double QSpreadTB
        {
            get { return _QSpreadTB; }
            set { _QSpreadTB = value; }
        }
        public double[] Fitinfo
        {
            get { return (double[])_fitinfo.Clone(); }
        }
    }
}

