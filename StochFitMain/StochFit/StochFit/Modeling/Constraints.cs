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
using System.Windows.Forms;


#pragma warning disable 1591

namespace StochasticModeling.Modeling
{
    /// <summary>
    /// This form allows one to enter constraints for the model dependent fitting routines
    /// </summary>
    public partial class Constraints : Form
    {
        #region Variables

        private bool IsInitialized = false;
        int m_iboxcount;

        /// <summary>
        /// Minimum allowed value of the roughness
        /// </summary>
        public double SubRoughMin;
        /// <summary>
        /// Maximum allowed value of the roughness
        /// </summary>
        public double SubRoughMax;
        /// <summary>
        /// Maximum allowed value of the normalization constant
        /// </summary>
        public double NormMax;
        /// <summary>
        /// Minimum allowed value of the normalization constant
        /// </summary>
        public double NormMin;

        List<TextBox> TBRhoHighArray;
        List<TextBox> TBRhoLowArray;
        List<TextBox> TBLengthHighArray;
        List<TextBox> TBLengthLowArray;
        List<TextBox> TBSigmaHighArray;
        List<TextBox> TBSigmaLowArray;

        /// <summary>
        /// Maximum allowed value of the electron density of a box
        /// </summary>
        public List<double> RhoHighArray;
        /// <summary>
        /// Minimum allowed value of the electron density of a box
        /// </summary>
        public List<double> RhoLowArray;
        /// <summary>
        /// Maximum allowed value of the thickness of a box
        /// </summary>
        public List<double> ThickHighArray;
        /// <summary>
        /// Minimum allowed value of the thickness of a box
        /// </summary>
        public List<double> ThickLowArray;
        /// <summary>
        /// Maximum allowed value of the rougness of an interface
        /// </summary>
        public List<double> SigmaHighArray;
        /// <summary>
        /// Minimum allowed value of the roughness of an interface
        /// </summary>
        public List<double> SigmaLowArray;

      
        #endregion

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="boxcount">Number of boxes in the fit</param>
        /// 
        public Constraints(int boxcount)
        {
            InitializeComponent();
                       
            m_iboxcount = boxcount;

            UpdateControlArrays();

            RhoHighArray = new List<double>(new double[boxcount]);
            RhoLowArray = new List<double>(new double[boxcount]);
            ThickHighArray = new List<double>(new double[boxcount]);
            ThickLowArray = new List<double>(new double[boxcount]);
            SigmaHighArray = new List<double>(new double[boxcount]);
            SigmaLowArray = new List<double>(new double[boxcount]);

            SubRoughMax = 10000;
            SubRoughMin = -10000;
            NormMax = 10000;
            NormMin = -10000;

            for (int i = 0; i < boxcount; i++)
            {
                RhoHighArray[i] = 10000;
                RhoLowArray[i] = -10000;
                ThickHighArray[i] = 10000;
                ThickLowArray[i] = -10000;
                SigmaHighArray[i] = 10000;
                SigmaLowArray[i] = -10000;
            }
        }

        private void UpdateControlArrays()
        {
            TBRhoHighArray = new List<TextBox>(new TextBox[6]);
            TBRhoLowArray = new List<TextBox>(new TextBox[6]);
            TBLengthHighArray = new List<TextBox>(new TextBox[6]);
            TBLengthLowArray = new List<TextBox>(new TextBox[6]);
            TBSigmaHighArray = new List<TextBox>(new TextBox[6]);
            TBSigmaLowArray = new List<TextBox>(new TextBox[6]);

            TBRhoHighArray[0] = RhoMaxTB1;
            TBRhoHighArray[1] = RhoMaxTB2;
            TBRhoHighArray[2] = RhoMaxTB3;
            TBRhoHighArray[3] = RhoMaxTB4;
            TBRhoHighArray[4] = RhoMaxTB5;
            TBRhoHighArray[5] = RhoMaxTB6;

            TBRhoLowArray[0] = RhoMinTB1;
            TBRhoLowArray[1] = RhoMinTB2;
            TBRhoLowArray[2] = RhoMinTB3;
            TBRhoLowArray[3] = RhoMinTB4;
            TBRhoLowArray[4] = RhoMinTB5;
            TBRhoLowArray[5] = RhoMinTB6;

            TBLengthHighArray[0] = LLMaxTB1;
            TBLengthHighArray[1] = LLMaxTB2;
            TBLengthHighArray[2] = LLMaxTB3;
            TBLengthHighArray[3] = LLMaxTB4;
            TBLengthHighArray[4] = LLMaxTB5;
            TBLengthHighArray[5] = LLMaxTB6;

            TBLengthLowArray[0] = LLMinTB1;
            TBLengthLowArray[1] = LLMinTB2;
            TBLengthLowArray[2] = LLMinTB3;
            TBLengthLowArray[3] = LLMinTB4;
            TBLengthLowArray[4] = LLMinTB5;
            TBLengthLowArray[5] = LLMinTB6;

            TBSigmaHighArray[0] = SigMaxTB1;
            TBSigmaHighArray[1] = SigMaxTB2;
            TBSigmaHighArray[2] = SigMaxTB3;
            TBSigmaHighArray[3] = SigMaxTB4;
            TBSigmaHighArray[4] = SigMaxTB5;
            TBSigmaHighArray[5] = SigMaxTB6;

            TBSigmaLowArray[0] = SigMinTB1;
            TBSigmaLowArray[1] = SigMinTB2;
            TBSigmaLowArray[2] = SigMinTB3;
            TBSigmaLowArray[3] = SigMinTB4;
            TBSigmaLowArray[4] = SigMinTB5;
            TBSigmaLowArray[5] = SigMinTB6;
        }

        private void OK_Click(object sender, EventArgs e)
        {
            //Check if any of our fields are initialized
            int checkforinit = 0;

            if (SubRoughMaxTB.IsEmpty() || SubRoughMinTB.IsEmpty() || NormMaxTB.IsEmpty() || NormMinTB.IsEmpty())
                checkforinit++;

            Action<TextBox> f = p => { if (p.IsEmpty())checkforinit++; };
            TBRhoHighArray.ForEach(f);
            TBRhoLowArray.ForEach(f);
            TBSigmaHighArray.ForEach(f);
            TBSigmaLowArray.ForEach(f);
            TBLengthHighArray.ForEach(f);
            TBLengthLowArray.ForEach(f);

            if (checkforinit == 0)
                IsInitialized = false;
            else
            {
                IsInitialized = true;

                if (SubRoughMinTB.IsEmpty())
                    SubRoughMin = SubRoughMinTB.ToDouble();
                if (SubRoughMaxTB.IsEmpty())
                    SubRoughMax = SubRoughMaxTB.ToDouble();

                if (NormMinTB.IsEmpty())
                    NormMin = NormMinTB.ToDouble();
                if (NormMaxTB.IsEmpty())
                    NormMax = NormMaxTB.ToDouble();

                //Update parameters
                for (int i = 0; i < m_iboxcount; i++)
                {
                    if (TBRhoHighArray[i].IsEmpty())
                        RhoHighArray[i] = TBRhoHighArray[i].ToDouble();
                    if (TBRhoLowArray[i].IsEmpty())
                        RhoLowArray[i] = TBRhoLowArray[i].ToDouble();
                    if (TBSigmaHighArray[i].IsEmpty())
                        SigmaHighArray[i] = TBSigmaHighArray[i].ToDouble();
                    if (TBSigmaLowArray[i].IsEmpty())
                        SigmaLowArray[i] = TBSigmaLowArray[i].ToDouble();
                    if (TBLengthHighArray[i].IsEmpty())
                        ThickHighArray[i] = TBLengthHighArray[i].ToDouble();
                    if (TBLengthLowArray[i].IsEmpty())
                        ThickLowArray[i] = TBLengthLowArray[i].ToDouble();
                }
            }
          
            this.Close();
        }

        private void Cancel_Click(object sender, EventArgs e)
        {
            if (IsInitialized)
            {
                if (MessageBox.Show("Do you want to discard your current constraints?", " ", MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    IsInitialized = false;
                    ClearBoxes();
                }
            }
        }

        public void ShowDialog(bool UseSLD)
        {
            if (UseSLD) Rholabel.Text = "SLD";
            
            ShowDialog();
        }

        private void ClearBoxes()
        {
            SubRoughMinTB.Text = SubRoughMaxTB.Text = string.Empty;

            for (int i = 0; i < m_iboxcount; i++)
            {
               TBLengthLowArray[i].Text = TBLengthHighArray[i].Text = TBSigmaLowArray[i].Text = TBSigmaHighArray[i].Text = TBRhoLowArray[i].Text = TBRhoHighArray[i].Text = string.Empty;
            }
        }

        /// <summary>
        /// Determine whether the constraints have been initialized
        /// </summary>
        public bool Initialized
        {
            get { return IsInitialized; }
        }


        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to a double or false if not</param>
        protected void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
                base.OnValidating(e);
                if(((TextBox)sender).IsEmpty())
                    Double.Parse(((TextBox)sender).Text);
            }
            catch
            {
                MessageBox.Show("Error in input - A real number was expected");
                e.Cancel = true;
            }
        }
    }
}