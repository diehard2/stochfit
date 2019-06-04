# stochfit

StochFit utilizes stochastic fitting methods to model specular x-ray reflectivity or neutron reflectivity data. It provides an easy to use graphical user interface for both model dependent and model independent methods. Please use the forums for questions, bug reports, or feature requests. You will need a SourceForge user id to access the forums.



If you use StochFit in your published work, the reference is



```
S. M. Danauskas, D. Li, M. Meron, B. Lin and K. Y. C. Lee
Stochastic fitting of specular X-ray reflectivity data using StochFit. J. Appl. Cryst. (2008). 41, 1187-1193
Latest Beta Release 1.7.0
```

The current beta release can be found here. While there are numerous changes and several bugfixes, the numerical routines produce the same output as 1.6.5. If you find any bugs, please report them in the appropriate forum

__Change Log__ </br>
* Fixed .NET framework crash (sorry) 
* MD fitting parameters from the minimization are now reported in the MD stochastic modeling module
* Full data is displayed and written to file regardless of the Q offsets
* Unused MD parameters are now "grayed" out
* MI fit for transparent media is 32% faster than version 1.6.5
* itextsharp (pdf library) was updated to version 4.1.2
* ZedGraph was upgraded to the 5.1.4, added to the StochFit source and modified to fix a bug with high quality copies of graphs
* Removed the Fresnel objective function from the MI routines
* Updated to .NET 3.5
* The percentages of MI iterations to spend on the roughness, imperfect normalization, and absorption search are now separately specifiable. The default is 10% for each option selected
* The MD reflectivity fitting and constrained MD reflectivity fitting have been combined into the same section. When no constraints are specified, the unconstrained minimization (which is faster) is performed
* Reorganization of the code to ease maintenance
* Reorganization of the code that generates the reflectivity.This will allow us to interchange methods of generating fits to XR data.
* Previous fits are now stored in a buffer so the user can see what they have been doing
* The pdf report information and mastergraph are only updated from the MD routines if the user actively saves the fit. Similarly, the user now must specify a file name and location for MD fits.

## Latest stable version 1.6.5

__Change Log__

* Fixed bug in MD routines where multiple roughnesses and normalization fluctuation was specified
* Fixed normalization constant was not respected by the Stochastic MD output window
* Fixed small graphing bug
* Updated random number generator for MD routines to Mersenne twister
* Works on 64 bit versions of Windows XP and Vista
