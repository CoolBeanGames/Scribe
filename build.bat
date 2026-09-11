@echo off
setlocal

:: Visual Studio 2022 developer environment
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

set CMAKE=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
set QTDIR=C:\Qt\6.9.3\msvc2022_64
set SRCDIR=C:\Users\haptic\Documents\git\Scribe
set BLDDIR=C:\Users\haptic\Documents\git\Scribe\build\release

set "PATH=%QTDIR%\bin;C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"

if not exist "%BLDDIR%" mkdir "%BLDDIR%"

echo.
echo === Configuring CMake ===
"%CMAKE%" -S "%SRCDIR%" -B "%BLDDIR%" -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=%QTDIR%" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl
if %errorlevel% neq 0 (
    echo [FAIL] CMake configure failed
    exit /b %errorlevel%
)

echo.
echo === Building ===
"%CMAKE%" --build "%BLDDIR%" --config Release
if %errorlevel% neq 0 (
    echo [FAIL] Build failed
    exit /b %errorlevel%
)

echo.
echo === Deploy Qt DLLs ===
if exist "%BLDDIR%\Scribe.exe" (
    windeployqt.exe --release --no-translations "%BLDDIR%\Scribe.exe"
    echo [OK] Executable: %BLDDIR%\Scribe.exe
) else (
    echo [WARN] Executable not found
)

echo.
echo === DONE ===
endlocal
