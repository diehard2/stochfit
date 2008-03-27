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
using ZedGraph;
using System.Drawing;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;
using System.Globalization;

namespace StochasticModeling
{
    
    /// <summary>
    /// Implements graphing for X-ray or neutron data
    /// </summary>
    public class Graphing:GraphingBase
    {
        public double m_dSLD = 0;
        public double m_dSupSLD = 0;
        public double m_dlambda = 1.0;
        public bool m_bDBF = false;
        private bool m_bnegativeerrorval = false;
        private bool m_biszoomed = false;
        private bool m_bisXR = true;
        private PointPairList RealReflData, RealReflErrors;
        CultureInfo CI_US = new CultureInfo("en-US");

        public Graphing(string name):base(name)
        {}

        public override void CreateGraph(ZedGraphControl zgc, string Title, string XAxis, string YAxis, AxisType blog)
        {
            base.CreateGraph(zgc, Title, XAxis, YAxis, blog);

            ZGControl.ZoomEvent += new ZedGraphControl.ZoomEventHandler(ZoomEvent);
            GraphContextMenuBuilder(MyContextMenuBuilder);
        }

        private void ZoomEvent(ZedGraphControl sender, ZoomState oldState, ZoomState newState)
        {
            if (m_bnegativeerrorval == true)
            {
                if (m_bDBF == true)
                {
                    if (sender.GraphPane.YAxis.Scale.Min == 3e-4)
                    {
                        m_biszoomed = false;
                        return;
                    }
                }
                else
                {
                    if (sender.GraphPane.YAxis.Scale.Min == 1e-11)
                    {
                        m_biszoomed = false;
                        return;
                    }
                }
                m_biszoomed = true;
            }
        }

        public bool IsXR
        {
            get
            {
                return m_bisXR;
            }
            set
            {
                if (value == false)
                    m_bDBF = false;

                m_bisXR = value;
            }
        }

        private void MyContextMenuBuilder(ZedGraphControl control, ContextMenuStrip menuStrip, Point mousePt, ZedGraphControl.ContextMenuObjectState objState)
        {
            AddMenuItem(menuStrip, "ClipCopy_tag", "ClipCopy_tag", "High Quality Copy", CopyMetatoClip);
            AddMenuItem(menuStrip, "SaveEMF_tag", "SaveEMF_tag", "Save High Quality Image", SaveEMFFile);

            RemoveMenuItem(menuStrip, "copy");
            RemoveMenuItem(menuStrip, "set_default");
            RemoveMenuItem(menuStrip, "page_setup");
            RemoveMenuItem(menuStrip, "print");
        }

        /// <summary>
        /// Clears all of the graphs and resets the Zoom attribute
        /// </summary>
        public override void Clear()
        {
            base.Clear();
            m_biszoomed = false;
        }
      
        protected override void AddCurvetoGraph(PointPairList list, PointPairList elist, string DataName, Color linecolor, SymbolType type, int symbolsize)
        {
            base.AddCurvetoGraph(list, elist, DataName, linecolor, type, symbolsize);
            //We have to manually set the error bars if we have a negative value due to error
            SetAxisScale();
        }

        protected override void AddCurvetoGraph(PointPairList list, string DataName, Color linecolor, SymbolType type, int symbolsize, bool isSmoothed)
        {
            base.AddCurvetoGraph(list, DataName, linecolor, type, symbolsize, isSmoothed);
            SetAxisScale();
        }


        private void SetAxisScale()
        {
            if (m_bnegativeerrorval == false)
                AxisChange();
            else
            {
                AxisChange();
                if (m_biszoomed == false)
                {
                    if (m_bDBF == true)
                    {
                        if (m_bisXR && CalcQc(m_dSLD, m_dSupSLD, m_dlambda) > 0.005)
                            Pane.YAxis.Scale.Min = 3e-5;
                        else
                            Pane.YAxis.Scale.Min = 1e-12;
                    }
                    else
                        Pane.YAxis.Scale.Min = 1e-11;
                }
            }
            Invalidate();
        }

      /// <summary>
      /// Load the data in the singleton <seealso cref="ReflData"/> to the graph
      /// </summary>
      /// <param name="name">A unique name for the data</param>
      /// <param name="color">The color of the data line</param>
      /// <param name="symbol">Symbol Shape</param>
      /// <param name="symbolsize">Size of the symbol</param>
        public void LoadDataFiletoGraph(string name, Color color, SymbolType symbol,int symbolsize)
        {
            try
            {
                Clear();
                m_bnegativeerrorval = false;

                double Qc = CalcQc(m_dSLD, m_dSupSLD, m_dlambda);
                PointPairList locRefl = new PointPairList();
                PointPairList locReflerror = new PointPairList();
                double poserrorval, negerrorval;


                //Initialize our pointpairlists and add our data
                //Add the real data
                    RealReflData = new PointPairList(ReflData.Instance.GetQData, ReflData.Instance.GetReflData);
                    RealReflErrors = new PointPairList();

                    for (int i = 0; i < ReflData.Instance.GetNumberDataPoints; i++)
                    {
                        poserrorval = ReflData.Instance.GetReflDataPt(i) + ReflData.Instance.GetRErrorPt(i);
                        negerrorval = ReflData.Instance.GetReflDataPt(i) - ReflData.Instance.GetRErrorPt(i);

                        if (negerrorval > 0.0)
                            RealReflErrors.Add(ReflData.Instance.GetQDataPt(i), poserrorval, negerrorval);
                        else
                        {
                            RealReflErrors.Add(ReflData.Instance.GetQDataPt(i), poserrorval, 1e-15);
                            m_bnegativeerrorval = true;
                        }
                    }

                if (m_bDBF == false)
                {
                    //Set the Q scale
                    if (RealReflData[RealReflData.Count - 1].X > 1.0)
                        Pane.XAxis.Scale.Max = RealReflData[RealReflData.Count - 1].X + 5;
                    else
                        Pane.XAxis.Scale.Max = RealReflData[RealReflData.Count - 1].X + .05;

                    SetAxisTitles("Q", "Intensity");

                    AddCurvetoGraph(RealReflData, RealReflErrors, name, color, symbol, symbolsize);
                }
                else
                {
                    for (int i = 0; i < ReflData.Instance.GetNumberDataPoints; i++)
                    {
                        if (Qc > .005 && m_bisXR)
                        {
                            double fresnelpt = CalcFresnelPoint(RealReflData[i].X, Qc);

                            locRefl.Add(RealReflData[i].X / Qc, RealReflData[i].Y / fresnelpt);
                            locReflerror.Add(RealReflErrors[i].X / Qc, RealReflErrors[i].Y / fresnelpt, RealReflErrors[i].Z / fresnelpt);
                        }
                        else
                        {
                          locRefl.Add(RealReflData[i].X, RealReflData[i].Y * Math.Pow(RealReflData[i].X, 4.0));
                          locReflerror.Add(RealReflErrors[i].X, RealReflErrors[i].Y * Math.Pow(RealReflData[i].X, 4.0),
                                    RealReflErrors[i].Z * Math.Pow(RealReflData[i].X, 4.0));
                        }
                    }

                    //Set the Q scale
                    if (locRefl[locRefl.Count - 1].X > 1.0)
                        Pane.XAxis.Scale.Max = locRefl[locRefl.Count - 1].X + 5;
                    else
                        Pane.XAxis.Scale.Max = locRefl[locRefl.Count - 1].X + .05;

                    if (Qc > 0.005 && m_bisXR)
                        SetAxisTitles("Q/Qc", "Intensity / Fresnel");
                    else
                        SetAxisTitles("Q", "Intensity (RQ^4)");

                    AddCurvetoGraph(locRefl, locReflerror, name, color, symbol, symbolsize);
                }
                
                m_alDatainGraph.Add(name);
                //To account for the error file
                m_alDatainGraph.Add(name);
            }
            catch
            {
                MessageBox.Show("Data file was not in the correct format");
            }
        }

        public void LoadFiletoGraph(string datafile, string filename, string plotname, Color color, SymbolType symbol, int symbolsize, bool isSmoothed)
        {
            try
            {
                RemoveGraphfromArray(filename);

                CultureInfo CI_US = new CultureInfo("en-US");
                if (datafile != string.Empty)
                {
                    PointPairList list = new PointPairList();
                    double Qc = CalcQc(m_dSLD, m_dSupSLD, m_dlambda);
                    using (StreamReader sr = new StreamReader(datafile))
                    {
                        String dataline;
                        double Q;
                        double Refl;

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

                            Q = Double.Parse((string)datastring[0], CI_US);

                            if (m_bDBF == false)
                            {
                                Refl = Double.Parse((string)datastring[1], CI_US);
                            }
                            else
                            {
                                if (Qc > .005 && m_bisXR)
                                {
                                    Refl = Double.Parse((string)datastring[1], CI_US) / CalcFresnelPoint(Q, Qc);
                                    Q = Q / Qc;
                                }
                                else
                                    Refl = Double.Parse((string)datastring[1], CI_US) * Math.Pow(Q, 4.0);
                            }

                            list.Add(Q, Refl);
                        }
                        AddCurvetoGraph(list, plotname, color, symbol, symbolsize, isSmoothed);
                        m_alDatainGraph.Add(filename);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public override void LoadfromArray(string name, double[] X, double[] Y, Color color, SymbolType symbol, int symbolsize, bool isSmoothed)
        {
            RemoveGraphfromArray(name);

            PointPairList list = new PointPairList();
            if (m_bDBF)
            {
                double Qc = CalcQc(m_dSLD, m_dSupSLD, m_dlambda);
                for (int i = 0; i < X.Length; i++)
                {
                    if (Qc > 0.005 && m_bisXR)
                        list.Add(X[i] / Qc, Y[i] / CalcFresnelPoint(X[i], Qc));
                    else
                        list.Add(X[i], Y[i] * Math.Pow(X[i], 4.0));
                }
            }
            else
            {
                for (int i = 0; i < X.Length; i++)
                {
                    list.Add(X[i], Y[i]);
                }
            }
            AddCurvetoGraph(list, name, color, symbol,symbolsize, isSmoothed);
            m_alDatainGraph.Add(name);
        }

        protected override void GetPointList(double[] X, double[] Y, ref PointPairList List)
        {
            double Qc = CalcQc(m_dSLD, m_dSupSLD, m_dlambda);

            for (int i = 0; i < X.Length; i++)
            {
                if (m_bDBF)
                {
                    if (Qc > .005)
                        List.Add(X[i] / Qc, Y[i] / CalcFresnelPoint(X[i], Qc));
                    else
                        List.Add(X[i], Y[i] * Math.Pow(X[i], 4.0));
                }
                else
                    List.Add(X[i], Y[i]);
            }
        }

        
        //Anyone can use the next two fucntions from anywhere. These should perhaps be moved
        static public double CalcQc(double dSLD, double SupSLD, double lambda)
        {
            if (dSLD - SupSLD > 0)
                return 4 * Math.Sqrt((Math.PI * (dSLD - SupSLD) * 1e-6));
            else
                return 0;
        }

        static public double CalcFresnelPoint(double Q, double Qc)
        {
            if (Q <= Qc)
                return 1;
            else
            {
                double term1 = Math.Sqrt(1 - Math.Pow((Qc / Q), 2.0));
                return Math.Pow((1.0 - term1) / (1.0 + term1), 2.0);
            }
        }
    }
}
