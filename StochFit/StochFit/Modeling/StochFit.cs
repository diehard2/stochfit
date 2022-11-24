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
using System.Drawing;
using System.Windows.Forms;
using ZedGraph;
using System.IO;
using System.Text.RegularExpressions;
using System.Threading;
using System.Reflection;
using System.Timers;
using System.Diagnostics;
using StochasticModeling.Modeling;
using System.Globalization;
using StochasticModeling.Settings;


#pragma warning disable 1591

namespace StochasticModeling
{
    /// <summary>
    /// Main GUI for the StochFit program
    /// </summary>
    public partial class Stochfit:Form
    {
        #region Variables

        string origreflfilename = string.Empty;
        double m_droughness = 0;
        string modelreflname;
        string rhomodelname;
        string settingsfile;
        Graphing reflgraphobject;
        Graphing rhographobject;
        DateTime previtertime;
        TimeSpan span;
        System.Timers.Timer myTimer;
        double[] Z;
        double[] Rho;
        double[] Q;
        double[] Refl;
        Object lockobj;
        ModelSettings InfoStruct;
        int colorswitch = 0;
        double m_dAnnealtemp = 10;
        int m_iAnnealplat = 4000;
        double m_dAnnealslope = 0.95;
        double m_dSTUNGamma = 0.05;
        int m_iSTUNfunc = 0;
        bool m_bSTUNAdaptive = false;
        int m_iSTUNtempiter = 100;
        int m_iSTUNdeciter = 200000;
        double m_dSTUNgammadec = 0.85;
        bool m_bmodelreset = false;
        bool m_bloadfromcommandline = false;
        int previnstanceiter = 0;
        /// <summary>
        /// The culture is set to US for the purposes of inputting data to the numerical routines
        /// </summary>
        protected CultureInfo m_CI = new CultureInfo("en-US");

        #endregion

        #region Constructor and Form Setup
        /// <summary>
        /// Default constructor
        /// </summary>
        public Stochfit(string[] args)
        {

            InitializeComponent();

            //Set properties that persist
            Properties.Settings.Default.UseSLDSingleSession = UseSLDToolStripMenuItem.Checked = Properties.Settings.Default.UseSLD;
            forceRQ4GraphingToolStripMenuItem.Checked = Properties.Settings.Default.ForceRQ4;
            SaveReminderCTB.Checked = Properties.Settings.Default.DisableSavePrompt;

            InfoStruct = new ModelSettings();
            //Object for thread synchronization
            lockobj = new Object();
            //Class instances that make controlling the graph easier
            reflgraphobject = new Graphing(string.Empty);
            rhographobject = new Graphing(string.Empty);
            modelreflname = "Model Independent Reflectivity";
            rhomodelname = "rhomodel";

            FileNameTB.ReadOnly = true;
            //Set default values here for internationalization reasons
            Rholipid.Text = (9.38).ToString();
            SubSLDTB.Text = (9.38).ToString();
            SurfAbs.Text = ((double)1.0e-14).ToString();
            SubAbs.Text = ((double)2.0e-8).ToString();
            layerlength.Text = ((double)25.0).ToString();
            SupSLDTB.Text = ((double)0).ToString();
            wavelength.Text = (1.24).ToString();
            SupAbsTB.Text = ((double)0).ToString();
            QErrTB.Text = ((double)0).ToString();
            ParamTempTB.Text = (0.03).ToString();


            ImpNormCB_CheckedChanged(null, null);
            UseAbsCB_CheckedChanged(null, null);

            //Setup the graphs
            reflgraphobject.CreateGraph(ReflGraph, "Model Independent Reflectivity Fit", "Q", "Intensity", AxisType.Log);
            rhographobject.CreateGraph(RhoGraph, "Model Independent Electron Density Fit", "Z",
                "Normalized Electron Density", AxisType.Linear);


            if (args.Length > 0)
            {
                m_bloadfromcommandline = true;
                origreflfilename = args[0];
                LoadDataFile(args[0]);
                IterationsTB.Text = args[1];
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
          

            reflgraphobject.SetAllFonts("Garamond", 20, 18);
            rhographobject.SetGraphType(false, false);
            rhographobject.SetAllFonts("Garamond", 20, 18);
            rhographobject.LegendState = false;
            progressBar1.Style = ProgressBarStyle.Continuous;

            Priority.SelectedIndex = 2;
            objectiveCB.SelectedIndex = 0;
            AlgorithmCB.SelectedIndex = 0;


            DisableInterface(true);
            ShowSATBs(false);

            //Setup the callback if the graph updates the bounds
            reflgraphobject.ChangedBounds += new Graphing.ChangedEventHandler(PointChanged);

            if (m_bloadfromcommandline)
            {
                Startbutton_Click(null, null);
            }
        }

        public void DisableInterface(bool onoff)
        {
            ParametersBox.Enabled = MiscParametersBox.Enabled = FittingParamBox.Enabled = Startbutton.Enabled =
                Rhomodel.Enabled = OptionsMenuItem.Enabled = Cancelbutton.Enabled = !onoff;
            
        }

        private void LoadDataFile(string origreflfilename)
        {
            try
            {
            
                    if (!ReflData.Instance.SetReflData(origreflfilename, !errorsAreInVarianceToolStripMenuItem.Checked))
                    {
                        throw new Exception("Could not load file");
                    }

                    FileNameTB.Text = origreflfilename;
                    settingsfile = origreflfilename + "settings.xml";
                    reflgraphobject.SetAllFonts("Garamond", 20, 18);
                    reflgraphobject.SubSLD = SubSLDTB.ToDouble();
                    reflgraphobject.SupSLD = SupSLDTB.ToDouble();
                    reflgraphobject.GetHighQOffset = ReflData.Instance.GetNumberDataPoints;
                    reflgraphobject.GetLowQOffset = 0;
                    reflgraphobject.SetGraphType(forceRQ4GraphingToolStripMenuItem.Checked, fresnelcb.Checked);
                    //Load the modeled files if they are available
                    if (File.Exists(settingsfile) && !m_bloadfromcommandline)
                    {
                        if (MessageBox.Show("Do you want to load the previous run?", "Resume?", MessageBoxButtons.YesNo) == DialogResult.Yes)
                        {
                            rhographobject.Clear();
                            reflgraphobject.Clear();

                            if (LoadSettings() == false)
                                return;

                                           
                            string tempfile = ReflData.Instance.GetWorkingDirectory + "\\rho.dat";

                            if (File.Exists(tempfile))
                            {
                                LoadZ(tempfile);
                                rhographobject.SubSLD = double.Parse(SubSLDTB.Text);
                                rhographobject.UseSLD = UseSLDToolStripMenuItem.Checked;

                                if (UseSLDToolStripMenuItem.Checked == true)
                                {
                                    rhographobject.SetAxisTitles("Z", "SLD");
                                    rhographobject.Title = "Model Independent SLD Fit";
                                }
                                else
                                {
                                    rhographobject.SetAxisTitles("Z", "Normalized Electron Density");
                                    rhographobject.Title = "Model Independent Electron Density Fit";
                                }
                                    rhographobject.LoadFiletoGraph(tempfile.ToString(), rhomodelname, "Model Independent Electron Density Fit", Color.Tomato, SymbolType.None, 0, true);
                            }

                            tempfile = ReflData.Instance.GetWorkingDirectory + "\\rf.dat";

                            if (File.Exists(tempfile))
                            {
                                reflgraphobject.SubSLD = double.Parse(SubSLDTB.Text);
                                reflgraphobject.SupSLD = double.Parse(SupSLDTB.Text);
                                reflgraphobject.SetGraphType(forceRQ4GraphingToolStripMenuItem.Checked, fresnelcb.Checked);
                                reflgraphobject.GetLowQOffset = int.Parse(critedgeoffTB.Text);
                                reflgraphobject.GetHighQOffset = ReflData.Instance.GetNumberDataPoints - int.Parse(HQoffsetTB.Text);
                                //Load the data file to the graph
                                reflgraphobject.LoadDatawithErrorstoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
                                reflgraphobject.LoadFiletoGraph(tempfile.ToString(), modelreflname, "Model Independent Reflectivity", Color.Tomato, SymbolType.Square, 2, true);
                            }

                            GraphCollection.Instance.MainReflGraph = reflgraphobject;
                            GraphCollection.Instance.MainRhoGraph = rhographobject;

                            UpdateReportParameters();
                        }
                        else
                        {
                            //Load the data file to the graph
                            reflgraphobject.LoadDatawithErrorstoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);

                            FileInfo info = new FileInfo(settingsfile);

                            if (File.Exists(info.DirectoryName + "\\pop.dat"))
                            {
                                //Create a backup folder if we don't have one
                                if (!System.IO.Directory.Exists(info.DirectoryName + "\\FitBackUp"))
                                    System.IO.Directory.CreateDirectory(info.DirectoryName + "\\FitBackUp");

                                int index = 0;
                                for (; ; )
                                {
                                    if (!System.IO.Directory.Exists(info.DirectoryName + "\\FitBackUp\\Fit" + index.ToString()))
                                    {
                                        System.IO.Directory.CreateDirectory(info.DirectoryName + "\\FitBackUp\\Fit" + index.ToString());
                                        break;
                                    }
                                    index++;
                                }
                                string fileloc = info.DirectoryName + "\\FitBackUp\\Fit" + index.ToString();


                                File.Move(info.DirectoryName + "\\pop.dat", fileloc + "\\pop.dat");
                                File.Move(settingsfile, fileloc + "\\" + info.Name);

                                if (File.Exists(info.DirectoryName + "\\rf.dat"))
                                    File.Move(info.DirectoryName + "\\rf.dat", fileloc + "\\rf.dat");

                                if (File.Exists(info.DirectoryName + "\\rho.dat"))
                                    File.Move(info.DirectoryName + "\\rho.dat", fileloc + "\\rho.dat");

                                if (File.Exists(info.DirectoryName + "\\reflfile.dat"))
                                    File.Move(info.DirectoryName + "\\reflfile.dat", fileloc + "\\reflfile.dat");

                                if (File.Exists(info.DirectoryName + "\\reflrhofit.dat"))
                                    File.Move(info.DirectoryName + "\\reflrhofit.dat", fileloc + "\\reflrhofit.dat");
                            }
                        }
                    }
                    else
                    {
                        //Load the data file to the graph
                        reflgraphobject.LoadDatawithErrorstoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
                    }

                    DisableInterface(false);
                    Cancelbutton.Enabled = false;
                    reflgraphobject.SetBounds();

                    if (UseAbsCB.Checked == false)
                        SubAbs.Enabled =  SupAbsTB.Enabled = SurfAbs.Enabled = false;
               
            }
            catch(Exception ex)
            {
                origreflfilename = string.Empty;
                FileNameTB.Clear();
                DisableInterface(true);
                MessageBox.Show(ex.Message);
            }


        }

        private void LoadFile_Click(object sender, EventArgs e)
        {
                reflgraphobject.Clear();
                rhographobject.Clear();

                openFileDialog1.Title = "Select a File";
                openFileDialog1.Filter = "DataFiles|*.txt";
                openFileDialog1.FileName = string.Empty;

                if (openFileDialog1.ShowDialog() != DialogResult.Cancel)
                    origreflfilename = openFileDialog1.FileName;
                else
                    origreflfilename = string.Empty;

                if(origreflfilename != string.Empty)
                    LoadDataFile(origreflfilename);
        }

        private void LoadZ(string filename)
        {
            string dataline;

            using (StreamReader sr = new StreamReader(filename))
            {
               
                List<double> tempZ = new List<double>(500);
                List<double> tempRho = new List<double>(500);

                while ((dataline = sr.ReadLine()) != null)
                {
                    string[] temp = new Regex(@"\s").Split(dataline);
                    tempZ.Add(double.Parse(temp[0]));
                    tempRho.Add(double.Parse(temp[1]));
                }

                Z = tempZ.ToArray();
                Rho = tempRho.ToArray();
            }
        }

        #endregion

        #region Save and Load Settings

        private void GetReflSettings(ref ModelSettings settings)
        {
            FileInfo info = new FileInfo(origreflfilename);
            ReflData ReflArr = ReflData.Instance;
            int newdatapoints = ReflData.Instance.GetNumberDataPoints;

            settings.Directory = info.DirectoryName;
            settings.SetArrays(ReflArr.GetQData, ReflArr.GetReflData, ReflArr.GetRErrors, ReflArr.GetQErrors);

            FillSettingsStruct(ref settings);
        }

        private void FillSettingsStruct(ref ModelSettings settings)
        {
            settings.Title = TitleTB.Text;
            settings.CritEdgeOffset = int.Parse(critedgeoffTB.Text);
            settings.HighQOffset = int.Parse(HQoffsetTB.Text);
            settings.SurflayerSLD = Rholipid.ToDouble();
            settings.SubSLD = SubSLDTB.ToDouble();
            settings.SupSLD = SupSLDTB.ToDouble();
            settings.Boxes = Boxlayers.ToInt();
            settings.Surflayerlength = layerlength.ToDouble();
            settings.SurflayerAbs = SurfAbs.ToDouble();
            settings.Wavelength = wavelength.ToDouble();
            settings.SubAbs = SubAbs.ToDouble();
            settings.SupAbs = SupAbsTB.ToDouble();
            settings.UseAbs = UseAbsCB.Checked;
            settings.SupOffset = Double.Parse(SupoffsetTB.Text);
            settings.Percerror = Double.Parse(QErrTB.Text);
            settings.Forcenorm = ForceNormCB.Checked;
            settings.Forcesig = Double.Parse(SigTSTB.Text);

            if (int.Parse(SigTSTB.Text) == 0)
                settings.SigmaSearchPerc = int.Parse(SigmaSearchTB.Text);
            else
                settings.SigmaSearchPerc = 0;
              
            settings.Debug = debugToolStripMenuItem.Checked;
            settings.ForceXR = forceXRToolStripMenuItem1.Checked;
            settings.Resolution = int.Parse(ResolutionTB.Text);
            settings.Totallength = Double.Parse(TotlengthTB.Text);
            settings.ImpNorm = ImpNormCB.Checked;
            settings.FitFunc = objectiveCB.SelectedIndex;
            settings.ParamTemp = Double.Parse(ParamTempTB.Text);
            
            if (UseAbsCB.Checked)
                settings.AbsorptionSearchPerc = int.Parse(AbsorptionSearchTB.Text);
            else
                settings.AbsorptionSearchPerc = 0;

            if (ImpNormCB.Checked)
                settings.NormalizationSearchPerc = int.Parse(NormSearchTB.Text);
            else
                settings.NormalizationSearchPerc = 0;

            settings.Algorithm = AlgorithmCB.SelectedIndex;
            settings.AnnealInitTemp = m_dAnnealtemp;
            settings.AnnealTempPlat = m_iAnnealplat;
            settings.AnnealSlope = m_dAnnealslope;
            settings.AnnealGamma = m_dSTUNGamma;
            settings.STUNfunc = m_iSTUNfunc;
            settings.STUNAdaptive = m_bSTUNAdaptive;
            settings.STUNtempiter = m_iSTUNtempiter;
            settings.STUNdeciter = m_iSTUNdeciter;
            settings.STUNgammadec = m_dSTUNgammadec;
        }

        private void WriteSettings()
        {
            MySettings PrevSettings = new MySettings();

            FillSettingsStruct(ref PrevSettings.Settings);
            PrevSettings.Settings.Iterations = int.Parse(IterationsTB.Text);
            PrevSettings.Settings.Version = Assembly.GetExecutingAssembly().GetName().Version.ToString();
            PrevSettings.Settings.IsNeutron = UseSLDToolStripMenuItem.Checked;
            PrevSettings.WriteSettings(settingsfile);
        }

        private bool LoadSettings()
        {
            MySettings PrevSettings = new MySettings();

            if (PrevSettings.PopulateSettings(settingsfile) == true)
            {
                Rholipid.Text = PrevSettings.Settings.SurflayerSLD.ToString();
                layerlength.Text = PrevSettings.Settings.Surflayerlength.ToString();
                SurfAbs.Text = PrevSettings.Settings.SurflayerAbs.ToString();
                AlgorithmCB.SelectedIndex = PrevSettings.Settings.Algorithm;
                objectiveCB.SelectedIndex = PrevSettings.Settings.FitFunc;

                if (AlgorithmCB.SelectedIndex != 0)
                    ShowSATBs(true);
                else
                    ShowSATBs(false);

                SubSLDTB.Text = PrevSettings.Settings.SubSLD.ToString();
                SubAbs.Text = PrevSettings.Settings.SubAbs.ToString();
                SupSLDTB.Text = PrevSettings.Settings.SupSLD.ToString();
                SupAbsTB.Text = PrevSettings.Settings.SupAbs.ToString();
                SupoffsetTB.Text = PrevSettings.Settings.SupOffset.ToString();
                wavelength.Text = PrevSettings.Settings.Wavelength.ToString();
                HQoffsetTB.Text = PrevSettings.Settings.HighQOffset.ToString();
                Boxlayers.Text = PrevSettings.Settings.Boxes.ToString();
                IterationsTB.Text = PrevSettings.Settings.Iterations.ToString();
                if (PrevSettings.Settings.IterationsCompleted > 0)
                    SetProgressBar(PrevSettings.Settings.Iterations, 0, PrevSettings.Settings.IterationsCompleted);

                ChiSquareTB.Text = PrevSettings.Settings.ChiSquare.ToString();
                SigTSTB.Text = PrevSettings.Settings.Forcesig.ToString();
                TitleTB.Text = PrevSettings.Settings.Title;
                ResolutionTB.Text = PrevSettings.Settings.Resolution.ToString();
                TotlengthTB.Text = PrevSettings.Settings.Totallength.ToString();
                critedgeoffTB.Text = PrevSettings.Settings.CritEdgeOffset.ToString();
                SigmaSearchTB.Text = PrevSettings.Settings.SigmaSearchPerc.ToString();
                UseAbsCB.Checked = PrevSettings.Settings.UseAbs;
                ForceNormCB.Checked = PrevSettings.Settings.Forcenorm;
                m_dAnnealtemp = PrevSettings.Settings.AnnealInitTemp;
                m_iAnnealplat = PrevSettings.Settings.AnnealTempPlat;
                m_dAnnealslope = PrevSettings.Settings.AnnealSlope;
                m_dSTUNGamma = PrevSettings.Settings.AnnealGamma;
                ImpNormCB.Checked = PrevSettings.Settings.ImpNorm;
                QErrTB.Text = PrevSettings.Settings.Percerror.ToString();
                debugToolStripMenuItem.Checked = PrevSettings.Settings.Debug;
                forceXRToolStripMenuItem1.Checked = PrevSettings.Settings.ForceXR;
                m_bSTUNAdaptive = PrevSettings.Settings.STUNAdaptive;
                m_iSTUNdeciter = PrevSettings.Settings.STUNdeciter;
                m_iSTUNfunc = PrevSettings.Settings.STUNfunc;
                m_dSTUNgammadec = PrevSettings.Settings.STUNgammadec;
                m_iSTUNtempiter = PrevSettings.Settings.STUNtempiter;
                ParamTempTB.Text = PrevSettings.Settings.ParamTemp.ToString();
                Properties.Settings.Default.UseSLDSingleSession = ReportGenerator.Instance.UseSLD = UseSLDToolStripMenuItem.Checked = UseSLDToolStripMenuItem.Checked = PrevSettings.Settings.IsNeutron;
                
                return true;
            }
            else
            {
                return false;
            }
        }

        #endregion

        #region Start and Cancel Events

        private void Startbutton_Click(object sender, EventArgs e)
        {
            //Initialize Stochfit
            if (FileNameTB.Text == string.Empty)
                return;

            //If we already have a rho loaded, invalidate it
            if (Z != null)
            {
                Z = null;
                Rho = null;

                reflgraphobject.RemoveGraphfromArray(modelreflname);
                rhographobject.RemoveGraphfromArray(rhomodelname);
            }

            //If RhoLipid was entered as zero, make it a bit larger
            if (double.Parse(Rholipid.Text) == 0)
                Rholipid.Text = "0.1";

            SetProgressBar(IterationsTB.ToInt()-1, 0, 0);

            int iterations = IterationsTB.ToInt();
            if (progressBar1.Value > 0 && progressBar1.Value < iterations)
            {
                iterations = IterationsTB.ToInt() - progressBar1.Value;
            }
            reflgraphobject.SubSLD = SubSLDTB.ToDouble();
            
            
            GetReflSettings(ref InfoStruct);

            Calculations.Init(InfoStruct);
            Calculations.Start(iterations);
            Calculations.GenPriority(Priority.SelectedIndex);

            myTimer = new System.Timers.Timer();
            myTimer.Elapsed += new ElapsedEventHandler(OnUpdateTimer);
            myTimer.Interval = 5000;
            myTimer.Start();

            DisableInterface(true);
           
            setModelOptionsToolStripMenuItem.Enabled = setResolutionOptionsToolStripMenuItem.Enabled =
            miscellaneousOptionsToolStripMenuItem.Enabled =  setModelOptionsToolStripMenuItem.DropDown.Enabled = setResolutionOptionsToolStripMenuItem.DropDown.Enabled =
            miscellaneousOptionsToolStripMenuItem.DropDown.Enabled = false;

            MiscParametersBox.Enabled = OptionsMenuItem.Enabled = reflgraphobject.ProgramRunningState = Rhomodel.Enabled = Cancelbutton.Enabled = true;
           
            previtertime = DateTime.Now;

            //WriteSettings to file
            WriteSettings();
            InfoStruct.Dispose();
        }

        void SetProgressBar(int maximum, int minimum, int currentiteration)
        {
            progressBar1.Maximum = maximum;
            progressBar1.Minimum = minimum;

            if (currentiteration > 0)
                progressBar1.Value = currentiteration;
        }

        private void Cancelbutton_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Verify fitting cancellation", "Cancelling", MessageBoxButtons.YesNo) == DialogResult.Yes)
                Canceled();
        }

        private void Canceled()
        {
            myTimer.Stop();
            Calculations.CancelFit();

            DisableInterface(false);
            Cancelbutton.Enabled = false;

            setModelOptionsToolStripMenuItem.Enabled = setResolutionOptionsToolStripMenuItem.Enabled =
              miscellaneousOptionsToolStripMenuItem.Enabled = true;

            setModelOptionsToolStripMenuItem.DropDown.Enabled = setResolutionOptionsToolStripMenuItem.DropDown.Enabled =
                miscellaneousOptionsToolStripMenuItem.DropDown.Enabled = true;

            reflgraphobject.ProgramRunningState = false;

            WriteSettings();

            if (m_bloadfromcommandline)
                this.Close();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (Cancelbutton.Enabled == true)
            {
                if (MessageBox.Show("Verify fitting cancellation", "Cancelling", MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    Calculations.CancelFit();
                    Thread.Sleep(700);
                }
                else
                    e.Cancel = true;
            }
        }
        #endregion

        #region Data Update Routines
        private void OnUpdateTimer(object source, ElapsedEventArgs e)
        {
            //Need to allow time for our dll to initialize itself
            if (Z == null)
            {
                int Zlength, Qlength;

                while (Calculations.WarmedUp() == false)
                {
                    Thread.Sleep(100);
                }

                Calculations.ArraySizes(out Zlength, out Qlength);

                Z = new double[Zlength];
                Rho = new double[Zlength];
                Q = new double[Qlength];
                Refl = new double[Qlength];

                //Set the iterations correctly if we resumed
                if (progressBar1.Value != 0)
                    previnstanceiter = progressBar1.Value;
            }

            //Lock the section so we don't enter twice
            lock (lockobj)
            {
                double lowestenergy, temp, chisquare, fitscore;
                int mode;
                bool isfinished;

                // Get the progress of the iterations
                int iterations = Calculations.GetData(Z, Rho, Q, Refl, out m_droughness, out chisquare, out fitscore, out isfinished);
                Calculations.SAparams(out lowestenergy, out temp, out mode);

                span = DateTime.Now - previtertime;
               
                //Update the front end in a theadsafe manner
                SAlowenergyTB.ThreadSafeSetText(lowestenergy.ToString("#.### E -000"));
                SATempTB.ThreadSafeSetText(temp.ToString("#.### E -000"));
                SAModeTB.ThreadSafeSetText("Annealing");
                itertimetextbox.ThreadSafeSetText(((double)iterations / (double)span.TotalSeconds).ToString("#.#"));
                ChiSquareTB.ThreadSafeSetText(chisquare.ToString("#.####E-000"));
                FitScoreTB.ThreadSafeSetText(fitscore.ToString("#.####E-000"));

                UpdateGraphs();
                UpdateReportParameters();
                progressBar1.ThreadSafeSetValue(iterations);
             
                //End the calculation if we have reached the maximum number of iterations
                if (iterations >= progressBar1.Maximum)
                    Canceled();
            }
        }

        private void UpdateGraphs()
        {
            try
            {

                if (origreflfilename == string.Empty)
                    return;

                Color color;

                if (m_bmodelreset == true)
                {
                    reflgraphobject.SetGraphType(forceRQ4GraphingToolStripMenuItem.Checked, fresnelcb.Checked);

                    if (m_bmodelreset == true || reflgraphobject.DataFileLoaded == false)
                    {
                        reflgraphobject.Clear();
                        reflgraphobject.LoadDatawithErrorstoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5, ReflData.Instance.GetQData, ReflData.Instance.GetReflData);

                        if (rhographobject.HasCurve)
                        {
                            rhographobject.Clear();
                            rhographobject.LoadFiletoGraph(ReflData.Instance.GetWorkingDirectory + "//rho.dat", rhomodelname, "Model Independent Electron Density Fit", Color.Tomato, SymbolType.None, 0, true);
                        }
                    }
                    else
                    {
                        reflgraphobject.ClearCurves();
                    }

                    if (Refl != null)
                        reflgraphobject.LoadfromArray(modelreflname, Q, Refl, Color.Tomato, SymbolType.Square, 2, true, string.Empty);
                    else
                    {
                        string tempfile = ReflData.Instance.GetWorkingDirectory + "\\rf.dat";

                        if (File.Exists(tempfile))
                            reflgraphobject.LoadFiletoGraph(tempfile.ToString(), modelreflname, "Model Independent Reflectivity", Color.Tomato, SymbolType.Square, 2, true);
                    }

                    m_bmodelreset = false;
                }
                else 
                {
                    if (colorswitch % 2 == 0)
                        color = Color.DeepSkyBlue;
                    else
                        color = Color.Tomato;


                    if (Q != null && Z != null)
                    {
                        reflgraphobject.LoadfromArray(modelreflname, Q, Refl, color, SymbolType.Triangle, 2, true, string.Empty);
                        rhographobject.LoadfromArray(rhomodelname, Z, Rho, color, SymbolType.None, 0, true, string.Empty);
                        colorswitch++;
                    }
                }

                //Update the report and internal graphs
                GraphCollection.Instance.MainReflGraph = reflgraphobject;
                GraphCollection.Instance.MainRhoGraph = rhographobject;

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void UpdateReportParameters()
        {
            ReportGenerator g = ReportGenerator.Instance;

            //Record the parameters
            g.ClearMainInformation();

            if (TitleTB.Text.Length > 0)
            {
                g.SetMainInformation = TitleTB.Text;
            }

            g.SetMainInformation = (String.Format("Surface SLD: {0}\n", Rholipid.Text));
            g.SetMainInformation = (String.Format("Surface layer height: {0}\n", layerlength.Text));

            if (UseAbsCB.Checked == true)
                g.SetMainInformation = String.Format("Surface layer absoption: {0}\n", SurfAbs.Text);
            else
                g.SetMainInformation = "The surface layer was treated as transparent\n";

            g.SetMainInformation = (String.Format("Superphase SLD: {0}\n", SupSLDTB.Text));

            if (UseAbsCB.Checked == true)
                g.SetMainInformation = String.Format("Superphase absorption: {0}\n", SupAbsTB.Text);
            else
                g.SetMainInformation = "The superphase layer was treated as transparent\n";

            g.SetMainInformation = (String.Format("Subphase SLD: {0}\n", SubSLDTB.Text));

            if (UseAbsCB.Checked == true)
                g.SetMainInformation = String.Format("Subphase absoption: {0}\n", SubAbs.Text);
            else
                g.SetMainInformation = "The surface layer was treated as transparent\n";

            g.SetMainInformation = "\n";
            g.SetMainInformation = String.Format("Number of layers in the reflectivity: {0}\n", Boxlayers.Text);
            g.SetMainInformation = String.Format("Iterations completed: {0}\n", string.Format("{0}", progressBar1.Value * 80));
            g.SetMainInformation = String.Format("Critical edge offset: {0}\n", critedgeoffTB.Text);
            g.SetMainInformation = String.Format("High Q offset: {0}\n", HQoffsetTB.Text);
            g.SetMainInformation = String.Format("Roughness Search Percentage: {0} %\n", SigmaSearchTB.Text);
            g.SetMainInformation = String.Format("X-ray wavelength: {0}\n", wavelength.Text);
            g.SetMainInformation = String.Format("Percent error in Q: {0}\n", QErrTB.Text);

            if (ImpNormCB.Checked)
                g.SetMainInformation = "The normalization constant was allowed to vary\n";
            if (ForceNormCB.Checked)
                g.SetMainInformation = "Normalization of the curve was forced\n";

            g.SetMainInformation = String.Format("Fit Score for the reflectivity: {0}\n", FitScoreTB.Text);
            g.SetMainInformation = String.Format("Chi Square for the reflectivity: {0}\n", ChiSquareTB.Text);
        }

        #endregion

        #region Launch Other Windows

        private void Rhomodel_Click(object sender, EventArgs e)
        {
            UpdateReportParameters();
            Rhomodeling RhoModel = new Rhomodeling(Z, Rho, m_droughness, SupoffsetTB.Text, SubSLDTB.Text, SupSLDTB.Text);
            RhoModel.Show();
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AboutBox1 about = new AboutBox1();
            about.ShowDialog(this);
        }

        private void OnViewMasterGraph(object sender, EventArgs e)
        {
            MasterPaneForm mp = new MasterPaneForm();
            mp.ShowDialog(this);
        }

        private void OnGenerateReport(object sender, EventArgs e)
        {
            if (origreflfilename != string.Empty)
            {
                ReportGenerator.Instance.DataFileName = origreflfilename;
                ReportGenerator.Instance.GeneratePDFReport();
                try
                {
                    Process.Start(ReportGenerator.Instance.DataFileName);
                }
                catch { }
            }
            else
            {
                MessageBox.Show("Please load a file before generating a report");
            }
        }

        private void tutorialToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                string temp = Application.StartupPath + "\\QuickStart.pdf";
                Process proc = new Process();
                proc.StartInfo = new ProcessStartInfo(temp);
                proc.Start();
            }
            catch
            {
                MessageBox.Show("Error launching pdf. Please make sure Acrobat is installed");
            }
        }

        private void AlgorithmCB_DropDownClosed(object sender, EventArgs e)
        {
            if (AlgorithmCB.SelectedIndex == 1)
            {
                AnnealingParams anneal = new AnnealingParams(m_dAnnealtemp, m_iAnnealplat, m_dAnnealslope);

                anneal.ShowDialog(this);
                anneal.GetParams(out m_dAnnealtemp, out m_iAnnealplat, out m_dAnnealslope);
                ShowSATBs(true);
            }
            else if (AlgorithmCB.SelectedIndex == 2)
            {
                STUNAnnealingParams anneal = new STUNAnnealingParams(m_dAnnealtemp, m_iAnnealplat, m_dAnnealslope, m_dSTUNGamma, m_iSTUNfunc, m_bSTUNAdaptive,
                    m_iSTUNtempiter, m_iSTUNdeciter, m_dSTUNgammadec);

                anneal.ShowDialog(this);
                anneal.GetParams(out m_dAnnealtemp, out m_iAnnealplat, out m_dAnnealslope, out m_dSTUNGamma, out m_iSTUNfunc,
                    out m_bSTUNAdaptive, out m_iSTUNtempiter, out m_iSTUNdeciter, out m_dSTUNgammadec);
                ShowSATBs(true);
            }
            else
            {
                ShowSATBs(false);
            }
        }

        private void sLDCalculatorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SLDCalculator SLD = new SLDCalculator();
            SLD.ShowDialog();
        }

        private void manualToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Process.Start("http://stochfit.sourceforge.net/forum/");
        }
        #endregion

        #region StochFit Miscellaneous Update Routines

        private void Priority_SelectedIndexChanged(object sender, EventArgs e)
        {
            //The highest two priorities were... problematic
            Calculations.GenPriority(Priority.SelectedIndex);
        }

        private void fresnelcb_CheckedChanged(object sender, EventArgs e)
        {
            m_bmodelreset = true;
            UpdateGraphs();
        }

        private void progressBar1_MouseHover(object sender, EventArgs e)
        {
            toolTip1.SetToolTip(progressBar1, "Iteration: " + progressBar1.Value.ToString());
        }

        private void ShowSATBs(bool onoff)
        {
            SATempTB.Visible = SAModelabel.Visible = SATemplabel.Visible = SALow.Visible = SAlowenergyTB.Visible = SAModeTB.Visible = onoff;
        }

        private void SupSLDTB_TextChanged(object sender, EventArgs e)
        {
            reflgraphobject.SupSLD = Double.Parse(SupSLDTB.Text);
            m_bmodelreset = true;
            UpdateGraphs();
        }

        private void rhowater_TextChanged(object sender, EventArgs e)
        {
            reflgraphobject.SubSLD = Double.Parse(SubSLDTB.Text);
            m_bmodelreset = true;
            UpdateGraphs();
        }

        private void UseAbsCB_CheckedChanged(object sender, EventArgs e)
        {
           AbsorptionSearchLI.DropDown.Enabled = SubAbs.Enabled = SupAbsTB.Enabled = 
               SurfAbs.Enabled = UseAbsCB.Checked;
        }

        private void errorsAreInVarianceToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MenuItem_Check(sender, e);
            ReflData.Instance.SetReflData(origreflfilename, !errorsAreInVarianceToolStripMenuItem.Checked);
            m_bmodelreset = true;
            UpdateGraphs();
        }

        private void forceRQ4GraphingToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MenuItem_Check(sender, e);
            m_bmodelreset = true;
            Properties.Settings.Default.ForceRQ4 = forceRQ4GraphingToolStripMenuItem.Checked;
            Properties.Settings.Default.Save();
            UpdateGraphs();
        }

        private void neutronDataToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MenuItem_Check(sender, e);

            rhographobject.UseSLD = UseSLDToolStripMenuItem.Checked;
            rhographobject.SubSLD = double.Parse(SubSLDTB.Text);
            ReportGenerator.Instance.UseSLD = UseSLDToolStripMenuItem.Checked;

            Properties.Settings.Default.UseSLDSingleSession = Properties.Settings.Default.UseSLD = UseSLDToolStripMenuItem.Checked;
            Properties.Settings.Default.Save();

            if (UseSLDToolStripMenuItem.Checked)
            {
                rhographobject.SetAxisTitles("", "SLD");
                rhographobject.Title = "Model Independent SLD Fit";
            }
            else
            {
                rhographobject.SetAxisTitles("", "Normalized Electron Density");
                rhographobject.Title = "Model Independent Electron Density Fit";
            }

            m_bmodelreset = true;
            UpdateGraphs();
        }


        private void debugToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MenuItem_Check(sender, e);
        } 
        #endregion

        #region Offset functions

        private void PointChanged(object sender, EventArgs e)
        {
            critedgeoffTB.Text = reflgraphobject.GetLowQOffset.ToString();
            HQoffsetTB.Text = (ReflData.Instance.GetNumberDataPoints - reflgraphobject.GetHighQOffset).ToString();
        }

        private void critedgeoffTB_Validated(object sender, EventArgs e)
        {
            reflgraphobject.GetLowQOffset = int.Parse(critedgeoffTB.Text);
            reflgraphobject.SetBounds();
        }

        private void HQoffsetTB_Validated(object sender, EventArgs e)
        {
            reflgraphobject.GetHighQOffset = int.Parse(HQoffsetTB.Text);
            reflgraphobject.SetBounds();
        }



        private void critedgeoffTB_TextChanged(object sender, EventArgs e)
        {
            try
            {
                Convert.ToInt32(critedgeoffTB.Text);
            }
            catch
            {
                MessageBox.Show("Error in input - An integer was expected");
                critedgeoffTB.Text = "0";
                return;
            }
            reflgraphobject.GetLowQOffset = int.Parse(critedgeoffTB.Text);
            reflgraphobject.SetBounds();
        }

        private void HQoffsetTB_TextChanged(object sender, EventArgs e)
        {
            try
            {
                Convert.ToInt32(HQoffsetTB.Text);
            }
            catch
            {
                MessageBox.Show("Error in input - An integer was expected");
                HQoffsetTB.Text = "0";
                return;
            }
            reflgraphobject.GetHighQOffset = ReflData.Instance.GetNumberDataPoints - int.Parse(HQoffsetTB.Text);
            reflgraphobject.SetBounds();
        } 
        #endregion

        private void sLDConverterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SLDConverter conv = new SLDConverter();
            conv.ShowDialog(this);
            }

        private void ImpNormCB_CheckedChanged(object sender, EventArgs e)
        {
            NormSearchLI.DropDown.Enabled = ImpNormCB.Checked;
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
        protected void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
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
        protected void MenuItem_Check(object sender, EventArgs e)
         {
             ((ToolStripMenuItem)sender).Checked = !((ToolStripMenuItem)sender).Checked;
         }

        private void DisableSavePrompt(object sender, EventArgs e)
        {
            MenuItem_Check(sender, e);
            Properties.Settings.Default.DisableSavePrompt = SaveReminderCTB.Checked;
            Properties.Settings.Default.Save();
        }
    
       
        
    }

}