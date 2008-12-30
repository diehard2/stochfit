/* 
 *	Copyright (C) 2008 Stephen Danauskas
 *	
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

using System;
using System.Windows.Forms;

namespace StochasticModeling.Modeling
{
    /// <summary>
    /// This form allows one to input the paramters for either Simulated Annealing or Stochastic Tunneling Annealing
    /// </summary>
    public partial class AnnealingParams : Form
    {
        /// <summary>
        /// Constructor for Annealing Parameters Form. 
        /// </summary>
        /// <param name="inittemp">Initial temperature</param>
        /// <param name="platnum">Number of iterations before the temperature is decreased by tempslope</param>
        /// <param name="tempslope">Fraction by which the temperature is decreased every platnum iterations</param>
        public AnnealingParams(double inittemp, int platnum, double tempslope)
        {
            InitializeComponent();

            tempTB.Text = inittemp.ToString();
            plattb.Text = platnum.ToString();
            slopeTB.Text = tempslope.ToString();
        }

        /// <summary>
        /// Allows for retrieving parameters defined by the user
        /// </summary>
        /// <param name="inittemp">Initial temperature</param>
        /// <param name="platnum">Number of iterations before the temperature is decreased by tempslope</param>
        /// <param name="tempslope">Fraction by which the temperature is decreased every platnum iterations</param>
        public void GetParams(out double inittemp, out int platnum, out double tempslope)
        {
            inittemp = double.Parse(tempTB.Text);
            platnum = int.Parse(plattb.Text);
            tempslope = double.Parse(slopeTB.Text);
        }

        private void OK_Click(object sender, EventArgs e)
        {
            this.Close();
        }

       
        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to a double or false if not</param>
        private void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
                base.OnValidating(e);
                Double.Parse(((TextBox)sender).Text);
            }
            catch
            {
                MessageBox.Show("Error in input - A real number was expected");
                e.Cancel = true;
            }
        }

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry 
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to an integer or false if not</param>
        private void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
                base.OnValidating(e);
                Convert.ToInt32(((TextBox)sender).Text);
            }
            catch
            {
                MessageBox.Show("Error in input - An integer was expected");
                e.Cancel = true;
            }
        }
    }
}