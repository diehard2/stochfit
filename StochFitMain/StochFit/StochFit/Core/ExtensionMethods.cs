using System;
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

        public static void ThreadSafeSetText(this TextBox t, string text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (t.InvokeRequired)
            {
                t.Invoke(new MethodInvoker(delegate() { ThreadSafeSetText(t, text); }));
            }
            else
            {
                t.Text = text;
            }
        }

        public static void ThreadSafeSetValue(this ProgressBar t, int value)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (t.InvokeRequired)
            {
                t.Invoke(new MethodInvoker(delegate() { ThreadSafeSetValue(t, value); }));
            }
            else
            {
                t.Value = value;
            }
        }
    }
}
