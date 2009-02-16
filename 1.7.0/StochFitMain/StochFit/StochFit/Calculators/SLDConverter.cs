using System;
using System.Windows.Forms;

#pragma warning disable 1591

namespace StochasticModeling
{

    public partial class SLDConverter : Form
    {
        public SLDConverter()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                if (XRSLD.IsEmpty() || NSLD.IsEmpty())
                {
                    if (XRSLD.IsEmpty())
                        NSLD.Text = string.Format("{0}",(((double.Parse(XRSLD.Text) * double.Parse(BSUM.Text)) / (2.818 * double.Parse(ECount.Text)))));
                    else if (NSLD.IsEmpty())
                        XRSLD.Text = string.Format("{0}",(double.Parse(NSLD.Text) * 2.818 * double.Parse(ECount.Text)) / (double.Parse(BSUM.Text)));
                }
                else
                {
                    MessageBox.Show("You need to fill in at least one SLD box");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }



    }
}