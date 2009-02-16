using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StochasticModeling.GraphingZoom
{
    public partial class AdvancedZoom : Form
    {
        public double xupperlimit = -1e6;
        public double xlowerlimit = -1e6;
        public double yupperlimit = -1e6;
        public double ylowerlimit = -1e6;

        public AdvancedZoom()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (xlowerTB.Text != string.Empty)
                xlowerlimit = double.Parse(xlowerTB.Text);
            if (xupperTB.Text != string.Empty)
                xupperlimit = double.Parse(xupperTB.Text);
            if (ylowerTB.Text != string.Empty)
                ylowerlimit = double.Parse(ylowerTB.Text);
            if (yupperTB.Text != string.Empty)
                yupperlimit = double.Parse(yupperTB.Text);

            this.Close();
        }
    }
}