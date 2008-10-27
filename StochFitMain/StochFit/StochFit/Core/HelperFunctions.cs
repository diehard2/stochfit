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
                errors = data;
            }

            for (int i = lowqoffset; i < data.Length - highqoffset; i++)
            {
                if(errors[i] > 0)
                    chi += (data[i] - fit[i]) * (data[i] - fit[i]) / (errors[i]*errors[i]);
            }

            chi /= (data.Length - highqoffset - lowqoffset - parameters);

            return chi;
        }

        public static double EDFitnessScore(double[] data, double[] fit, int highqoffset, int lowqoffset, int parameters)
        {
            double fitness = 0;
            for (int i = lowqoffset; i < data.Length - highqoffset; i++)
            {
                fitness += (data[i] - fit[i]) * (data[i] - fit[i]);
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
    }
}
