@echo off
setlocal

echo ============================================
echo  LoLScan - Add Dear ImGui to Project
echo ============================================
echo.

:: Resolve paths relative to this script's location
cd /d "%~dp0"

set IMGUI_DIR=AppImgui\imgui
set IMGUI_VERSION=v1.91.8
set BASE_URL=https://raw.githubusercontent.com/ocornut/imgui/%IMGUI_VERSION%
set FAIL=0

:: Check curl is available
where curl >nul 2>&1
if errorlevel 1 (
    echo ERROR: curl not found. Install curl or use Windows 10+.
    goto :error
)

if exist "%IMGUI_DIR%\imgui.h" (
    echo ImGui already exists in %IMGUI_DIR%. Redownloading...
    echo.
)

if not exist "%IMGUI_DIR%" mkdir "%IMGUI_DIR%"

echo Downloading Dear ImGui %IMGUI_VERSION% ...
echo.

:: Core source files
call :dl "imgui.h"              "%BASE_URL%/imgui.h"
call :dl "imgui.cpp"            "%BASE_URL%/imgui.cpp"
call :dl "imgui_draw.cpp"       "%BASE_URL%/imgui_draw.cpp"
call :dl "imgui_tables.cpp"     "%BASE_URL%/imgui_tables.cpp"
call :dl "imgui_widgets.cpp"    "%BASE_URL%/imgui_widgets.cpp"

:: Core headers
call :dl "imgui_internal.h"     "%BASE_URL%/imgui_internal.h"
call :dl "imconfig.h"           "%BASE_URL%/imconfig.h"
call :dl "imstb_rectpack.h"     "%BASE_URL%/imstb_rectpack.h"
call :dl "imstb_textedit.h"     "%BASE_URL%/imstb_textedit.h"
call :dl "imstb_truetype.h"     "%BASE_URL%/imstb_truetype.h"

:: Backend: Win32 + DirectX 11
call :dl "imgui_impl_win32.h"   "%BASE_URL%/backends/imgui_impl_win32.h"
call :dl "imgui_impl_win32.cpp" "%BASE_URL%/backends/imgui_impl_win32.cpp"
call :dl "imgui_impl_dx11.h"    "%BASE_URL%/backends/imgui_impl_dx11.h"
call :dl "imgui_impl_dx11.cpp"  "%BASE_URL%/backends/imgui_impl_dx11.cpp"

:: ---- Font: JetBrains Mono ----
set FONT_DIR=AppImgui\fonts
if not exist "%FONT_DIR%" mkdir "%FONT_DIR%"
echo.
echo Downloading JetBrains Mono font...
set JBM_URL=https://github.com/JetBrains/JetBrainsMono/raw/master/fonts/ttf
echo   JetBrainsMono-Regular.ttf
curl -sfL "%JBM_URL%/JetBrainsMono-Regular.ttf" -o "%FONT_DIR%\JetBrainsMono-Regular.ttf"
if errorlevel 1 (
    echo     FAILED: JetBrainsMono-Regular.ttf
    set FAIL=1
)
echo   JetBrainsMono-Bold.ttf
curl -sfL "%JBM_URL%/JetBrainsMono-Bold.ttf" -o "%FONT_DIR%\JetBrainsMono-Bold.ttf"
if errorlevel 1 (
    echo     FAILED: JetBrainsMono-Bold.ttf
    set FAIL=1
)
echo   JetBrainsMono-Medium.ttf
curl -sfL "%JBM_URL%/JetBrainsMono-Medium.ttf" -o "%FONT_DIR%\JetBrainsMono-Medium.ttf"
if errorlevel 1 (
    echo     FAILED: JetBrainsMono-Medium.ttf
    set FAIL=1
)

echo.
if %FAIL% equ 0 (
    echo ============================================
    echo  SUCCESS - 14 files downloaded to %IMGUI_DIR%
    echo.
    echo  Open AppImgui.sln in Visual Studio and build.
    echo ============================================
) else (
    echo ============================================
    echo  WARNING: Some downloads failed. Check above.
    echo ============================================
)

pause
exit /b 0

:: ---- Download helper ----
:dl
set "NAME=%~1"
set "URL=%~2"
echo   %NAME%
curl -sfL "%URL%" -o "%IMGUI_DIR%\%NAME%"
if errorlevel 1 (
    echo     FAILED: %NAME%
    set FAIL=1
)
exit /b

:error
pause
exit /b 1
