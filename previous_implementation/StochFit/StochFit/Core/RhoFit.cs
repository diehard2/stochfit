using System;
using System.Collections.Generic;
using StochasticModeling.Settings;
using StochasticModeling.Core;
using System.IO;

#pragma warning disable 1591

namespace StochasticModeling
{
    public class RhoFit:BoxReflFitBase
    {
        public RhoFit(double[] Z, double[] ERho): base(Z, ERho)
        { }

        public override void SaveParamsForReport()
        {
            ReportGenerator g = ReportGenerator.Instance;
            g.ClearRhoModelInfo();

            List<string> info = new List<string>();

            if (HoldsigmaCB)
            {
                if (m_bUseSLD == false)
                    info.Add("The electron density profile was fit with a single roughness parameter\n");
                else
                    info.Add("The SLD profile was fit with a single roughness parameter\n");
            }
            else
            {
                if (m_bUseSLD == false)
                    info.Add(String.Format("The electron density profile was fit with {0} roughness parameters\n", (BoxCountTB + 1)));
                else
                    info.Add(String.Format("The SLD profile was fit with {0} roughness parameters\n", (BoxCountTB + 1)));
            }

            info.Add(string.Format("Chi Square: " + ChiSquare + "\n"));
            info.Add(string.Format("The subphase roughness was: {0:#.### E-0} " + (char)0x00B1 + " {1:#.### E-0}\n", SubRoughTB, CovarArray[0]));

            int offset = 0;

            if (!HoldsigmaCB)
                offset = 1;

            for (int i = 0; i < BoxCountTB; i++)
            {
                info.Add((i + 1).ToString());
                info.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + CovarArray[(2 + offset) * i + 1].ToString("#.### E-0"));

                if (m_bUseSLD == false)
                    info.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + CovarArray[(2 + offset) * i + 2].ToString("#.### E-0"));
                else
                    info.Add((RhoArray[i]).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (CovarArray[(2 + offset) * i + 2]).ToString("#.### E-0"));

                if (HoldsigmaCB)
                    info.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + CovarArray[0].ToString("#.### E-0"));
                else
                    info.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + CovarArray[3 * i + 3].ToString("#.### E-0"));
            }
            g.SetRhoModelInfo = info;
        }

        public override string DataFit()
        {
            BackupArrays();
            double[] parameters = null;
            int arrayconst;

            MakeParameters(ref parameters, true);

            m_dCovarArray = new double[parameters.Length];

            InfoStruct = new BoxModelSettings();
            SetInitStruct(ref InfoStruct, null);

            Calculations.Rhofit(InfoStruct, parameters, m_dCovarArray, parameters.Length, _fitinfo);
           
            InfoStruct.Dispose();

            SubRoughTB = parameters[0];
            ZOffsetTB = parameters[1];

            if (HoldsigmaCB == true)
                arrayconst = 2;
            else
                arrayconst = 3;

            //Update paramters
            for (int i = 0; i < BoxCountTB; i++)
            {
                LengthArray[i] = parameters[arrayconst * i + 2];

                if (m_bUseSLD == false)
                    RhoArray[i] = parameters[arrayconst * i + 3];
                else
                    RhoArray[i] = parameters[arrayconst * i + 3] * SubphaseSLDTB;

                if (HoldsigmaCB)
                    SigmaArray[i] = parameters[0];
                else
                    SigmaArray[i] = parameters[3 * i + 4];
            }

            //Display the fit
            UpdateProfile();
           

            return MakeChiSquare();
        }

        public override string ErrorReport()
        {
            string zstring = string.Empty;
            if (m_dCovarArray != null)
            {
                zstring = "Z Offset = " + string.Format("{0:#.### E-0} ", ZOffsetTB) + " " +
                    (char)0x00B1 + " " + m_dCovarArray[1].ToString("#.### E-0") + Environment.NewLine + Environment.NewLine;
            }
           return ErrorReport(zstring);
        }

        public override string StochFit(double[] parampercs, int iterations)
        {
            throw new NotImplementedException();
        }

        public override BoxReflFitBase CreateLightWeightClone()
        {
            throw new NotImplementedException();
        }

        public override void LoadLightWeightClone(BoxReflFitBase b)
        {
            throw new NotImplementedException();
        }


        public override void UpdatefromParameterArray(double[] paramarray)
        {
            throw new NotImplementedException();
        }

        public override string MakeChiSquare()
        {
            ChiSquare = HelperFunctions.MakeChiSquare(RealRho, ElectronDensityArray,null, 0, 0, ParamSize());
            return ChiSquare.ToString("##.### E-0");
        }

        public override string MakeFitnessScore()
        {
            FitnessScore = HelperFunctions.EDFitnessScore(RealRho, ElectronDensityArray, 0, 0, ParamSize());
            return FitnessScore.ToString("##.### E-0");
        }

        private int ParamSize()
        {
            int paramsize = 0;
            if (HoldsigmaCB)
                paramsize = BoxCountTB * 2 + 2;
            else
                paramsize = BoxCountTB * 3 + 2;

            return paramsize;
        }

        public override void WriteFiles(System.IO.FileInfo path)
        {
            using (StreamWriter file = new StreamWriter(path.FullName))
            {
                file.WriteLine("Z \t ED \t BoxED");
                for (int i = 0; i < _Z.Length; i++)
                {
                    file.WriteLine(_Z[i] + "\t" + _ElectronDensityArray[i] + "\t" + BoxElectronDensityArray[i]);
                }
            }
        }

        public override void ClearReports()
        {
            ReportGenerator.Instance.ClearRhoModelInfo();
        }
    }
}
