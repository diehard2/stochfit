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

namespace StochasticModeling
{
     public partial class SLDCalculator : StochFormBase
    {
        public SLDCalculator()
        {
            //Thread.CurrentThread.CurrentCulture = new CultureInfo("en-US");
            //Thread.CurrentThread.CurrentUICulture = new CultureInfo("en-US");

            InitializeComponent();

            DensTB.Text = ((double)(0.998)).ToString();
            ApmTB.Text = ((double)(40.2)).ToString();
            ThickTB.Text = ((double)(25.1)).ToString();


            UseElemCB_CheckedChanged(null, null);
            UseDensCB_CheckedChanged(null, null);

            SLDCalc();
        }

        void SLDCalc()
        {
            double MWVol;

            if (UseDensCB.Checked)
                MWVol = CalcMWVolfromDens();
            else
                MWVol = CalcMWVolfromBox();

            if (NeutronCB.Checked)
            {
                SLDTB.Text = NeutronCalc(MWVol).ToString("0.000");
            }
            else
            {
                if (UseElemCB.Checked)
                    SLDTB.Text = XRaySLDcalcEl(MWVol).ToString("0.000");
                else
                    SLDTB.Text = XRaySLDcalcECount(MWVol).ToString("0.000");
            }
        }

        double NeutronCalc(double MWVol)
        {
            try
            {
                return MWVol * (int.Parse(HsTB.Text) * -3.7390 + int.Parse(DsTB.Text) * 6.671 + int.Parse(OsTB.Text) * 5.803 +
                   int.Parse(CsTB.Text) * 6.646 + int.Parse(PsTB.Text) * 5.13 + int.Parse(NsTB.Text) * 9.36) * 10;
            }
            catch
            {
                ErrorBox();
                return -1;
            }
        }
        
        double XRaySLDcalcEl(double MWVol)
        {
            try
            {
              return  MWVol * 0.000028179 * (int.Parse(HsTB.Text) * 1 + int.Parse(DsTB.Text) * 1 + int.Parse(OsTB.Text) * 8 +
                int.Parse(CsTB.Text) * 6 + int.Parse(PsTB.Text) * 15 + int.Parse(NsTB.Text) * 7) * 1000000;
            }
            catch
            {
                ErrorBox();
                return -1;
            }
        }

        double XRaySLDcalcECount(double MWVol)
        {
            try
            {
                return MWVol * 0.000028179 * int.Parse(ElecTB.Text) * 1000000;
            }
            catch
            {
                ErrorBox();
                return -1;
            }
        }

       

         double CalcMW()
         {
             try
             {
                 return (int.Parse(HsTB.Text) * 1 + int.Parse(DsTB.Text) * 2 + int.Parse(OsTB.Text) * 16 +
                        int.Parse(CsTB.Text) * 12 + int.Parse(PsTB.Text) * 30.97 + int.Parse(NsTB.Text) * 14);

             }
             catch
             {
                 ErrorBox();
                 return -1;
             }


         }

         double CalcMWVolfromDens()
         {
             try
             {
                 return Double.Parse(DensTB.Text) * 6.022e23 * 1E-24 / CalcMW();
             }
             catch
             {
                 ErrorBox();
                 return -1;
             }
         }

        double CalcMWVolfromBox()
        {
            try
            {
                return 1 / (Double.Parse(ThickTB.Text) * Double.Parse(ApmTB.Text));
            }
            catch
            {
                ErrorBox();
                return -1;
            }
        }

        private void CalcSLDBB_Click(object sender, EventArgs e)
        {
            SLDCalc();
        }

         void ErrorBox()
         {
             MessageBox.Show("All boxes need to have a number correctly entered");
         }

         private void NeutronCB_CheckedChanged(object sender, EventArgs e)
         {
             UseElemCB.Checked = NeutronCB.Checked;
             UseElemCB.Enabled = !NeutronCB.Checked;
                 
         }

         private void UseElemCB_CheckedChanged(object sender, EventArgs e)
         {
             HsTB.Enabled = OsTB.Enabled = PsTB.Enabled = CsTB.Enabled = DsTB.Enabled = NsTB.Enabled = UseElemCB.Checked;
             ElecTB.Enabled = !UseElemCB.Checked;
         }

         private void UseDensCB_CheckedChanged(object sender, EventArgs e)
         {
             DensTB.Enabled = UseDensCB.Checked;
             ApmTB.Enabled = ThickTB.Enabled = !UseDensCB.Checked;
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