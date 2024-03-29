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

using System.Windows.Forms;

namespace StochasticModeling
{
    /// <summary>
    /// Simple output window for the least square fitting algorithm
    /// </summary>
    public partial class LevmarOutput : Form
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public LevmarOutput()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Displays the string to the form window
        /// </summary>
        /// <param name="display">Formatted output string</param>
        public void DisplayOutput(string display)
        {
            LevMarOut.Text = display;
        }
    }
}