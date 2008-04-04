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
    /// This form allows one to enter constraints for the model dependent fitting routines
    /// </summary>
    public partial class Constraints : StochFormBase
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

        TextBox[] TBRhoHighArray;
        TextBox[] TBRhoLowArray;
        TextBox[] TBLengthHighArray;
        TextBox[] TBLengthLowArray;
        TextBox[] TBSigmaHighArray;
        TextBox[] TBSigmaLowArray;

        /// <summary>
        /// Maximum allowed value of the electron density of a box
        /// </summary>
        public double[] RhoHighArray;
        /// <summary>
        /// Minimum allowed value of the electron density of a box
        /// </summary>
        public double[] RhoLowArray;
        /// <summary>
        /// Maximum allowed value of the thickness of a box
        /// </summary>
        public double[] ThickHighArray;
        /// <summary>
        /// Minimum allowed value of the thickness of a box
        /// </summary>
        public double[] ThickLowArray;
        /// <summary>
        /// Maximum allowed value of the rougness of an interface
        /// </summary>
        public double[] SigmaHighArray;
        /// <summary>
        /// Minimum allowed value of the roughness of an interface
        /// </summary>
        public double[] SigmaLowArray;

        #endregion

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="boxcount">Number of boxes in the fit</param>
        public Constraints(int boxcount)
        {
            InitializeComponent();

            m_iboxcount = boxcount;

            UpdateControlArrays();

            RhoHighArray = new double[boxcount];
            RhoLowArray = new double[boxcount];
            ThickHighArray = new double[boxcount];
            ThickLowArray = new double[boxcount];
            SigmaHighArray = new double[boxcount];
            SigmaLowArray = new double[boxcount];

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
            TBRhoHighArray = new TextBox[m_iboxcount];
            TBRhoLowArray = new TextBox[m_iboxcount];
            TBLengthHighArray = new TextBox[m_iboxcount];
            TBLengthLowArray = new TextBox[m_iboxcount];
            TBSigmaHighArray = new TextBox[m_iboxcount];
            TBSigmaLowArray = new TextBox[m_iboxcount];

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
            IsInitialized = true;

            if (SubRoughMinTB.Text != string.Empty)
                SubRoughMin = double.Parse(SubRoughMinTB.Text);
            if (SubRoughMaxTB.Text != string.Empty)
                SubRoughMax = double.Parse(SubRoughMaxTB.Text);

            if (NormMinTB.Text != string.Empty)
                NormMin = double.Parse(NormMinTB.Text);
            if (NormMaxTB.Text != string.Empty)
                NormMax = double.Parse(NormMaxTB.Text);

            //Update parameters
            for (int i = 0; i < m_iboxcount; i++)
            {
                if (TBRhoHighArray[i].Text != string.Empty)
                    RhoHighArray[i] = double.Parse(TBRhoHighArray[i].Text);
                if (TBRhoLowArray[i].Text != string.Empty)
                    RhoLowArray[i] = double.Parse(TBRhoLowArray[i].Text);
                if (TBSigmaHighArray[i].Text != string.Empty)
                    SigmaHighArray[i] = double.Parse(TBSigmaHighArray[i].Text);
                if (TBSigmaLowArray[i].Text != string.Empty)
                    SigmaLowArray[i] = double.Parse(TBSigmaLowArray[i].Text);
                if (TBLengthHighArray[i].Text != string.Empty)
                    ThickHighArray[i] = double.Parse(TBLengthHighArray[i].Text);
                if (TBLengthLowArray[i].Text != string.Empty)
                    ThickLowArray[i] = double.Parse(TBLengthLowArray[i].Text);
            }

            this.Close();
        }

        private void Cancel_Click(object sender, EventArgs e)
        {
            if (IsInitialized == true)
            {
                if (MessageBox.Show("Do you want to discard your current constraints?", " ", MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    IsInitialized = false;
                    ClearBoxes();
                }
            }
        }

        private void ClearBoxes()
        {
            for (int i = 0; i < m_iboxcount; i++)
            {
                TBRhoHighArray[i].Text = string.Empty;
                TBRhoLowArray[i].Text = string.Empty;
                TBSigmaHighArray[i].Text = string.Empty;
                TBSigmaLowArray[i].Text = string.Empty;
                TBLengthHighArray[i].Text = string.Empty;
                TBLengthLowArray[i].Text = string.Empty;
                SubRoughMaxTB.Text = string.Empty;
                SubRoughMinTB.Text = string.Empty;
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
        protected override void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if(((TextBox)sender).Text != string.Empty)
                base.ValidateNumericalInput(sender, e);
        }
    }
}