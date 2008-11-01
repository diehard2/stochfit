using System;
using System.Windows.Forms;
using System.Collections.Generic;

namespace StochasticModeling
{
    static class ExtensionMethods
    {
        #region TextBox extension methods
        public static double ToDouble(this TextBox t)
        {
            return Double.Parse(t.Text);
        }

        public static int ToInt(this TextBox t)
        {
            return Int32.Parse(t.Text);
        }

        /// <summary>
        /// Returns true if the textbox is empty
        /// </summary>
        /// <param name="t"></param>
        /// <returns></returns>
        public static bool IsEmpty(this TextBox t)
        {
            return t.Text == string.Empty;
        } 
        #endregion

        #region GUI ThreadSafety Functions

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

        public static void ThreadSafeChecked(this CheckBox t, bool check)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (t.InvokeRequired)
            {
                t.Invoke(new MethodInvoker(delegate() { ThreadSafeChecked(t, check); }));
            }
            else
            {
                t.Checked = check;
            }
        } 
        #endregion
      
        #region ForEach extensions

        public static void ForEach<T>(this IEnumerable<T> source, Action<T> action)
        {
            foreach (T element in source)
            {
                action(element);
            }
        }

        public static IEnumerable<T> LINQForEach<T>(this IEnumerable<T> source, Action<T> action)
        {
            foreach (T element in source)
            {
                action(element);
                yield return element;
            }
        }

        public static IEnumerable<T> IndexedForEach<T>(this IEnumerable<T> source, Action<T, int> action)
        {
            int i = 0;
            foreach (T element in source)
            {
                action(element, i);
                yield return element;
            }
        } 
        #endregion
    }
}
