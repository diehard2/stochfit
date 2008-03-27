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
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using ZedGraph;
using StochasticModeling;
using System.Threading;
using System.Globalization;

namespace StochasticModeling
{
    /// <remarks>
    /// This class creates a MasterPane using a Zedgraph control <seealso cref="http://zedgraph.org/wiki/index.php?title=Main_Page"/>
    /// that collects all of the graphical output for the program and 
    /// </remarks>
    public partial class MasterPaneForm : Form
    {
        private bool m_bAtleastonegraph = false;
        private GraphingBase m_gMyMaster;

        /// <summary>
        /// Initializes all members of the master graph and checks whether the master graph should load
        /// </summary>
        public MasterPaneForm()
        {
            InitializeComponent();

            m_gMyMaster = new GraphingBase(string.Empty);
            m_gMyMaster.CreateGraph(MasterGraph);

            m_gMyMaster.GraphMasterPane.PaneList.Clear();
            m_gMyMaster.GraphMasterPane.Title.IsVisible = false;

            // Fill the pane background with a nice looking color gradient
            m_gMyMaster.GraphMasterPane.Fill = new Fill(Color.White, Color.LightGoldenrodYellow, 45.0F);

            // Set the margins and the space between panes to 10 points
            m_gMyMaster.GraphMasterPane.Margin.All = 10;
            m_gMyMaster.GraphMasterPane.InnerPaneGap = 10;
           
            // Add the new GraphPane to the MasterPane
            if (GraphCollection.Instance.MainReflGraph.IsDeepCopyFull == true)
            {
                GraphCollection.Instance.SetGraphUpdateLock = true;
                m_gMyMaster.GraphMasterPane.Add(GraphCollection.Instance.MainReflGraph.Pane);
                m_bAtleastonegraph = true;
            }

            if (GraphCollection.Instance.MainRhoGraph.IsDeepCopyFull == true)
            {
                GraphCollection.Instance.SetGraphUpdateLock = true;
                m_gMyMaster.GraphMasterPane.Add(GraphCollection.Instance.MainRhoGraph.Pane);
                m_bAtleastonegraph = true;
            }

            if (GraphCollection.Instance.RhoGraph.IsDeepCopyFull == true)
            {
                m_gMyMaster.GraphMasterPane.Add(GraphCollection.Instance.RhoGraph.Pane);
                m_bAtleastonegraph = true;
            }

            if (GraphCollection.Instance.ReflGraph.IsDeepCopyFull == true)
            {
                m_gMyMaster.GraphMasterPane.Add(GraphCollection.Instance.ReflGraph.Pane);
                m_bAtleastonegraph = true;
            }
            if (GraphCollection.Instance.ReflEGraph.IsDeepCopyFull == true)
            {
                m_gMyMaster.GraphMasterPane.Add(GraphCollection.Instance.ReflEGraph.Pane);
                m_bAtleastonegraph = true;
            }
            //Align the graphs in a custom fashion. The default options don't maintain aspect ratio
            if (m_bAtleastonegraph)
            {
                int counter = 0;
                foreach (GraphPane i in m_gMyMaster.GraphMasterPane.PaneList)
                {
                    if (counter % 2 == 0)
                    {
                        RectangleF recf = new RectangleF(10, (340/2)*counter+30, 439, 350);
                        i.Rect = recf;
                    }
                    else
                    {
                        RectangleF recf = new RectangleF(459, (340/2)*(counter-1)+30, 439, 350);
                        i.Rect = recf;
                    }
                    counter++;
                }

                //Create event handler for the context menu
                m_gMyMaster.GraphContextMenuBuilder(new ZedGraphControl.ContextMenuBuilderEventHandler(MyLocalGraphContextMenuBuilder));
            }
        }

        /// <summary>
        /// Builds the appropriate right click menu for the master graph
        /// </summary>
        /// <param name="control"></param>
        /// <param name="menuStrip"></param>
        /// <param name="mousePt"></param>
        /// <param name="objState"></param>
        private void MyLocalGraphContextMenuBuilder(ZedGraphControl control, ContextMenuStrip menuStrip, Point mousePt, ZedGraphControl.ContextMenuObjectState objState)
        {
            m_gMyMaster.AddMenuItem(menuStrip, "ClipCopy_tag", "ClipCopy_tag", "High Quality Copy", m_gMyMaster.CopyMetatoClip);
            m_gMyMaster.AddMenuItem(menuStrip, "ClipLocalCopy_tag", "ClipCopy_tag", "Copy Nearest Local Graph", m_gMyMaster.CopyLocalGraph);
            m_gMyMaster.AddMenuItem(menuStrip, "SaveEMF_tag", "SaveEMF_tag", "Save Nearest Graph as Picture", m_gMyMaster.SaveLocalEMFFile);
          
            //Eliminate menu items that aren't needed or are confusing. Copy is replaced with the emf methods
            m_gMyMaster.RemoveMenuItem(menuStrip, "copy");
            m_gMyMaster.RemoveMenuItem(menuStrip, "set_default");
            m_gMyMaster.RemoveMenuItem(menuStrip, "page_setup");
            m_gMyMaster.RemoveMenuItem(menuStrip, "print");
            
            //Pass the mouse position - can't use windows native version for some reason...
            m_gMyMaster.MousePosition = mousePt;
        }

        private void OnFormClosing(object sender, FormClosingEventArgs e)
        {
            GraphCollection.Instance.SetGraphUpdateLock = false;
        }

        /// <summary>
        /// Will fail with a messagebox error if no graphs are loaded. 
        /// </summary>
        /// <param name="owner"></param>
        /// <returns> DialogResult.OK on success and DialogResult.Abort on failure </returns>
        new public DialogResult ShowDialog(IWin32Window owner)
        {
            if (m_bAtleastonegraph)
                return (base.ShowDialog(owner));
            else
            {
                MessageBox.Show("There are no graphs loaded");
                return DialogResult.Abort;
            }
        }


        /// <summary>
        /// Will fail with a messagebox error if no graphs are loaded. 
        /// </summary>
        /// <param name="owner"></param>
        /// <returns> DialogResult.OK on success and DialogResult.Abort on failure </returns>
        new public DialogResult Show(IWin32Window owner)
        {
            if (m_bAtleastonegraph)
                return (base.ShowDialog(owner));
            else
            {
                MessageBox.Show("There are no graphs loaded");
                return DialogResult.Abort;
            }
        }
    }
}