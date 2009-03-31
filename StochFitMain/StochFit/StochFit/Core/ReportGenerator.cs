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
using iTextSharp.text;
using iTextSharp.text.pdf;
using System.IO;
using System.Drawing.Imaging;
using System.Windows.Forms;


#pragma warning disable 1591

namespace StochasticModeling
{
    /// <summary>
    /// Singleton class that collects information to be written into a pdf report using itextsharp
    /// </summary>
    public sealed class ReportGenerator
    {
        /// <summary>
        /// The publicly available instance of the class. All functions are called through instance
        /// </summary>
        public static readonly ReportGenerator Instance = new ReportGenerator();

        private List<Phrase> m_alMainInformation;
        private List<Phrase> m_alRhoModelingInformation;
        private List<Phrase> m_alReflModelingInformation;

        private const string m_pBlankline = "\n";
        private string m_sFilePDF;
        private bool m_bUseSLD = false;

        // make the default constructor private, so that no can directly create it.
        private ReportGenerator()
        {
            m_alMainInformation = new List<Phrase>();
            m_alRhoModelingInformation = new List<Phrase>();
            m_alReflModelingInformation = new List<Phrase>();
        }

        /// <summary>
        /// Get/Set the name for the report. The pdf extension is automatically included at the end
        /// </summary>
        public string DataFileName
        {
            get
            {
                return m_sFilePDF;
            }
            set
            {
                try
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
                catch{}
            }
        }

        /// <summary>
        /// Clears the information from the model independent portion of the report
        /// </summary>
        public void ClearMainInformation()
        {
            if(m_alMainInformation.Count > 0)
                m_alMainInformation.Clear();
        }

        /// <summary>
        /// Set information for the model independent portion of the report
        /// </summary>
        public string SetMainInformation
        {
            set
            {
                m_alMainInformation.Add(new Phrase(new Chunk((string)value,FontFactory.GetFont(FontFactory.HELVETICA, 8, 0))));
            }
        }

        /// <summary>
        /// Clear information related to the electron density profile fitting
        /// </summary>
        public void ClearRhoModelInfo()
        {
            if(m_alRhoModelingInformation.Count > 0)
                m_alRhoModelingInformation.Clear();
        }

        /// <summary>
        /// Add a List of formatted text to the electron density fitting portion of the report
        /// </summary>
        public List<String> SetRhoModelInfo
        {
            set
            {
                for (int i = 0; i < value.Count; i++)
                {
                    m_alRhoModelingInformation.Add(new Phrase(new Chunk(value[i], FontFactory.GetFont(FontFactory.HELVETICA, 8, 0))));
                }
            }
        }

        /// <summary>
        /// Clears the information in the model dependent fitting report section
        /// </summary>
        public void ClearReflModelInfo()
        {
            if(m_alReflModelingInformation.Count > 0)
                m_alReflModelingInformation.Clear();
        }

        /// <summary>
        /// Set the information for the model dependent reflectivity fit
        /// </summary>
        public List<string> SetReflModelInfo
        {
            set
            {
                for (int i = 0; i < value.Count; i++)
                {
                    m_alReflModelingInformation.Add( new Phrase(new Chunk(value[i],FontFactory.GetFont(FontFactory.HELVETICA, 8, 0))));
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

        private void GenerateReflChart(List<Phrase> info, Document document)
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

            if(!m_bUseSLD)
                datatable.AddCell("Rho/Rho(infinity)");
            else
                datatable.AddCell("SLD");

            datatable.AddCell("Sigma");
            datatable.HeaderRows = 1;

            datatable.DefaultCell.BorderWidth = 1;

            for (int i = 0; i < 10; i++)
            {
                document.Add((Phrase)info[i]);
            }

            for (int i = 10; i < (info.Count-3); i++)
            {
                datatable.AddCell((Phrase)info[i++]);
                datatable.AddCell((Phrase)info[i++]);
                datatable.AddCell((Phrase)info[i++]);
                datatable.AddCell((Phrase)info[i]);
            }
            document.Add(datatable);
        }

        private void GenerateRhoChart(List<Phrase> info, Document document)
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

            if (!m_bUseSLD)
                datatable.AddCell("Rho/Rho(infinity)");
            else
                datatable.AddCell("SLD");

            datatable.AddCell("Sigma");
            datatable.HeaderRows = 1;

            datatable.DefaultCell.BorderWidth = 1;

            for (int i = 0; i < 3; i++)
            {
                document.Add((Phrase)info[i]);
            }

            for (int i = 3; i < (info.Count - 3); i++)
            {
                datatable.AddCell((Phrase)info[i++]);
                datatable.AddCell((Phrase)info[i++]);
                datatable.AddCell((Phrase)info[i++]);
                datatable.AddCell((Phrase)info[i]);
            }
            document.Add(datatable);
        }

        /// <summary>
        /// Generate the pdf report
        /// </summary>
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
                if (GraphCollection.Instance.MainReflGraph.IsDeepCopyFull)
                {
                    Header = new Phrase(new Chunk("Model Independent Parameters\n", FontFactory.GetFont(FontFactory.HELVETICA, 12, 1)));
                    document.Add(Header);

                    for (int i = 0; i < m_alMainInformation.Count; i++)
                    {
                        document.Add((Phrase)m_alMainInformation[i]);
                    }
                    document.Add(new Phrase(m_pBlankline));

                    //Add the modeled reflectivity graph
                    if (GraphCollection.Instance.MainReflGraph.IsDeepCopyFull)
                    {
                        AddGraph(GraphCollection.Instance.MainReflGraph, document, writer);
                    }

                    if (GraphCollection.Instance.MainRhoGraph.IsDeepCopyFull)
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
                        if (!m_bUseSLD)
                        {
                            Header = new Phrase(new Chunk("Electron Density Non-Linear Regression Fit\n\n", FontFactory.GetFont(FontFactory.HELVETICA, 14, 1)));
                        }
                        else
                        {
                            Header = new Phrase(new Chunk("SLD Non-Linear Regression Fit\n\n", FontFactory.GetFont(FontFactory.HELVETICA, 14, 1)));
                        }

                        document.Add(Header);
                        //Add the fitted electron density
                        if (GraphCollection.Instance.RhoGraph.IsDeepCopyFull)
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
                    if (GraphCollection.Instance.ReflGraph.IsDeepCopyFull)
                    {
                        AddGraph(GraphCollection.Instance.ReflGraph, document, writer);
                    }

                    if (GraphCollection.Instance.ReflEGraph.IsDeepCopyFull)
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

        public bool UseSLD
        {
            set
            { m_bUseSLD = value; }
        }
    
    }
}
