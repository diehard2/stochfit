using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using ZedGraph;
using GIDFit;

namespace GIDFit
{
    public partial class Pubgraph : Form
    {
        Graphing PublicationGraph;

        public Pubgraph(Graphing ingraph)
        {
            InitializeComponent();
            PublicationGraph = new Graphing(string.Empty);
            PublicationGraph.CreateGraph(GIDGraph);
            PublicationGraph.SetAllFonts("Garamond", 20, 18);
            PublicationGraph.AlterGraph(ingraph);
        }
    }
}