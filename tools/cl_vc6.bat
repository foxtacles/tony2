@echo off
rem Compile a translation unit with the VC6 RTM toolchain (cl 12.00.8168) on Windows.
rem The GSM voice codec library linked into TONY2.EXE was built with VC6 while the game
rem code is VC5; CMake invokes this wrapper for the VC6-vintage TUs (native Windows
rem counterpart of tools/cl_vc6.sh, which drives the same compiler under Wine).
rem
rem Usage: cl_vc6.bat <MSVC600_DIR> <cl args...>
setlocal
set "VC6DIR=%~1"
if not exist "%VC6DIR%\VC98\Bin\VCVARS32.BAT" (
  echo cl_vc6.bat: VCVARS32.BAT not found in "%VC6DIR%" 1>&2
  exit /b 1
)
call "%VC6DIR%\VC98\Bin\VCVARS32.BAT" x86 >nul
shift
set "CLARGS="
:loop
if "%~1"=="" goto run
set "CLARGS=%CLARGS% %1"
shift
goto loop
:run
cl%CLARGS%
exit /b %errorlevel%
