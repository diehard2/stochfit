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
    public partial class AnnealingParams : StochFormBase
    {
        public AnnealingParams()
        {
            InitializeComponent();
        }

        public AnnealingParams(double inittemp, int platnum, double tempslope)
        {
            //Thread.CurrentThread.CurrentCulture = new CultureInfo("en-US");
            //Thread.CurrentThread.CurrentUICulture = new CultureInfo("en-US");

            InitializeComponent();

            tempTB.Text = inittemp.ToString();
            plattb.Text = platnum.ToString();
            slopeTB.Text = tempslope.ToString();
        }

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

        protected override void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            base.ValidateNumericalInput(sender, e);
        }

        protected override void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            base.ValidateIntegerInput(sender, e);
        }
    }
}