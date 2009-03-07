using System;
using System.Collections.Generic;
using StochasticModeling.Settings;
using System.Windows.Forms;
using StochasticModeling.Core;
using System.IO;

#pragma warning disable 1591

namespace StochasticModeling
{
    public class ReflFit:BoxReflFitBase
    {
        public delegate bool StochasticModel(double[] ParamArray, double[] ChiSquareArray, double[] CovarArray, double[] info, int size, int paramcount, BoxModelSettings InfoStruct);
        
        public StochasticModel ModelChooser;


        public ReflFit(BoxReflFitBase previousFitBase) : base(previousFitBase) { }


        public override string StochFit(double[] parampercs, int iterations)
        {
            double[] parameters = null;
            string chosenchisquare = string.Empty;
        

            MakeParameters(ref parameters, false);

            double[] ParamArray = new double[1000 * parameters.Length];
            double[] ChiSquareArray = new double[1000];
            double[] CovarArray = new double[1000 * parameters.Length];
            double[] locinfo = new double[9 * 1000];
            int Size = 0;


            InfoStruct = new BoxModelSettings();
            SetInitStruct(ref InfoStruct, parampercs);
            InfoStruct.Iterations = iterations;
           

            NativeMethods.ConstrainedStochFit(InfoStruct, parameters, CovarArray, parameters.Length, locinfo, ParamArray, ChiSquareArray,ref Size);
            
            //Not ideal, will always back up regardless of whether the new model is accepted or not
            BackupArrays();

            if (ModelChooser != null)
            {
                if (ModelChooser(ParamArray, ChiSquareArray, CovarArray, locinfo, Size, parameters.Length, InfoStruct))
                {
                    
                }
            }

            UpdateProfile();
            InfoStruct.Dispose();
            return ChiSquare.ToString("##.### E-0");
        }

        public override void SaveParamsForReport()
        {

            ReportGenerator g = ReportGenerator.Instance;
            g.ClearReflModelInfo();
            int arrayconst = 2;

            List<string> ginfo = new List<string>();

            if (HoldsigmaCB)
                ginfo.Add("The reflectivity curve was fit with a single roughness parameter\n");
            else
            {
                ginfo.Add(String.Format("The reflectivity curve was fit with {0} roughness parameters\n", BoxCountTB + 1));
                arrayconst = 3;
            }

            ginfo.Add("Percent Error in Q: " + QSpreadTB.ToString() + "\n");
            ginfo.Add("Normalization Factor: " + NormalizationFactor.ToString() + "\n");
            ginfo.Add("Low Q Offset: " + LowQOffset.ToString()  + "\n");
            ginfo.Add("High Q Offset: " + HighQOffset.ToString() + "\n");
            ginfo.Add("Superphase SLD: " + SubphaseSLDTB.ToString() + "\n");
            ginfo.Add("Subphase SLD: " + SubphaseSLD.ToString() + "\n");
            ginfo.Add("Wavelength: " + WavelengthTB.ToString() + "\n");
            ginfo.Add("Chi Square for reflectivity fit: " + ChiSquare.ToString() + "\n");
            ginfo.Add(string.Format("The subphase roughness was: {0:#.### E-0} " + (char)0x00B1 + " {1:#.### E-0}\n", SubRoughTB, CovarArray[0]));

         
            for (int i = 0; i < BoxCountTB; i++)
            {
                ginfo.Add((i + 1).ToString());
                ginfo.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + CovarArray[arrayconst * i + 1].ToString("#.### E-0"));

                if (m_bUseSLD == false)
                    ginfo.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + CovarArray[arrayconst * i + 2].ToString("#.### E-0"));
                else
                    ginfo.Add((RhoArray[i] * SubphaseSLDTB).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (CovarArray[arrayconst * i + 2] * SubphaseSLDTB).ToString("#.### E-0"));

                if (HoldsigmaCB)
                    ginfo.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + CovarArray[0].ToString("#.### E-0"));
                else
                    ginfo.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + CovarArray[arrayconst * i + 3].ToString("#.### E-0"));
            }
           
            g.SetReflModelInfo = ginfo;
        }

        public override string DataFit()
        {
            BackupArrays();
            double[] parameters = null;
           
            MakeParameters(ref parameters, false);

            m_dCovarArray = new double[parameters.Length];

            InfoStruct = new BoxModelSettings();
            SetInitStruct(ref InfoStruct, null);


            NativeMethods.FastReflfit(InfoStruct, parameters, m_dCovarArray, parameters.Length, _fitinfo);

            InfoStruct.Dispose();

            UpdatefromParameterArray(parameters);
            UpdateFit(parameters);
           

            return MakeChiSquare();
        }

        private void UpdateFit(double[] parameters)
        {
            //Display the fit
            UpdateProfile();
            

            //Update our save list
            FitHolder.Add(CreateLightWeightClone());


            //Make sure parameters are reasonable
            for (int i = 0; i < parameters.Length; i++)
            {
                if (parameters[i] * 0.3 < CovarArray[i])
                {
                    MessageBox.Show("The error in a fitting parameter is greater than 30% of the parameter value. Check the 'Fit details' button for more information");
                    break;
                }
            }
        }

        public override void UpdatefromParameterArray(double[] parameters)
        {
            int arrayconst;

            SubRoughTB = parameters[0];

            if (HoldsigmaCB == true)
                arrayconst = 2;
            else
                arrayconst = 3;

            //Update paramters
            for (int i = 0; i < BoxCountTB; i++)
            {
                LengthArray[i] = parameters[arrayconst * i + 1];

                if (m_bUseSLD == false)
                    RhoArray[i] = parameters[arrayconst * i + 2];
                else
                    RhoArray[i] = parameters[arrayconst * i + 2] * SubphaseSLDTB;

                if (HoldsigmaCB)
                    SigmaArray[i] = parameters[0];
                else
                    SigmaArray[i] = parameters[3 * i + 3];
            }

            NormalizationFactor = parameters[parameters.Length - 1];
        }

        public override string ErrorReport()
        {
            string output = string.Empty;

            if (ImpNormCB)
            {
               output = (Environment.NewLine + "Normalization factor = " + NormalizationFactor.ToString("#.###") + " " +
                         (char)0x00B1 + " " + CovarArray[CovarArray.Length - 1].ToString("#.### E-0") + Environment.NewLine);
            }

            return ErrorReport(output);
        }

        public override BoxReflFitBase CreateLightWeightClone()
        {
            ReflFit b = new ReflFit(this);
            
            b.m_dCovarArray = CovarArray;
            b.QSpreadTB = QSpreadTB;
            b.ImpNormCB = ImpNormCB;
            b.NormalizationFactor = NormalizationFactor;
            b.HighQOffset = HighQOffset;
            b.LowQOffset = LowQOffset;
            b.m_dChiSquare = m_dChiSquare;
            b.UpdateProfile();

            return b as BoxReflFitBase;
        }

        public override void LoadLightWeightClone(BoxReflFitBase b)
        {
            _Z = b.Z;
            _ElectronDensityArray = b.ElectronDensityArray;
            _BoxElectronDensityArray = b.BoxElectronDensityArray;
            _ReflectivityMap = b.ReflectivityMap;
            NormalizationFactor = b.NormalizationFactor;
            ImpNormCB = b.ImpNormCB;
            SubphaseSLD = b.SubphaseSLD;
            SuperphaseSLD = b.SuperphaseSLD;
            QSpreadTB = b.QSpreadTB;
            HighQOffset = b.HighQOffset;
            LowQOffset = b.LowQOffset;
            BoxCountTB = b.BoxCount;
            IsOneSigma = b.IsOneSigma;
            _RhoArray = new List<double>(b.RhoArray.ToArray());
            _LengthArray = new List<double>(b.LengthArray.ToArray());
            _SigmaArray = new List<double>(b.SigmaArray.ToArray());
            _fitinfo = (double[])((ReflFit)b)._fitinfo.Clone();
            m_dCovarArray = b.CovarArray;
            m_dChiSquare = b.ChiSquare;
            SubRoughTB = b.GetSubRoughness;
        }


        public override string MakeChiSquare()
        {
            ChiSquare = HelperFunctions.MakeChiSquare(ReflData.Instance.GetReflData, ReflectivityMap, ReflData.Instance.GetRErrors,
                HighQOffset, LowQOffset, ParamSize());
            return ChiSquare.ToString("#.### E-0");
        }



        public override string MakeFitnessScore()
        {
            FitnessScore = HelperFunctions.ReflFitnessScore(ReflData.Instance.GetReflData, ReflectivityMap, HighQOffset, LowQOffset, ParamSize());
            return FitnessScore.ToString("#.### E-0");
        }

        private int ParamSize()
        {
            int paramsize = 0;
            int paramcount = 1;

            if (!ImpNormCB)
                paramcount = 2;

            if (HoldsigmaCB)
                paramsize = BoxCountTB * 2 + paramcount;
            else
                paramsize = BoxCountTB * 3 + paramcount;

            return paramsize;
        }

        public override void WriteFiles(System.IO.FileInfo path)
        {
            string EDFile = path.FullName.TrimEnd(path.Extension.ToCharArray()) + "MDEDP" + path.Extension;
            string ReflFile = path.FullName.TrimEnd(path.Extension.ToCharArray()) + "MIRefl" + path.Extension;;
            
            using (StreamWriter file = new StreamWriter(EDFile))
            {
                file.WriteLine("Z \t ED \t BoxED");

                for (int i = 0; i < _Z.Length; i++)
                {
                    file.WriteLine(_Z[i] + "\t" + _ElectronDensityArray[i] + "\t" + BoxElectronDensityArray[i]);
                }
            }


            using (StreamWriter file = new StreamWriter(ReflFile))
            {
                double[] Q = ReflData.Instance.GetQData;
                double[] R = ReflData.Instance.GetReflData;
                double[] eR = ReflData.Instance.GetRErrors;
                double Qc = HelperFunctions.CalcQc(SubphaseSLD, SuperphaseSLD);
                int datapoints = ReflData.Instance.GetNumberDataPoints;
                double[] FresnelCurve = new double[datapoints];
                HelperFunctions.MakeFresnelCurve(FresnelCurve, Q, datapoints, SubphaseSLD, SuperphaseSLD);

                //Write out a header
                file.WriteLine("Q \t R \t eR \t R/Fresnel \t eR/Fresnel \t Rfit \t Rfit/Fresnel");

                for (int i = 0; i < ReflData.Instance.GetNumberDataPoints; i++)
                {
                    file.WriteLine(Q[i] + "\t" + R[i] + "\t" + eR[i] + "\t" + R[i]/FresnelCurve[i] + "\t" +
                        eR[i]/FresnelCurve[i] + "\t" + _ReflectivityMap[i] + "\t" + _ReflectivityMap[i]/FresnelCurve[i]);
                }
            }
        }

        public override void ClearReports()
        {
            ReportGenerator.Instance.ClearReflModelInfo();
        }
    }
}
