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
using System.Windows.Forms;
using System.Collections;
using System.IO;
using System.Text.RegularExpressions;
using System.Globalization;
using StochasticModeling.Settings;
using System.Runtime.InteropServices;

namespace StochasticModeling
{
    /// <summary>
    /// Base class for almost all of our forms that require numerical input
    /// </summary>
    public class StochFormBase : Form
    {
        /// <summary>
        /// Calculates a fit for a given reflectivity based on give parameter restraints
        /// </summary>
        /// <param name="directory">The working directory</param>
        /// <param name="boxes">Number of boxes in the model</param>
        /// <param name="SLD">Subphase SLD</param>
        /// <param name="SupSLD">Superphase SLD</param>
        /// <param name="wavelength">Wavelength in Angstroms</param>
        /// <param name="parameters">The parameter array</param>
        /// <param name="paramsize">The size of the parameter array</param>
        /// <param name="QRange">Q array</param>
        /// <param name="QErrors">Error in Q array if it exists. NULL otherwise</param>
        /// <param name="QSize">Size of QRange</param>
        /// <param name="Reflectivity">Array that will recieve the reflectivity.</param>
        /// <param name="reflectivitysize">Size of Reflectivity</param>
        /// <param name="Errors">Array containing the error for Reflectivity in standard deviation</param>
        /// <param name="covar">Array which recieves the covariance matrix</param>
        /// <param name="covarsize">Size of covar</param>
        /// <param name="info">Array which recieves information regarding the terminating conditions of the fit</param>
        /// <param name="infosize">Size of info</param>
        /// <param name="onesigma">True if the system can be treated as an elastic sheet</param>
        /// <param name="writefiles">Write output files to the specified directory</param>
        /// <param name="UL">array specifying the upper bounds for the corresponding element in the parameter array</param>
        /// <param name="LL">array specifying the lower bounds for the corresponding element in the parameter array</param>
        /// <param name="QSpread">Error in Q as a fixed percentage</param>
        /// <param name="ImpNorm">Choose whether to correct the reflectivity for imperfect normalization</param>
        /// <returns></returns>
        [DllImport("LevMardll.dll", EntryPoint = "ConstrainedFastReflfit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double ConstrainedFastReflfit(string directory, int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
            double[] QRange, double[] QErrors, int QSize, double[] Reflectivity, int reflectivitysize, double[] Errors, double[] covar, int covarsize, double[] info, int infosize, bool onesigma, bool writefiles,
            double[] UL, double[] LL, double QSpread, bool ImpNorm);

        /// <summary>
        /// Generates a reflectivity using the Nevot-Croce correction to the Parratt recursion
        /// </summary>
        /// <param name="boxes">Number of boxes in the model</param>
        /// <param name="SLD">Subphase SLD</param>
        /// <param name="SupSLD">Superphase SLD</param>
        /// <param name="wavelength">X-ray wavelength</param>
        /// <param name="parameters">Parameter array. See code for setup</param>
        /// <param name="paramsize">Element count of parameters</param>
        /// <param name="QRange">Array of Q values to calculate reflectivity points for</param>
        /// <param name="QError">Array of error in Q. Can be NULL</param>
        /// <param name="QSize">QRange element count</param>
        /// <param name="Reflectivity">Allocated array of QSize elements that recieves the </param>
        /// <param name="reflectivitysize">Element count of the reflectivity array</param>
        /// <param name="QSpread">Percent error in Q</param>
        /// <param name="ImpNorm">True if the curve is believed to be imperfectly normalized, false otherwise</param>
        /// <returns></returns>
        [DllImport("LevMardll.dll", EntryPoint = "FastReflGenerate", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void FastReflGenerate(int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
            double[] QRange, double[] QError, int QSize, double[] Reflectivity, int reflectivitysize, double QSpread, bool ImpNorm);

        /// <summary>
        /// Performs a constrained Levenberg-Marquadt least squares fit of the data
        /// </summary>
        /// <param name="boxes">Number of boxes in the model</param>
        /// <param name="SLD">Subphase SLD</param>
        /// <param name="SupSLD">Superphase SLD</param>
        /// <param name="wavelength">X-ray wavelength</param>
        /// <param name="parameters">Parameter array. See code for setup</param>
        /// <param name="paramsize">Element count of parameters</param>
        /// <param name="QRange">Array of Q values to calculate reflectivity points for</param>
        /// <param name="QError">Array of error in Q. Can be NULL</param>
        /// <param name="QSize">QRange element count</param>
        /// <param name="Reflectivity">Allocated array of QSize elements that recieves the </param>
        /// <param name="reflectivitysize">Element count of the reflectivity array</param>
        /// <param name="Errors">Array of errors in the reflectivity data</param>
        /// <param name="covar">The original covariance array to fill</param>
        /// <param name="covarsize">Element count of covar</param>
        /// <param name="info">Array of size 6, returns information pertaining to the fit</param>
        /// <param name="infosize">Element count of info</param>
        /// <param name="onesigma">If the film is to be treated as an elastic sheet, set to true</param>
        /// <param name="writefiles">Set to true to write files</param>
        /// <param name="iterations">Total number of iterations for the stochastic fitting</param>
        /// <param name="ParamArray">Returns the array of all fits. Allocated to a size of 1000 * paramsize</param>
        /// <param name="paramarraysize">Element count of ParamArray</param>
        /// <param name="parampercs">The percentage by which each parameter can vary, with an element count of 6. 1 and 2 correspond to the thickness
        /// of the file, 2 and 3 correspond to the electron density, and 4 and 5 correspond to the roughness</param>
        /// <param name="chisquarearray">Array of Chi square values for each model found</param>
        /// <param name="Covararray">Covariance array for each model found</param>
        /// <param name="UL">Array of upper limits of paramsize elements for each parameter in ParamArray</param>
        /// <param name="LL">Array of lower limits of paramsize elements for each parameter in ParamArray</param>
        /// <param name="QSpread">Percent error in Q</param>
        /// <param name="ImpNorm">True if the curve is believed to be imperfectly normalized, false otherwise</param>   
        [DllImport("LevMardll.dll", EntryPoint = "ConstrainedStochFit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void ConstrainedStochFit(int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
            double[] QRange, double[] QError, int QSize, double[] Reflectivity, int reflectivitysize, double[] Errors, double[] covar, int covarsize, double[] info, int infosize, bool onesigma, bool writefiles, int iterations,
            double[] ParamArray, out int paramarraysize, double[] parampercs, double[] chisquarearray, double[] Covararray, double[] UL, double[] LL, double QSpread, bool ImpNorm);

        /// <summary>
        /// Generates a smoothed and box electron density profile
        /// </summary>
        /// <param name="boxes"></param>
        /// <param name="SLD"></param>
        /// <param name="SupSLD"></param>
        /// <param name="parameters"></param>
        /// <param name="paramsize"></param>
        /// <param name="ZRange"></param>
        /// <param name="ZSize"></param>
        /// <param name="ED"></param>
        /// <param name="BoxED"></param>
        /// <param name="EDSize"></param>
        [DllImport("LevMardll.dll", EntryPoint = "RhoGenerate", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void RhoGenerate(int boxes, double SLD, double SupSLD, double[] parameters, int paramsize,
           double[] ZRange, int ZSize, double[] ED, double[] BoxED, int EDSize);

        /// <summary>
        /// Performs a Levenberg-Marquadt least squares fit of the data
        /// </summary>
        /// <param name="directory">The working directory</param>
        /// <param name="boxes">Number of boxes in the model</param>
        /// <param name="SLD">Subphase SLD</param>
        /// <param name="SupSLD">Superphase SLD</param>
        /// <param name="wavelength">Wavelength in Angstroms</param>
        /// <param name="parameters">The parameter array</param>
        /// <param name="paramsize">The size of the parameter array</param>
        /// <param name="QRange">Q array</param>
        /// <param name="QErrors">Error in Q array if it exists. NULL otherwise</param>
        /// <param name="QSize">Size of QRange</param>
        /// <param name="Reflectivity">Array that will recieve the reflectivity</param>
        /// <param name="reflectivitysize">Size of Reflectivity</param>
        /// <param name="Errors">Array containing the error for Reflectivity in standard deviation</param>
        /// <param name="covar">Array which recieves the covariance matrix</param>
        /// <param name="covarsize">Size of covar</param>
        /// <param name="info">Array which recieves information regarding the terminating conditions of the fit</param>
        /// <param name="infosize">Size of info</param>
        /// <param name="onesigma">True if the system can be treated as an elastic sheet</param>
        /// <param name="writefiles">Write output files to the specified directory</param>
        /// <param name="Qspread">Error in Q as a fixed percentage</param>
        /// <param name="ImpNorm">Choose whether to correct the reflectivity for imperfect normalization</param>
        /// <returns>The Chi square value for the fit</returns>
        [DllImport("LevMardll.dll", EntryPoint = "FastReflfit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double FastReflfit(string directory, int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
            double[] QRange, double[] QErrors, int QSize, double[] Reflectivity, int reflectivitysize, double[] Errors, double[] covar, int covarsize, double[] info, int infosize,
            bool onesigma, bool writefiles, double Qspread, bool ImpNorm);

        /// <summary>
        /// Performs a constrained Levenberg-Marquadt least squares fit of the data
        /// </summary>
        /// <param name="boxes">Number of boxes in the model</param>
        /// <param name="SLD">Subphase SLD</param>
        /// <param name="SupSLD">Superphase SLD</param>
        /// <param name="wavelength">X-ray wavelength</param>
        /// <param name="parameters">Parameter array. See code for setup</param>
        /// <param name="paramsize">Element count of parameters</param>
        /// <param name="QRange">Array of Q values to calculate reflectivity points for</param>
        /// <param name="QError">Array of error in Q. Can be NULL</param>
        /// <param name="QSize">QRange element count</param>
        /// <param name="Reflectivity">Allocated array of QSize elements that recieves the </param>
        /// <param name="reflectivitysize">Element count of the reflectivity array</param>
        /// <param name="Errors">Array of errors in the reflectivity data</param>
        /// <param name="covar">The original covariance array to fill</param>
        /// <param name="covarsize">Element count of covar</param>
        /// <param name="info">Array of size 6, returns information pertaining to the fit</param>
        /// <param name="infosize">Element count of info</param>
        /// <param name="onesigma">If the film is to be treated as an elastic sheet, set to true</param>
        /// <param name="writefiles">Set to true to write files</param>
        /// <param name="iterations">Total number of iterations for the stochastic fitting</param>
        /// <param name="ParamArray">Returns the array of all fits. Allocated to a size of 1000 * paramsize</param>
        /// <param name="paramarraysize">Element count of ParamArray</param>
        /// <param name="parampercs">The percentage by which each parameter can vary, with an element count of 6. 1 and 2 correspond to the thickness
        /// of the file, 2 and 3 correspond to the electron density, and 4 and 5 correspond to the roughness</param>
        /// <param name="chisquarearray">Array of Chi square values for each model found</param>
        /// <param name="Covararray">Covariance array for each model found</param>
        /// <param name="QSpread">Percent error in Q</param>
        /// <param name="ImpNorm">True if the curve is believed to be imperfectly normalized, false otherwise</param>  
        [DllImport("LevMardll.dll", EntryPoint = "StochFit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void StochFit(int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
            double[] QRange, double[] QError, int QSize, double[] Reflectivity, int reflectivitysize, double[] Errors, double[] covar, int covarsize, double[] info,
            int infosize, bool onesigma, bool writefiles, int iterations, double[] ParamArray, out int paramarraysize, double[] parampercs, double[] chisquarearray, double[] Covararray,
            double QSpread, bool ImpNorm);
        
        /// <summary>
        /// Fits an electron density profile to the electron density profile generated by the model independent fit
        /// </summary>
        /// <param name="directory"></param>
        /// <param name="boxes"></param>
        /// <param name="SLD"></param>
        /// <param name="SupSLD"></param>
        /// <param name="parameters"></param>
        /// <param name="paramsize"></param>
        /// <param name="ZRange"></param>
        /// <param name="ZSize"></param>
        /// <param name="ED"></param>
        /// <param name="EDsize"></param>
        /// <param name="covariance"></param>
        /// <param name="covarsize"></param>
        /// <param name="info"></param>
        /// <param name="infosize"></param>
        /// <param name="onesigma"></param>
        /// <returns></returns>
        [DllImport("LevMardll.dll", EntryPoint = "Rhofit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double Rhofit(string directory, int boxes, double SLD, double SupSLD, double[] parameters, int paramsize,
            double[] ZRange, int ZSize, double[] ED, int EDsize, double[] covariance,
            int covarsize, double[] info, int infosize, bool onesigma);

        /// <summary>
        /// Initialize the model independent fitting procedures. This must be called before any other model independent routines
        /// </summary>
        /// <param name="settings">Struct of type <see cref="ModelSettings"/> containing all parameters</param>
        [DllImport("stochfitdll.dll", EntryPoint = "Init", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void Init([MarshalAs(UnmanagedType.LPStruct)] ModelSettings settings);
       
        /// <summary>
        /// Sets the windows priority of the model independent algorithm thread
        /// </summary>
        /// <param name="priority">Integer corresponding to priority. 0 = Idle, 1 = Below Idle, 2 = Normal</param>
        /// <returns></returns>
        [DllImport("stochfitdll.dll", EntryPoint = "GenPriority", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void GenPriority(int priority);
        
        /// <summary>
        /// Starts the model idependent fitting process 
        /// </summary>
        /// <param name="iterations">Number of iterations to run the model independent fit</param>
        /// <returns></returns>
        [DllImport("stochfitdll.dll", EntryPoint = "Start", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void Start(int iterations);
        
        /// <summary>
        /// Cancel the model independent fit
        /// </summary>
        /// <returns></returns>
        [DllImport("stochfitdll.dll", EntryPoint = "Cancel", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void CancelFit();
        
        /// <summary>
        /// Retrieves the data from the model independent algorithm
        /// </summary>
        /// <param name="ZRange">Z array for the electron density profile of the model independent fit </param>
        /// <param name="Rho">Electron density array for the model independent fit</param>
        /// <param name="QRange">The Q data corresponding to the model independent fit</param>
        /// <param name="Refl">The reflectivity data for the model independent fit </param>
        /// <param name="roughness">The smoothing parameter sigma for the electron density profile</param>
        /// <param name="chisquare">The Chi Square of the fit</param>
        /// <param name="goodnessoffit">The value of the fitting function</param>
        /// <param name="isfinished">True if the specified number of iterations has been reached, false otherwise</param>
        /// <returns>Number of completed iterations</returns>
        [DllImport("stochfitdll.dll", EntryPoint = "GetData", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern int GetData(double[] ZRange, double[] Rho, double[] QRange, double[] Refl, out double roughness, out double chisquare, out double goodnessoffit, out bool isfinished);
        
               
        /// <summary>
        /// Gets the size needed to store the Reflectivity and Electron Density arrays generated by the model independent algorithm
        /// </summary>
        /// <param name="RhoSize">Size of the Electron Density Array</param>
        /// <param name="Reflsize">Size of the Reflectivity Array</param>
        [DllImport("stochfitdll.dll", EntryPoint = "ArraySizes", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void ArraySizes(out int RhoSize, out int Reflsize);
        
        /// <summary>
        /// Return true if the numerical routines are ready to return data, false otherwise
        /// </summary>
        /// <returns></returns>
        [DllImport("stochfitdll.dll", EntryPoint = "WarmedUp", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern bool WarmedUp();
        
        /// <summary>
        /// Retrieves the current values for the simulated annealing parameters
        /// </summary>
        /// <param name="lowestenergy">The lowest energy found so far by the SA algorithm</param>
        /// <param name="temp">The current annealing temperature</param>
        /// <param name="mode">Not used</param>
        [DllImport("stochfitdll.dll", EntryPoint = "SAparams", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void SAparams(out double lowestenergy, out double temp, out int mode);

        /// <summary>
        /// The culture is set to US for the purposes of inputting data to the numerical routines
        /// </summary>
        protected CultureInfo m_CI = new CultureInfo("en-US");

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to a double or false if not</param>
        protected virtual void ValidateNumericalInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
                base.OnValidating(e);
                Double.Parse(((TextBox)sender).Text);
            }
            catch
            {
                MessageBox.Show("Error in input - A real number was expected");
                e.Cancel = true;
            }
        }

        /// <summary>
        /// Checks to verify that the Textbox has valid numerical input. This check respects cultural variations
        /// in number entry 
        /// </summary>
        /// <param name="sender">A textbox is expected as input</param>
        /// <param name="e">return true if the number can be cast to an integer or false if not</param>
        protected virtual void ValidateIntegerInput(object sender, System.ComponentModel.CancelEventArgs e)
        {
            try
            {
                base.OnValidating(e);
                Convert.ToInt32(((TextBox)sender).Text);
            }
            catch
            {
                MessageBox.Show("Error in input - An integer was expected");
                e.Cancel = true;
            }
        }

        /// <summary>
        /// Sets the "check" state on a menu item
        /// </summary>
        /// <param name="sender">Expects a ToolStripMenuItem</param>
        /// <param name="e"></param>
        protected virtual void MenuItem_Check(object sender, EventArgs e)
         {
             ((ToolStripMenuItem)sender).Checked = !((ToolStripMenuItem)sender).Checked;
         }
    }
}
