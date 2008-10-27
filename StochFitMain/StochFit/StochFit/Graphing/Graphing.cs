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
#pragma warning disable 1591

using System;
using ZedGraph;
using System.Drawing;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Globalization;
using System.Collections.Generic;
using StochasticModeling.Core;

namespace StochasticModeling
{
    
    /// <summary>
    /// Implements graphing for X-ray or neutron data
    /// </summary>
    public class Graphing:GraphingBase
    {
        private double m_dSubSLD = 0;
        private double m_dSupSLD = 0;
        private bool m_bDBF = false;
        private string m_sfilename;
        private bool m_bRunning = false;
        private bool m_bdatafile = false;
        private int lowqindex = 0;
        private int highqindex = 0;
        private bool m_bnegativeerrorval = false;
        private bool m_biszoomed = false;
        private bool m_bisQfour = true;
        private bool m_bUseSLD = false;
        private PointPairList RealReflData, RealReflErrors;
        CultureInfo CI_US = new CultureInfo("en-US");

        /// <summary>
        /// Called if the bounds on the curve are changed
        /// </summary>
        /// <param name="sender">null</param>
        /// <param name="e">null</param>
        public delegate void ChangedEventHandler(object sender, EventArgs e);
        
        /// <summary>
        /// Event to subsribe to if the bounds are changed
        /// </summary>
        public event ChangedEventHandler ChangedBounds;

      

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="name">Name of the graph. Can be string.Empty</param>
        public Graphing(string name):base(name)
        {}

        /// <summary>
        /// Create the graph
        /// </summary>
        /// <param name="zgc">The associated ZedGraph control. Can be null</param>
        /// <param name="Title">Title for the graph</param>
        /// <param name="XAxis">Title for the X axis</param>
        /// <param name="YAxis">Title for the Y axis</param>
        /// <param name="blog">Graphed as log(Y) if true, no scaling if false</param>
        public override void CreateGraph(ZedGraphControl zgc, string Title, string XAxis, string YAxis, AxisType blog)
        {
            base.CreateGraph(zgc, Title, XAxis, YAxis, blog);

            ZGControl.ZoomEvent += new ZedGraphControl.ZoomEventHandler(ZoomEvent);
            GraphContextMenuBuilder(MyContextMenuBuilder);
            //Set all fonts here to give a consistent look
            SetAllFonts("Garamond", 20, 18);
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
                else if (m_bisQfour == true)
                {
                    if (sender.GraphPane.YAxis.Scale.Min == 1e-12)
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

        public void SetGraphType(bool IsQFour, bool IsDBF)
        {
            if (IsQFour == false && IsDBF == true)
            {
                m_bisQfour = false;
                m_bDBF = true;
            }
            else if ((IsQFour = true && IsDBF == true) || (IsQFour == true && IsDBF == false))
            {
                m_bisQfour = true;
                m_bDBF = false;
            }
            else
            {
                m_bDBF = false;
                m_bisQfour = false;
            }
           
        }

        private void MyContextMenuBuilder(ZedGraphControl control, ContextMenuStrip menuStrip, Point mousePt, ZedGraphControl.ContextMenuObjectState objState)
        {
            AddMenuItem(menuStrip, "ClipCopy_tag", "ClipCopy_tag", "High Quality Copy", CopyMetatoClip);
            AddMenuItem(menuStrip, "SaveEMF_tag", "SaveEMF_tag", "Save High Quality Image", SaveEMFFile);

            if (m_bdatafile)
            {
                if (m_bRunning == false)
                {
                    AddMenuItem(menuStrip, "LowQCut_tag", "LowQCut_tag", "Select Low Q Cutoff", SelectLowQPoint);
                    AddMenuItem(menuStrip, "HighQCut_tag", "HighQCut_tag", "Select High Q Cutoff", SelectHighQPoint);
                    AddMenuItem(menuStrip, "ClearOffset_tag", "ClearOffset_tag", "Clear Offsets", ClearQOffsets);
                }
            }

            RemoveMenuItem(menuStrip, "copy");
            RemoveMenuItem(menuStrip, "set_default");
            RemoveMenuItem(menuStrip, "page_setup");
            RemoveMenuItem(menuStrip, "print");

            MousePosition = mousePt;
        }

        /// <summary>
        /// Clears all of the graphs and resets the Zoom attribute
        /// </summary>
        public override void Clear()
        {
            base.Clear();
            m_bdatafile = m_biszoomed = false;
        }
      
        /// <summary>
        /// Add a curve to the graph
        /// </summary>
        /// <param name="list">The X,Y list of data points</param>
        /// <param name="elist">The list of errors associated with list</param>
        /// <param name="DataName">Name for the curve to be displayed in the legend</param>
        /// <param name="linecolor">Color for the line</param>
        /// <param name="type">Type of symbol for the curve</param>
        /// <param name="symbolsize">Size of the symbols on the curve </param>
        /// <param name="tag">An internal name for the curve</param>
        protected override void AddCurvetoGraph(PointPairList list, PointPairList elist, string DataName, Color linecolor, SymbolType type, int symbolsize, string tag)
        {
            base.AddCurvetoGraph(list, elist, DataName, linecolor, type, symbolsize, tag);
            //We have to manually set the error bars if we have a negative value due to error
            SetAxisScale();
        }

        /// <summary>
        /// Add a curve to the graph
        /// </summary>
        /// <param name="list">The X,Y list of data points</param>
        /// <param name="DataName">Name for the curve to be displayed in the legend</param>
        /// <param name="linecolor">Color for the line</param>
        /// <param name="type">Type of symbol for the curve</param>
        /// <param name="symbolsize">Size of the symbols on the curve </param>
        /// <param name="isSmoothed">The curve is antialiased if true</param>
        /// <param name="tag">An internal name for the curve</param>
        protected override void AddCurvetoGraph(PointPairList list, string DataName, Color linecolor, SymbolType type, int symbolsize, bool isSmoothed, string tag)
        {
            base.AddCurvetoGraph(list, DataName, linecolor, type, symbolsize, isSmoothed, tag);
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
                    if (m_bDBF)
                        Pane.YAxis.Scale.Min = 3e-4;
                    else if(m_bisQfour)
                        Pane.YAxis.Scale.Min = 1e-12;
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
        /// <param name="XData">Q Data Points in A^-1</param>
        /// <param name="YData">Reflectivity data in intensity</param>
        public void LoadDatawithErrorstoGraph(string name, Color color, SymbolType symbol,int symbolsize, double[] XData, double[] YData)
        {
            try
            {
                Clear();
                m_bdatafile = true;
                m_bnegativeerrorval = false;

                double Qc = HelperFunctions.CalcQc(m_dSubSLD, m_dSupSLD);

                if (Qc < .005 && m_bDBF)
                {
                        m_bDBF = false;
                        m_bisQfour = true;
                }

                PointPairList locRefl = new PointPairList();
                PointPairList locReflerror = new PointPairList();
                double poserrorval, negerrorval;

                //Initialize our pointpairlists and add our data
                //Add the real data

                RealReflData = new PointPairList(XData, YData);

                for (int i = 0; i < ReflData.Instance.GetNumberDataPoints; i++)
                    RealReflData[i].Z = 0.0;

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

                if (m_bDBF == false && m_bisQfour == false)
                {
                    //Set the Q scale
                    if (RealReflData[RealReflData.Count - 1].X > 1.0)
                        Pane.XAxis.Scale.Max = RealReflData[RealReflData.Count - 1].X + 5;
                    else
                        Pane.XAxis.Scale.Max = RealReflData[RealReflData.Count - 1].X + .05;

                    Pane.XAxis.Scale.Min = 0.0;
                    SetAxisTitles("Q", "Intensity");
                   
                    AddCurvetoGraph(RealReflData, RealReflErrors, name, color, symbol, symbolsize, "realdatafile");
                }
                else
                {
                    for (int i = 0; i < ReflData.Instance.GetNumberDataPoints; i++)
                    {
                        if (m_bDBF)
                        {
                            double fresnelpt = HelperFunctions.CalcFresnelPoint(RealReflData[i].X, Qc);

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

                    if (!m_bisQfour)
                        SetAxisTitles("Q/Qc", "Intensity / Fresnel");
                    else
                        SetAxisTitles("Q", "Intensity (RQ^4)");

                    AddCurvetoGraph(locRefl, locReflerror, name, color, symbol, symbolsize, "realdatafile");
                }
                
                m_alDatainGraph.Add(name);
                //To account for the error file
                m_alDatainGraph.Add(name);
                m_sfilename = name;

                if (highqindex == 0)
                    highqindex = ReflData.Instance.GetNumberDataPoints;

                if (highqindex < ReflData.Instance.GetNumberDataPoints || lowqindex > 0)
                {
                    SetBounds();
                }
                else
                {
                    highqindex = ReflData.Instance.GetNumberDataPoints;
                }
            }
            catch
            {
                throw new Exception("Data file was not in the correct format");
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
                    double Qc = HelperFunctions.CalcQc(m_dSubSLD, m_dSupSLD);
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
                            List<string> datastring = new List<string>();

                            for (int i = 0; i < temp.Length; i++)
                            {
                                if (temp[i] != "")
                                    datastring.Add(temp[i]);
                            }

                            Q = Double.Parse(datastring[0], CI_US);

                            if (m_bDBF == true)
                            {
                                Refl = Double.Parse(datastring[1], CI_US) / HelperFunctions.CalcFresnelPoint(Q, Qc);
                                Q = Q / Qc;
                            }
                            else if (m_bisQfour == true)
                            {
                                Refl = Double.Parse(datastring[1], CI_US) * Math.Pow(Q, 4.0);
                            }
                            else
                            {
                                if (m_bUseSLD == false)
                                    Refl = Double.Parse(datastring[1], CI_US);
                                else
                                    Refl = Double.Parse(datastring[1], CI_US) * SubSLD;
                            }

                            list.Add(Q, Refl);
                        }
                        AddCurvetoGraph(list, plotname, color, symbol, symbolsize, isSmoothed, filename);
                        m_alDatainGraph.Add(filename);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public override void LoadfromArray(string name, double[] X, double[] Y, Color color, SymbolType symbol, int symbolsize, bool isSmoothed, string tag)
        {
            RemoveGraphfromArray(name);

            double Qc = HelperFunctions.CalcQc(m_dSubSLD, m_dSupSLD);
            
            if (Qc < .005 && m_bDBF)
            {
                m_bDBF = false;
                m_bisQfour = true;
            }

            PointPairList list = new PointPairList();
            if (m_bDBF)
            {
                
                for (int i = 0; i < X.Length; i++)
                {
                    list.Add(X[i] / Qc, Y[i] / HelperFunctions.CalcFresnelPoint(X[i], Qc));
                    
                }
            }
            else if (m_bisQfour)
            {
                for (int i = 0; i < X.Length; i++)
                {
                    list.Add(X[i], Y[i] * Math.Pow(X[i], 4.0));
                }
            }
            else
            {
                for (int i = 0; i < X.Length; i++)
                {
                    if (m_bUseSLD == false)
                        list.Add(X[i], Y[i]);
                    else
                        list.Add(X[i], Y[i] * m_dSubSLD);

                }
            }
            AddCurvetoGraph(list, name, color, symbol,symbolsize, isSmoothed, string.Empty);
            m_alDatainGraph.Add(name);
        }

        protected override void GetPointList(double[] X, double[] Y, ref PointPairList List)
        {
            double Qc = HelperFunctions.CalcQc(m_dSubSLD, m_dSupSLD);

            for (int i = 0; i < X.Length; i++)
            {
                if (m_bDBF)
                {
                    if (Qc > .005)
                        List.Add(X[i] / Qc, Y[i] / HelperFunctions.CalcFresnelPoint(X[i], Qc));
                    else
                        List.Add(X[i], Y[i] * Math.Pow(X[i], 4.0));
                }
                else
                    List.Add(X[i], Y[i]);
            }
        }

        
        

        private void SelectLowQPoint(object sender, System.EventArgs e)
        {
            CurveItem curve;
            int nearest;
            
            Pane.FindNearestPoint(MousePosition, out curve, out nearest);

            if (curve.NPts > ReflData.Instance.GetNumberDataPoints)
            {
                MessageBox.Show("You have attempted to selecte points on the wrong cuve. Please zoom in and try again");
                return;
            }

            if (nearest + (ReflData.Instance.GetNumberDataPoints - highqindex) > ReflData.Instance.GetNumberDataPoints)
            {
                MessageBox.Show("Error in setting bounds");
                return;
            }

                lowqindex = nearest;
                
                SetBounds();
                OnChanged(this, null);
        }



        private void ClearQOffsets(object sender, System.EventArgs e)
        {
            int i = 0;
            for (i = 0; i < Pane.CurveList.Count; i++)
            {

                if ((string)Pane.CurveList[i].Tag == "realdatafile")
                    break;
            }


            for (int k = 0; k < Pane.CurveList[i].NPts; k++)
                Pane.CurveList[i][k].Z = 0.0;

            highqindex = ReflData.Instance.GetNumberDataPoints;
            lowqindex = 0;

            Invalidate();
            OnChanged(this, null);
        }

        private void SelectHighQPoint(object sender, System.EventArgs e)
        {
            CurveItem curve;
            int nearest;

            Pane.FindNearestPoint(MousePosition, out curve, out nearest);

            if (curve.NPts > ReflData.Instance.GetNumberDataPoints)
            {
                MessageBox.Show("You have attempted to selecte points on the wrong cuve. Please zoom in and try again");
                return;
            }

            if ((nearest - ReflData.Instance.GetNumberDataPoints) + lowqindex > ReflData.Instance.GetNumberDataPoints)
            {
                MessageBox.Show("Error in setting bounds");
                return;
            }

            highqindex = nearest;

            SetBounds();

            OnChanged(this, null);
            
        }

        public void SetBounds()
        {
            if ((highqindex > 0 || lowqindex > 0) && Pane.CurveList.Count > 0)
            {

                int i = 0;
                for (i = 0; i < Pane.CurveList.Count; i++)
                {

                    if ((string)Pane.CurveList[i].Tag == "realdatafile")
                        break;
                }


                for (int k = 0; k < Pane.CurveList[i].NPts; k++)
                    Pane.CurveList[i][k].Z = 0.0;

                if (highqindex > 0)
                {
                    for (int k = highqindex; k < Pane.CurveList[i].NPts; k++)
                        Pane.CurveList[i][k].Z = 1.0;
                }

                if (lowqindex > 0)
                {
                    for (int k = 0; k <= lowqindex; k++)
                        Pane.CurveList[i][k].Z = 1.0;
                }

                Invalidate();
            }
        }

        /// <summary>
        /// Get/Set the offset for the high Q portion of the curve
        /// </summary>
        public int GetHighQOffset
        {
            get
            { return highqindex; }
            set
            { highqindex = value;}
        }

        /// <summary>
        /// Get/Set the offset for the low Q portion of the curve
        /// </summary>
        public int GetLowQOffset
        {
            get
            { return lowqindex;}
            set
            { lowqindex = value;}
        }

        /// <summary>
        /// Get/Set the scattering length density of the substrate
        /// </summary>
        public double SubSLD
        {
            get { return m_dSubSLD; }
            set { m_dSubSLD = value; }
        }

        /// <summary>
        /// Get/Set the scattering length density of the superphase
        /// </summary>
        public double SupSLD
        {
            get { return m_dSupSLD; }
            set { m_dSupSLD = value; }
        }

        /// <summary>
        /// Get/Set whether the graph should be divided by Fresnel
        /// </summary>
        public bool DivbyFresnel
        {
            get { return m_bisQfour || m_bDBF; }
        }

        /// <summary>
        /// Gets whether a datafile has been loaded at least once to the graph
        /// </summary>
        public bool DataFileLoaded
        {
            get{return m_bdatafile;}
        }

        /// <summary>
        /// Determines whether the main program is running in order to provide the correct context menu. This class should not set this value
        /// </summary>
        public bool ProgramRunningState
        {
            get { return m_bRunning; }
            set { m_bRunning = value; }
        }

        public bool UseSLD
        {
            get { return m_bUseSLD; }
            set { m_bUseSLD = value; }
        }

        public bool HasCurve
        {
            get
            {
                if (Pane.CurveList.Count > 0)
                    return true;
                else
                    return false;
            }
        }
        /// <summary>
        /// Communicate with the graphing class. This will let us know if we've changed boundaries
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnChanged(object sender, EventArgs e)
        {
            if (ChangedBounds != null)
                ChangedBounds(sender, e);
        }

        /// <summary>
        /// Delete all curves that are not data
        /// </summary>
        internal void ClearCurves()
        {
            for (int i = 0; i < m_alDatainGraph.Count; i++)
            {
                if ((string)m_alDatainGraph[i] != m_sfilename)
                    RemoveGraphfromArray((string)m_alDatainGraph[i]);
            }

        }
    }
}
