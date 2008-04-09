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
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Drawing.Drawing2D;

namespace StochasticModeling
{

    //This class is necessary until Zedgraph cleans up bugs in current versions. Last working version is 4.1.1
    public class ClipboardMetafileHelper
    {
        [DllImport("user32.dll")]
        static extern bool OpenClipboard(IntPtr hWndNewOwner);
        [DllImport("user32.dll")]
        static extern bool EmptyClipboard();
        [DllImport("user32.dll")]
        static extern IntPtr SetClipboardData(uint uFormat, IntPtr hMem);
        [DllImport("user32.dll")]
        static extern bool CloseClipboard();
        [DllImport("gdi32.dll")]
        static extern IntPtr CopyEnhMetaFile(IntPtr hemfSrc, StringBuilder hNULL);
        [DllImport("gdi32.dll")]
        static extern bool DeleteEnhMetaFile(IntPtr hemf);

        static public bool SaveEnhMetafileToFile(Metafile mf)
        {
            if (mf == null)
                return false;

            bool bResult = false;
            IntPtr hEMF;
            hEMF = mf.GetHenhmetafile(); // invalidates mf

            if (!hEMF.Equals(new IntPtr(0)))
            {
                SaveFileDialog sfd = new SaveFileDialog();
                sfd.Filter = "Extended Metafile (*.emf)|*.emf";
                sfd.DefaultExt = ".emf";
                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    StringBuilder temp = new StringBuilder(sfd.FileName);
                    CopyEnhMetaFile(hEMF, temp);
                }
                DeleteEnhMetaFile(hEMF);
            }
            return bResult;
        }
        // Metafile mf is set to a state that is not valid inside this function.
        static public bool PutEnhMetafileOnClipboard(Metafile mf)
        {

            if (mf == null)
                return false;

            bool bResult = false;
            IntPtr hEMF, hEMF2;

            hEMF = mf.GetHenhmetafile(); // invalidates mf
            if (!hEMF.Equals(new IntPtr(0)))
            {
                hEMF2 = CopyEnhMetaFile(hEMF, null);
                if (!hEMF2.Equals(new IntPtr(0)))
                {
                    if (OpenClipboard(IntPtr.Zero))
                    {
                        if (EmptyClipboard())
                        {
                            IntPtr hRes = SetClipboardData(14 /*CF_ENHMETAFILE*/, hEMF2);
                            bResult = hRes.Equals(hEMF2);
                            CloseClipboard();
                        }
                    }
                }
                DeleteEnhMetaFile(hEMF);
            }
            return bResult;
        }

        static public Metafile GetMetafile(bool copylocal, bool masterpane, ZedGraphControl zg, PointF MousePos)
        {
            try
            {
                Graphics g = zg.CreateGraphics();
                IntPtr hdc = g.GetHdc();
                Metafile metaFile = new Metafile(hdc, EmfType.EmfPlusOnly);
                g.ReleaseHdc(hdc);
                g.Dispose();

                Graphics gMeta = Graphics.FromImage(metaFile);

                if (masterpane == true)
                    if (copylocal)
                        zg.MasterPane.FindPane(MousePos).Draw(gMeta);
                    else
                        zg.MasterPane.Draw(gMeta);
                else
                    zg.GraphPane.Draw(gMeta);

                gMeta.Dispose();

                return metaFile;
            }
            catch
            {
                MessageBox.Show("Could not capture image. Please place the cursor over a graph.");
                return null;
            }
        }
    }


/// <summary>
/// Base class for using the ZedGraph graphing control for a scatter plot. While this class can be extended, it also
/// functions
/// </summary>
    public class GraphingBase
    {
        private ZedGraphControl m_cZG;
        protected ArrayList m_alDatainGraph;
        private GraphPane m_cMyPane;
        protected PointF m_cMousePos = PointF.Empty;
        private bool m_bThisisadeepcopy = false;
        private string m_sGraphname;

        public GraphingBase(string name)
        {
            m_alDatainGraph = new ArrayList();
            m_sGraphname = name;
        }

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
            m_cMyPane.Chart.Fill = new Fill(Color.White,
            Color.LightGoldenrodYellow, 45.0F);
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

        
        //public void SetSize(Rectangle rect)
        //{
        //    //Resizing of these controls can be difficult. It is better to handle it from the GUI side
        //    // zg.Location = new Point( 10, 10 );
        //    //// Leave a small margin around the outside of the control
        //    // zg.Size = new Size(rect.Width - 300, rect.Height - 300);
        //}

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



        #region Get/Set Base Class Variables

        public Size GraphSize
        {
            set
            {
                m_cZG.ClientSize = value;
            }
            get
            {
                return m_cZG.ClientSize;
            }
        }

        public bool LegendState
        {
            get
            {
                return m_cMyPane.Legend.IsVisible;
            }
            set
            {
                m_cMyPane.Legend.IsVisible = value;
            }
        }

        public bool TitleState
        {
            get
            {
                return m_cMyPane.Title.IsVisible;
            }
            set
            {
                m_cMyPane.Title.IsVisible = value;
            }
        }

        public Color ChartFill
        {
            get
            {
                return m_cMyPane.Chart.Fill.Color;

            }
            set
            {
                m_cMyPane.Chart.Fill.Color = value;
            }
        }

        public Color PaneFill
        {
            get
            {

                return m_cMyPane.Fill.Color;
            }
            set
            {
                m_cMyPane.Fill.Color = value;

            }
        }

        public bool BorderState
        {
            get
            {
                return m_cMyPane.Border.IsVisible;
            }
            set
            {
                m_cMyPane.Border.IsVisible = value;
            }
        }

        public string GraphName
        {
            get
            {
                return m_sGraphname;
            }
        }

        public string Title
        {
            get
            {
                return m_cMyPane.Title.Text;
            }
            set
            {
                m_cMyPane.Title.Text = value;
            }
        }

        public GraphPane Pane
        {
            get
            {
                return m_cMyPane;
            }
        }

        protected ZedGraphControl ZGControl
        {
            get
            {
                return m_cZG;
            }
        }

        public MasterPane GraphMasterPane
        {
            get
            {
                return m_cZG.MasterPane;
            }
        }

        public PointF MousePosition
        {
            set
            {
                m_cMousePos = value;
            }
            get
            {
                return m_cMousePos;
            }
        }

        public CurveList GraphCurveList
        {
            get
            {
                return m_cMyPane.CurveList;
            }
        }

        public void SetAxisTitles(string xaxis, string yaxis)
        {
            if (xaxis != string.Empty)
                m_cMyPane.XAxis.Title.Text = xaxis;
            if (yaxis != string.Empty)
                m_cMyPane.YAxis.Title.Text = yaxis;

            AxisChange();
        }

        public bool IsDeepCopyFull
        {
            get
            {
                if (m_alDatainGraph.Count == 0 && m_bThisisadeepcopy == false)
                    return false;
                else
                    return true;
            }
        }

        public bool IsThisaDeepCopy
        {
            get
            {
                return m_bThisisadeepcopy;
            }
            set
            {
                m_bThisisadeepcopy = value;
            }
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

            Title = graph.Title;
            SetAllFonts(graph.Pane.Title.FontSpec.Family, (int)graph.Pane.Title.FontSpec.Size, (int)graph.Pane.XAxis.Title.FontSpec.Size);
            SetAxisTitles(graph.Pane.XAxis.Title.Text, graph.Pane.YAxis.Title.Text);
            Pane.CurveList = graph.Pane.CurveList.Clone();
            
            for (int i = 0; i < Pane.CurveList.Count; i++)
            {
                if (GraphCurveList[i].IsLine == true)
                {
                    ((LineItem)(Pane.CurveList[i])).Color = Color.Black;//((LineItem)(graph.myPane.CurveList[i])).Color;

                    ((LineItem)(Pane.CurveList[i])).Symbol.Type = SymbolType.None;
                    ((LineItem)(Pane.CurveList[i])).Line.IsAntiAlias = ((LineItem)(graph.GraphCurveList[i])).Line.IsAntiAlias;
                    ((LineItem)(Pane.CurveList[i])).Line.Style = (DashStyle)i;
                }
            }

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

            for (int i = 0; i < GraphCurveList.Count; i++)
            {
                if (GraphCurveList[i].IsLine == true)
                {
                    //This is needed, or else the top numbers of the graph are cut off.
                    GraphCurveList[i].Label.Text = " ";
                    GraphCurveList[i].Label.FontSpec = new FontSpec("Garamond", (float)12.0, Color.Transparent,false,false,false);
                    GraphCurveList[i].Label.FontSpec.Border.IsVisible = false;

                    if (((LineItem)GraphCurveList[i]).Line.Style != 0)
                        ((LineItem)GraphCurveList[i]).Line.Width += 1;
                }
            }
        }

        public void Copy(GraphingBase graph)
        {
            m_cMyPane = graph.m_cMyPane.Clone();
            m_bThisisadeepcopy = false;
        }

        public virtual void LoadfromArray(string name, double[] X, double[] Y, Color color, SymbolType symbol, int symbolsize, bool isSmoothed, string tag)
        {
            PointPairList list = new PointPairList();
            for (int i = 0; i < X.Length; i++)
            {
                    list.Add(X[i] , Y[i]);
            }
            
            AddCurvetoGraph(list, name, color, symbol, symbolsize, isSmoothed, tag);
            m_alDatainGraph.Add(name);
        }

        public virtual void LoadfromArray(string name, double[] X, double[] Y, Color color, SymbolType symbol, int symbolsize, DashStyle style, bool isSmoothed, string tag)
        {
            PointPairList list = new PointPairList();
            for (int i = 0; i < X.Length; i++)
            {
                list.Add(X[i], Y[i]);
            }

            AddCurvetoGraph(list, name, color, symbol, symbolsize, style, isSmoothed,tag);
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
                while (m_cMyPane.CurveList.Count != 0)
                {
                    CurveItem curve = m_cMyPane.CurveList[0] as CurveItem;

                    if (curve == null)
                        return;

                    m_cMyPane.CurveList.Remove(curve);
                }

                m_cMyPane.CurveList.TrimExcess();

                if (m_alDatainGraph != null)
                {
                    m_alDatainGraph.Clear();
                    m_alDatainGraph.TrimToSize();
                }
            }
            ZGControl.ZoomOutAll(m_cMyPane);
        }

        protected void AxisChange()
        {
            m_cZG.AxisChange();
        }

        protected void Invalidate()
        {
            m_cZG.Invalidate();
        }
        
        public bool DataLoadedtoGraph(string filename)
        {
            for (int i = 0; i < m_alDatainGraph.Count; i++)
            {
                if (filename == (string)m_alDatainGraph[i])
                    return true;

            }

            return false;
        }


        protected virtual void GetPointList(double[] X, double[] Y, ref PointPairList List)
        {
            for (int i = 0; i < X.Length; i++)
            {
                List.Add(X[i], Y[i]);
            }
        }

        protected virtual void GetPointList(ArrayList X, ArrayList Y, ref PointPairList List) 
        {
            for (int i = 0; i < X.Count; i++)
            {
                List.Add((double)X[i], (double)Y[i]);
            }
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

            if (isSmoothed == true)
                myCurve.Line.IsSmooth = true;

            AxisChange();
            Invalidate();
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

            if (isSmoothed == true)
                myCurve.Line.IsSmooth = true;

            AxisChange();
            Invalidate();
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
                myECurve.Bar.Symbol.Border.Width = .1f;
                myECurve.Bar.Symbol.IsVisible = true;
                myECurve.Bar.Symbol.Size = 1;
            }

            AxisChange();
            Invalidate();
        }

        public void RemoveGraphfromArray(string name)
        {
            int index = -1;

            for (int i = 0; i < m_alDatainGraph.Count; i++)
            {
                if (name == (string)m_alDatainGraph[i])
                {
                    index = i;
                    break;
                }
            }

            if (index < 0)
                return;
            else
            {
                CurveItem curve = m_cMyPane.CurveList[index] as CurveItem;

                if (curve == null)
                    return;

                m_alDatainGraph.RemoveAt(index);
                m_alDatainGraph.TrimToSize();
                m_cMyPane.CurveList.Remove(curve);
                m_cMyPane.CurveList.TrimExcess();
            }
        }

        public void CopyMetatoClip(object sender, System.EventArgs e)
        {
            ClipboardMetafileHelper.PutEnhMetafileOnClipboard(ClipboardMetafileHelper.GetMetafile(false, true, m_cZG, m_cMousePos));
        }

        public void CopyLocalGraph(object sender, System.EventArgs e)
        {
            ClipboardMetafileHelper.PutEnhMetafileOnClipboard(ClipboardMetafileHelper.GetMetafile(true, true, m_cZG, m_cMousePos));
        }

        public void SaveLocalEMFFile(object sender, System.EventArgs e)
        {
            ClipboardMetafileHelper.SaveEnhMetafileToFile(ClipboardMetafileHelper.GetMetafile(true, true, m_cZG, m_cMousePos));
        }

        public void SaveEMFFile(object sender, System.EventArgs e)
        {
            ClipboardMetafileHelper.SaveEnhMetafileToFile(ClipboardMetafileHelper.GetMetafile(false, true, m_cZG, m_cMousePos));
        }
    }
}