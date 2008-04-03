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

namespace StochasticModeling
{
    /// <summary>
    /// Serializable class for holding model independent parameters
    /// </summary>
    [Serializable]
    public class SettingsStruct
    {
        //Surface Settings
        /// <summary>
        /// Film SLD
        /// </summary>
        public string SurflayerSLD;
        /// <summary>
        /// Film length
        /// </summary>
        public string Surflayerlength;
        /// <summary>
        /// Film absorption
        /// </summary>
        public string SurflayerAbs;

        //Substrate Settings
        /// <summary>
        /// Substrate SLD
        /// </summary>
        public string SubSLD;
        /// <summary>
        /// Substrate absorption
        /// </summary>
        public string SubAbs;

        //Superphase Settings
        /// <summary>
        /// Superphas SLD
        /// </summary>
        public string SupSLD;
        /// <summary>
        /// Superphase absorption
        /// </summary>
        public string SupAbs;

        //Misc Settings

        /// <summary>
        /// X-ray wavenlength
        /// </summary>
        public string Wavelength;
        /// <summary>
        /// Number of small boxes
        /// </summary>
        public string BoxCount;
        /// <summary>
        /// Number of iterations
        /// </summary>
        public string Iterations;
        /// <summary>
        /// Number of iterations completed
        /// </summary>
        public string IterationsCompleted;
        /// <summary>
        /// Error in Q
        /// </summary>
        public string Percerror;
        /// <summary>
        /// True if correcting for imperfect normalization, false otherwise
        /// </summary>
        public bool ImpNorm;

        /// <summary>
        /// System description
        /// </summary>
        public string Title;
        /// <summary>
        /// Number of point per Angstrom in the electron density profile
        /// </summary>
        public string Resolution;
        /// <summary>
        /// Estimated length of the film + subphase + superphase(set in SupOffset) + 7
        /// </summary>
        public string length;
        /// <summary>
        /// Low Q offset in datapoints from the beginning of the curve
        /// </summary>
        public string CritEdgeOffset;
        /// <summary>
        /// High Q offset in datapoints from the end of the curve
        /// </summary>
        public string HighQOffset;
        /// <summary>
        /// The percentage of time spent searching the roughness parameter space
        /// </summary>
        public string SigmaSearchPerc;
        /// <summary>
        /// ChiSquare value for the current fit
        /// </summary>
        public string ChiSquare;
        /// <summary>
        /// True if absorption was used, false otherwise
        /// </summary>
        public bool UseAbs;
        /// <summary>
        /// True if the first point in the reflectivity was forced be equal to 1.0
        /// </summary>
        public bool Forcenorm;
        /// <summary>
        /// Algorithm for the model independent fit (Greedy search = 0; Simulated Annealing = 1; STUN Annealing = 2)
        /// </summary>
        public string Algorithm;
        /// <summary>
        /// Fitness function. See documentation for further details
        /// </summary>
        public string FitFunc;
        /// <summary>
        /// The distance in angstroms from Z = 0 to the first small box (default of 35)
        /// </summary>
        public string SupOffset;
        /// <summary>
        /// Writes several debug files. This can be useful for determining the progression of a fit
        /// </summary>
        public bool Debug;
        /// <summary>
        /// Severely penalizes fits with negative electron density
        /// </summary>
        public bool ForceXR;

        //Annealing Settings
        /// <summary>
        /// Initial annealing temperature
        /// </summary>
        public double AnnealInitTemp;
        /// <summary>
        /// Number of iterations before <see cref="AnnealInitTemp"/> is decreased
        /// </summary>
        public int AnnealTempPlat;
        /// <summary>
        /// Percentage by which <see cref="AnnealInitTemp"/> is decreased after <see cref="AnnealTempPlat"/> iterations
        /// </summary>
        public double AnnealSlope;
        /// <summary>
        /// The Gamma parameter for STUN tunneling
        /// </summary>
        public double AnnealGamma;
        /// <summary>
        /// The STUN function utilized. See the documentation for more details
        /// </summary>
        public int STUNfunc;
        /// <summary>
        /// Whether STUN annealing is adaptive or not
        /// </summary>
        public bool STUNAdaptive;
        public int STUNtempiter;
        public int STUNdeciter;
        /// <summary>
        /// The percentage to change Gamma by depending on the circumstances in adaptive STUN annealing
        /// </summary>
        public double STUNgammadec;
    }

    class MySettings
    {
        public SettingsStruct Settings;

        public MySettings()
        {
            Settings = new SettingsStruct();
        }

        public bool PopulateSettings(string settingsfile)
        {
            if (File.Exists(settingsfile))
            {
                try
                {
                    FileStream settings = new FileStream(settingsfile, FileMode.Open);
                    XmlSerializer xmls = new XmlSerializer(typeof(SettingsStruct));
                    Settings = (SettingsStruct)xmls.Deserialize(settings);
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
                XmlSerializer xmls = new XmlSerializer(typeof(SettingsStruct));
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
