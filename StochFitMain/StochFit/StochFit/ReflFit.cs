using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using StochasticModeling.Settings;
using System.Windows.Forms;

namespace StochasticModeling
{
    class ReflFit:BoxReflFitBase
    {
        public delegate bool StochasticModel(double[] ParamArray, double[] ChiSquareArray, double[] CovarArray, int size, int paramcount, BoxModelSettings InfoStruct,
                ref double[] chosenparameters, ref double[] chosencovar, ref string chosenchisquare);
        
        public StochasticModel ModelChooser;


        public ReflFit(BoxReflFitBase previousFitBase) : base(previousFitBase) { }


        public override string StochFit(double[] parampercs, int iterations)
        {
            

              

            double[] info = new double[9];
            double[] parameters = null;
            double[] covar = null;
            string chosenchisquare = string.Empty;
        

            MakeParameters(ref parameters, false);

            covar = new double[parameters.Length];
            double[] ParamArray = new double[1000 * parameters.Length];
            double[] ChiSquareArray = new double[1000];
            double[] CovarArray = new double[1000 * parameters.Length];
            int size = 0;


            InfoStruct = new BoxModelSettings();
            SetInitStruct(ref InfoStruct, parampercs);
            InfoStruct.Iterations = iterations;
           

            Calculations.ConstrainedStochFit(InfoStruct, parameters, covar, covar.Length, info, ParamArray, ChiSquareArray,ref size);

            //if (ModelChooser(ParamArray, ChiSquareArray, CovarArray, size, parameters.Length, InfoStruct, ref parameters, ref covar,ref chosenchisquare))
            //{
            //    BackupArrays();
            //    m_dCovarArray = covar;
            //    UpdatefromParameterArray(parameters);
            //    InfoStruct.Dispose();
            //    return chosenchisquare;
            //}
            
            InfoStruct.Dispose();
            return m_dChiSquare.ToString("##.### E-0");
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
            ginfo.Add(string.Format("The subphase roughness was: {0:#.### E-0} " + (char)0x00B1 + " {1:#.### E-0}\n", SubRoughTB, m_dCovarArray[0]));

            if (HoldsigmaCB == true)
            {
                for (int i = 0; i < BoxCountTB; i++)
                {
                    ginfo.Add((i + 1).ToString());
                    ginfo.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[2 * i + 1].ToString("#.### E-0"));

                    if (m_bUseSLD == false)
                        ginfo.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[2 * i + 2].ToString("#.### E-0"));
                    else
                        ginfo.Add((RhoArray[i] * SubphaseSLDTB).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (m_dCovarArray[2 * i + 2] * SubphaseSLDTB).ToString("#.### E-0"));

                    ginfo.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[0].ToString("#.### E-0"));
                }
            }
            else
            {
                for (int i = 0; i < BoxCount; i++)
                {
                    ginfo.Add((i + 1).ToString());
                    ginfo.Add(LengthArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[3 * i + 1].ToString("#.### E-0"));

                    if (m_bUseSLD == false)
                        ginfo.Add(RhoArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[3 * i + 2].ToString("#.### E-0"));
                    else
                        ginfo.Add((RhoArray[i] * SubphaseSLDTB).ToString("#.### E-0") + " " + (char)0x00B1 + " " + (m_dCovarArray[3 * i + 2] * SubphaseSLDTB).ToString("#.### E-0"));

                    ginfo.Add(SigmaArray[i].ToString("#.### E-0") + " " + (char)0x00B1 + " " + m_dCovarArray[3 * i + 3].ToString("#.### E-0"));
                }
            }
            g.SetReflModelInfo = ginfo;
        }

        public override string DataFit()
        {
            BackupArrays();

            double[] info = new double[9];
            double[] parameters = null;
           
            MakeParameters(ref parameters, false);

            m_dCovarArray = new double[parameters.Length];

            InfoStruct = new BoxModelSettings();
            SetInitStruct(ref InfoStruct, null);


            m_dChiSquare = Calculations.FastReflfit(InfoStruct, parameters, m_dCovarArray, parameters.Length, info, info.Length);

            InfoStruct.Dispose();

            return UpdatefromParameterArray(parameters);
        }

        private string UpdatefromParameterArray(double[] parameters)
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

            //Display the fit
            UpdateProfile();
            SaveParamsForReport();

            //Make sure parameters are reasonable
            for (int i = 0; i < parameters.Length; i++)
            {
                if (parameters[i] * 0.3 < m_dCovarArray[i])
                {
                    MessageBox.Show("The error in a fitting parameter is greater than 30% of the parameter value. Check the 'Fit details' button for more information");
                    break;
                }
            }

            return m_dChiSquare.ToString("##.### E-0");
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
                         (char)0x00B1 + " " + m_dCovarArray[m_dCovarArray.Length - 1].ToString("#.### E-0") + Environment.NewLine);
            }

            return ErrorReport(output);
        }
    }
}
