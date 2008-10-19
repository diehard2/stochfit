using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using StochasticModeling.Settings;

namespace StochasticModeling
{
    public class RhoFit:BoxReflFitBase
    {
        public RhoFit(double[] Z, double[] ERho): base(Z, ERho)
        { }

        protected override void SaveParamsForReport()
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

            info.Add(string.Format("Chi Square: " + m_dChiSquare + "\n"));
            info.Add(string.Format("The subphase roughness was: {0:#.### E-0} " + (char)0x00B1 + " {1:#.### E-0}\n", SubRoughTB, m_dCovarArray[0]));

            int offset = 0;

            if (!HoldsigmaCB)
                offset = 1;

            for (int i = 0; i < BoxCountTB; i++)
            {
                info.Add((i + 1).ToString());
                info.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[(2 + offset) * i + 1].ToString("#.### E-0"));

                if (m_bUseSLD == false)
                    info.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[(2 + offset) * i + 2].ToString("#.### E-0"));
                else
                    info.Add((RhoArray[i]).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (m_dCovarArray[(2 + offset) * i + 2]).ToString("#.### E-0"));

                if (HoldsigmaCB)
                    info.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[0].ToString("#.### E-0"));
                else
                    info.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[3 * i + 3].ToString("#.### E-0"));
            }
            g.SetRhoModelInfo = info;
        }

        public override string DataFit()
        {
            BackupArrays();

            double[] info = new double[9];
            double[] parameters = null;
            int arrayconst;

            MakeParameters(ref parameters, true);

            m_dCovarArray = new double[parameters.Length];

            InfoStruct = new BoxModelSettings();
            SetInitStruct(ref InfoStruct, null, null, null);


            m_dChiSquare = Calculations.Rhofit(InfoStruct, parameters, m_dCovarArray, parameters.Length, info, info.Length);

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
            SaveParamsForReport();

            return m_dChiSquare.ToString("##.### E-0");
        }

        public string ErrorReport()
        {
           string zstring = "Z Offset = " + string.Format("{0:#.### E-0} ", ZOffsetTB) + " " +
                    (char)0x00B1 + " " + m_dCovarArray[1].ToString("#.### E-0") + Environment.NewLine + Environment.NewLine;
           return ErrorReport(zstring);
        }

        public override void UpdateBoundsArrays(double[] UL, double[] LL)
        {
            throw new NotImplementedException();
        }

        public override void StochFit()
        {
            throw new NotImplementedException();
        }
    }
}
