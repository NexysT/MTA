@echo off
setlocal
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo ERROR: vswhere.exe not found.
  pause
  exit /b 1
)
for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -property installationPath`) do set "VSROOT=%%I"
if not defined VSROOT (
  echo ERROR: Visual Studio not found.
  pause
  exit /b 1
)
call "%VSROOT%\Common7\Tools\VsDevCmd.bat" -arch=x86 -host_arch=x64
if errorlevel 1 (pause & exit /b 1)

if not exist build mkdir build
del /q "build\playerbots_native.dll" "build\playerbots_native.lib" "build\playerbots_native.exp" "build\playerbots_native.obj" 2>nul

cl /nologo /c /O2 /Oi- /GS- /GR- /Zl /Fo"build\playerbots_native.obj" "playerbots_native_windows_x86.cpp"
if errorlevel 1 (
  echo.
  echo ERROR: C++ compilation failed.
  pause
  exit /b 1
)

link /nologo /dll /noentry /nodefaultlib /machine:x86 /out:"build\playerbots_native.dll" "build\playerbots_native.obj" kernel32.lib
if errorlevel 1 (
  echo.
  echo ERROR: linker failed. No valid DLL was produced.
  pause
  exit /b 1
)

echo.
echo OK: build\playerbots_native.dll
pause
endlocal
