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
using System.Runtime.Serialization.Formatters.Binary;
using System.Xml.Serialization;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Runtime.Serialization;
using System.Security.Permissions;

#pragma warning disable 1591

namespace StochasticModeling.Settings
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 8)]
    public class ModelSettings:IDisposable
    {
        #region Variables
        public string Directory;
        [XmlIgnoreAttribute] public IntPtr Q;
        [XmlIgnoreAttribute] public IntPtr Refl;
        [XmlIgnoreAttribute] public IntPtr ReflError;
        [XmlIgnoreAttribute] public IntPtr QError;
        [XmlIgnoreAttribute] public int QPoints;
        /// <summary>
        /// Subphase SLD
        /// </summary>
        public double SubSLD;
        /// <summary>
        /// Film SLD
        /// </summary>
        public double SurflayerSLD;
        /// <summary>
        /// Superphase SLD
        /// </summary>
        public double SupSLD;
        /// <summary>
        /// Number of Small Boxes
        /// </summary>
        public int Boxes;
        /// <summary>
        /// Surface Layer Absorption
        /// </summary>
        public double SurflayerAbs;
        /// <summary>
        /// Subphase Absorption
        /// </summary>
        public double SubAbs;
        /// <summary>
        /// Superphase Absorption
        /// </summary>
        public double SupAbs;
        /// <summary>
        /// XR Wavelenght
        /// </summary>
        public double Wavelength;
        /// <summary>
        /// True if absorptions are used, false otherwise
        /// </summary>
        public bool UseAbs;
        /// <summary>
        /// Superphase offset for the electron density profile
        /// </summary>
        public double SupOffset;
        /// <summary>
        /// Percent Error in Q
        /// </summary>
        public double Percerror;
        /// <summary>
        /// True if normalization is forced, false otherwise. This set the first point 
        /// </summary>
        public bool Forcenorm;
        /// <summary>
        /// Forces the sigma parameter to a user specified value. Sigma will not vary in this case. Useful for neutrons
        /// </summary>
        public double Forcesig;
        /// <summary>
        /// Set to true to output debug files.
        /// </summary>
        public bool Debug;
        /// <summary>
        /// Heavily penalizes models with negative electron densities
        /// </summary>
        public bool ForceXR;
        public int Resolution;
        public double Totallength;
        public double Surflayerlength;
        public bool ImpNorm;
        public int FitFunc;
        public double ParamTemp;

        //Annealing parameters
        public double SigmaSearchPerc;
        public int Algorithm;
        public double AnnealInitTemp;
        public int AnnealTempPlat;
        public double AnnealSlope;
        public double AnnealGamma;
        public int STUNfunc;
        public bool STUNAdaptive;
        public int STUNtempiter;
        public int STUNdeciter;
        public double STUNgammadec;

        /// <summary>
        /// Number of iterations
        /// </summary>
        public int Iterations;
        /// <summary>
        /// Number of iterations completed
        /// </summary>
        public int IterationsCompleted;
        /// <summary>
        /// ChiSquare value for the current fit
        /// </summary>
        public double ChiSquare;
        /// <summary>
        /// System description
        /// </summary>
        public string Title;
        /// <summary>
        /// Low Q offset in datapoints from the beginning of the curve
        /// </summary>
        public int CritEdgeOffset;
        /// <summary>
        /// High Q offset in datapoints from the end of the curve
        /// </summary>
        public int HighQOffset;

        public bool IsNeutron;
        [XmlIgnoreAttribute] private bool disposed = false;

    #endregion

        #region Constructor/Destructor
        /// <summary>
        /// Constructor
        /// </summary>
        public ModelSettings()
        { }

        ~ModelSettings()
        {
            Dispose(false);
        }
        #endregion

        #region Public Methods
        public void SetArrays(double[] iQ, double[] iR, double[] iRerr, double[] iQerr, int iQSize)
        {
            int size = Marshal.SizeOf(iQ[0])*iQ.Length;

            try
            {
                Q = Marshal.AllocHGlobal(size);
                Refl = Marshal.AllocHGlobal(size);
                ReflError = Marshal.AllocHGlobal(size);

                if (iQerr != null)
                    QError = Marshal.AllocHGlobal(size);
                else
                    QError = IntPtr.Zero;

                Marshal.Copy(iQ, 0,Q, iQ.Length);
                Marshal.Copy(iR, 0, Refl, iR.Length);
                Marshal.Copy(iRerr, 0, ReflError, iRerr.Length);

                if (iQerr != null)
                    Marshal.Copy(iQerr, 0, QError, iQerr.Length);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

        }
        #endregion

        #region IDisposable Members

        void IDisposable.Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                // Call the appropriate methods to clean up
                // unmanaged resources here.
                // If disposing is false,
                // only the following code is executed.
                if (Q != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(Q);
                    Marshal.FreeHGlobal(Refl);
                    Marshal.FreeHGlobal(ReflError);

                    if (QError != IntPtr.Zero)
                        Marshal.FreeHGlobal(QError);
                }
                // Note disposing has been done.
                disposed = true;
            }
        }

        #endregion
    }
   
    /// <summary>
    /// Serializes/Deserializes a ModelSettings class
    /// </summary>
    class MySettings
    {
        public ModelSettings Settings;

        public MySettings()
        {
            Settings = new ModelSettings();
        }

        public bool PopulateSettings(string settingsfile)
        {
            if (File.Exists(settingsfile))
            {
                try
                {
                    if (settingsfile.Length >= 260)
                    {
                        throw new Exception("File Path has to many characters, please relocate");
                    }
                    FileStream settings = new FileStream(settingsfile, FileMode.Open);
                    XmlSerializer xmls = new XmlSerializer(typeof(ModelSettings));
                    Settings = (ModelSettings)xmls.Deserialize(settings);
                    settings.Close();
                    return true;
                }
                catch (Exception ex)
                {
                    System.Windows.Forms.MessageBox.Show(ex.Message + " - settings file is likely from an older Stochfit version");
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        public void WriteSettings(string settingsfile)
        {
            try
            {
                FileStream settings = new FileStream(settingsfile, FileMode.Create);
                XmlSerializer xmls = new XmlSerializer(typeof(ModelSettings));
                xmls.Serialize(settings, Settings);
                settings.Close();
            }
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show(ex.Message);
            }
        }
    }
}
