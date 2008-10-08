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
    /// <summary>
    /// This class passes all of the settings to the C++ dll. If a value is added that is not going to be passed
    /// to the dll, it should be added to the end of the parameter list. If a value is added towards the top, 
    /// SettingsStruct.h in the C++ dll section will need to be appropriately updated
    /// </summary>
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
        public int SigmaSearchPerc;
        public int NormalizationSearchPerc;
        public int AbsorptionSearchPerc;
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
        /// Low Q offset in datapoints from the beginning of the curve
        /// </summary>
        public int CritEdgeOffset;
        /// <summary>
        /// High Q offset in datapoints from the end of the curve
        /// </summary>
        public int HighQOffset;


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
        

        public bool IsNeutron;
        public string Version = "0.0.0";
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

        public void Dispose()
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

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode, Pack = 8)]
    public class BoxModelSettings : IDisposable
    {
        #region Variables

        public string Directory;
        public IntPtr Q = IntPtr.Zero;
        public IntPtr Refl = IntPtr.Zero;
        public IntPtr ReflError = IntPtr.Zero;
        public IntPtr QError = IntPtr.Zero;
        public IntPtr UL = IntPtr.Zero;
        public IntPtr LL = IntPtr.Zero;
        public IntPtr ParamPercs = IntPtr.Zero;
        public int QPoints;
        public bool OneSigma;
        public bool WriteFiles;
        public double SubSLD;
        public double SupSLD;
        public int Boxes;
        public double Wavelength;
        public double QSpread;
        public bool Forcenorm;
        public bool ImpNorm;
        public int FitFunc;


        public int LowQOffset;
        public int HighQOffset;
        public int Iterations;

        //EDP Specific Settings
        public IntPtr MIEDP = IntPtr.Zero;
        public IntPtr ZIncrement = IntPtr.Zero;
        public int ZLength;
 
         [XmlIgnoreAttribute]
        private bool disposed = false;

        #endregion

        #region Constructor/Destructor
        /// <summary>
        /// Constructor
        /// </summary>
        public BoxModelSettings()
        { }

        ~BoxModelSettings()
        {
            Dispose(false);
        }
        #endregion

        #region Public Methods
        public void SetArrays(double[] iQ, double[] iR, double[] iRerr, double[] iQerr, 
            double[] ParamPercs, double[] UL, double[] LL)
        {
            int datasize = Marshal.SizeOf(iQ[0]) * iQ.Length;

            try
            {
                Q = Marshal.AllocHGlobal(datasize);
                Refl = Marshal.AllocHGlobal(datasize);
                ReflError = Marshal.AllocHGlobal(datasize);
                QPoints = iQ.Length;

                if (iQerr != null)
                {
                    QError = Marshal.AllocHGlobal(datasize);
                    Marshal.Copy(iQerr, 0, QError, iQerr.Length);
                }
                else
                    QError = IntPtr.Zero;

                Marshal.Copy(iQ, 0, Q, iQ.Length);
                Marshal.Copy(iR, 0, Refl, iR.Length);
                Marshal.Copy(iRerr, 0, ReflError, iRerr.Length);
              
                if (ParamPercs != null)
                {
                    this.ParamPercs = Marshal.AllocHGlobal(Marshal.SizeOf(ParamPercs[0]) * ParamPercs.Length);
                    Marshal.Copy(ParamPercs, 0, this.ParamPercs, ParamPercs.Length);
                }
                else
                    this.ParamPercs = IntPtr.Zero;

                if (LL != null)
                {
                    this.LL = Marshal.AllocHGlobal(Marshal.SizeOf(LL[0])*LL.Length);
                    Marshal.Copy(LL, 0, this.LL, LL.Length);
                }
                else
                    this.LL = IntPtr.Zero;

                if (UL != null)
                {
                    this.UL = Marshal.AllocHGlobal(Marshal.SizeOf(UL[0])*UL.Length);
                    Marshal.Copy(UL, 0, this.UL, UL.Length);
                }
                else
                    this.UL = IntPtr.Zero;
                ZLength = 100;

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

        }

        public void SetZ(double[] Z, double[] iMIEDP)
        {
            try
            {
                ZIncrement = Marshal.AllocHGlobal(Marshal.SizeOf(Z[0]) * Z.Length);
                Marshal.Copy(Z, 0, ZIncrement, Z.Length);

                if (iMIEDP != null)
                {
                    MIEDP = Marshal.AllocHGlobal(Marshal.SizeOf(iMIEDP[0]) * iMIEDP.Length);
                    Marshal.Copy(iMIEDP, 0, MIEDP, iMIEDP.Length);
                }
                ZLength = Z.Length;

            }
            catch { }

        }
        #endregion

        #region IDisposable Members

        public void Dispose()
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
                    if (ParamPercs != IntPtr.Zero)
                        Marshal.FreeHGlobal(ParamPercs);
                    if (UL != IntPtr.Zero)
                        Marshal.FreeHGlobal(UL);
                    if (LL != IntPtr.Zero)
                        Marshal.FreeHGlobal(LL);
                    if (MIEDP != IntPtr.Zero)
                        Marshal.FreeHGlobal(MIEDP);
                    if (ZIncrement != IntPtr.Zero)
                        Marshal.FreeHGlobal(ZIncrement);
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

                    Version v1 = new Version(Settings.Version), v2 = new Version(Properties.Settings.Default.ResumeBreakingVersion);
                    if (v1 < v2) 
                    {
                        if(v1.Major != 0)
                            MessageBox.Show(String.Format("The resume file was built with StochFit {0}, and cannot be reliably loaded in this version. Please see the help file.",Settings.Version));  
                        else
                            MessageBox.Show(String.Format("The resume file was built with an older version, and cannot be reliably loaded in this version. Please see the help file.", Settings.Version));  

                        return false;
                    }

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
