using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace StochasticModeling
{
    static class ExtensionMethods
    {
        public static double ToDouble(this TextBox t)
        {
            return Double.Parse(t.Text);
        }

        public static int ToInt(this TextBox t)
        {
            return Int32.Parse(t.Text);
        }
    }
}
