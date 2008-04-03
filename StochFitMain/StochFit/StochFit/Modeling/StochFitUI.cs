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
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Globalization;
using StochasticModeling;

namespace StochasticModeling.Modeling
{
    /// <summary>
    /// This class allows the user to set parameters for the stochastic search of the parameter space
    /// </summary>
    public partial class StochFitUI : StochFormBase
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public StochFitUI()
        {
            InitializeComponent();
            EDHighTB.Text = "130";
            EDLowTB.Text = "70";
            LengthHighTB.Text = "130";
            LengthLowTB.Text = "70";
            RoughHighTB.Text = "130";
            RoughLowTB.Text = "70";

            //Cutoff for errors. Any solution with errors greater than this percentage
            //of the parameter value will be discarded
            ErrorCutTB.Text = "30";
        }

        /// <summary>
        /// Get the number of iterations to run the parameter space search
        /// </summary>
        public int IterationCount
        {
            get
            {
                return int.Parse(IterTB.Text);
            }
        }

        /// <summary>
        /// Returns the ranges for the paramters to vary by
        /// </summary>
        /// <param name="parampercs"></param>
        public void GetParamPercs(ref double[] parampercs)
        {
            //Fill ED, length, and roughness in that order
            parampercs[0] = double.Parse(LengthHighTB.Text) / 100.0;
            parampercs[1] = double.Parse(LengthLowTB.Text) / 100.0;
            parampercs[2] = double.Parse(EDHighTB.Text) / 100.0;
            parampercs[3] = double.Parse(EDLowTB.Text) / 100.0;
            parampercs[4] = double.Parse(RoughHighTB.Text) / 100.0;
            parampercs[5] = double.Parse(RoughLowTB.Text) / 100.0;
            
            if(ErrCutCB.Checked)
                parampercs[6] = (double)int.Parse(ErrorCutTB.Text) / 100.0;
            else
                parampercs[6] = -1;
        }

        private void ErrCutCB_CheckedChanged(object sender, EventArgs e)
        {
            ErrorCutTB.Enabled = ErrCutCB.Checked;
        }

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry 
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to a double or false if not</param>
        protected override void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            base.ValidateNumericalInput(sender, e);
        }

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry 
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to an integer or false if not</param>
        protected override void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            base.ValidateIntegerInput(sender, e);
        }
    }
}