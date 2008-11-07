SET UtilPath=%~dp0
SET MKLPath=%ProgramFiles%\Intel\Compiler\11.0\061\cpp\mkl\tools\builder
SET VSPath=%ProgramFiles%\Microsoft Visual Studio 9.0\vc\bin

::Set the MSVC environmental variables
if "%MSVCDir%" == "" call "%VSPath%\vcvars32.bat"

::Make our custom MKL dll so we don't have to include the full library
copy "%UtilPath%bin\dlls\Myfuncs.txt" "%MKLPath%"
cd "%MKLPath%"

nmake ia32 export=Myfuncs.txt name=levmarmkl"

copy levmarmkl.dll "%UtilPath%bin\dlls"
copy levmarmkl.lib "%UtilPath%bin\dlls"

cd "%UtilPath%"