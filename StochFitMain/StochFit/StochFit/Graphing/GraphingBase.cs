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
using System.Text;
using ZedGraph;
using System.Drawing;
using System.IO;
using System.Windows.Forms;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Drawing.Drawing2D;
using System.Collections.Generic;
using System.Linq;

namespace StochasticModeling
{
/// <summary>
/// Base class for using the ZedGraph graphing control for a scatter plot. While this class can be extended, it also
/// functions
/// </summary>
    public class GraphingBase
    {
        private ZedGraphControl m_cZG;
        protected List<string> m_alDatainGraph;
        private GraphPane m_cMyPane;
        protected PointF m_cMousePos = PointF.Empty;
        private bool m_bThisisadeepcopy = false;
        private string m_sGraphname;
        private bool m_bHide = false;

        public virtual void CreateGraph(ZedGraphControl zgc, string Title, string XAxis, string YAxis, AxisType blog)
        {
            m_cZG = zgc;
            m_cZG.BackColor = Color.Transparent;
            m_cMyPane = zgc.GraphPane;

            // Set the titles and axis labels
            m_cMyPane.Title.Text = Title;
            m_cMyPane.XAxis.Title.Text = XAxis;
            m_cMyPane.YAxis.Title.Text = YAxis;
            m_cMyPane.YAxis.Type = blog;
            m_cMyPane.YAxis.MinorTic.Color = Color.Transparent;

            // Fill the axis background with a color gradient
            m_cMyPane.Chart.Fill = new Fill(Color.White,Color.LightGoldenrodYellow, 45.0F);
        }

       
        //either pass in a zedgraphcontrol from a control, or we'll create our own
        public void CreateGraph(ZedGraphControl zgc)
        {
            if (zgc != null)
                m_cZG = zgc;
            else
                m_cZG = new ZedGraphControl();

            m_cMyPane = m_cZG.GraphPane;
        }

        public void AddMenuItem(ContextMenuStrip menustrip, string name, string tag, string text, EventHandler handler)
        {
            ToolStripMenuItem item = new ToolStripMenuItem();
            item.Name = name;
            item.Tag = tag;
            item.Text = text;

            item.Click += new EventHandler(handler);
            menustrip.Items.Add(item);
        }


        public void RemoveMenuItem(ContextMenuStrip menuStrip, string name)
        {
            ToolStripItem[] items = menuStrip.Items.Find(name, true);
            if (items.Length == 1)
                menuStrip.Items.Remove(items[0]);
        }

        public void GraphContextMenuBuilder(ZedGraphControl.ContextMenuBuilderEventHandler menuevent)
        {
            ZGControl.ContextMenuBuilder += menuevent;
        }

        public void SetAllFonts(string Font, int TitleFontSize, int AxisFontSize)
        {
            m_cMyPane.Title.FontSpec.Family = Font;
            m_cMyPane.Title.FontSpec.Size = TitleFontSize;
            m_cMyPane.Title.FontSpec.IsBold = true;
            m_cMyPane.XAxis.Title.FontSpec.Family = Font;
            m_cMyPane.XAxis.Title.FontSpec.Size = AxisFontSize;
            m_cMyPane.YAxis.Title.FontSpec.IsBold = m_cMyPane.XAxis.Title.FontSpec.IsBold = false;
            m_cMyPane.YAxis.Title.FontSpec.Family = Font;
            m_cMyPane.YAxis.Title.FontSpec.Size = AxisFontSize;
        }

        public void SetAxisTitles(string xaxis, string yaxis)
        {
            if (xaxis != string.Empty)
                m_cMyPane.XAxis.Title.Text = xaxis;
            if (yaxis != string.Empty)
                m_cMyPane.YAxis.Title.Text = yaxis;

            AxisChange();
        }

        #region Get/Set Base Class Variables

        public Size GraphSize
        {
            set{m_cZG.ClientSize = value;}
            get{return m_cZG.ClientSize;}
        }

        public bool LegendState
        {
            get{return m_cMyPane.Legend.IsVisible;}
            set{m_cMyPane.Legend.IsVisible = value;}
        }

        public bool TitleState
        {
            get{return m_cMyPane.Title.IsVisible;}
            set{m_cMyPane.Title.IsVisible = value;}
        }

        public Color ChartFill
        {
            get{return m_cMyPane.Chart.Fill.Color;}
            set{m_cMyPane.Chart.Fill.Color = value;}
        }

        public Color PaneFill
        {
            get{return m_cMyPane.Fill.Color; }
            set{m_cMyPane.Fill.Color = value;}
        }

        public bool BorderState
        {
            get{return m_cMyPane.Border.IsVisible;}
            set{m_cMyPane.Border.IsVisible = value;}
        }

        public string GraphName
        {
            get{return m_sGraphname;}
        }

        public string Title
        {
            get{return m_cMyPane.Title.Text;}
            set{m_cMyPane.Title.Text = value;}
        }

        public GraphPane Pane
        {
            get{return m_cMyPane;}
        }

        protected ZedGraphControl ZGControl
        {
            get{return m_cZG;}
        }

        public MasterPane GraphMasterPane
        {
            get{return m_cZG.MasterPane;}
        }

        public PointF MousePosition
        {
            set{m_cMousePos = value;}
            get{return m_cMousePos;}
        }

        public CurveList GraphCurveList
        {
            get{return m_cMyPane.CurveList;}
        }

        public bool IsDeepCopyFull
        {
            get
            {
                return m_bThisisadeepcopy;
            }
        }

        public bool IsThisaDeepCopy
        {
            get{return m_bThisisadeepcopy;}
            set{m_bThisisadeepcopy = value;}
        }

        public bool Hide
        {
            get { return m_bHide; }
            set { m_bHide = value; }
        }

        public GraphingBase(string name)
        {
            m_alDatainGraph = new List<string>();
            m_sGraphname = name;
        }

        #endregion

        public virtual void DeepCopy(GraphingBase graph)
        {
          
            //this allows the class to be completely self contained on a deep copy. the calling function doesn't need to 
            //create a graph
            if (ZGControl == null)
            {
                CreateGraph(null);
            }

            //Fix if we're updating from a thread
            if (ZGControl.InvokeRequired)
            {
                ZGControl.Invoke(new MethodInvoker(() => DeepCopy(graph)));
                return;
            }

            Title = graph.Title;
            SetAllFonts(graph.Pane.Title.FontSpec.Family, (int)graph.Pane.Title.FontSpec.Size, (int)graph.Pane.XAxis.Title.FontSpec.Size);
            SetAxisTitles(graph.Pane.XAxis.Title.Text, graph.Pane.YAxis.Title.Text);
            Pane.CurveList = graph.Pane.CurveList.Clone();

            var y = Pane.CurveList.Where(curve => curve.IsLine).Cast<LineItem>();

            Pane.CurveList.Where(curve => curve.IsLine).Cast<LineItem>().IndexedForEach((curve, index) =>
                {
                    curve.Color = Color.Black;
                    curve.Symbol.Type = SymbolType.None;
                    curve.Line.Style = (DashStyle)index;
                    curve.Line.IsAntiAlias = true;
                });

            GraphSize = graph.GraphSize;
            BorderState = graph.BorderState;
            Pane.YAxis.Type = graph.Pane.YAxis.Type;
            Pane.YAxis.MinorTic.Color = graph.Pane.YAxis.MinorTic.Color;
            AxisChange();
            Pane.YAxis.Scale.Min = graph.Pane.YAxis.Scale.Min;
            Pane.YAxis.Scale.Max = graph.Pane.YAxis.Scale.Max;
            Pane.XAxis.Scale.Min = graph.Pane.XAxis.Scale.Min;
            Pane.XAxis.Scale.Max = graph.Pane.XAxis.Scale.Max;
            Pane.XAxis.MinorGrid.Color = Color.Transparent;
            Invalidate();
            IsThisaDeepCopy = true;
        }


        //This gives us publication quality graphs
        public void AlterGraph(GraphingBase oldgraph)
        {
            Clear();
            DeepCopy(oldgraph);
            LegendState = true;
            Pane.Legend.IsShowLegendSymbols = false;
            Pane.Legend.Border.IsVisible = false;
            BorderState = false;
            Pane.Title.IsVisible = false;
            Pane.Chart.Fill = new Fill(Color.Transparent);
            Pane.IsPenWidthScaled = true;
            Pane.Fill = new Fill(Color.Transparent);
            Pane.XAxis.Title.FontSpec.IsAntiAlias = true;
            Pane.YAxis.Title.FontSpec.IsAntiAlias = true;
            Pane.XAxis.Scale.FontSpec.IsAntiAlias = true;
            Pane.YAxis.Scale.FontSpec.IsAntiAlias = true;
            Pane.XAxis.Title.FontSpec.Size = 26;
            Pane.YAxis.Title.FontSpec.Size = 26;
            Pane.XAxis.Scale.FontSpec.Size = 18;
            Pane.YAxis.Scale.FontSpec.Size = 18;

            GraphCurveList.Where(curve => curve.IsLine).Cast<LineItem>().ForEach(curve =>
            {
                //This is needed, or else the top numbers of the graph are cut off.
                curve.Label.Text = " ";
                curve.Label.FontSpec = new FontSpec("Garamond", (float)12.0, Color.Transparent, false, false, false);
                curve.Label.FontSpec.Border.IsVisible = false;

                if (curve.Line.Style != DashStyle.Solid)
                {
                    curve.Line.Width += 1;
                }
            });
        }

        public virtual void LoadfromArray(string name, double[] X, double[] Y, Color color, SymbolType symbol, int symbolsize, bool isSmoothed, string tag)
        {
            PointPairList list = new PointPairList(X, Y);
            AddCurvetoGraph(list, name, color, symbol, symbolsize, isSmoothed, tag);
            m_alDatainGraph.Add(name);
        }

        public Bitmap GetImage()
        {
            MemoryStream graphstream = new MemoryStream();
            m_cMyPane.GetImage().Save(graphstream, ImageFormat.Bmp);
            Bitmap graphmap = new Bitmap(graphstream);
            return graphmap;
        }

        public virtual void Clear()
        {
            if (m_cMyPane != null)
            {
                m_cMyPane.CurveList.Clear();
               
                if (m_alDatainGraph != null)
                {
                    m_alDatainGraph.Clear();
                }
            }
            ZGControl.ZoomOutAll(m_cMyPane);
            Invalidate();
        }

        protected void AxisChange()
        {
            m_cZG.AxisChange();
        }

        protected void Invalidate()
        {
            m_cZG.Invalidate();
        }
    
        protected virtual void AddCurvetoGraph(PointPairList list, string DataName, Color linecolor, SymbolType type, int symbolsize, bool isSmoothed,string tag)
        {
            LineItem myCurve = m_cMyPane.AddCurve(DataName, list, linecolor, type);
            myCurve.Symbol.Fill = new Fill(Color.DeepSkyBlue,Color.Red );
            myCurve.Symbol.Fill.Type = FillType.GradientByZ;
            myCurve.Symbol.Fill.RangeMin = 0;
            myCurve.Symbol.Fill.RangeMax = 1;
            myCurve.Symbol.Size = symbolsize;
            myCurve.Line.IsAntiAlias = true;
            myCurve.Tag = tag;

            myCurve.Line.IsSmooth = isSmoothed;

        }

        protected virtual void AddCurvetoGraph(PointPairList list, string DataName, Color linecolor, SymbolType type, int symbolsize, DashStyle style, bool isSmoothed, string tag)
        {
            LineItem myCurve = m_cMyPane.AddCurve(DataName, list, linecolor, type);
            myCurve.Symbol.Fill = new Fill(Color.DeepSkyBlue,Color.Red);
            myCurve.Symbol.Fill.Type = FillType.GradientByZ;
            myCurve.Symbol.Fill.RangeMin = 0;
            myCurve.Symbol.Fill.RangeMax = 1;
            myCurve.Symbol.Size = symbolsize;
            myCurve.Line.Style = style;
            myCurve.Line.IsAntiAlias = true;
            myCurve.Tag = tag;

            myCurve.Line.IsSmooth = isSmoothed;

          
        }

        protected virtual void AddCurvetoGraph(PointPairList list, PointPairList elist, string DataName, Color linecolor, SymbolType type, int symbolsize, string tag)
        {
            LineItem myCurve = m_cMyPane.AddCurve(DataName, list, linecolor, type);
            myCurve.Symbol.Fill = new Fill(Color.DeepSkyBlue,Color.Red);
            myCurve.Symbol.Fill.Type = FillType.GradientByZ;
            myCurve.Symbol.Fill.RangeMin = 0;
            myCurve.Symbol.Fill.RangeMax = 1;
            myCurve.Symbol.Size = symbolsize;
            myCurve.Line.IsAntiAlias = true;
            myCurve.Line.IsSmooth = true;
            myCurve.Tag = tag;

            if (elist != null)
            {
                ErrorBarItem myECurve;
                myECurve = m_cMyPane.AddErrorBar("", elist, Color.Black);
                myECurve.Bar.PenWidth = 1f;
                // Use the HDash symbol so that the error bars look like I-beams

                myECurve.Bar.Symbol.Type = SymbolType.HDash;
               /// myECurve.Bar.Symbol.Border.Width = .1f;
                myECurve.Bar.Symbol.IsVisible = true;
                myECurve.Bar.Symbol.Size = 2;
            }

         
        }

        public void RemoveGraphfromArray(string name)
        {
            int indexer;
            indexer = m_alDatainGraph.FindIndex(p => p == name);

            if (indexer >= 0)
            {
                m_cMyPane.CurveList.RemoveAt(indexer);
                m_alDatainGraph.RemoveAt(indexer);
            }
        }

        public void CopyMetatoClip(object sender, System.EventArgs e)
        {
            ZGControl.CopyEmf(false);
        }

        public void CopyLocalGraph(object sender, System.EventArgs e)
        {
            ZGControl.CopyEmf(false, m_cMousePos);
        }

        public void SaveLocalEMFFile(object sender, System.EventArgs e)
        {
            ZGControl.SaveAsEmf((PointF?)m_cMousePos);
        }

        public void SaveEMFFile(object sender, System.EventArgs e)
        {
            ZGControl.SaveAsEmf(m_cMousePos);
        }
    }
}