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
using System.Text;
using System.Collections;
using iTextSharp.text;
using iTextSharp.text.pdf;
using System.IO;
using StochasticModeling;
using System.Drawing.Imaging;
using System.Windows.Forms;

namespace StochasticModeling
{
    public sealed class ReportGenerator
    {
        public static readonly ReportGenerator Instance = new ReportGenerator();

        private ArrayList m_alMainInformation;
        private ArrayList m_alRhoModelingInformation;
        private ArrayList m_alReflModelingInformation;

        private Phrase m_pBlankline = new Phrase("\n");
        private Phrase m_pLine;
        private string m_sFilePDF;

        // make the default constructor private, so that no can directly create it.
        private ReportGenerator()
        {
            m_alMainInformation = new ArrayList();
            m_alRhoModelingInformation = new ArrayList();
            m_alReflModelingInformation = new ArrayList();
            m_pLine = new Phrase();
        }

        public string DataFileName
        {
            get
            {
                return m_sFilePDF;
            }
            set
            {
                FileInfo info = new FileInfo(value);
                int counter = 0;
                //Don't overwrite previous reports
                while (true)
                {
                    if (counter == 0)
                        m_sFilePDF = info.DirectoryName + "\\" + info.Name + ".pdf";
                    else
                        m_sFilePDF = info.DirectoryName + "\\" + info.Name + counter.ToString() + ".pdf";

                    if (!File.Exists(m_sFilePDF))
                        break;

                    counter++;
                }
            }
        }

        public void ClearMainInformation()
        {
            if(m_alMainInformation.Count > 0)
                m_alMainInformation.Clear();
        }

        public string SetMainInformation
        {
            set
            {
                m_pLine = new Phrase(new Chunk((string)value,FontFactory.GetFont(FontFactory.HELVETICA, 8, 0)));
                m_alMainInformation.Add(m_pLine);
            }
        }

        public void ClearRhoModelInfo()
        {
            if(m_alRhoModelingInformation.Count > 0)
                m_alRhoModelingInformation.Clear();
        }

        public ArrayList SetRhoModelInfo
        {
            set
            {
                for (int i = 0; i < ((ArrayList)value).Count; i++)
                {
                    m_pLine = new Phrase(new Chunk((string)(((ArrayList)value)[i]), FontFactory.GetFont(FontFactory.HELVETICA, 8, 0)));
                    m_alRhoModelingInformation.Add(m_pLine);
                }
            }
        }

        public void ClearReflModelInfo()
        {
            if(m_alReflModelingInformation.Count > 0)
                m_alReflModelingInformation.Clear();
        }

        public ArrayList SetReflModelInfo
        {
            set
            {
                for (int i = 0; i < ((ArrayList)value).Count; i++)
                {
                    m_pLine = new Phrase(new Chunk((string)(((ArrayList)value)[i]),FontFactory.GetFont(FontFactory.HELVETICA, 8, 0)));
                    m_alReflModelingInformation.Add(m_pLine);
                }
            }
        }

        private void AddGraph(Graphing graphobject, Document document, PdfWriter writer)
        {
            PdfContentByte cb = writer.DirectContent;

            //Can't use transparency in graph object, so create a temp object and fix it
            Graphing tempgraph = new Graphing(string.Empty);
            tempgraph.DeepCopy(graphobject);
            tempgraph.PaneFill = System.Drawing.Color.White;
            tempgraph.ChartFill = System.Drawing.Color.White;
            tempgraph.Pane.Title.IsVisible = true;
            tempgraph.Pane.Legend.IsVisible = false;
             
            //Add the image
            iTextSharp.text.Image image = iTextSharp.text.Image.GetInstance(tempgraph.GetImage(), ImageFormat.Bmp);
            image.ScalePercent(65);
            document.Add(image);
        }

        private void GenerateReflChart(ArrayList info, Document document)
        {
            PdfPTable datatable = new PdfPTable(4);
            datatable.WidthPercentage = 100;

            float[] headerwidths = { 10, 30, 30, 30 }; // percentage
            datatable.DefaultCell.Padding = 1;
            datatable.SetWidths(headerwidths);
            datatable.DefaultCell.BorderWidth = 2;
            datatable.DefaultCell.HorizontalAlignment = Element.ALIGN_CENTER;
            datatable.DefaultCell.VerticalAlignment = Element.ALIGN_CENTER;
            datatable.AddCell("Layer #");
            datatable.AddCell("Length");
            datatable.AddCell("Rho/Rho(infinity)");
            datatable.AddCell("Sigma");
            datatable.HeaderRows = 1;

            datatable.DefaultCell.BorderWidth = 1;

            for (int i = 0; i < 10; i++)
            {
                document.Add((Phrase)info[i]);
            }

            for (int i = 10; i < (info.Count-3); i++)
            {
                datatable.AddCell((Phrase)info[i]);
                i++;
                datatable.AddCell((Phrase)info[i]);
                i++;
                datatable.AddCell((Phrase)info[i]);
                i++;
                datatable.AddCell((Phrase)info[i]);
            }
            document.Add(datatable);
        }

        private void GenerateRhoChart(ArrayList info, Document document)
        {
            PdfPTable datatable = new PdfPTable(4);
            datatable.WidthPercentage = 100;

            float[] headerwidths = { 10, 30, 30, 30 }; // percentage
            datatable.DefaultCell.Padding = 1;
            datatable.SetWidths(headerwidths);
            datatable.DefaultCell.BorderWidth = 2;
            datatable.DefaultCell.HorizontalAlignment = Element.ALIGN_CENTER;
            datatable.AddCell("Layer #");
            datatable.AddCell("Length");
            datatable.AddCell("Rho/Rho(infinity)");
            datatable.AddCell("Sigma");
            datatable.HeaderRows = 1;

            datatable.DefaultCell.BorderWidth = 1;

            for (int i = 0; i < 3; i++)
            {
                document.Add((Phrase)info[i]);
            }

            for (int i = 3; i < (info.Count - 3); i++)
            {
                datatable.AddCell((Phrase)info[i]);
                i++;
                datatable.AddCell((Phrase)info[i]);
                i++;
                datatable.AddCell((Phrase)info[i]);
                i++;
                datatable.AddCell((Phrase)info[i]);
            }
            document.Add(datatable);
        }

        public void GeneratePDFReport()
        {
            try
            {
                if (m_alMainInformation.Count == 0)
                {
                    MessageBox.Show("There is no data to put in the report!");
                    return;
                }
                
                Document document = new Document(PageSize.A4);
                PdfWriter writer = PdfWriter.GetInstance(document,
                                new FileStream(m_sFilePDF, FileMode.Create));

                document.Open();

                //Add standard header
                Phrase Header = new Phrase(new Chunk("StochFit Modeling Report\n", FontFactory.GetFont(FontFactory.HELVETICA, 18, 1)));
                Phrase Date = new Phrase(new Chunk(DateTime.Now.ToString("MM/dd/yyyy h:mm tt\n\n"), FontFactory.GetFont(FontFactory.HELVETICA, 10, 0)));
                document.Add(Header);
                document.Add(Date);

                //Make sure we aren't just model dependent
                if (GraphCollection.Instance.MainReflGraph.IsDeepCopyFull == true)
                {
                    Header = new Phrase(new Chunk("Model Independent Parameters\n", FontFactory.GetFont(FontFactory.HELVETICA, 12, 1)));
                    document.Add(Header);

                    for (int i = 0; i < m_alMainInformation.Count; i++)
                    {
                        document.Add((Phrase)m_alMainInformation[i]);
                    }
                    document.Add(m_pBlankline);

                    //Add the modeled reflectivity graph
                    if (GraphCollection.Instance.MainReflGraph.IsDeepCopyFull == true)
                    {
                        AddGraph(GraphCollection.Instance.MainReflGraph, document, writer);
                    }

                    if (GraphCollection.Instance.MainRhoGraph.IsDeepCopyFull == true)
                    {
                        AddGraph(GraphCollection.Instance.MainRhoGraph, document, writer);
                    }

                    document.NewPage();
                }
                //Add the modeled electron density if it exists
                if (m_alRhoModelingInformation.Count > 0)
                {
                    if (m_alRhoModelingInformation.Count > 0)
                    {
                        //Add a header for the fitted data
                        Header = new Phrase(new Chunk("Electron Density Non-Linear Regression Fit\n\n", FontFactory.GetFont(FontFactory.HELVETICA, 14, 1)));
                        document.Add(Header);
                        //Add the fitted electron density
                        if (GraphCollection.Instance.RhoGraph.IsDeepCopyFull == true)
                        {
                            AddGraph(GraphCollection.Instance.RhoGraph, document, writer);
                        }

                        //Add the chart
                        GenerateRhoChart(m_alRhoModelingInformation, document);
                    }
                    document.NewPage();
                }
                //Add the reflectivity if it exists
                if (m_alReflModelingInformation.Count > 0)
                {
                    //Add a header for the fitted data
                    Header = new Phrase(new Chunk("Reflectivity Non-Linear Regression Fit\n\n", FontFactory.GetFont(FontFactory.HELVETICA, 14, 1)));
                    document.Add(Header);
                    //Add the fitted electron density
                    if (GraphCollection.Instance.ReflGraph.IsDeepCopyFull == true)
                    {
                        AddGraph(GraphCollection.Instance.ReflGraph, document, writer);
                    }
                    if (GraphCollection.Instance.ReflEGraph.IsDeepCopyFull == true)
                    {
                        AddGraph(GraphCollection.Instance.ReflEGraph, document, writer);
                    }
                    //Add the chart
                    GenerateReflChart(m_alReflModelingInformation,document);
                }
                document.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
    }
}