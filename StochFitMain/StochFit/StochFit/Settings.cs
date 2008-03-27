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
    [Serializable]
    public class SettingsStruct
    {
        //Surface Settings
        public string SurflayerSLD;
        public string Surflayerlength;
        public string SurflayerAbs;

        //Substrate Settings
        public string SubSLD;
        public string SubAbs;

        //Superphase Settings
        public string SupSLD;
        public string SupAbs;

        //Misc Settings
        public string Wavelength;
        public string BoxCount;
        public string Iterations;
        public string IterationsCompleted;
        public string Percerror;
        public bool ImpNorm;

        public string Title;
        public string Resolution;
        public string length;
        public string CritEdgeOffset;
        public string HighQOffset;
        public string SigmaSearchPerc;
        public string ChiSquare;
        public bool UseAbs;
        public bool Forcenorm;
        public string Algorithm;
        public string FitFunc;
        public string SupOffset;
        public bool Debug;
        public bool ForceXR;

        //Annealing Settings
        public double AnnealInitTemp;
        public int AnnealTempPlat;
        public double AnnealSlope;
        public double AnnealGamma;
        public int STUNfunc;
        public bool STUNAdaptive;
        public int STUNtempiter;
        public int STUNdeciter;
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
