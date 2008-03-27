using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Drawing.Imaging;
using ZedGraph;
using System.Collections;
using System.Windows.Forms;
using System.Drawing;
using System.IO;
using System.Text.RegularExpressions;
using System.Drawing.Drawing2D;

namespace GIDFit
{
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
        static public bool PutEnhMetafileOnClipboard(IntPtr hWnd, Metafile mf)
        {

            bool bResult = false;
            IntPtr hEMF, hEMF2;

            hEMF = mf.GetHenhmetafile(); // invalidates mf
            if (!hEMF.Equals(new IntPtr(0)))
            {
                hEMF2 = CopyEnhMetaFile(hEMF, null);
                if (!hEMF2.Equals(new IntPtr(0)))
                {
                    if (OpenClipboard(hWnd))
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
    }

    public class Graphing
    {
        public GraphPane myPane;
        public ZedGraphControl zg;
        ArrayList filesinGraph;
        ErrorBarItem myECurve;
        public double SLD = 0;
        public double SupSLD = 0;
        public double lambda = 1.0;
        public bool DBF = false;
        public bool negativeerrorval = false;
        bool legendon = true;
        bool borderon = true;
        bool thisisashallowcopy = false;
        bool titleon = false;
        bool iszoomed = false;
        string datafilename;
        public bool QSquaredCorrection = false;

        public Graphing()
        {
            filesinGraph = new ArrayList();
        }

        private void ZoomEvent(ZedGraphControl sender, ZoomState oldState, ZoomState newState)
        {
            if (negativeerrorval == true)
            {
                if (DBF == true)
                {
                    if (sender.GraphPane.YAxis.Scale.Min == 3e-4)
                    {
                        iszoomed = false;
                        return;
                    }
                }
                else
                {
                    if (sender.GraphPane.YAxis.Scale.Min == 1e-11)
                    {
                        iszoomed = false;
                        return;
                    }
                }
                iszoomed = true;
            }
        }

        public void CreateGraph(ZedGraphControl zgc, string Title, string XAxis, string YAxis, AxisType blog)
        {
            zg = zgc;
            myPane = zgc.GraphPane;

            // Set the titles and axis labels
            myPane.Title.Text = Title;
            myPane.XAxis.Title.Text = XAxis;
            myPane.YAxis.Title.Text = YAxis;
            myPane.YAxis.Type = blog;
            myPane.YAxis.MinorTic.Color = Color.Transparent;

            // Fill the axis background with a color gradient
            myPane.Chart.Fill = new Fill(Color.White,
            Color.LightGoldenrodYellow, 45.0F);
            zgc.ZoomEvent += new ZedGraphControl.ZoomEventHandler(ZoomEvent);
            zg.ContextMenuBuilder += new ZedGraphControl.ContextMenuBuilderEventHandler(MyContextMenuBuilder);
        }

        public void CreateGraph(ZedGraphControl zgc)
        {
            zg = zgc;
            myPane = zgc.GraphPane;

           // Fill the axis background with a color gradient
            myPane.Chart.Fill = new Fill(Color.White,
            Color.LightGoldenrodYellow, 45.0F);
            zgc.ZoomEvent += new ZedGraphControl.ZoomEventHandler(ZoomEvent);
            zg.ContextMenuBuilder += new ZedGraphControl.ContextMenuBuilderEventHandler(MyContextMenuBuilder);
        }

        private void MyContextMenuBuilder(ZedGraphControl control, ContextMenuStrip menuStrip, Point mousePt, ZedGraphControl.ContextMenuObjectState objState)
        {
            // create a new menu item
            ToolStripMenuItem item = new ToolStripMenuItem();
            ToolStripMenuItem saveemf = new ToolStripMenuItem();

            // This is the user-defined Tag so you can find this menu item later if necessary
            item.Name = "ClipCopy_tag";
            item.Tag = "ClipCopy_tag";
            saveemf.Name = "SaveEMF_tag";
            saveemf.Tag = "SaveEMF_tag";
            // This is the text that will show up in the menu
            item.Text = "Hi Quality Copy";
            saveemf.Text = "Save Hi Quality Copy";

            // Add a handler that will respond when that menu item is selected
            saveemf.Click += new System.EventHandler(SaveEMFFile);
            item.Click += new System.EventHandler(CopyMetatoClip);

            // Add the menu item to the menu
            menuStrip.Items.Add(item);
            menuStrip.Items.Add(saveemf);

            ToolStripItem[] items = menuStrip.Items.Find("copy", true);
            if (items.Length == 1)
            {
                menuStrip.Items.Remove(items[0]);
            }

            items = menuStrip.Items.Find("set_default", true);
            if (items.Length == 1)
            {
                menuStrip.Items.Remove(items[0]);
            }
        }


        protected void CopyMetatoClip(object sender, System.EventArgs e)
        {
            Graphics g = zg.CreateGraphics();
            IntPtr hdc = g.GetHdc();
            Metafile metaFile = new Metafile(hdc, EmfType.EmfPlusOnly);
            g.ReleaseHdc(hdc);
            g.Dispose();

            Graphics gMeta = Graphics.FromImage(metaFile);
            myPane.Draw(gMeta);

            gMeta.Dispose();

            ClipboardMetafileHelper.PutEnhMetafileOnClipboard(zg.Handle, metaFile);
        }

        protected void SaveEMFFile(object sender, System.EventArgs e)
        {
            Graphics g = zg.CreateGraphics();
            IntPtr hdc = g.GetHdc();

            Metafile metaFile = new Metafile(hdc, EmfType.EmfPlusOnly);
            g.ReleaseHdc(hdc);
            g.Dispose();

            Graphics gMeta = Graphics.FromImage(metaFile);
            myPane.Draw(gMeta);
            gMeta.Dispose();

            ClipboardMetafileHelper.SaveEnhMetafileToFile(metaFile);
        }

        public bool IsShallowCopyFull
        {
            get
            {
                if (filesinGraph.Count == 0 && thisisashallowcopy == false)
                    return false;
                else
                    return true;
            }
        }

    
        public void Copy(Graphing graph)
        {
            myPane = graph.myPane.Clone();
            thisisashallowcopy = true;
        }

        public void DeepCopy(Graphing graph)
        {
            SetFont(graph.myPane.Title.FontSpec.Family, (int)graph.myPane.Title.FontSpec.Size, (int)graph.myPane.XAxis.Title.FontSpec.Size);
            SetAxisTitle(graph.myPane.XAxis.Title.Text, graph.myPane.YAxis.Title.Text);
            myPane.CurveList = graph.myPane.CurveList.Clone();
            for (int i = 0; i < myPane.CurveList.Count; i++)
            {
                if (myPane.CurveList[i].IsLine == true)
                {
                    ((LineItem)(myPane.CurveList[i])).Color = Color.Black;
                    ((LineItem)(myPane.CurveList[i])).Symbol.Fill = new Fill(Color.Black);
                    ((LineItem)(myPane.CurveList[i])).Line.IsAntiAlias = ((LineItem)(graph.myPane.CurveList[i])).Line.IsAntiAlias;
                    ((LineItem)(myPane.CurveList[i])).Line.Style = ((LineItem)(graph.myPane.CurveList[i])).Line.Style;
                }
            }


            myPane.YAxis.Type = graph.myPane.YAxis.Type;
            myPane.YAxis.MinorTic.Color = graph.myPane.YAxis.MinorTic.Color;
            zg.AxisChange();
            myPane.YAxis.Scale.Min = graph.myPane.YAxis.Scale.Min;
            myPane.YAxis.Scale.Max = graph.myPane.YAxis.Scale.Max;
            myPane.XAxis.Scale.Min = graph.myPane.XAxis.Scale.Min;
            myPane.XAxis.Scale.Max = graph.myPane.XAxis.Scale.Max;
            myPane.XAxis.MinorGrid.Color = Color.Transparent;
            myPane.Rect = graph.myPane.Rect;
            zg.Invalidate();
            thisisashallowcopy = true;
        }

        public void SetFont(string Font, int TitleFontSize, int AxisFontSize)
        {
            myPane.Title.FontSpec.Family = Font;
            myPane.Title.FontSpec.Size = TitleFontSize;
            myPane.Title.FontSpec.IsBold = true;
            myPane.XAxis.Title.FontSpec.Family = Font;
            myPane.XAxis.Title.FontSpec.Size = AxisFontSize;
            myPane.XAxis.Title.FontSpec.IsBold = false;
            myPane.YAxis.Title.FontSpec.Family = Font;
            myPane.YAxis.Title.FontSpec.Size = AxisFontSize;
            myPane.YAxis.Title.FontSpec.IsBold = false;
        }

        void BorderOnOff(bool onoff)
        {
            myPane.Border.IsVisible = onoff;
            borderon = onoff;
        }
        void LegendOnOff(bool onoff)
        {
            myPane.Legend.IsVisible = onoff;
            legendon = onoff;
        }
        void TitleOnOff(bool onoff)
        {
            myPane.Title.IsVisible = onoff;
            titleon = onoff;
        }

        public bool TitleState
        {
            get
            {
                return titleon;
            }
            set
            {
                TitleOnOff(value);
            }
        }

        public bool LegendState
        {
            get
            {
                return legendon;
            }
            set
            {
                LegendOnOff(value);
            }
        }

        public bool BorderState
        {
            get
            {
                return borderon;
            }
            set
            {
                BorderOnOff(value);
            }
        }

        public GraphPane Pane
        {
            get
            {
                return myPane;
            }
        }

        public Bitmap GetImage()
        {
            MemoryStream graphstream = new MemoryStream();
            myPane.GetImage().Save(graphstream, ImageFormat.Bmp);
            Bitmap graphmap = new Bitmap(graphstream);
            return graphmap;
        }

        public void SetSize(Rectangle rect)
        {
            // zg.Location = new Point( 10, 10 );
            //// Leave a small margin around the outside of the control
            // zg.Size = new Size(rect.Width - 300, rect.Height - 300);
        }

        public void SetAxisTitle(string xaxis, string yaxis)
        {
            if (xaxis != string.Empty)
                myPane.XAxis.Title.Text = xaxis;
            if (yaxis != string.Empty)
                myPane.YAxis.Title.Text = yaxis;
        }

        public void SetTitle(string title)
        {
            myPane.Title.Text = title;
        }
        public void AddCurvetoGraph(PointPairList list, PointPairList elist, string DataName, Color linecolor, SymbolType type, int symbolsize)
        {
            LineItem myCurve = myPane.AddCurve(DataName, list, linecolor, type);
            myCurve.Symbol.Fill = new Fill(Color.Red);
            myCurve.Symbol.Size = symbolsize;
            myCurve.Line.IsAntiAlias = true;
            myCurve.Line.IsSmooth = true;

            if (elist != null)
            {
                myECurve = myPane.AddErrorBar("", elist, Color.Black);
                myECurve.Bar.PenWidth = 1f;
                // Use the HDash symbol so that the error bars look like I-beams
                myECurve.Bar.Symbol.Type = SymbolType.HDash;
                myECurve.Bar.Symbol.Border.Width = .1f;
                myECurve.Bar.Symbol.IsVisible = true;
                myECurve.Bar.Symbol.Size = 1;
            }

            //We have to manually set the error bars if we have a negative value due to error
            if (negativeerrorval == false)
                zg.AxisChange();
            else
            {
                zg.AxisChange();
                if (iszoomed == false)
                {
                    if (DBF == true)
                        myPane.YAxis.Scale.Min = 3e-4;
                    else
                        myPane.YAxis.Scale.Min = 1e-11;
                }
            }
            zg.Invalidate();
        }

        public void AddCurvetoGraph(PointPairList list, string DataName, Color linecolor, SymbolType type, int symbolsize, bool isSmoothed)
        {
            LineItem myCurve = myPane.AddCurve(DataName, list, linecolor, type);
            myCurve.Symbol.Fill = new Fill(Color.Red);
            myCurve.Symbol.Size = symbolsize;
            myCurve.Line.IsAntiAlias = true;
            
            if (isSmoothed == true)
                myCurve.Line.IsSmooth = true;

            if (negativeerrorval == false)
                zg.AxisChange();
            else
            {
                zg.AxisChange();
                if (iszoomed == false)
                {
                    if (DBF == true)
                        myPane.YAxis.Scale.Min = 3e-4;
                    else
                        myPane.YAxis.Scale.Min = 1e-11;
                }
            }
            zg.Invalidate();
        }

        public void AddCurvetoGraph(PointPairList list, string DataName, Color linecolor, SymbolType type, int symbolsize, DashStyle style, bool isSmoothed)
        {
            LineItem myCurve = myPane.AddCurve(DataName, list, linecolor, type);
            myCurve.Symbol.Fill = new Fill(Color.Red);
            myCurve.Symbol.Size = symbolsize;
            myCurve.Line.IsAntiAlias = true;
            myCurve.Line.Style = style;

            if (isSmoothed == true)
                myCurve.Line.IsSmooth = true;



            if (negativeerrorval == false)
                zg.AxisChange();
            else
            {
                zg.AxisChange();
                if (iszoomed == false)
                {
                    if (DBF == true)
                        myPane.YAxis.Scale.Min = 3e-4;
                    else
                        myPane.YAxis.Scale.Min = 1e-11;
                }
            }
            zg.Invalidate();
        }

        static public double CalcQc(double dSLD, double SupSLD, double lambda)
        {
            if (dSLD - SupSLD > 0)
                return 4 * Math.Sqrt((3.14159265 * (dSLD - SupSLD) * 1e-6) / (1 - (lambda * lambda / 2 * 3.14159265) * SupSLD * 1e-6));
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

        public void LoadDataFiletoGraph(string datafile, string name, Color color, SymbolType symbol, int symbolsize)
        {
            double Qc = CalcQc(SLD, SupSLD, lambda);
            if (datafile != string.Empty && (filesinGraph.BinarySearch(datafile) <= 0))
            {
                PointPairList list = new PointPairList();
                PointPairList elist = new PointPairList();

                using (StreamReader sr = new StreamReader(datafile))
                {
                    String dataline;
                    double Q;
                    double Refl;
                    double Reflerror;

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

                        if (DBF == false)
                        {
                            Q = Double.Parse((string)datastring[0]);
                            if (QSquaredCorrection == false)
                            {
                                Refl = Double.Parse((string)datastring[1]);
                                Reflerror = Double.Parse((string)datastring[2]);
                            }
                            else
                            {
                                Refl = Double.Parse((string)datastring[1]) * Q * Q;
                                Reflerror = Double.Parse((string)datastring[2]) * Q * Q;
                            }
                        }
                        else
                        {
                            Q = Double.Parse((string)datastring[0]);
                            if (Qc > .005)
                            {
                                Refl = Double.Parse((string)datastring[1]) / CalcFresnelPoint(Q, Qc);
                                Reflerror = Double.Parse((string)datastring[2]) / CalcFresnelPoint(Q, Qc);
                                Q = Q / Qc;
                                SetAxisTitle("Q/Qc", "Intensity");
                            }
                            else
                            {
                                Refl = Double.Parse((string)datastring[1]) * Math.Pow(Q, 4.0);
                                Reflerror = Double.Parse((string)datastring[2]) * Math.Pow(Q, 4.0);
                                SetAxisTitle("Q", "Intensity (RQ^4)");
                            }
                        }
                        //Add data to the points

                        list.Add(Q, Refl);

                        //Account for negative numbers in the error list
                        Double negerror = Refl - Reflerror;
                        if (negerror < 0)
                        {
                            elist.Add(Q, Refl + Reflerror, 1E-15);
                            //At least one negative point we need to consider for plotting
                            negativeerrorval = true;
                        }
                        else
                            elist.Add(Q, Refl + Reflerror, Refl - Reflerror);
                    }

                    datafilename = datafile;
                    AddCurvetoGraph(list, elist, name, color, symbol, symbolsize);
                    filesinGraph.Add(datafile);
                    //To account for the error file
                    filesinGraph.Add(datafile);
                }
            }
        }

        public void LoadfromArray(string name, ArrayList X, ArrayList Y, Color color, SymbolType symbol, int symbolsize, bool isSmoothed)
        {
            PointPairList list = new PointPairList();
            if (DBF)
            {
                double Qc = CalcQc(SLD, SupSLD, lambda);
                for (int i = 0; i < X.Count; i++)
                {
                    if (Qc > .005)
                        list.Add((double)X[i] / Qc, (double)Y[i] / CalcFresnelPoint((double)X[i], Qc));
                    else
                        list.Add((double)X[i] / Qc, (double)Y[i] * Math.Pow((double)X[i], 4.0));
                }
            }
            else
            {
                for (int i = 0; i < X.Count; i++)
                {
                    list.Add((double)X[i], (double)Y[i]);
                }
            }
            AddCurvetoGraph(list, name, color, symbol, symbolsize, isSmoothed);
            filesinGraph.Add(name);
        }

        public void RemoveGraphfromArray(string name)
        {
            int index = filesinGraph.BinarySearch(name);

            if (index < 0)
                return;
            else
            {
                CurveItem curve = myPane.CurveList[index] as CurveItem;

                if (curve == null)
                    return;

                filesinGraph.RemoveAt(index);
                filesinGraph.TrimToSize();
                myPane.CurveList.Remove(curve);
                myPane.CurveList.TrimExcess();
            }
        }

        public bool UpdateGraphfromArray(string name, ArrayList X, ArrayList Y, Color color, SymbolType symbol, int symbolsize, bool isSmoothed)
        {
            int index = filesinGraph.BinarySearch(name);

            if (index < 0)
                return false;
            else
            {
                CurveItem curve = myPane.CurveList[index] as CurveItem;

                if (curve == null)
                    return false;

                filesinGraph.RemoveAt(index);
                filesinGraph.TrimToSize();
                myPane.CurveList.Remove(curve);
                myPane.CurveList.TrimExcess();
                PointPairList list = new PointPairList();
                GetPointList(X, Y, ref list);
                AddCurvetoGraph(list, name, color, symbol, symbolsize, isSmoothed);
                zg.Invalidate();
                filesinGraph.Add(name);

                return true;
            }
        }

        public void LoadfromArray(string name, double[] X, double[] Y, Color color, SymbolType symbol, int symbolsize, bool isSmoothed)
        {
            PointPairList list = new PointPairList();
            if (DBF)
            {
                double Qc = CalcQc(SLD, SupSLD, lambda);
                for (int i = 0; i < X.Length; i++)
                {
                    if (Qc > 0.005)
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
            AddCurvetoGraph(list, name, color, symbol, symbolsize, isSmoothed);
            filesinGraph.Add(name);
        }

        public void LoadfromArray(string name, double[] X, double[] Y, Color color, SymbolType symbol, int symbolsize, DashStyle style, bool isSmoothed)
        {
            PointPairList list = new PointPairList();
            if (DBF)
            {
                double Qc = CalcQc(SLD, SupSLD, lambda);
                for (int i = 0; i < X.Length; i++)
                {
                    if (Qc > 0.005)
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
            AddCurvetoGraph(list, name, color, symbol, symbolsize,style, isSmoothed);
            filesinGraph.Add(name);
        }

        public bool UpdateGraphfromArray(string name, double[] X, double[] Y, Color color, SymbolType symbol, int symbolsize, bool isSmoothed)
        {
            int index = filesinGraph.BinarySearch(name);

            if (index < 0)
                return false;
            else
            {
                CurveItem curve = myPane.CurveList[index] as CurveItem;

                if (curve == null)
                    return false;

                filesinGraph.RemoveAt(index);
                filesinGraph.TrimToSize();
                myPane.CurveList.Remove(curve);
                myPane.CurveList.TrimExcess();

                PointPairList list = new PointPairList();
                GetPointList(X, Y, ref list);
                AddCurvetoGraph(list, name, color, symbol, symbolsize, isSmoothed);
                zg.Invalidate();
                filesinGraph.Add(name);

                return true;
            }
        }

        public void LoadFiletoGraph(string datafile, string plotname, Color color, SymbolType symbol, int symbolsize, bool isSmoothed)
        {
            try
            {
                if (datafile != string.Empty && (filesinGraph.BinarySearch(datafile) <= 0))
                {
                    PointPairList list = new PointPairList();
                    double Qc = CalcQc(SLD, SupSLD, lambda);
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

                            if (DBF == false)
                            {
                                Q = Double.Parse((string)datastring[0]);
                                Refl = Double.Parse((string)datastring[1]);
                            }
                            else
                            {

                                Q = Double.Parse((string)datastring[0]);
                                if (Qc > .005)
                                {
                                    Refl = Double.Parse((string)datastring[1]) / CalcFresnelPoint(Q, Qc);
                                    Q = Q / Qc;
                                }
                                else
                                {
                                    Refl = Double.Parse((string)datastring[1]) * Math.Pow(Q, 4.0);
                                }
                            }
                            list.Add(Q, Refl);
                        }
                        AddCurvetoGraph(list, plotname, color, symbol, symbolsize, isSmoothed);
                        filesinGraph.Add(datafile);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public void LoadFiletoGraph(string datafile, string filename, string plotname, Color color, SymbolType symbol, int symbolsize, bool isSmoothed)
        {
            try
            {
                if (datafile != string.Empty && (filesinGraph.BinarySearch(datafile) <= 0))
                {
                    PointPairList list = new PointPairList();
                    double Qc = CalcQc(SLD, SupSLD, lambda);
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

                            Q = Double.Parse((string)datastring[0]);

                            if (DBF == false)
                            {
                                Refl = Double.Parse((string)datastring[1]);
                            }
                            else
                            {
                                if (Qc > .005)
                                {
                                    Refl = Double.Parse((string)datastring[1]) / CalcFresnelPoint(Q, Qc);
                                    Q = Q / Qc;
                                }
                                else
                                {
                                    Refl = Double.Parse((string)datastring[1]) * Math.Pow(Q, 4.0);
                                }
                            }

                            list.Add(Q, Refl);
                        }
                        AddCurvetoGraph(list, plotname, color, symbol, symbolsize, isSmoothed);
                        filesinGraph.Add(filename);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public bool FileLoadedtoGraph(string filename)
        {
            if (filesinGraph.BinarySearch(filename) >= 0)
                return true;

            return false;
        }

        public void Clear()
        {
            if (myPane != null)
            {
                while (myPane.CurveList.Count != 0)
                {
                    CurveItem curve = myPane.CurveList[0] as CurveItem;

                    if (curve == null)
                        return;

                    myPane.CurveList.Remove(curve);
                }
                myPane.CurveList.TrimExcess();
                filesinGraph.Clear();
                filesinGraph.TrimToSize();
            }
        }

        public void GetPointList(ArrayList X, ArrayList Y, ref PointPairList List)
        {
            double Qc = CalcQc(SLD, SupSLD, lambda);
            for (int jj = 0; jj < X.Count; jj++)
            {
                if (DBF)
                {
                    if (Qc > .005)
                        List.Add((double)X[jj] / Qc, (double)Y[jj] / CalcFresnelPoint((double)X[jj], Qc));
                    else
                        List.Add((double)X[jj], (double)Y[jj] * Math.Pow((double)X[jj], 4.0));
                }
                else
                {
                    List.Add((double)X[jj], (double)Y[jj]);
                }
            }
        }

        public void GetPointList(double[] X, double[] Y, ref PointPairList List)
        {
            double Qc = CalcQc(SLD, SupSLD, lambda);
            for (int jj = 0; jj < X.Length; jj++)
            {
                if (DBF)
                {
                    if (Qc > .005)
                        List.Add(X[jj] / Qc, Y[jj] / CalcFresnelPoint(X[jj], Qc));
                    else
                        List.Add(X[jj], Y[jj] * Math.Pow(X[jj], 4.0));
                }
                else
                {
                    List.Add(X[jj], Y[jj]);
                }
            }
        }

        public void GetWMF(ref Metafile meta)
        {
            Graphics gMeta = Graphics.FromImage(meta);
            myPane.Draw(gMeta);
            gMeta.Dispose();
        }

        public void ClearModels()
        {
            int i = filesinGraph.Count - 1;
            while( i != 0)
            {
                if((string)filesinGraph[i] != datafilename)
                { 
                    CurveItem curve = myPane.CurveList[i] as CurveItem;

                    if (curve != null)
                    {
                        filesinGraph.RemoveAt(i);
                        filesinGraph.TrimToSize();
                        myPane.CurveList.Remove(curve);
                        myPane.CurveList.TrimExcess();
                    }
                 }
                 i--;
              }
        }
    }
}
