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
    /// Form for setting the Stochastic Tunneling parameters
    /// </summary>
    public partial class STUNAnnealingParams : Form
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="inittemp"></param>
        /// <param name="platnum"></param>
        /// <param name="tempslope"></param>
        /// <param name="gamma"></param>
        /// <param name="STUNfunc"></param>
        /// <param name="STUNAdaptive"></param>
        /// <param name="STUNtempiter"></param>
        /// <param name="STUNdeciter"></param>
        /// <param name="STUNgammadec"></param>
       public STUNAnnealingParams(double inittemp, int platnum, double tempslope, double gamma, int STUNfunc, bool STUNAdaptive,
                    int STUNtempiter,int STUNdeciter, double STUNgammadec)
        {
            InitializeComponent();

            tempTB.Text = inittemp.ToString();
            plattb.Text = platnum.ToString();
            slopeTB.Text = tempslope.ToString();
            GammaTB.Text = gamma.ToString();

            funcCombo.SelectedIndex = STUNfunc;
            adaptiveCB.Checked = STUNAdaptive;
            TempIterTB.Text = STUNtempiter.ToString();
            STUNdecTB.Text = STUNdeciter.ToString();
            TempIterTB.Text = STUNtempiter.ToString();
            gammadecTB.Text = STUNgammadec.ToString();

        }

        /// <summary>
        /// Returns the parameters generated by the user
        /// </summary>
        /// <param name="inittemp">Initial annealing temperature</param>
        /// <param name="runningaverageiter"></param>
        /// <param name="tempslope"></param>
        /// <param name="gamma"></param>
        /// <param name="func"></param>
        /// <param name="adaptive"></param>
        /// <param name="tempchangeiter"></param>
        /// <param name="STUNdeciter"></param>
        /// <param name="STUNgammadec"></param>
        public void GetParams(out double inittemp, out int  runningaverageiter, out double tempslope, out double gamma, out int func,
            out bool adaptive, out int tempchangeiter, out int STUNdeciter, out double STUNgammadec)
        {
            inittemp = double.Parse(tempTB.Text);
            runningaverageiter = int.Parse(plattb.Text);
            tempslope = double.Parse(slopeTB.Text);
            gamma = double.Parse(GammaTB.Text);

            func = funcCombo.SelectedIndex;
            adaptive = adaptiveCB.Checked;
            
            tempchangeiter = int.Parse(TempIterTB.Text);
            STUNdeciter = int.Parse(STUNdecTB.Text);
            STUNgammadec = double.Parse(gammadecTB.Text);
        }

        private void OK_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void adaptiveCB_CheckedChanged(object sender, EventArgs e)
        {
            if (adaptiveCB.Checked)
            {
                
                Tempiterlabel.Visible = true;
                label1.Text = "Average STUN value";
                label3.Text = "Running Average Iterations";
                TempIterTB.Visible = true;
                STUNdecTB.Visible = true;
                label5.Visible = true;
                label6.Visible = true;
                gammadecTB.Visible = true;
               
               if(double.Parse(tempTB.Text) > 1)
                    tempTB.Text = ".03";
            }
            else
            {
                label1.Text = "Initial temperature";
                label3.Text = "Plateau iterations";
                Tempiterlabel.Visible = false;
                TempIterTB.Visible = false;
                STUNdecTB.Visible = false;
                label5.Visible = false;
                label6.Visible = false;
                gammadecTB.Visible = false;
                tempTB.Text = "10";
                slopeTB.Text = "0.95";
            }
        }
    }
}