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
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using ZedGraph;
using System.IO;
using System.Text.RegularExpressions;
using System.Runtime.InteropServices;
using System.Threading;
using System.Collections;
using StochasticModeling.Properties;
using StochasticModeling;
using System.Reflection;
using System.Timers;
using System.Diagnostics;
using StochasticModeling.Modeling;
using System.Globalization;

namespace StochasticModeling
{
    public partial class Stochfit : StochFormBase
    {
        [DllImport("stochfitdll.dll", EntryPoint = "Init", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            private static extern int Init(string directory, double[] Q, double[] Refl, double[] ReflError, double[] QErr, int Qpoints, double rholipid, double rhoh2o, double supSLD, int parratlayers,
            double layerlength, double slABS, double XRlambda, double SubAbs, double SupAbs, bool UseAbs, double leftoffset, double Qerr, bool forcenorm, 
            double forcesigma, bool debug, bool XRonly,double resolution,double totallength, bool impnorm, int objfunc);
        [DllImport("stochfitdll.dll", EntryPoint = "GenPriority", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            private static extern int GenPriority(int priority);
        [DllImport("stochfitdll.dll", EntryPoint = "Start", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            private static extern int Start(int iterations);
        [DllImport("stochfitdll.dll", EntryPoint = "Cancel", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            private static extern int Cancel();
        [DllImport("stochfitdll.dll", EntryPoint = "GetData", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            private static extern int GetData(double[] ZRange, double[] Rho, double[] QRange, double[] Refl, out double roughness, out double chisquare, out double goodnessoffit, out bool isfinished);
        [DllImport("stochfitdll.dll", EntryPoint = "SetSAParameters", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            private static extern void SetSAParameters(int sigmasearch, int algorithm, double inittemp, int platiter, double slope, double gamma, int STUNfunc, bool adaptive, int tempiter, int deciter, double gammadec);
        [DllImport("stochfitdll.dll", EntryPoint = "ArraySizes", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            private static extern void ArraySizes(out int RhoSize, out int Reflsize);
        [DllImport("stochfitdll.dll", EntryPoint = "WarmedUp", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            private static extern bool WarmedUp();
        [DllImport("stochfitdll.dll", EntryPoint = "SAparams", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
            private static extern void SAparams(out double lowestenergy, out double temp, out int mode);

        string origreflfilename;
        double m_droughness = 0;
        double m_dChiSquare = 0;
        double m_dGoodnessOfFit = 0;
        string modelreflname;
        string rhomodelname;
        string settingsfile;
        Graphing reflgraphobject;
        Graphing rhographobject;
        bool divbyfresnel = false;
        DateTime previtertime;
        TimeSpan span;
        System.Timers.Timer myTimer;
        double[] Z;
        double[] Rho;
        double[] Q;
        double[] Refl;
        Object lockobj;

        bool m_bupdating = false;
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
        bool m_bIsXR = true;
        int previnstanceiter = 0;
       
        /// <summary>
        /// Default constructor
        /// </summary>
        public Stochfit()
        {
            //Object for thread synchronization
            lockobj = new Object();
            //Class instances that make controlling the graph easier
            reflgraphobject = new Graphing(string.Empty);
            rhographobject = new Graphing(string.Empty);
            modelreflname = "Model Independent Reflectivity";
            rhomodelname = "rhomodel";
            
            //Thread.CurrentThread.CurrentCulture = new CultureInfo("en-US");
            //Thread.CurrentThread.CurrentUICulture = new CultureInfo("en-US");
            InitializeComponent();

            //Set default values here for internationalization reasons
            Rholipid.Text = ((double)9.38).ToString();
            rhowater.Text = ((double)9.38).ToString();
            SurfAbs.Text = ((double)1.0e-14).ToString();
            SubAbs.Text = ((double)2.0e-8).ToString();
            layerlength.Text = ((double)25.0).ToString();
            SupSLDTB.Text = ((double)0).ToString();
            wavelength.Text = ((double)1.24).ToString();
            SupAbsTB.Text = ((double)0).ToString();
            QErrTB.Text = ((double)0).ToString();

            //Needed for GUI stuff, not really important
            CheckForIllegalCrossThreadCalls = false;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            //Setup the graphs
            reflgraphobject.CreateGraph(ReflGraph, "Model Independent Reflectivity Fit", "Q", "Intensity", AxisType.Log);
            rhographobject.CreateGraph(RhoGraph, "Model Independent Electron Density Fit", "Z",
                "Normalized Electron Density", AxisType.Linear);

            reflgraphobject.SetAllFonts("Garamond", 20, 18);
            rhographobject.SetAllFonts("Garamond", 20, 18);
            rhographobject.LegendState = false;
            progressBar1.Style = ProgressBarStyle.Continuous;
            progressBar1.ForeColor = Color.AliceBlue;
            
            FileName.ReadOnly = true;
            ParametersBox.Enabled = false;
            Priority.Enabled = false;
            Startbutton.Enabled = false;
            Cancelbutton.Enabled = false;
            Rhomodel.Enabled = false;
            Priority.SelectedIndex = 2;
            m_droughness = 3.25;

            objectiveCB.SelectedIndex = 0;
            AlgorithmCB.SelectedIndex = 0;

            if (AlgorithmCB.SelectedIndex == 0)
            {
                SAModeTB.Visible = false;
                SAlowenergyTB.Visible = false;
                SALow.Visible = false;
                SATemplabel.Visible = false;
                SAModelabel.Visible = false;
                SATempTB.Visible = false;
            }
        }

        private void LoadFile_Click(object sender, EventArgs e)
        {
            try
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

                if (origreflfilename != string.Empty)
                {
                    ReflData.Instance.SetReflData(origreflfilename, !errorsAreInVarianceToolStripMenuItem.Checked);

                    settingsfile = origreflfilename + "settings.xml";
                    reflgraphobject.m_bDBF = fresnelcb.Checked;
                    divbyfresnel = fresnelcb.Checked;
                    reflgraphobject.SetAllFonts("Garamond", 20, 18);
                    reflgraphobject.m_dSLD = Double.Parse(rhowater.Text);
                    reflgraphobject.m_dSupSLD = Double.Parse(SupSLDTB.Text);
                    reflgraphobject.m_dlambda = Double.Parse(wavelength.Text);
                    reflgraphobject.LoadDataFiletoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5);

                    //Load the modeled files if they are available
                    if (File.Exists(settingsfile))
                    {
                        if (MessageBox.Show("Do you want to load the previous run?", "Resume?", MessageBoxButtons.YesNo) == DialogResult.Yes)
                        {
                            LoadSettings();

                            string tempfile = ReflData.Instance.GetWorkingDirectory + "\\rho.dat";

                            if (File.Exists(tempfile))
                            {
                                LoadZ(tempfile);
                                rhographobject.LoadFiletoGraph(tempfile.ToString(), rhomodelname, "Model Independent Electron Density Fit", Color.Tomato, SymbolType.None, 0, true);
                                rhographobject.SetAxisTitles("Z", string.Empty);
                            }

                            tempfile = ReflData.Instance.GetWorkingDirectory + "\\rf.dat";

                            if (File.Exists(tempfile))
                            {
                                if (divbyfresnel == true)
                                    reflgraphobject.SetAxisTitles("Q/Qc", "Intensity / Fresnel");
                                else
                                    reflgraphobject.SetAxisTitles("Q", "Intensity");

                                reflgraphobject.m_bDBF = divbyfresnel;
                                reflgraphobject.m_dSLD = double.Parse(rhowater.Text);
                                reflgraphobject.LoadFiletoGraph(tempfile.ToString(), modelreflname, "Model Independent Reflectivity", Color.Tomato, SymbolType.Square, 2, true);
                            }

                            GraphCollection.Instance.MainReflGraph = reflgraphobject;
                            GraphCollection.Instance.MainRhoGraph = rhographobject;

                            UpdateReportParameters();
                        }
                        else
                        {
                            FileInfo info = new FileInfo(settingsfile);
                            if (File.Exists(info.DirectoryName + "\\pop.dat"))
                            {
                              //Create a backup folder if we don't have one
                              if(!System.IO.Directory.Exists(info.DirectoryName + "\\FitBackUp"))
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

                                
                                File.Move(info.DirectoryName + "\\pop.dat", fileloc + "\\pop.dat" );
                                File.Move(settingsfile, fileloc + "\\" + info.Name) ;

                                if(File.Exists(info.DirectoryName + "\\rf.dat"))
                                    File.Move(info.DirectoryName + "\\rf.dat", fileloc + "\\rf.dat");

                                if(File.Exists(info.DirectoryName + "\\rho.dat"))
                                    File.Move(info.DirectoryName + "\\rho.dat", fileloc + "\\rho.dat");

                                if(File.Exists(info.DirectoryName + "\\reflfile.dat"))
                                    File.Move(info.DirectoryName + "\\reflfile.dat", fileloc + "\\reflfile.dat");

                                if(File.Exists(info.DirectoryName + "\\reflrhofit.dat"))
                                    File.Move(info.DirectoryName + "\\reflrhofit.dat", fileloc + "\\reflrhofit.dat");
                            }
                        }
                    }

                    FileName.Text = origreflfilename;
                    ParametersBox.Enabled = true;
                    Priority.Enabled = true;
                    Startbutton.Enabled = true;
                    Rhomodel.Enabled = true;


                    if (UseAbsCB.Checked == false)
                    {
                        SubAbs.Enabled = false;
                        SupAbsTB.Enabled = false;
                        SurfAbs.Enabled = false;
                    }
                }
            }
            catch
            {
                MessageBox.Show("Error loading the previous run");
            }
        }

        private bool LoadSettings()
        {
            MySettings PrevSettings = new MySettings();

            if (PrevSettings.PopulateSettings(settingsfile) == true)
            {
                Rholipid.Text = PrevSettings.Settings.SurflayerSLD;
                layerlength.Text = PrevSettings.Settings.Surflayerlength;
                SurfAbs.Text = PrevSettings.Settings.SurflayerAbs;
                AlgorithmCB.Text = PrevSettings.Settings.Algorithm;
                objectiveCB.Text = PrevSettings.Settings.FitFunc;

                if (AlgorithmCB.SelectedIndex != 0)
                    ShowSATBs(true);
                else
                    ShowSATBs(false);

                rhowater.Text = PrevSettings.Settings.SubSLD;
                SubAbs.Text = PrevSettings.Settings.SubAbs;
                SupSLDTB.Text = PrevSettings.Settings.SupSLD;
                SupAbsTB.Text = PrevSettings.Settings.SupAbs;
                SupoffsetTB.Text = PrevSettings.Settings.SupOffset;
                wavelength.Text = PrevSettings.Settings.Wavelength;
                HQoffsetTB.Text = PrevSettings.Settings.HighQOffset;
                Boxlayers.Text = PrevSettings.Settings.BoxCount;
                Iterations.Text = PrevSettings.Settings.Iterations;
                if (int.Parse(PrevSettings.Settings.IterationsCompleted) > 0)
                    SetProgressBar(int.Parse(PrevSettings.Settings.Iterations), 0, int.Parse(PrevSettings.Settings.IterationsCompleted));

                ChiSquareTB.Text = PrevSettings.Settings.ChiSquare;

                TitleTB.Text = PrevSettings.Settings.Title;
                ResolutionTB.Text = PrevSettings.Settings.Resolution;
                TotlengthTB.Text = PrevSettings.Settings.length;
                critedgeoffTB.Text = PrevSettings.Settings.CritEdgeOffset;
                SigmaSearchTB.Text = PrevSettings.Settings.SigmaSearchPerc;
                UseAbsCB.Checked = PrevSettings.Settings.UseAbs;
                ForceNormCB.Checked = PrevSettings.Settings.Forcenorm;
                m_dAnnealtemp = PrevSettings.Settings.AnnealInitTemp;
                m_iAnnealplat = PrevSettings.Settings.AnnealTempPlat;
                m_dAnnealslope = PrevSettings.Settings.AnnealSlope;
                m_dSTUNGamma = PrevSettings.Settings.AnnealGamma;
                ImpNormCB.Checked = PrevSettings.Settings.ImpNorm;
                QErrTB.Text = PrevSettings.Settings.Percerror;
                debugToolStripMenuItem.Checked = PrevSettings.Settings.Debug;
                forceXRToolStripMenuItem1.Checked = PrevSettings.Settings.ForceXR;
                m_bSTUNAdaptive = PrevSettings.Settings.STUNAdaptive;
                m_iSTUNdeciter = PrevSettings.Settings.STUNdeciter;
                m_iSTUNfunc = PrevSettings.Settings.STUNfunc;
                m_dSTUNgammadec = PrevSettings.Settings.STUNgammadec;
                m_iSTUNtempiter = PrevSettings.Settings.STUNtempiter;

                return true;
            }
            else
            {
                return false;
            }
        }

        private void Startbutton_Click(object sender, EventArgs e)
        {
            //Initialize Stochfit

            if (FileName.Text == string.Empty)
                return;

            //If we already have a rho loaded, invalidate it
            if (Z != null)
            {
                Z = null;
                Rho = null;

                reflgraphobject.RemoveGraphfromArray(modelreflname);
                rhographobject.RemoveGraphfromArray(rhomodelname);
            }

            SetProgressBar(Int32.Parse(Iterations.Text), 0, -1);

            int iterations = Int32.Parse(Iterations.Text);
            if (progressBar1.Value > 0 && progressBar1.Value < iterations)
            {
                iterations = int.Parse(Iterations.Text) - progressBar1.Value;
            }

            FileInfo info = new FileInfo(origreflfilename);
            int newdatapoints = ReflData.Instance.GetNumberDataPoints - int.Parse(critedgeoffTB.Text) - int.Parse(HQoffsetTB.Text);

            double[] q = new double[newdatapoints];
            double[] r = new double[newdatapoints];
            double[] re = new double[newdatapoints];
            double[] qe = null;

            if (ReflData.Instance.HaveErrorinQ) 
               qe = new double[newdatapoints];
            
            //Move the data into truncated arrays so we don't have to constantly recalculate in the numerically intensive portions
            for (int i = int.Parse(critedgeoffTB.Text); i < ReflData.Instance.GetNumberDataPoints - int.Parse(HQoffsetTB.Text); i++)
            {
                q[i - int.Parse(critedgeoffTB.Text)] = ReflData.Instance.GetQDataPt(i);
                r[i - int.Parse(critedgeoffTB.Text)] = ReflData.Instance.GetReflDataPt(i);
                re[i - int.Parse(critedgeoffTB.Text)] = ReflData.Instance.GetRErrorPt(i);
                
                if(qe != null)
                    qe[i - int.Parse(critedgeoffTB.Text)] = ReflData.Instance.GetQErrorPt(i);
            }
            
            Init(info.DirectoryName, q, r, re, qe, newdatapoints, Double.Parse(Rholipid.Text), Double.Parse(rhowater.Text),Double.Parse(SupSLDTB.Text),
                    Int32.Parse(Boxlayers.Text), Double.Parse(layerlength.Text),
                     Double.Parse(SurfAbs.Text), Double.Parse(wavelength.Text), Double.Parse(SubAbs.Text), Double.Parse(SupAbsTB.Text), UseAbsCB.Checked, Double.Parse(SupoffsetTB.Text), Double.Parse(QErrTB.Text), ForceNormCB.Checked,
                     Double.Parse(SigTSTB.Text), debugToolStripMenuItem.Checked, forceXRToolStripMenuItem1.Checked, double.Parse(ResolutionTB.Text), double.Parse(TotlengthTB.Text), ImpNormCB.Checked, objectiveCB.SelectedIndex);
            
            SetSAParameters(int.Parse(SigmaSearchTB.Text), AlgorithmCB.SelectedIndex, m_dAnnealtemp,m_iAnnealplat,m_dAnnealslope,m_dSTUNGamma,m_iSTUNfunc,m_bSTUNAdaptive, m_iSTUNtempiter, m_iSTUNdeciter, m_dSTUNgammadec);
            Start(iterations);
            GenPriority(Priority.SelectedIndex);

            myTimer = new System.Timers.Timer();
            myTimer.Elapsed += new ElapsedEventHandler(OnUpdateTimer);
            myTimer.Interval = 5000;
            myTimer.Start();

            ParametersBox.Enabled = false;
            Startbutton.Enabled = false;
            FittingParamBox.Enabled = false;
            setResolutionOptionsToolStripMenuItem.Enabled = false;
            Cancelbutton.Enabled = true;
            debugToolStripMenuItem.Enabled = false;
            LoadFile.Enabled = false;

            previtertime = DateTime.Now;
        }

        void SetProgressBar(int maximum, int minimum, int currentiteration)
        {
            progressBar1.Maximum = maximum;
            progressBar1.Minimum = minimum;

            if (currentiteration > 0)
            {
                progressBar1.Value = currentiteration;
            }
        }

        private void WriteSettings()
        {
            MySettings PrevSettings = new MySettings();

            PrevSettings.Settings.SurflayerSLD = Rholipid.Text;
            PrevSettings.Settings.Surflayerlength = layerlength.Text;
            PrevSettings.Settings.SurflayerAbs = SurfAbs.Text;
            PrevSettings.Settings.Algorithm = AlgorithmCB.Text;
            PrevSettings.Settings.FitFunc = objectiveCB.Text;
            PrevSettings.Settings.SubSLD = rhowater.Text;
            PrevSettings.Settings.SubAbs = SubAbs.Text;
            PrevSettings.Settings.Wavelength = wavelength.Text;
            PrevSettings.Settings.BoxCount = Boxlayers.Text;
            PrevSettings.Settings.Iterations = Iterations.Text;
            PrevSettings.Settings.IterationsCompleted = progressBar1.Value.ToString();
            PrevSettings.Settings.ChiSquare = ChiSquareTB.Text;
            PrevSettings.Settings.Title = TitleTB.Text;
            PrevSettings.Settings.Resolution = ResolutionTB.Text;
            PrevSettings.Settings.length = TotlengthTB.Text;
            PrevSettings.Settings.CritEdgeOffset = critedgeoffTB.Text;
            PrevSettings.Settings.SigmaSearchPerc = SigmaSearchTB.Text;
            PrevSettings.Settings.UseAbs = UseAbsCB.Checked;
            PrevSettings.Settings.HighQOffset = HQoffsetTB.Text;
            PrevSettings.Settings.SupOffset = SupoffsetTB.Text;
            PrevSettings.Settings.SupSLD = SupSLDTB.Text;
            PrevSettings.Settings.SupAbs = SupAbsTB.Text;
            PrevSettings.Settings.Forcenorm = ForceNormCB.Checked;
            PrevSettings.Settings.AnnealInitTemp = m_dAnnealtemp;
            PrevSettings.Settings.AnnealSlope = m_dAnnealslope;
            PrevSettings.Settings.AnnealTempPlat = m_iAnnealplat;
            PrevSettings.Settings.AnnealGamma = m_dSTUNGamma;
            PrevSettings.Settings.ImpNorm = ImpNormCB.Checked;
            PrevSettings.Settings.Percerror = QErrTB.Text;
            PrevSettings.Settings.Debug = debugToolStripMenuItem.Checked;
            PrevSettings.Settings.ForceXR = forceXRToolStripMenuItem1.Checked;
            PrevSettings.Settings.STUNAdaptive = m_bSTUNAdaptive;
            PrevSettings.Settings.STUNdeciter = m_iSTUNdeciter;
            PrevSettings.Settings.STUNfunc = m_iSTUNfunc;
            PrevSettings.Settings.STUNgammadec = m_dSTUNgammadec;
            PrevSettings.Settings.STUNtempiter = m_iSTUNtempiter;
            PrevSettings.WriteSettings(settingsfile);
        }

      
        private void OnUpdateTimer(object source, ElapsedEventArgs e)
        {
            //Thread.CurrentThread.CurrentCulture = new CultureInfo("en-US");
            //Thread.CurrentThread.CurrentUICulture = new CultureInfo("en-US");
            //Need to allow time for our dll to initialize itself
            if (Z == null)
            {
                int Zlength;
                int Qlength;

                while (WarmedUp() == false)
                {
                    Thread.Sleep(100);
                }
                ArraySizes(out Zlength, out Qlength);

                Z = new double[Zlength];
                Rho = new double[Zlength];
                Q = new double[Qlength];
                Refl = new double[Qlength];

                //Set the iterations correctly if we resumed
                if (progressBar1.Value != 0)
                    previnstanceiter = progressBar1.Value;

                //WriteSettings to file
                WriteSettings();
            }

            //We don't allow an update if we're stuck for some reason
            if (m_bupdating == false)
            {
                m_bupdating = true;
                //If the if statement doesn't work, lets lock the section to prevent double
                //execution
                lock (lockobj)
                {
                    double lowestenergy;
                    double temp;
                    int mode;
                    bool isfinished;
                    // Get the progress of the iterations
                    int iterations = GetData(Z, Rho, Q, Refl, out m_droughness, out m_dChiSquare, out m_dGoodnessOfFit, out isfinished);
                    SAparams(out lowestenergy, out temp, out mode);

                    SAlowenergyTB.Text = lowestenergy.ToString("#.#### E-000");
                    SATempTB.Text = temp.ToString("#.#### E-000");
                    //if (mode > 0)
                        SAModeTB.Text = "Annealing";
                    //else
                    //{
                    //    if (double.Parse(ChiSquareTB.Text) > 2)
                    //        SAModeTB.Text = "Tunneling";
                    //    else
                    //        SAModeTB.Text = "In minimum";
                    //}
                    if (iterations <= progressBar1.Maximum)
                        progressBar1.Value = iterations + previnstanceiter;

                    span = DateTime.Now - previtertime;
                    itertimetextbox.Text = ((double)iterations / (double)span.TotalSeconds).ToString("#.#");
                    ChiSquareTB.Text = m_dChiSquare.ToString("#.#### E-000");
                    FitScoreTB.Text = m_dGoodnessOfFit.ToString("#.#### E-000");

                    if (Double.Parse(rhowater.Text) < 0)
                    {
                        for (int i = 0; i < Rho.Length; i++)
                        {
                            Rho[i] *= Double.Parse(rhowater.Text);
                        }
                    }

                    UpdateGraphs();

                    //Update the report and internal graphs
                    GraphCollection.Instance.MainReflGraph = reflgraphobject;
                    GraphCollection.Instance.MainRhoGraph = rhographobject;
                    UpdateReportParameters();

                    //End the calculation if we have reached the maximum number of iterations
                    if (progressBar1.Maximum == progressBar1.Value || isfinished == true)
                    {
                        Canceled();
                    }
                }
                m_bupdating = false;
            }
        }
       
        private void LoadZ(string filename)
        {
            using (StreamReader sr = new StreamReader(filename))
            {
                string dataline;
                int linecount = 0;

                while ((dataline = sr.ReadLine()) != null)
                    linecount++;

                Z = new double[linecount];
                Rho = new double[linecount];
            }
            using (StreamReader sr = new StreamReader(filename))
            {
                string dataline;
                int linecount = 0;
 
                while ((dataline = sr.ReadLine()) != null)
                {
                    Regex r = new Regex(@"\s");
                    string[] temp = r.Split(dataline);
                    ArrayList datastring = new ArrayList();
                    for (int i = 0; i < temp.Length; i++)
                    {
                        if (temp[i] != "")
                            datastring.Add(temp[i]);
                    }

                    Z[linecount] = Double.Parse((string)datastring[0]);
                    Rho[linecount] = Double.Parse((string)datastring[1]);
                    linecount++;
                }
            }
        }

        private void Cancelbutton_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Verify fitting cancellation", "Cancelling", MessageBoxButtons.YesNo) == DialogResult.Yes)
                Canceled();
        }

        private void Canceled()
         {
             WriteSettings();
             myTimer.Stop();
             Cancel();
             Cancelbutton.Enabled = false;
             FittingParamBox.Enabled = LoadFile.Enabled = Startbutton.Enabled = true;
         }

        private void UpdateGraphs()
        {
            try
            {
                Color color;
                reflgraphobject.IsXR = m_bIsXR;
                
                if ((divbyfresnel != reflgraphobject.m_bDBF || m_bmodelreset == true) && origreflfilename != string.Empty)
                {
                    reflgraphobject.Clear();
                    reflgraphobject.m_bDBF = divbyfresnel;
                    reflgraphobject.LoadDataFiletoGraph("Reflectivity Data", Color.Black, SymbolType.Circle, 5);

                    if (Refl != null)
                        reflgraphobject.LoadfromArray(modelreflname, Q, Refl, Color.Tomato, SymbolType.Square, 2, true);
                    else
                    {
                        string tempfile = origreflfilename + "rf.dat";

                        if (File.Exists(tempfile))
                            reflgraphobject.LoadFiletoGraph(tempfile.ToString(), modelreflname, "Model Independent Reflectivity", Color.Tomato, SymbolType.Square, 2, true);
                    }

                    m_bmodelreset = false;
                }
                else
                {
                    if (colorswitch % 2 == 0)
                        color = Color.Turquoise;
                    else
                        color = Color.Tomato;


                    if (Q != null && Z != null)
                    {
                       reflgraphobject.LoadfromArray(modelreflname, Q, Refl, color, SymbolType.Triangle, 2, true);
                       rhographobject.LoadfromArray(rhomodelname, Z, Rho, color, SymbolType.None, 0, true);
                       colorswitch++;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void Priority_SelectedIndexChanged(object sender, EventArgs e)
        {
            //The highest two priorities were... problematic
            GenPriority(Priority.SelectedIndex);
        }

        private void fresnelcb_CheckedChanged(object sender, EventArgs e)
        {
            divbyfresnel = fresnelcb.Checked;

            if(m_bIsXR)
                UpdateGraphs();
        }

        private void progressBar1_MouseHover(object sender, EventArgs e)
        {
            toolTip1.SetToolTip(progressBar1, "Iteration: " + progressBar1.Value.ToString());
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (Cancelbutton.Enabled == true)
            {
                if (MessageBox.Show("Verify fitting cancellation", "Cancelling", MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    WriteSettings();
                    Cancel();
                    Thread.Sleep(700);
                }
                else
                    e.Cancel = true;
            }
        }

        private void Rhomodel_Click(object sender, EventArgs e)
        {
            UpdateReportParameters();
            Rhomodeling RhoModel = new Rhomodeling(Z, Rho, m_droughness, SupoffsetTB.Text, rhowater.Text, SupSLDTB.Text);
            RhoModel.Show();
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

            g.SetMainInformation = (String.Format("Subphase SLD: {0}\n", rhowater.Text));
            
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
            
            if(ImpNormCB.Checked)
                g.SetMainInformation = "The normalization constant was allowed to vary\n";
            if(ForceNormCB.Checked)
                g.SetMainInformation = "Normalization of the curve was forced\n";

            g.SetMainInformation = String.Format("Fit Score for the reflectivity: {0}\n", FitScoreTB.Text);
            g.SetMainInformation = String.Format("Chi Square for the reflectivity: {0}\n", ChiSquareTB.Text);
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
                catch{}
            }
            else
            {
                MessageBox.Show("Please load a file before generating a report");
            }
        }

        private void ShowSATBs(bool onoff)
        {
             SATempTB.Visible = SAModelabel.Visible = SATemplabel.Visible = SALow.Visible = SAlowenergyTB.Visible = SAModeTB.Visible = onoff;
        }

        private void AlgorithmCB_DropDownClosed(object sender, EventArgs e)
        {
            if (AlgorithmCB.SelectedIndex == 1)
            {
                AnnealingParams anneal = new AnnealingParams(m_dAnnealtemp, m_iAnnealplat, m_dAnnealslope);
                
                anneal.ShowDialog(this);
                anneal.GetParams(out m_dAnnealtemp,out m_iAnnealplat,out m_dAnnealslope);
                ShowSATBs(true);
            }
            else if (AlgorithmCB.SelectedIndex == 2)
            {
                STUNAnnealingParams anneal = new STUNAnnealingParams(m_dAnnealtemp, m_iAnnealplat, m_dAnnealslope, m_dSTUNGamma, m_iSTUNfunc,m_bSTUNAdaptive,
                    m_iSTUNtempiter,m_iSTUNdeciter, m_dSTUNgammadec);
        
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

        private void SupSLDTB_TextChanged(object sender, EventArgs e)
        {
                reflgraphobject.m_dSupSLD = Double.Parse(SupSLDTB.Text);
                m_bmodelreset = true;
                UpdateGraphs();
        }

        private void rhowater_TextChanged(object sender, EventArgs e)
        {
                reflgraphobject.m_dSLD = Double.Parse(rhowater.Text);
                m_bmodelreset = true;
                UpdateGraphs();
        }

        private void UseAbsCB_CheckedChanged(object sender, EventArgs e)
        {
            SubAbs.Enabled = SupAbsTB.Enabled = SurfAbs.Enabled = UseAbsCB.Checked;
        }

        private void sLDCalculatorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SLDCalculator SLD = new SLDCalculator();
            SLD.ShowDialog();
        }

         

         protected override void MenuItem_Check(object sender, EventArgs e)
         {
             base.MenuItem_Check(sender, e);
         }

         protected override void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
         {
             base.ValidateNumericalInput(sender, e);
         }

         protected override void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
         {
             base.ValidateIntegerInput(sender, e);
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
             base.MenuItem_Check(sender, e);
             
             m_bIsXR = !forceRQ4GraphingToolStripMenuItem.Checked;
             m_bmodelreset = true;
             UpdateGraphs();
         }

        private void debugToolStripMenuItem_Click(object sender, EventArgs e)
        {
            base.MenuItem_Check(sender, e);
        }

        private void manualToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("The manual is not included in this distribution");
        }
    }
}