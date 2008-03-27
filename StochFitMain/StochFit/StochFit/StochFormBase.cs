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
using System.Text;
using System.Windows.Forms;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using System.Globalization;

namespace StochasticModeling
{
    /// <summary>
    /// Base class for almost all of our forms that require numerical input
    /// </summary>
    public class StochFormBase : Form
    {
        private CultureInfo m_CI = new CultureInfo("en-US");

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to a double or false if not</param>
        protected virtual void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
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
        protected virtual void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
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

        /// <summary>
        /// Sets the "check" state on a menu item
        /// </summary>
        /// <param name="sender">Expects a ToolStripMenuItem</param>
        /// <param name="e"></param>
        protected virtual void MenuItem_Check(object sender, EventArgs e)
         {
             ((ToolStripMenuItem)sender).Checked = !((ToolStripMenuItem)sender).Checked;
         }

   

        //public virtual void FillRealData(string Rhofile,ref double[] Qincrement,ref double[] ReflectivityMap,ref ArrayList RealRefl,ref ArrayList RealReflErrors)
        //{
        //    Get the number of lines in the file
        //    int lines = 0;
        //    string dataline;
        //    using (StreamReader sr = new StreamReader(Rhofile))
        //    {
        //        while (!sr.EndOfStream)
        //        {
        //            dataline = sr.ReadLine().Trim();

        //            if (dataline.Length != 0)
        //                lines++;
        //        }
        //    }

        //    Instantiate our reflecivity arrays
        //    Qincrement = new double[lines];
        //    ReflectivityMap = new double[lines];
        //    ArrayList datastring = new ArrayList();

        //    using (StreamReader sr = new StreamReader(Rhofile))
        //    {

        //        int j = 0;
        //        Regex r = new Regex(@"\s");
        //        while (!sr.EndOfStream)
        //        {
        //            dataline = sr.ReadLine().Trim();

        //            if (dataline.Length != 0)
        //            {

        //                string[] temp = r.Split(dataline);

        //                for (int i = 0; i < temp.Length; i++)
        //                {
        //                    if (temp[i] != "")
        //                        datastring.Add(temp[i]);
        //                }

        //                if (Double.Parse((string)datastring[1], m_CI) > 0.0 && Double.Parse((string)datastring[0], m_CI) > 0.0)
        //                {
        //                    Qincrement[j] = Double.Parse((string)datastring[0], m_CI);
        //                    RealRefl.Add(Double.Parse((string)datastring[1], m_CI));
        //                    RealReflErrors.Add(Double.Parse((string)datastring[2], m_CI));
        //                    datastring.Clear();
        //                    j++;
        //                }
        //            }
        //        }
        //    }
        //}
    }
}
