@echo off
setlocal

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

set CMAKE=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
set QTDIR=C:\Qt\6.9.3\msvc2022_64
set SRCDIR=C:\Users\haptic\Documents\git\Scribe
set BLDDIR=C:\Users\haptic\Documents\git\Scribe\build\debug
set OUTDIR=C:\Users\haptic\Documents\git\Scribe\builds\debug

set "PATH=%QTDIR%\bin;C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"

if not exist "%BLDDIR%" mkdir "%BLDDIR%"
if not exist "%OUTDIR%" mkdir "%OUTDIR%"

echo === Configuring CMake (Debug) ===
"%CMAKE%" -S "%SRCDIR%" -B "%BLDDIR%" -G Ninja -DCMAKE_BUILD_TYPE=Debug "-DCMAKE_PREFIX_PATH=%QTDIR%" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl
if %errorlevel% neq 0 exit /b %errorlevel%

echo === Building (Debug) ===
"%CMAKE%" --build "%BLDDIR%" --config Debug
if %errorlevel% neq 0 exit /b %errorlevel%

echo === Copying to builds\debug ===
copy /y "%BLDDIR%\Scribe.exe" "%OUTDIR%\Scribe.exe"
if exist "%BLDDIR%\Scribe.pdb" copy /y "%BLDDIR%\Scribe.pdb" "%OUTDIR%\Scribe.pdb"

echo === Deploying Qt Debug DLLs ===
windeployqt.exe --debug --compiler-runtime --no-translations "%OUTDIR%\Scribe.exe"

echo === DONE DEBUG BUILD ===
endlocal
