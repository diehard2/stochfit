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

using StochasticModeling.Settings;
using System.Runtime.InteropServices;

namespace StochasticModeling
{
    /// <summary>
    /// All interop calculations are accessed from this class as static mehtods
    /// </summary>
    public class Calculations
    {

        /// <summary>
        /// Initialize the model independent fitting procedures. This must be called before any other model independent routines
        /// </summary>
        /// <param name="settings">Struct of type <see cref="ModelSettings"/> containing all parameters</param>
        [DllImport("stochfitdll.dll", EntryPoint = "Init", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void Init([MarshalAs(UnmanagedType.LPStruct)] ModelSettings settings);

        /// <summary>
        /// Sets the windows priority of the model independent algorithm thread
        /// </summary>
        /// <param name="priority">Integer corresponding to priority. 0 = Idle, 1 = Below Idle, 2 = Normal</param>
        /// <returns></returns>
        [DllImport("stochfitdll.dll", EntryPoint = "GenPriority", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void GenPriority(int priority);

        /// <summary>
        /// Starts the model idependent fitting process 
        /// </summary>
        /// <param name="iterations">Number of iterations to run the model independent fit</param>
        /// <returns></returns>
        [DllImport("stochfitdll.dll", EntryPoint = "Start", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void Start(int iterations);

        /// <summary>
        /// Cancel the model independent fit
        /// </summary>
        /// <returns></returns>
        [DllImport("stochfitdll.dll", EntryPoint = "Cancel", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void CancelFit();

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
        public static extern int GetData(double[] ZRange, double[] Rho, double[] QRange, double[] Refl, out double roughness, out double chisquare, out double goodnessoffit, out bool isfinished);


        /// <summary>
        /// Gets the size needed to store the Reflectivity and Electron Density arrays generated by the model independent algorithm
        /// </summary>
        /// <param name="RhoSize">Size of the Electron Density Array</param>
        /// <param name="Reflsize">Size of the Reflectivity Array</param>
        [DllImport("stochfitdll.dll", EntryPoint = "ArraySizes", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void ArraySizes(out int RhoSize, out int Reflsize);

        /// <summary>
        /// Return true if the numerical routines are ready to return data, false otherwise
        /// </summary>
        /// <returns></returns>
        [DllImport("stochfitdll.dll", EntryPoint = "WarmedUp", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool WarmedUp();

        /// <summary>
        /// Retrieves the current values for the simulated annealing parameters
        /// </summary>
        /// <param name="lowestenergy">The lowest energy found so far by the SA algorithm</param>
        /// <param name="temp">The current annealing temperature</param>
        /// <param name="mode">Not used</param>
        [DllImport("stochfitdll.dll", EntryPoint = "SAparams", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void SAparams(out double lowestenergy, out double temp, out int mode);



        /// <summary>
        /// Generates a reflectivity using the Nevot-Croce correction to the Parratt recursion
        /// </summary>
        /// <param name="InitStruct"></param>
        /// <param name="parameters"></param>
        /// <param name="parametersize"></param>
        /// <param name="ReflectivityMap"></param>
        [DllImport("LevMardll.dll", EntryPoint = "FastReflGenerate", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void FastReflGenerate([MarshalAs(UnmanagedType.LPStruct)]BoxModelSettings InitStruct, double[] parameters, int parametersize, double[] ReflectivityMap);

        /// <summary>
        /// Performs a Levenberg-Marquadt least squares fit of Reflectivity data
        /// </summary>
        /// <param name="InitStruct"></param>
        /// <param name="parameters"></param>
        /// <param name="covar"></param>
        /// <param name="covarsize"></param>
        /// <param name="info"></param>
        /// <param name="infosize"></param>
        [DllImport("LevMardll.dll", EntryPoint = "FastReflfit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void FastReflfit([MarshalAs(UnmanagedType.LPStruct)] BoxModelSettings InitStruct, double[] parameters, double[] covar, int covarsize, double[] info, int infosize);

        /// <summary>
        /// Generates a smoothed and box electron density profile
        /// </summary>
        /// <param name="InitStruct"></param>
        /// <param name="parameters"></param>
        /// <param name="parametersize"></param>
        /// <param name="ED"></param>
        /// <param name="BoxED"></param>
        [DllImport("LevMardll.dll", EntryPoint = "RhoGenerate", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void RhoGenerate([MarshalAs(UnmanagedType.LPStruct)]BoxModelSettings InitStruct, double[] parameters, int parametersize, double[] ED, double[] BoxED);

        /// <summary>
        ///  Fits an electron density profile to the electron density profile generated by the model independent fit
        /// </summary>
        /// <param name="InitStruct"></param>
        /// <param name="parameters"></param>
        /// <param name="covar"></param>
        /// <param name="covarsize"></param>
        /// <param name="info"></param>
        /// <param name="infosize"></param>
        [DllImport("LevMardll.dll", EntryPoint = "Rhofit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void Rhofit([MarshalAs(UnmanagedType.LPStruct)] BoxModelSettings InitStruct, double[] parameters, double[] covar, int covarsize, double[] info, int infosize);

        /// <summary>
        /// Performs a stochastic search of the parameter space using a constrained Levenberg-Marquadt least squares 
        /// minimization
        /// </summary>
        /// <param name="InitStruct"></param>
        /// <param name="parameters"></param>
        /// <param name="covar"></param>
        /// <param name="paramsize"></param>
        /// <param name="info"></param>
        /// <param name="ParamArray"></param>
        /// <param name="chisquarearray"></param>
        /// <param name="paramarraysize"></param>
        [DllImport("LevMardll.dll", EntryPoint = "StochFit", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void ConstrainedStochFit([MarshalAs(UnmanagedType.LPStruct)] BoxModelSettings InitStruct, double[] parameters, double[] covar, int paramsize, double[] info, double[] ParamArray, double[] chisquarearray, ref int paramarraysize);
    }
}
