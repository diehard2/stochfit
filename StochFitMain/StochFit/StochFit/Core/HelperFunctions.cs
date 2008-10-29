using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StochasticModeling.Core
{
    class HelperFunctions
    {

        public static void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
               if (((TextBox)sender).IsEmpty() == false)
                    Double.Parse(((TextBox)sender).Text);
            }
            catch
            {
                MessageBox.Show("Error in input - A real number was expected");
                e.Cancel = true;
            }
        }

        public static void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
               Convert.ToInt32(((TextBox)sender).Text);
            }
            catch
            {
                MessageBox.Show("Error in input - An integer was expected");
                e.Cancel = true;
            }
        }

        public static double MakeChiSquare(double[] data, double[] fit, double[] errors, int highqoffset, int lowqoffset, int parameters)
        {
            double chi = 0;

            if (data != null)
            {
                if (errors == null)
                {
                    errors = data;
                }

                for (int i = lowqoffset; i < data.Length - highqoffset; i++)
                {
                    if (errors[i] > 0)
                        chi += (data[i] - fit[i]) * (data[i] - fit[i]) / (errors[i] * errors[i]);
                }

                chi /= (data.Length - highqoffset - lowqoffset - parameters);

            }

            return chi;
        }

        public static double EDFitnessScore(double[] data, double[] fit, int highqoffset, int lowqoffset, int parameters)
        {
            double fitness = 0;

            if (data != null)
            {
                for (int i = lowqoffset; i < data.Length - highqoffset; i++)
                {
                    fitness += (data[i] - fit[i]) * (data[i] - fit[i]);
                }
            }
            return fitness;
        }

        public static double ReflFitnessScore(double[] data, double[] fit, int highqoffset, int lowqoffset, int parameters)
        {
            double fitness = 0;
            for (int i = lowqoffset; i < data.Length - highqoffset; i++)
            {
                fitness += Math.Pow((Math.Log(data[i]) - Math.Log(fit[i])), 2.0);
            }

            return fitness;
        }

        /// <summary>
        /// Calculate the critical Q for a system
        /// </summary>
        /// <param name="dSLD">Substrate SLD</param>
        /// <param name="SupSLD">Superphase SLD</param>
        /// <param name="lambda">X-ray wavelength</param>
        /// <returns>Critical Q</returns>
        static public double CalcQc(double dSLD, double SupSLD)
        {
            if (dSLD - SupSLD > 0)
                return 4 * Math.Sqrt((Math.PI * (dSLD - SupSLD) * 1e-6));
            else
                return 0;
        }

        /// <summary>
        /// Calculates the value of a Fresnel curve at a given Q and Qc
        /// </summary>
        /// <param name="Q">Q value</param>
        /// <param name="Qc">Critical Q for the system</param>
        /// <returns>Fresnel reflectivity point</returns>
        static public double CalcFresnelPoint(double Q, double Qc)
        {
            if (Q <= Qc)
                return 1;
            else
            {
                double term1 = Math.Sqrt(1 - Math.Pow((Qc / Q), 2.0));
                return Math.Pow((1.0 - term1) / (1.0 + term1), 2.0);
            }
        }

        internal static void MakeFresnelCurve(double[] FresnelCurve, double[] Q, int datapoints, double SubphaseSLD, double SuperphaseSLD)
        {
            
            double Qc = CalcQc(SubphaseSLD, SuperphaseSLD);

            for (int i = 0; i < datapoints; i++)
            {
                FresnelCurve[i] = CalcFresnelPoint(Q[i], Qc);
            }
        }
    }
}
