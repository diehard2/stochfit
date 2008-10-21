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
using ZedGraph;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using System.Threading;
using iTextSharp.text.pdf;
using iTextSharp.text;
using System.Drawing.Imaging;
using StochasticModeling;
using System.Runtime.InteropServices;
using System.Globalization;
using StochasticModeling.Settings;


namespace StochasticModeling
{
    /// <summary>
    /// Fits a model dependent electron density profile to the model independent electron density profile
    /// </summary>
    public partial class Rhomodeling : Form
    {
        #region Variables


        List<TextBox> BoxSigmaArray;
        List<TextBox> BoxLengthArray;
        List<TextBox> BoxRhoArray;

        private RhoFit RhoCalc;
        private Graphing m_gRhoGraphing;

        #endregion

        /// <summary>
        /// Graphs and fits an Electron Density Profile against a user defined Electron Density Profile
        /// </summary>
        /// <param name="Z">Array of thickness data points</param>
        /// <param name="ERho">Electron density of the profile to be fit</param>
        /// <param name="roughness">The overall smoothing parameter found by the model independent fit</param>
        /// <param name="SupOffset">The offset in Z for the first box</param>
        /// <param name="subsld">The subphase SLD</param>
        /// <param name="supsld">The superphase SLD</param>
        public Rhomodeling(double[] Z, double[] ERho, double roughness, string SupOffset, string subsld, string supsld)
           // base(roughness, 
        {
            InitializeComponent();
            bool SetupWithSLD = Properties.Settings.Default.UseSLDSingleSession;
            RhoCalc = new RhoFit(Z, ERho);

            BoxSigmaArray = new List<TextBox>(6);
            BoxLengthArray = new List<TextBox>(6);
            BoxRhoArray = new List<TextBox>(6);

            this.Text = "Profile Modeling";

            //Setup variables
            if (roughness == 0)
            {
                roughness = 3.0;
            }

            RhoCalc.GetSubRoughness = roughness;
            SubRough.Text = roughness.ToString();
            Zoffset.Text = SupOffset.ToString();
          
            SubphaseSLD.Text = subsld;
            SupSLDTB.Text = supsld;

            //Set default array values
            if (SetupWithSLD == false)
                Rho2.Text = Rho1.Text = (1.1).ToString();
            else
                Rho2.Text = Rho1.Text = (1.3*double.Parse(subsld)).ToString();

            Sigma2.Text = Sigma1.Text = (3.25).ToString();
            LLength1.Text = LLength2.Text = (15.1).ToString();
            

            //Initialize the arrays
            MakeArrays();

            if(Z == null)
                 Report_btn.Enabled = UndoFit.Enabled = LevenbergFit.Enabled = false;
            
             //Setup the Graph
             m_gRhoGraphing = new Graphing(string.Empty);
             m_gRhoGraphing.SubSLD = double.Parse(subsld);
             m_gRhoGraphing.IsNeutron = SetupWithSLD;

             if (SetupWithSLD == false)
             {
                 m_gRhoGraphing.CreateGraph(RhoGraph, "Electron Density Profile", "Z", "Normalized Electron Density", AxisType.Linear);
                 RhoLabel.Text = "Normalized Rho";
             }
             else
             {
                 m_gRhoGraphing.CreateGraph(RhoGraph, "SLD Profile", "Z", "SLD", AxisType.Linear);
                 RhoLabel.Text = "SLD";
             }

             m_gRhoGraphing.SetGraphType(false, false);

             if (Z != null)
                 m_gRhoGraphing.LoadfromArray("Model Independent Fit", Z, ERho, System.Drawing.Color.Black, SymbolType.None, 0, true, string.Empty);

             ChangeRoughnessArray();
             GreyFields(); 
             
             //Subscribe to the event that notifies us to update the frontend
             RhoCalc.Update += new BoxReflFitBase.UpdateProfileHandler(UpdateProfile);

             //Create the electron density graph
             MakeED();
        }

        private void UpdateProfile(object sender, EventArgs e)
        {
            Zoffset.Text = RhoCalc.ZOffset.ToString();
            SubRough.Text = RhoCalc.GetSubRoughness.ToString();
            
            //Blank our Rho data from the previous iteration
            for (int i = 0; i < BoxRhoArray.Count; i++)
            {
                BoxRhoArray[i].Text = RhoCalc.RhoArray[i].ToString();
                BoxSigmaArray[i].Text = RhoCalc.SigmaArray[i].ToString();
                BoxLengthArray[i].Text = RhoCalc.LengthArray[i].ToString();
            }

            m_gRhoGraphing.LoadfromArray("Model Dependent Fit", RhoCalc.Z, RhoCalc.ElectronDensityArray, System.Drawing.Color.Turquoise, SymbolType.None, 0, true, string.Empty);
            m_gRhoGraphing.LoadfromArray("Model Dependent Box Fit", RhoCalc.Z, RhoCalc.BoxElectronDensityArray, System.Drawing.Color.Red, SymbolType.None, 0, false, string.Empty);

            //Write graph to the master graph
            GraphCollection.Instance.RhoGraph = m_gRhoGraphing;
        }

        /// <summary>
        /// Make textbox arrays so we can more easily iterate over them
        /// </summary>
        private void MakeArrays()
        {
            BoxSigmaArray.Add(Sigma1);
            BoxSigmaArray.Add(Sigma2);
            BoxSigmaArray.Add(Sigma3);
            BoxSigmaArray.Add(Sigma4);
            BoxSigmaArray.Add(Sigma5);
            BoxSigmaArray.Add(Sigma6);

            BoxLengthArray.Add(LLength1);
            BoxLengthArray.Add(LLength2);
            BoxLengthArray.Add(LLength3);
            BoxLengthArray.Add(LLength4);
            BoxLengthArray.Add(LLength5);
            BoxLengthArray.Add(LLength6);

            BoxRhoArray.Add(Rho1);
            BoxRhoArray.Add(Rho2);
            BoxRhoArray.Add(Rho3);
            BoxRhoArray.Add(Rho4);
            BoxRhoArray.Add(Rho5);
            BoxRhoArray.Add(Rho6);
        }

        private void ChangeRoughnessArray()
        {
            if (Holdsigma.Checked)
            {
                SubRough.Text = RhoCalc.GetSubRoughness.ToString();
                BoxSigmaArray.ForEach(p => p.Text = RhoCalc.GetSubRoughness.ToString());
            }
        }


        private void MakeED()
        {
            //Fill Rho array
            try
            {
                RhoCalc.IsOneSigma = Holdsigma.Checked;
                RhoCalc.ZOffset = Zoffset.ToDouble();
                RhoCalc.GetSubRoughness = SubRough.ToDouble();
                RhoCalc.BoxCount = BoxCount.ToInt();
                RhoCalc.SubphaseSLD = SubphaseSLD.ToDouble();
                RhoCalc.SuperphaseSLD = SupSLDTB.ToDouble();

                //Blank our Rho data from the previous iteration
                RhoCalc.RhoArray.Clear();
                RhoCalc.LengthArray.Clear();
                RhoCalc.SigmaArray.Clear();

                BoxRhoArray.ForEach(p => RhoCalc.RhoArray.Add(p.ToDouble()));
                BoxLengthArray.ForEach(p => RhoCalc.LengthArray.Add(p.ToDouble()));
                BoxSigmaArray.ForEach(p => RhoCalc.SigmaArray.Add(p.ToDouble()));

                if (Holdsigma.Checked)
                    ChangeRoughnessArray();

                RhoCalc.UpdateProfile();
            }
            catch (Exception ex)
            {
               MessageBox.Show(ex.Message.ToString());
            }
        }

    

     
        private void Field_Validated (object sender, EventArgs e)
        {
            GreyFields();
            MakeED();
        }

       

        private void LevenbergFit_Click(object sender, EventArgs e)
        {
            RhoCalc.DataFit();
        }

        

        private void UndoFit_Click(object sender, EventArgs e)
        {
            RhoCalc.UndoFit();
        }

        //Show errors associated with the fit
        private void Report_btn_Click(object sender, EventArgs e)
        {
            LevmarOutput lo = new LevmarOutput();
            lo.DisplayOutput(RhoCalc.ErrorReport());
            lo.ShowDialog();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            ConstrainedReflmodeling Refl = new ConstrainedReflmodeling(RhoCalc);
            Refl.ShowDialog(this);
        }

        private void OnFormClosing(object sender, FormClosingEventArgs e)
        {
            if (m_gRhoGraphing != null && Report_btn.Enabled == true)
                GraphCollection.Instance.RhoGraph = m_gRhoGraphing;
        }

        private void GreyFields()
        {

            for (int i = 0; i < 6; i++)
            {
                if (i < BoxCount.ToInt())
                {
                    BoxLengthArray[i].Enabled = true;
                    BoxRhoArray[i].Enabled = true;
                    BoxSigmaArray[i].Enabled = !Holdsigma.Checked;
                }
                else
                {
                    BoxLengthArray[i].Enabled = false;
                    BoxRhoArray[i].Enabled = false;
                    BoxSigmaArray[i].Enabled = false;
                }
            }
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