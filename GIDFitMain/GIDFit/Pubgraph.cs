using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using ZedGraph;

namespace GIDFit
{
    public partial class Pubgraph : Form
    {
        Graphing PublicationGraph;

        public Pubgraph(Graphing ingraph)
        {
            InitializeComponent();
            PublicationGraph = new Graphing();
            PublicationGraph.CreateGraph(GIDGraph);
            PublicationGraph.SetFont("Garamond", 20, 18);
            DeepCopy(ingraph);
        }

        private void DeepCopy(Graphing ingraph)
        {
              PublicationGraph.Clear();
              PublicationGraph.DeepCopy(ingraph);
              PublicationGraph.LegendState = false;
              PublicationGraph.BorderState = false;
              PublicationGraph.Pane.Title.Text = " ";
              PublicationGraph.zg.BackColor = Color.Transparent;
              PublicationGraph.Pane.Chart.Fill = new Fill(Color.Transparent);
              PublicationGraph.Pane.IsPenWidthScaled = true;
              PublicationGraph.Pane.Fill = new Fill(Color.Transparent);
              PublicationGraph.Pane.XAxis.Title.FontSpec.IsAntiAlias = true;
              PublicationGraph.Pane.YAxis.Title.FontSpec.IsAntiAlias = true;
              PublicationGraph.Pane.XAxis.Scale.FontSpec.IsAntiAlias = true;
              PublicationGraph.Pane.YAxis.Scale.FontSpec.IsAntiAlias = true;
              PublicationGraph.Pane.XAxis.Title.FontSpec.Size = 26;
              PublicationGraph.Pane.YAxis.Title.FontSpec.Size = 26;
              PublicationGraph.Pane.XAxis.Scale.FontSpec.Size = 18;
              PublicationGraph.Pane.YAxis.Scale.FontSpec.Size = 18;
              
              for (int i = 0; i < PublicationGraph.myPane.CurveList.Count; i++)
              {
                  if (PublicationGraph.myPane.CurveList[i].IsLine == true)
                  {
                      if (((LineItem)PublicationGraph.myPane.CurveList[i]).Line.Style != 0)
                          ((LineItem)PublicationGraph.myPane.CurveList[i]).Line.Width += 1;
                  }
              }

        }
    }
}