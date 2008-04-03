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
        protected static extern double FastReflGenerate(int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
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
        /// <param name="covar"></param>
        /// <param name="covarsize"></param>
        /// <param name="info"></param>
        /// <param name="infosize"></param>
        /// <param name="onesigma"></param>
        /// <param name="writefiles"></param>
        /// <param name="iterations"></param>
        /// <param name="ParamArray"></param>
        /// <param name="paramarraysize"></param>
        /// <param name="parampercs"></param>
        /// <param name="chisquarearray"></param>
        /// <param name="Covararray"></param>
        /// <param name="UL"></param>
        /// <param name="LL"></param>
        /// <param name="QSpread">Percent error in Q</param>
        /// <param name="ImpNorm">True if the curve is believed to be imperfectly normalized, false otherwise</param>   
        /// <returns></returns>
        [DllImport("LevMardll.dll", EntryPoint = "ConstrainedStochFit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double ConstrainedStochFit(int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
            double[] QRange, double[] QErrors, int QSize, double[] Reflectivity, int reflectivitysize, double[] Errors, double[] covar, int covarsize, double[] info, int infosize, bool onesigma, bool writefiles, int iterations,
            double[] ParamArray, out int paramarraysize, double[] parampercs, double[] chisquarearray, double[] Covararray, double[] UL, double[] LL, double Qspread, bool ImpNorm);

        [DllImport("LevMardll.dll", EntryPoint = "RhoGenerate", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double RhoGenerate(int boxes, double SLD, double SupSLD, double[] parameters, int paramsize,
           double[] ZRange, int ZSize, double[] ED, double[] BoxED, int EDSize);

        [DllImport("LevMardll.dll", EntryPoint = "FastReflfit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double FastReflfit(string directory, int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
            double[] QRange, double[] QErrors, int QSize, double[] Reflectivity, int reflectivitysize, double[] Errors, double[] covar, int covarsize, double[] info, int infosize,
            bool onesigma, bool writefiles, double Qspread, bool correctfornorm);
     
        [DllImport("LevMardll.dll", EntryPoint = "StochFit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double StochFit(int boxes, double SLD, double SupSLD, double wavelength, double[] parameters, int paramsize,
            double[] QRange, double[] QErrors, int QSize, double[] Reflectivity, int reflectivitysize, double[] Errors, double[] covar, int covarsize, double[] info,
            int infosize, bool onesigma, bool writefiles, int iterations, double[] ParamArray, out int paramarraysize, double[] paramperc, double[] chisquarearray, double[] Covararray,
            double QSpread, bool correctfornorm);
        
        [DllImport("LevMardll.dll", EntryPoint = "Rhofit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern double Rhofit(string directory, int boxes, double SLD, double SupSLD, double[] parameters, int paramsize,
            double[] ZRange, int ZSize, double[] ED, int EDsize, double[] covariance,
            int covarsize, double[] info, int infosize, bool onesigma);

        [DllImport("stochfitdll.dll", EntryPoint = "Init", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void Init(string directory, double[] Q, double[] Refl, double[] ReflError, double[] QErr, int Qpoints, double rholipid, double rhoh2o, double supSLD, int parratlayers,
            double layerlength, double slABS, double XRlambda, double SubAbs, double SupAbs, bool UseAbs, double leftoffset, double Qerr, bool forcenorm,
             double forcesigma, bool debug, bool XRonly, double resolution, double totallength, bool impnorm, int objfunc);
       
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
        
        [DllImport("stochfitdll.dll", EntryPoint = "SetSAParameters", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        protected static extern void SetSAParameters(int sigmasearch, int algorithm, double inittemp, int platiter, double slope, double gamma, int STUNfunc, bool adaptive, int tempiter, int deciter, double gammadec);
        
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
