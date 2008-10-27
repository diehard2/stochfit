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
using System.Text;
using System.Windows.Forms;
using ZedGraph;
using System.Collections.Generic;

#pragma warning disable 1591

namespace StochasticModeling.Modeling
{


    /// <summary>
    /// Displays a collection of fits
    /// </summary>
    public partial class StochOutputWindow : Form
    {
        int _SelectedIndex = -1;
        bool _StochWindow;
        Graphing ReflGraphing;
        Graphing RhoGraphing;
        List<BoxReflFitBase> ReflectivityList;
        
        /// <summary>
        /// Contains all of the parameters for all of the fits. The format of which is determined in the class
        /// </summary>
        /// <param name="FullParameterArray"></param>
        /// <param name="ParameterArraysize"></param>
        /// <param name="paramsize"></param>
        /// <param name="FullChisquareArray"></param>
        /// <param name="FullCovariance"></param>
        /// <param name="Refl"></param>
        public StochOutputWindow(double[] FullParameterArray, int ParameterArraysize, int paramsize, double[] FullChisquareArray, double[] FullCovariance, BoxReflFitBase Refl)
        {
            InitializeComponent();
            ReflectivityList = new List<BoxReflFitBase>(100);
            _StochWindow = true;
            //Fill the ParameterArray
            double[] param = new double[paramsize];
            double[] covar = new double[paramsize];

            for (int i = 0; i < ParameterArraysize; i++)
            {
                ReflectivityList.Add(Refl.CreateLightWeightClone());
                
                for (int j = 0; j < paramsize; j++)
                {
                    param[j] = FullParameterArray[i * paramsize + j];
                    covar[j] = FullCovariance[i * paramsize + j];
                }

                ReflectivityList[i].UpdatefromParameterArray(param);
                ReflectivityList[i].CovarArray = covar;
                ReflectivityList[i].ChiSquare = FullChisquareArray[i];

            }

            //Fill the list
            for (int i = 0; i < ParameterArraysize; i++)
            {
                ModelLB.Items.Add("Model " + (i+1).ToString() + " - Fit Score = " + FullChisquareArray[i].ToString("#.### E-000"));
            }

            SetUpGraphs(Refl.SubphaseSLD, Refl.SuperphaseSLD);


            ReflectivityList.ForEach(p => p.UpdateProfile());
          
            ModelLB.SelectedIndex = 0;
            ModelLB.Focus();

            //Play a chime when the stochastic methods are complete
            System.Media.SystemSounds.Exclamation.Play();
        }

        
        public StochOutputWindow()
        {
            InitializeComponent();
            ReflectivityList = new List<BoxReflFitBase>(100);
            SetUpGraphs(9.38, 0);
            _StochWindow = false;

        }

        private void SetUpGraphs(double SubSLD, double SupSLD)
        {
            //Setup the graphs
            ReflGraphing = new Graphing(string.Empty);
            ReflGraphing.SetGraphType(Properties.Settings.Default.ForceRQ4, true);
            ReflGraphing.SubSLD = SubSLD;
            ReflGraphing.SupSLD = SupSLD;
            ReflGraphing.CreateGraph(ReflGraph, "Reflectivity", "Q/Qc", "Intensity / Fresnel", AxisType.Log);
            ReflGraphing.LoadDatawithErrorstoGraph("Reflectivity Data", System.Drawing.Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);


            RhoGraphing = new Graphing(string.Empty);
            RhoGraphing.SubSLD = SubSLD;
            RhoGraphing.SupSLD = SupSLD;
            RhoGraphing.UseSLD = Properties.Settings.Default.UseSLDSingleSession;
            RhoGraphing.SetGraphType(false, false);

            if (!Properties.Settings.Default.UseSLDSingleSession)
                RhoGraphing.CreateGraph(RhoGraph, "Electron Density Profile", "Z", "Normalized Electron Density", AxisType.Linear);
            else
                RhoGraphing.CreateGraph(RhoGraph, "SLD Profile", "Z", "SLD", AxisType.Linear);


        }

        public void AddModel(BoxReflFitBase BoxRefl)
        {
            ReflectivityList.Add(BoxRefl.CreateLightWeightClone());
            ModelLB.Items.Add("Model " + (ModelLB.Items.Count + 1).ToString() + " - Fit Score = " + BoxRefl.ChiSquare.ToString("#.### E-000"));
            ReflectivityList[ReflectivityList.Count - 1].UpdateProfile(); ;
        }

        private void OnModelIndexChange(object sender, EventArgs e)
        {
            if (ModelLB.SelectedIndex != -1)
            {
                ParametersTB.Text = ReflectivityList[ModelLB.SelectedIndex].ErrorReport();
                UpdateGraphs(ModelLB.SelectedIndex, _StochWindow);
            }
        }

        private void UpdateGraphs(int index, bool majorupdate)
        {
            RhoGraphing.LoadfromArray("Model Dependent Fit", ReflectivityList[index].Z, ReflectivityList[index].ElectronDensityArray, System.Drawing.Color.Turquoise, SymbolType.None, 0, true, string.Empty);
            RhoGraphing.LoadfromArray("Model Dependent Box Fit", ReflectivityList[index].Z, ReflectivityList[index].BoxElectronDensityArray, System.Drawing.Color.Red, SymbolType.None, 0, false, string.Empty);

            if(majorupdate)
                ReflGraphing.LoadDatawithErrorstoGraph("Reflectivity Data", System.Drawing.Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
           
            ReflGraphing.LoadfromArray("Model Dependent Fit", ReflData.Instance.GetQData, ReflectivityList[index].ReflectivityMap, System.Drawing.Color.Black, SymbolType.XCross, 4, true, string.Empty);
        }

        private void SelModelBT_Click(object sender, EventArgs e)
        {
            _SelectedIndex = ModelLB.SelectedIndex;
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        public void GetParameters(ReflFit Refl)
        {
            if(_SelectedIndex != -1)
                Refl.LoadLightWeightClone(ReflectivityList[_SelectedIndex]);
        }
    }
}