using System;
using System.Windows.Forms;

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
                if (XRSLD.Text != string.Empty || NSLD.Text != string.Empty)
                {
                    if (XRSLD.Text != string.Empty)
                        NSLD.Text = string.Format("{0}",(((double.Parse(XRSLD.Text) * double.Parse(BSUM.Text)) / (2.818 * double.Parse(ECount.Text)))));
                    else if (NSLD.Text != string.Empty)
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