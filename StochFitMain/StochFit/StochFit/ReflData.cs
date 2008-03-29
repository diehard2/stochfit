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
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Collections;
using System.Globalization;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace StochasticModeling
{
/// <summary>
/// This class holds the reflectivity data loaded from the file. It is expected that the data be in the format of 
/// Q, R, Variance in R, Variance in Q. It is implemented as a Singleton
/// </summary>
    public sealed class ReflData
    {
        /// <summary>
        /// The instance of the Singleton class ReflData
        /// </summary>
        public static readonly ReflData Instance = new ReflData();

        private double[] refldata;
        private double[] qdata;
        private double[] rerrors;
        private double[] qerrors;
        private bool haveQerr = false;
        private int datapointcount = -1;
        private CultureInfo m_CI = new CultureInfo("en-US");
        private string direc;

        private ReflData()
        {}

        /// <summary>
        /// Returns a copy of an array for the reflectivity data
        /// </summary>
        public double[] GetReflData
        {
            get
            {
                if (refldata != null)
                    return (double[])refldata.Clone();
                else
                    return null;
            }
        }

        /// <summary>
        /// Get the value for the reflectivity point at position index
        /// </summary>
        /// <param name="index">Datapoint position</param>
        /// <returns>Reflectivity for the datapoint index</returns>
        public double GetReflDataPt(int index)
        {
            if (refldata != null)
                return refldata[index];
            else
                return -1;
        }

        /// <summary>
        /// Returns a copy of an array for the data in Q
        /// </summary>
        public double[] GetQData
        {
            get
            {
                if (qdata != null)
                    return (double[])qdata.Clone();
                else
                    return null;
            }
        }

        /// <summary>
        /// Get the value for the Q point at position index
        /// </summary>
        /// <param name="index">Datapoint position</param>
        /// <returns>Q for the datapoint index</returns>
        public double GetQDataPt(int index)
        {
            if (qdata != null)
                return qdata[index];
            else
                return -1;
        }

        /// <summary>
        /// Returns a copy of anarray for the SD in the reflectivity
        /// </summary>
        public double[] GetRErrors
        {
            get
            {
                if (rerrors != null)
                    return (double[])rerrors.Clone();
                else
                    return null;
            }
        }

        /// <summary>
        /// Get the value for the SD in R at position index
        /// </summary>
        /// <param name="index">Datapoint position</param>
        /// <returns>SD in R for the datapoint index</returns>
        public double GetRErrorPt(int index)
        {
            if (rerrors != null)
                return rerrors[index];
            else
                return -1;
        }

        /// <summary>
        /// Returns a copy of an array for the SD in Q
        /// </summary>
        public double[] GetQErrors
        {
            get
            {
                if (qerrors != null)
                    return (double[])qerrors.Clone();
                else
                    return null;
            }
        }

        /// <summary>
        /// Get the value for the SD in Q at position index
        /// </summary>
        /// <param name="index">Datapoint position</param>
        /// <returns>SD in Q for the datapoint index</returns>
        public double GetQErrorPt(int index)
        {
            if (qerrors != null)
                return qerrors[index];
            else
                return -1;
        }

        /// <summary>
        /// Returns the number of valid data points in the reflectivity folder
        /// </summary>
        public int GetNumberDataPoints
        {
            get
            {
                return datapointcount;
            }
        }

        /// <summary>
        /// Returns true if there is a 4th data column with error in Q, false otherwise
        /// </summary>
        public bool HaveErrorinQ
        {
            get
            {
                return haveQerr;
            }
        }

        /// <summary>
        /// Returns the working directory
        /// </summary>
        public string GetWorkingDirectory
        {
            get
            {
                return direc;
            }
        }

        /// <summary>
        /// Load the data from the file. The expected columns are Q, R, variance in R, and variance in Q (optional).
        /// Negative values for reflectivities are discarded, as are negative values for the variance in R. The variance
        /// in the parameters is recalculated as standard deviation in this function.
        /// </summary>
        /// <param name="filename"></param>
        /// <param name="IsSD">Set to true if the data errors are in standard deviation</param>
        /// <returns>true if the file loaded, false otherwise</returns>
        public bool SetReflData(string filename, bool IsSD)
        {
            //Return if we don't have a file
            if (filename == string.Empty || !File.Exists(filename))
                return false;

            FileInfo info = new FileInfo(filename);

            direc = info.DirectoryName;

            //Get the number of lines in the file
            int lines = 0;
            string dataline;
            ArrayList datastring = new ArrayList();
            Regex r = new Regex(@"\s");
          
            try
            {
                //Count the lines in the file while ignoring blank lines, negative reflectivities, and errors <= 0
                using (StreamReader sr = new StreamReader(filename))
                {
                    while (!sr.EndOfStream)
                    {
                        dataline = sr.ReadLine().Trim();

                        if (dataline.Length != 0)
                        {
                            string[] temp = r.Split(dataline);

                            for (int i = 0; i < temp.Length; i++)
                            {
                                if (temp[i] != "")
                                    datastring.Add(temp[i]);
                            }

                            if (Double.Parse((string)datastring[1], m_CI) > 0.0 && Double.Parse((string)datastring[2], m_CI) >= 0.0)
                                lines++;

                            datastring.Clear();
                        }
                    }
                }

                //Setup all of our arrays
                refldata = new double[lines];
                qdata = new double[lines];
                rerrors = new double[lines];
                qerrors = new double[lines];
                datapointcount = lines;

                //Read in the data
                using (StreamReader sr = new StreamReader(filename))
                {
                    int j = 0;
                    

                    while (!sr.EndOfStream)
                    {
                        dataline = sr.ReadLine().Trim();

                        if (dataline.Length != 0)
                        {
                            string[] temp = r.Split(dataline);

                            for (int i = 0; i < temp.Length; i++)
                            {
                                if (temp[i] != "")
                                    datastring.Add(temp[i]);
                            }

                            if (Double.Parse((string)datastring[1], m_CI) > 0.0 && Double.Parse((string)datastring[2], m_CI) > 0.0)
                            {
                                if (temp.Length < 3)
                                    return false;

                                qdata[j] = Double.Parse((string)datastring[0], m_CI);
                                refldata[j] = Double.Parse((string)datastring[1], m_CI);

                                if (IsSD)
                                {
                                    rerrors[j] = Double.Parse((string)datastring[2], m_CI);

                                    if (temp.Length == 4)
                                    {
                                        qerrors[j] = Double.Parse((string)datastring[3], m_CI);
                                        haveQerr = true;
                                    }
                                }
                                else
                                {
                                    rerrors[j] = Math.Sqrt(Double.Parse((string)datastring[2], m_CI));

                                    if (temp.Length == 4)
                                    {
                                        qerrors[j] = Math.Sqrt(Double.Parse((string)datastring[3], m_CI));
                                        haveQerr = true;
                                    }
                                }

                                j++;
                            }
                            datastring.Clear();
                        }
                    }
                }
            }
            catch
            {
                MessageBox.Show("Could not add file");
                return false;
            }
            return true;
        }
    }
}
