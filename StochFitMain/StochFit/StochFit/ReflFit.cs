using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace StochasticModeling
{
    class ReflFit:BoxReflFitBase
    {
        public ReflFit(BoxReflFitBase previousFitBase) : base(previousFitBase) { }


        public override void StochFit()
        {
            throw new NotImplementedException();
        }

        protected override void SaveParamsForReport()
        {

            ReportGenerator g = ReportGenerator.Instance;
            g.ClearReflModelInfo();

            List<string> ginfo = new List<string>();

            if (HoldsigmaCB)
                ginfo.Add("The reflectivity curve was fit with a single roughness parameter\n");
            else
                ginfo.Add(String.Format("The reflectivity curve was fit with {0} roughness parameters\n", BoxCountTB + 1));

            ginfo.Add(string.Format("Percent Error in Q: " + QSpreadTB + "\n"));
            ginfo.Add(string.Format("Normalization Factor: " + NormalizationFactor + "\n"));
            ginfo.Add(string.Format("Critical Edge Offset: " + LowQOffset  + "\n"));
            ginfo.Add(string.Format("High Q Offset: " + HighQOffset + "\n"));
            ginfo.Add(string.Format("Superphase SLD: " + SubphaseSLDTB + "\n"));
            ginfo.Add(string.Format("Subphase SLD: " + SubphaseSLD + "\n"));
            ginfo.Add(string.Format("Wavelength: " + WavelengthTB + "\n"));
            ginfo.Add(string.Format("Chi Square for reflectivity fit: " + m_dChiSquare.ToString() + "\n"));
            ginfo.Add(string.Format("The subphase roughness was: {0:#.### E-0} " + (char)0x00B1 + " {1:#.### E-0}\n", SubRoughTB, covar[0]));

            if (HoldsigmaCB == true)
            {
                for (int i = 0; i < BoxCountTB; i++)
                {
                    ginfo.Add((i + 1).ToString());
                    ginfo.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[2 * i + 1].ToString("#.### E-0"));

                    if (m_bUseSLD == false)
                        ginfo.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[2 * i + 2].ToString("#.### E-0"));
                    else
                        ginfo.Add((RhoArray[i] * SubphaseSLDTB).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (covar[2 * i + 2] * SubphaseSLDTB ).ToString("#.### E-0"));

                    ginfo.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[0].ToString("#.### E-0"));
                }
            }
            else
            {
                for (int i = 0; i < BoxCount; i++)
                {
                    ginfo.Add((i + 1).ToString());
                    ginfo.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[3 * i + 1].ToString("#.### E-0"));

                    if (m_bUseSLD == false)
                        ginfo.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[3 * i + 2].ToString("#.### E-0"));
                    else
                        ginfo.Add((RhoArray[i] * SubphaseSLDTB).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (covar[3 * i + 2] * SubphaseSLDTB).ToString("#.### E-0"));

                    ginfo.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + covar[3 * i + 3].ToString("#.### E-0"));
                }
            }
            g.SetReflModelInfo = ginfo;
        }

        public override string DataFit()
        {
            throw new NotImplementedException();
        }

        public override void UpdateBoundsArrays(double[] UL, double[] LL)
        {
            throw new NotImplementedException();
        }

        public string ErrorReport()
        {
            string output = string.Empty;

            if (ImpNormCB)
            {
               output = (Environment.NewLine + "Normalization factor = " + NormalizationFactor.ToString("#.###") + " " +
                         (char)0x00B1 + " " + covar[covar.Length - 1].ToString("#.### E-0") + Environment.NewLine);
            }

            return ErrorReport(output);
        }
    }
}
