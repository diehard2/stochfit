using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using ZedGraph;
using System.Runtime.InteropServices;
using System.IO;
using System.Collections;
using System.Text.RegularExpressions;

namespace GIDFit
{
    public partial class GIDFit : Form
    {
        Graphing GIDGraphobject;
        string GIDfilename;
        double[] QRange;
        double[] RealGIDPoints;
        double[] RealGIDErrors;
        double[] ModelGIDPoints;
        double[] IndividualGraphs;
        double[][] IndividGraphs;
        double[] covar;
        ComboBox[] FuncCBHolder;
        TextBox[] PositCBHolder;
        TextBox[] SigmaCBHolder;
        TextBox[] GammaCBHolder;
        TextBox[] IntensityCBHolder;
        TextBox[] FWHMHolder;
        bool QSquaredCorrection = false;
        bool m_binitialized = false;
        Color[] colarray = new Color[] { Color.Red, Color.Blue, Color.Green, Color.Lavender, Color.HotPink, Color.LightCoral };

        //C++ numberical routines
        [DllImport("LevMardll.dll", EntryPoint = "GIDFit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern double GIDFitFunc(int numberofGID, double[] parameters, int paramsize, double[] QRange, int QSize, double[] GIDpoints, int GIDsize,
										double[] IndividGraphs, int IndividGraphslength, double[] covariance, int covarsize, double[] GIDRealPoints, double[] GIDErrors, double[] info);

        [DllImport("LevMardll.dll", EntryPoint = "GIDGenerate", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int GIDGenerate(int numberofGID, double[] parameters, int paramsize, double[] QRange, int QSize, double[] GIDpoints, int GIDsize, double[] IndividGraphs, int IndividGraph);

        public GIDFit()
        {
            GIDGraphobject = new Graphing();
            FuncCBHolder = new ComboBox[6];
            PositCBHolder = new  TextBox[6];
            SigmaCBHolder = new  TextBox[6];
            IntensityCBHolder = new  TextBox[6];
            GammaCBHolder = new  TextBox[6];
            FWHMHolder = new TextBox[6];
            IndividGraphs = new double[6][];
            InitializeComponent();
        }

       private void GIDFit_Load(object sender, EventArgs e)
        {
            //Setup the graph
            GIDGraphobject.CreateGraph(GIDGraph, "GID", "Qxy" ,"Intensity", AxisType.Linear);
            GIDGraphobject.SetFont("Garamond", 20, 18);
            GIDGraphobject.LegendState = false;

            FileNameTB.ReadOnly = true;

           //Initialize ComboBoxes
            Func1CoB.SelectedIndex = 0;
            Func2CoB.SelectedIndex = 0;
            Func3CoB.SelectedIndex = 0;
            Func4CoB.SelectedIndex = 0;
            Func5CoB.SelectedIndex = 0;
            Func6CoB.SelectedIndex = 0;

           //Move our UI elements to arrays for easer access later on
            FuncCBHolder[0] = Func1CoB;
            FuncCBHolder[1] = Func2CoB;
            FuncCBHolder[2] = Func3CoB;
            FuncCBHolder[3] = Func4CoB;
            FuncCBHolder[4] = Func5CoB;
            FuncCBHolder[5] = Func6CoB;

            PositCBHolder[0] = Position1TB;
            PositCBHolder[1] = Position2TB;
            PositCBHolder[2] = Position3TB;
            PositCBHolder[3] = Position4TB;
            PositCBHolder[4] = Position5TB;
            PositCBHolder[5] = Position6TB;

            SigmaCBHolder[0] = Sigma1TB;
            SigmaCBHolder[1] = Sigma2TB;
            SigmaCBHolder[2] = Sigma3TB;
            SigmaCBHolder[3] = Sigma4TB;
            SigmaCBHolder[4] = Sigma5TB;
            SigmaCBHolder[5] = Sigma6TB;

            IntensityCBHolder[0] = Intensity1TB;
            IntensityCBHolder[1] = Intensity2TB;
            IntensityCBHolder[2] = Intensity3TB;
            IntensityCBHolder[3] = Intensity4TB;
            IntensityCBHolder[4] = Intensity5TB;
            IntensityCBHolder[5] = Intensity6TB;

            GammaCBHolder[0] = Gamma1TB;
            GammaCBHolder[1] = Gamma2TB;
            GammaCBHolder[2] = Gamma3TB;
            GammaCBHolder[3] = Gamma4TB;
            GammaCBHolder[4] = Gamma5TB;
            GammaCBHolder[5] = Gamma6TB;

            FWHMHolder[0] = FWHM1;
            FWHMHolder[1] = FWHM2;
            FWHMHolder[2] = FWHM3;
            FWHMHolder[3] = FWHM4;
            FWHMHolder[4] = FWHM5;
            FWHMHolder[5] = FWHM6;

           //Disable everything intially so we don't change uninitialized variables
            Params.Enabled = false;
            SlopeTB.Enabled = false;
            FitBT.Enabled = false;
            OffSetTB.Enabled = false;
            FunctionNumberTB.Enabled = false;
        }

        private void LoadFileBT_Click(object sender, EventArgs e)
        {
            openFileDialog1.Title = "Select a File";
            openFileDialog1.Filter = "DataFiles|*.txt";
            openFileDialog1.FileName = string.Empty;

            if (openFileDialog1.ShowDialog() != DialogResult.Cancel)
                GIDfilename = openFileDialog1.FileName;
            else
                GIDfilename = string.Empty;

            if(GIDfilename != string.Empty)
            {
                //Allow all of the boxes to have text - enable everything
                m_binitialized = true;

                Params.Enabled = true;
                SlopeTB.Enabled = true;
                FitBT.Enabled = true;
                OffSetTB.Enabled = true;
                FunctionNumberTB.Enabled = true;

                if (MessageBox.Show("Apply correction for q^2?", "Correction", MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    QSquaredCorrection = true;
                    GIDGraphobject.QSquaredCorrection = true;
                }
                else
                {
                    QSquaredCorrection = false;
                }

                FileNameTB.Text = GIDfilename;
                GIDGraphobject.DBF = false;
               
                GIDGraphobject.LoadDataFiletoGraph(GIDfilename, "GID data", Color.Black, SymbolType.Circle, 5);
                LoadArrayswithData(GIDfilename);
              
                //Get the scale value
                OffSetTB.Text = RealGIDPoints[0].ToString();
                
                UpdateGraphs();
            }
        }

        private void LoadArrayswithData(string datafile)
        {
           //Get number of lines
            int lines = 0;
            using (StreamReader sr = new StreamReader(datafile))
            {
                String dataline;
                while ((dataline = sr.ReadLine()) != null)
                {
                    lines++;
                }
            }

            RealGIDPoints = new double[lines];
            RealGIDErrors = new double[lines];
            ModelGIDPoints = new double[lines];
            QRange = new double[lines];

            for(int i = 0; i < 6; i++)
            {
                IndividGraphs[i] = new double[QRange.Length];
            }

           using (StreamReader sr = new StreamReader(datafile))
           {
                    String dataline;
                    int currentline = 0;
                    while ((dataline = sr.ReadLine()) != null)
                    {
                        //Parse text file
                        Regex r = new Regex(@"\s");
                        string[] temp = r.Split(dataline);
                        ArrayList datastring = new ArrayList();
                        for (int i = 0; i < temp.Length; i++)
                        {
                            if (temp[i] != "")
                                datastring.Add(temp[i]);
                        }
                       
                        QRange[currentline] = Double.Parse((string)datastring[0]);

                        if (QSquaredCorrection == true)
                        {
                            RealGIDPoints[currentline] = Double.Parse((string)datastring[1]) * QRange[currentline] * QRange[currentline];
                            RealGIDErrors[currentline] = Double.Parse((string)datastring[2]) * QRange[currentline] * QRange[currentline];
                        }
                        else
                        {
                            RealGIDPoints[currentline] = Double.Parse((string)datastring[1]);
                            RealGIDErrors[currentline] = Double.Parse((string)datastring[2]);
                        }
                        currentline++;
                    }
             }
        }
     
        //Fit the data
        private void FitBT_Click(object sender, EventArgs e)
        {
            int numberoffuncs = int.Parse(FunctionNumberTB.Text);
            double[] parameters = new double[5 * numberoffuncs + 2];
            IndividualGraphs = new double[QRange.Length * numberoffuncs];
            double[] info = new double[9];
            covar = new double[parameters.Length];

            FillParameterArray(ref parameters);

            covar = new double[parameters.Length];

            ChiSquareTB.Text = GIDFitFunc(numberoffuncs, parameters, parameters.Length, QRange, QRange.Length, ModelGIDPoints,
                QRange.Length, IndividualGraphs, IndividualGraphs.Length, covar, covar.Length, RealGIDPoints, RealGIDErrors, info).ToString("##.### E-0");

            ParameterUpdate(parameters);
            UpdateGraphs();
        }

        //Update all of the parameters
        private void ParameterUpdate(double[] parameters)
        {
            SlopeTB.Text = parameters[0].ToString();
            OffSetTB.Text = parameters[1].ToString();

            for (int i = 0; i < int.Parse(FunctionNumberTB.Text); i++)
            {
                FuncCBHolder[i].SelectedIndex = (int)parameters[5 * i + 2];

                if (FuncCBHolder[i].SelectedIndex == 0)
                {
                    IntensityCBHolder[i].Text = parameters[5 * i + 2 + 1].ToString();
                    SigmaCBHolder[i].Text = parameters[5 * i + 2 + 2].ToString();
                    PositCBHolder[i].Text = parameters[5 * i + 2 + 3].ToString();
                    GammaCBHolder[i].Text = "0";

                }
                else if (FuncCBHolder[i].SelectedIndex == 1)
                {
                    IntensityCBHolder[i].Text = parameters[5 * i + 2 + 1].ToString();
                    SigmaCBHolder[i].Text = "0";
                    PositCBHolder[i].Text = parameters[5 * i + 2 + 3].ToString();
                    GammaCBHolder[i].Text = parameters[5 * i + 2 + 4].ToString();
                }
                else
                {
                    IntensityCBHolder[i].Text = parameters[5 * i + 2 + 1].ToString();
                    SigmaCBHolder[i].Text = parameters[5 * i + 2 + 2].ToString();
                    PositCBHolder[i].Text = parameters[5 * i + 2 + 3].ToString();
                    GammaCBHolder[i].Text = parameters[5 * i + 2 + 4].ToString();
                }
            }
        }
        
        void UpdateGraphs()
        {
            try
            {
                if (m_binitialized == true)
                {
                    int numberoffuncs = int.Parse(FunctionNumberTB.Text);
                    double[] parameters = new double[5 * numberoffuncs + 2];

                    FillParameterArray(ref parameters);
                    IndividualGraphs = new double[QRange.Length * numberoffuncs];

                    GIDGenerate(numberoffuncs, parameters, parameters.Length, QRange, QRange.Length, ModelGIDPoints, ModelGIDPoints.Length, IndividualGraphs, IndividualGraphs.Length);

                    //Add our new graphs to the original graph

                    GIDGraphobject.ClearModels();

                    if (numberoffuncs > 1)
                    {
                        //Fill individual graphs
                        for (int i = 0; i < numberoffuncs; i++)
                        {
                            for (int j = 0; j < QRange.Length; j++)
                            {
                                IndividGraphs[i][j] = IndividualGraphs[i * QRange.Length + j];
                            }

                        }
                        string modname = "temp";
                        //Add stuff here to show each individual scan
                        for (int i = 0; i < numberoffuncs; i++)
                        {
                            GIDGraphobject.LoadfromArray(modname + i.ToString(), QRange, IndividGraphs[i], colarray[i], SymbolType.None, 4, System.Drawing.Drawing2D.DashStyle.Dot, false);
                        }
                    }
                    SetFWHM();
                    GIDGraphobject.LoadfromArray("superimposedmodel", QRange, ModelGIDPoints, Color.Blue, SymbolType.None, 1, false);
                }
            }
            catch
            {}
        }

        private void SetFWHM()
        {
            for (int i = 0; i < int.Parse(FunctionNumberTB.Text); i++)
            {
                if (FuncCBHolder[i].SelectedIndex == 0)
                {
                    FWHMHolder[i].Text = string.Format("{0:#.###}",(2.0 * Math.Sqrt(2 * Math.Log(2)) * double.Parse(SigmaCBHolder[i].Text)));
                }
                else if (FuncCBHolder[i].SelectedIndex == 1)
                {
                    FWHMHolder[i].Text = string.Format("{0:#.###}",2*double.Parse(GammaCBHolder[i].Text));
                }
                else if (FuncCBHolder[i].SelectedIndex == 2)
                {
                    //From J.J.Olivero and R.L. Longbothum in Empirical fits to the Voigt line width: A brief review, JQSRT 17, P233
                    double gamma = double.Parse(GammaCBHolder[i].Text);
                    double sigma = double.Parse(SigmaCBHolder[i].Text);
                    FWHMHolder[i].Text = string.Format("{0:#.###}", (0.5346 * gamma + Math.Sqrt(0.2166 * sigma * sigma + gamma * gamma)));
                }
            }
        }

        private void FillParameterArray(ref double[] parameters)
        {
            try
            {
                parameters[0] = double.Parse(SlopeTB.Text);
                parameters[1] = double.Parse(OffSetTB.Text);

                for (int i = 0; i < int.Parse(FunctionNumberTB.Text); i++)
                {
                    if (FuncCBHolder[i].SelectedIndex == 0)
                    {
                        parameters[5 * i + 2] = FuncCBHolder[i].SelectedIndex;
                        parameters[5 * i + 2 + 1] = double.Parse(IntensityCBHolder[i].Text);
                        parameters[5 * i + 2 + 2] = double.Parse(SigmaCBHolder[i].Text);
                        parameters[5 * i + 2 + 3] = double.Parse(PositCBHolder[i].Text);
                        parameters[5 * i + 2 + 4] = 0;

                    }
                    else if (FuncCBHolder[i].SelectedIndex == 1)
                    {
                        parameters[5 * i + 2] = FuncCBHolder[i].SelectedIndex;
                        parameters[5 * i + 2 + 1] = double.Parse(IntensityCBHolder[i].Text);
                        parameters[5 * i + 2 + 2] = 0;
                        parameters[5 * i + 2 + 3] = double.Parse(PositCBHolder[i].Text);
                        parameters[5 * i + 2 + 4] = double.Parse(GammaCBHolder[i].Text);
                    }
                    else
                    {
                        parameters[5 * i + 2] = FuncCBHolder[i].SelectedIndex;
                        parameters[5 * i + 2 + 1] = double.Parse(IntensityCBHolder[i].Text);
                        parameters[5 * i + 2 + 2] = double.Parse(SigmaCBHolder[i].Text);
                        parameters[5 * i + 2 + 3] = double.Parse(PositCBHolder[i].Text);
                        parameters[5 * i + 2 + 4] = double.Parse(GammaCBHolder[i].Text);
                    }
                }
            }
            catch
            {
                MessageBox.Show("Invalid input");
            }
        }

        #region Parameter Updates
        private void SlopeTB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void FunctionNumberTB_TextChanged(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void OffSetTB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Intensity1TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Sigma1TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Position1TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Gamma1TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Intensity2TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Sigma2TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Position2TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Gamma2TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Intensity3TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Sigma3TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Position3TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Gamma3TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Intensity4TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Sigma4TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Position4TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Gamma4TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Intensity5TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Sigma5TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Position5TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Gamma5TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Intensity6TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Sigma6TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Position6TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Gamma6TB_Validated(object sender, EventArgs e)
        {
            UpdateGraphs();
        }
        private void Func1CoB_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Func2CoB_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Func3CoB_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Func4CoB_SelectionChangeCommitted(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Func5CoB_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateGraphs();
        }

        private void Func6CoB_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateGraphs();
        }
        #endregion

        private void pubgraph_Click(object sender, EventArgs e)
        {
            Pubgraph win = new Pubgraph(GIDGraphobject);
            win.ShowDialog(this);
        }


    }
}