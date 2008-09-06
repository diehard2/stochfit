:: This script file builds all of the versions of STochFit. It requires the 
:: Intel compiler, Microsoft Visual Studio, and the Intel MKL.
:: You may have to change the paths below if you have a different version.

SET MKLVER=10.0.2.019
SET UtilPath=%~dp0
SET MKLPath=%ProgramFiles%\Intel\MKL\%MKLVER%\tools\builder
SET VSPath=%ProgramFiles%\Microsoft Visual Studio 8\vc\bin

::Set the MSVC environmental variables
if "%MSVCDir%" == "" call "%VSPath%\vcvars32.bat"

::Make directories if they don't already exist
mkdir "%UtilPath%bin\Release"
mkdir "%UtilPath%bin\Debug"
mkdir "%UtilPath%bin\GIXOS"

::Make our custom MKL dll so we don't have to include the full library
copy "%UtilPath%bin\dlls\Myfuncs.txt" "%MKLPath%"
cd "%MKLPath%"

nmake ia32 export=Myfuncs.txt name=levmarmkl"

copy levmarmkl.dll "%UtilPath%bin\dlls"
copy levmarmkl.lib "%UtilPath%bin\dlls"


::Copy the dlls we need to the program directories

copy  "%UtilPath%StochFit\StochFit\genf.ico"  "%UtilPath%bin\Release"
copy  "%UtilPath%bin\dlls\levmarmkl.dll"  "%UtilPath%bin\Release"
copy  "%UtilPath%bin\dlls\libguide40.dll"  "%UtilPath%bin\Release"
copy  "%UtilPath%bin\dlls\libmmd.dll"  "%UtilPath%bin\Release"
copy  "%UtilPath%bin\dlls\Model Independent Fitting Tutorial.pdf" "%UtilPath%bin\Release"

copy  "%UtilPath%StochFit\StochFit\genf.ico"  "%UtilPath%bin\GIXOS"
copy  "%UtilPath%bin\dlls\levmarmkl.dll"  "%UtilPath%bin\GIXOS"
copy  "%UtilPath%bin\dlls\libguide40.dll"  "%UtilPath%bin\GIXOS"
copy  "%UtilPath%bin\dlls\libmmd.dll"  "%UtilPath%bin\GIXOS"
copy  "%UtilPath%bin\dlls\Model Independent Fitting Tutorial.pdf" "%UtilPath%bin\GIXOS"

copy  "%UtilPath%StochFit\StochFit\genf.ico"  "%UtilPath%bin\Debug"
copy  "%UtilPath%bin\dlls\levmarmkl.dll"  "%UtilPath%bin\Debug"
copy  "%UtilPath%bin\dlls\libguide40.dll"  "%UtilPath%bin\Debug"
copy  "%UtilPath%bin\dlls\libmmdd.dll"  "%UtilPath%bin\Debug"
copy  "%UtilPath%bin\dlls\Model Independent Fitting Tutorial.pdf" "%UtilPath%bin\Debug"

cd %UtilPath%

::Build the StochFit front end

::"%ProgramFiles%\Microsoft Visual Studio 8\Common7\IDE\devenv.exe" /rebuild Debug StochFit.sln 
::"%ProgramFiles%\Microsoft Visual Studio 8\Common7\IDE\devenv.exe" /rebuild Release StochFit.sln 





