SET UtilPath=%~dp0
SET MKLPath=%ProgramFiles%\Intel\Compiler\11.0\066\cpp\mkl\tools\builder
SET VSPath=%ProgramFiles%\Microsoft Visual Studio 9.0\vc\bin
SET INTELPATH=%ProgramFiles%\Intel\Compiler\11.0\066\cpp\bin

::Set the MSVC environmental variables
::call "%INTELPATH%\iclvars.bat" ia32
call "%VSPATH%\vcvars32.bat"

@set LIB=%LIB%;C:\Program Files\Intel\Compiler\11.0\066\cpp\lib\ia32
@echo %LIB%
::Make our custom MKL dll so we don't have to include the full library
copy "%UtilPath%bin\dlls\Myfuncs.txt" "%MKLPath%"
cd "%MKLPath%"

nmake ia32 export=Myfuncs.txt name=levmarmkl"

copy levmarmkl.dll "%UtilPath%bin\dlls"
copy levmarmkl.lib "%UtilPath%bin\dlls"

cd "%UtilPath%"