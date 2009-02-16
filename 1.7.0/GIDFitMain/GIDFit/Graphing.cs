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
using StochasticModeling;

namespace GIDFit
{
    
    /// <summary>
    /// Implements graphing for X-ray or neutron data
    /// </summary>
    public class Graphing:GraphingBase
    {
        private PointPairList RealReflData, RealReflErrors;
        public bool QSquaredCorrection = false;
        CultureInfo CI_US = new CultureInfo("en-US");

        public Graphing(string name):base(name)
        {
            
        }

        public override void CreateGraph(ZedGraphControl zgc, string Title, string XAxis, string YAxis, AxisType blog)
        {
            base.CreateGraph(zgc, Title, XAxis, YAxis, blog);
            GraphContextMenuBuilder(MyContextMenuBuilder);
        }

        protected override void SetAxisScale()
        {
            AxisChange();
        }

        public void SetUpGraphMenu()
        {
            GraphContextMenuBuilder(MyContextMenuBuilder);
        }

        private void MyContextMenuBuilder(ZedGraphControl control, ContextMenuStrip menuStrip, Point mousePt, ZedGraphControl.ContextMenuObjectState objState)
        {
            AddMenuItem(menuStrip, "ClipCopy_tag", "ClipCopy_tag", "High Quality Copy", CopyLocalGraph);
            AddMenuItem(menuStrip, "SaveEMF_tag", "SaveEMF_tag", "Save High Quality Image", SaveEMFFile);
            AddMenuItem(menuStrip, "AdvancedZoom", "AdvancedZoom", "Advanced Zoom", AdvancedZoom);
            AddMenuItem(menuStrip, "UndoAdvancedZoom", "UndoAdvancedZoom", "Undo Advanced Zoom", UndoAdvancedZoom);
            RemoveMenuItem(menuStrip, "copy");
            RemoveMenuItem(menuStrip, "set_default");
            RemoveMenuItem(menuStrip, "page_setup");
            RemoveMenuItem(menuStrip, "print");
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
                        }
                    }

                    if (QSquaredCorrection)
                    {
                        for (int i = 0; i < ReflData.Instance.GetNumberDataPoints; i++)
                        {
                            RealReflData[i].Y *= RealReflData[i].X * RealReflData[i].X;
                            RealReflErrors[i].Y = RealReflErrors[i].X * RealReflErrors[i].X;
                            RealReflErrors[i].Z = RealReflErrors[i].X * RealReflErrors[i].X;
                        }
                    }
               
                AddCurvetoGraph(RealReflData, RealReflErrors, name, color, symbol, symbolsize, "realdatafile");
                
                m_alDatainGraph.Add(name);
                //To account for the error file
                m_alDatainGraph.Add(name);
            }
            catch
            {
                MessageBox.Show("Error in data loading - contact the author");
            }
        }
    }
}
