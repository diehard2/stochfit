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

            if (errors == null)
            {
                errors = new double[data.Length];

                for (int i = 0; i < data.Length; i++)
                {
                    errors[i] = 1.0;
                }
            }

            for (int i = lowqoffset; i < data.Length - highqoffset; i++)
            {
                chi = (data[i] - fit[i]) * (data[i] - fit[i]) / errors[i];
            }

            chi /= (data.Length - highqoffset - lowqoffset - parameters);

            return chi;
        }

        public static double FitnessScore(double[] data, double[] fit, int highqoffset, int lowqoffset, int parameters)
        {
            double fitness = 0;
            for (int i = lowqoffset; i < data.Length - highqoffset; i++)
            {
                fitness = (Math.Log(data[i]) - Math.Log(fit[i])) * (Math.Log(data[i]) - Math.Log(fit[i]));
            }

            return fitness;
        }
    }
}
