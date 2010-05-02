SET UtilPath=%~dp0
SET MKLPath=C:\Program Files (x86)\Intel\Compiler\11.1\051\mkl\tools\builder
SET VSPath=C:\Program Files (x86)\Microsoft Visual Studio 9.0\vc
SET INTELPATH=C:\Program Files (x86)\Intel\Compiler\11.1\051\bin

::Set the MSVC environmental variables
SET VAR=x86
call "%VSPATH%\vcvarsall.bat" %ABC%
SET VAR=ia32
call "C:\Program Files (x86)\Intel\Compiler\11.1\051\bin\iclvars.bat" %CDE%

::@set LIB=%LIB%;C:\Program Files (x86)\Intel\Compiler\11.0\066\cpp\lib\ia32
::@echo %LIB%
::Make our custom MKL dll so we don't have to include the full library
::copy "%UtilPath%bin\Myfuncs.txt" "%MKLPath%"
cd "%MKLPath%"

nmake ia32 export="%UtilPath%..\bin\Myfuncs.txt" name=levmarmkl32


::Set the MSVC environmental variables
SET ABC=amd64
call "%VSPATH%\vcvarsall.bat" %ABC%
SET CDE=intel64
call "C:\Program Files (x86)\Intel\Compiler\11.1\051\bin\iclvars.bat" %CDE%
nmake em64t interface=lp64 export="%UtilPath%\..\bin\Myfuncs.txt" name=levmarmkl64

copy levmarmkl32.dll "%UtilPath%..\bin\x86\dlls"
copy levmarmkl32.lib "%UtilPath%..\bin\x86\dlls"
copy levmarmkl64.dll "%UtilPath%..\bin\x64\dlls"
copy levmarmkl64.lib "%UtilPath%..\bin\x64\dlls"

cd "%UtilPath%"