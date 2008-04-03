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

        #region Methods to get film parameters

        public int IterationCount
        {
            get
            {
                return int.Parse(IterTB.Text);
            }
        }

        public double LengthHigh
        {
            get
            {
                return double.Parse(LengthHighTB.Text);
            }
        }

        public double LengthLow
        {
            get
            {
                return double.Parse(LengthLowTB.Text);
            }
        }

        public double EDHigh
        {
            get
            {
                return double.Parse(EDHighTB.Text);
            }
        }

        public double EDLow
        {
            get
            {
                return double.Parse(EDLowTB.Text);
            }
        }

        public double RoughHigh
        {
            get
            {
                return double.Parse(RoughHighTB.Text);
            }
        }

        public double RoughLow
        {
            get
            {
                return double.Parse(RoughLowTB.Text);
            }
        }

        public int PercentCutoff
        {
            get
            {
                if (ErrCutCB.Checked == true)
                    return int.Parse(ErrorCutTB.Text);
                else
                    return -1;
            }
        }

        #endregion

        public void GetParamPercs(ref double[] parampercs)
        {
            //Fill ED, length, and roughness in that order
            parampercs[0] = LengthHigh/100.0;
            parampercs[1] = LengthLow / 100.0;
            parampercs[2] = EDHigh / 100.0;
            parampercs[3] = EDLow / 100.0;
            parampercs[4] = RoughHigh / 100.0;
            parampercs[5] = RoughLow / 100.0;
            parampercs[6] = PercentCutoff / 100.0;
        }

        private void ErrCutCB_CheckedChanged(object sender, EventArgs e)
        {
            if (ErrCutCB.Checked == true)
            {
                ErrorCutTB.Enabled = true;
            }
            else
            {
                ErrorCutTB.Enabled = false;
            }
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