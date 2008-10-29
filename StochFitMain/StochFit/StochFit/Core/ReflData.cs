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
    public sealed class ReflData:IComparer
    {
        /// <summary>
        /// The instance of the Singleton class ReflData
        /// </summary>
        public static readonly ReflData Instance = new ReflData();

        private double[][] refldata;
        private double[] refldataarray;
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
                    return (double[])refldataarray.Clone();
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
                return refldataarray[index];
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
        /// Returns a copy of an array for the SD in the reflectivity
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
        /// Load the data from the file. The expected columns are Q, R, SD in R, and SD in Q (optional).
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

            //Clear the Qerr array, as it won't be reset if a previous model had error in Q and this data does not
            qerrors = null;

            FileInfo info = new FileInfo(filename);

            direc = info.DirectoryName;

            //Get the number of lines in the file
            int lines = 0;
            string dataline;
            List<string> datastring = new List<string>();
            Regex r = new Regex(@"\s");
            haveQerr = false;

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

                            if (temp.Length == 4)
                                haveQerr = true;

                            if (Double.Parse(datastring[1], m_CI) > 0.0 && Double.Parse(datastring[2], m_CI) >= 0.0)
                                lines++;

                            datastring.Clear();
                        }
                    }
                }

                //Setup all of our arrays
                refldata = new double[lines][];

                for(int i = 0; i < lines; i++)
                {
                    if (haveQerr)
                        refldata[i] = new double[4];
                    else
                        refldata[i] = new double[3];
                }

            
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

                            if (Double.Parse(datastring[1], m_CI) > 0.0 && Double.Parse(datastring[2], m_CI) > 0.0)
                            {
                                if (temp.Length < 3)
                                    return false;

                                refldata[j][0] = Double.Parse(datastring[0], m_CI);
                                refldata[j][1] = Double.Parse(datastring[1], m_CI);

                                if (IsSD)
                                {
                                    refldata[j][2] = Double.Parse(datastring[2], m_CI);

                                    if (temp.Length == 4)
                                    {
                                        refldata[j][3] = Double.Parse(datastring[3], m_CI);
                                        haveQerr = true;
                                    }
                                }
                                else
                                {
                                    refldata[j][2] = Math.Sqrt(Double.Parse(datastring[2], m_CI));

                                    if (temp.Length == 4)
                                    {
                                        refldata[j][3] = Math.Sqrt(Double.Parse(datastring[3], m_CI));
                                        haveQerr = true;
                                    }
                                }

                                j++;
                            }
                            datastring.Clear();
                        }
                    }
                }
           

            //Check if our data monotonically increases in Q. Some beamlines don't patch data
            bool outoforderq = false;

            for (int i = 0; i < GetNumberDataPoints - 1; i++)
            {
                if (refldata[i + 1][0] < refldata[i][0])
                {
                    outoforderq = true;
                    break;
                }
            }

            if (outoforderq == true)
            {
                if (MessageBox.Show("Some of your Q data is out of order. Would you like to attempt rudimentary patching",
                    string.Empty, MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    //Patch data
                    DataPatch(outoforderq, true);
                }
                else
                    DataPatch(outoforderq, false);
            }
            else
                DataPatch(outoforderq, false);
            
            return true;

        }
        catch
        {
            //Blank our data if we fail for some reason and let the user know there is a problem with the file
            qerrors = null;
            rerrors = null;
            qdata = null;
            refldata = null;
            refldataarray = null;
            MessageBox.Show("The data file could not be loaded");
            return false;
        }
       }

        private void DataPatch(bool outoforderq, bool patch)
        {

            try
            {
                double[][] refldatatemp;
                bool nooverlapregion = false;

                if (patch == true)
                {

                    for (int i = 0; i < GetNumberDataPoints - 1; i++)
                    {

                        List<int> beginindex = new List<int>(30);
                        List<int> endindex = new List<int>(30);
                        int overlapcount = 0;

                        //We have a discontinuity in Q - find the jump back. If we don't have an actual overlap,
                        //ignore and sort
                        if (refldata[i + 1][0] <= refldata[i][0])
                        {
                                //Find our overlap points
                                for (int counter1 = 0; counter1 <= i; counter1++)
                                {
                                    for (int counter2 = i + 1; counter2 < GetNumberDataPoints; counter2++)
                                    {
                                        if (refldata[counter1][0] == refldata[counter2][0] && counter1 != counter2)
                                        {
                                            if (beginindex.Contains(counter1) == false)
                                            {
                                                beginindex.Add(counter1);
                                                endindex.Add(counter2);
                                                overlapcount++;
                                            }
                                        }
                                    }

                                }

                                //We have non monotonically increasing q, but no overlap region
                                if (overlapcount == 0)
                                    nooverlapregion = true;

                                //We have our overlap region
                                if (overlapcount > 0)
                                {
                                    double shift = 0;

                                    for (int counter1 = 0; counter1 < overlapcount; counter1++)
                                        shift += refldata[beginindex[counter1]][1] / refldata[endindex[counter1]][1];

                                    shift /= overlapcount;

                                    //Multiply the curve
                                    refldatatemp = new double[refldata.Length - overlapcount][];
                                    int indexoffset = 0;
                                    for (int counter = 0; counter < GetNumberDataPoints; counter++)
                                    {
                                        //Copy over all points lower than the first overlap point
                                        if(counter < beginindex[0])
                                            refldatatemp[counter] = (double[])refldata[counter].Clone();
                                        //Test
                                        else if(counter < i + 1)
                                        {
                                            bool isoverlappt = false;

                                            for (int counter1 = 0; counter1 < overlapcount; counter1++)
                                            {
                                                if (counter == beginindex[counter1])
                                                {
                                                    isoverlappt = true;
                                                    indexoffset++;
                                                }
                                            }

                                            if (isoverlappt == false)
                                                refldatatemp[counter - indexoffset] = (double[])refldata[counter].Clone();

                                        }
                                        else
                                        {
                                            refldata[counter][1] *= shift;
                                            refldata[counter][2] *= shift;

                                            refldatatemp[counter - indexoffset] = (double[])refldata[counter].Clone();
                                        }
                                    }
                                    refldata = null;
                                    refldata = refldatatemp.Clone() as double[][];
                                    i = 0;
                                    datapointcount -= overlapcount;

                                }
                            }
                    }

                    if (nooverlapregion)
                        MessageBox.Show("There were areas of discontinuity in Q where no overlap was detected");


                    MoveintoArrays(true);

                }
                else
                {
                    //Move into our arrays
                    MoveintoArrays(outoforderq);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void MoveintoArrays(bool outoforderq)
        {
            //Sort the arrays if we are out of order to prevent ugly graphing and prog crash
            if (outoforderq == true)
                Array.Sort(refldata, this);

            refldataarray = new double[GetNumberDataPoints];
            qdata = new double[GetNumberDataPoints];
            rerrors = new double[GetNumberDataPoints];

            if (HaveErrorinQ == true)
                qerrors = new double[GetNumberDataPoints];


            for (int i = 0; i < GetNumberDataPoints; i++)
            {
                qdata[i] = refldata[i][0];
                refldataarray[i] = refldata[i][1];
                rerrors[i] = refldata[i][2];

                if (haveQerr)
                    qerrors[i] = refldata[i][3];
            }
        }


        #region IComparer Members

        int IComparer.Compare(object x, object y)
        {
            double[] x1 = null;
            double[] x2 = null;

            try
            {
               x1  = (double[])x;
               x2  = (double[])y;
            }
            catch
            { }
           
            if (x1 != null && x2 != null)
                return x1[0].CompareTo(x2[0]);
            else if (x1 == null)
                return -1000;
            else
                return 1000;
        }

        #endregion
    }
}
