@echo off
setlocal EnableDelayedExpansion

REM Build settings
set OUTPUT_DIR=bin
set GCC_FLAGS=-Wall -Wextra -O2
set MSVC_FLAGS=/W3 /O2

echo Starting build process...

REM Create output directory
if not exist %OUTPUT_DIR% (
    mkdir %OUTPUT_DIR%
    if !ERRORLEVEL! neq 0 (
        echo Failed to create %OUTPUT_DIR% directory
        exit /b !ERRORLEVEL!
    )
    echo Created %OUTPUT_DIR% directory
)

REM Detect compiler
set COMPILER=
where gcc >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set COMPILER=gcc
    set FLAGS=%GCC_FLAGS%
    set EXT=.exe
    echo Using MinGW GCC compiler
    goto :compile
)

where cl >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set COMPILER=cl
    set FLAGS=%MSVC_FLAGS%
    set EXT=.exe
    echo Using MSVC compiler
    goto :compile
)

echo Error: No suitable compiler found!
echo Please install either MinGW (gcc) or Microsoft Visual Studio (cl)
exit /b 1

:compile
REM Compile each .c and .cpp file
set BUILD_COUNT=0
set ERROR_COUNT=0

for %%f in (*.c *.cpp) do (
    set SRC_FILE=%%f
    set OUT_FILE=%OUTPUT_DIR%\%%~nf!EXT!
    
    echo Compiling !SRC_FILE!...
    if "!COMPILER!"=="gcc" (
        gcc !FLAGS! "!SRC_FILE!" -o "!OUT_FILE!"
    ) else (
        cl !FLAGS! "!SRC_FILE!" /Fe"!OUT_FILE!" /Fo"%OUTPUT_DIR%\%%~nf.obj"
    )
    
    if !ERRORLEVEL! equ 0 (
        echo Built: !OUT_FILE!
        set /a BUILD_COUNT+=1
    ) else (
        echo Failed to build !SRC_FILE!
        set /a ERROR_COUNT+=1
    )
)

REM Summary
echo.
echo Build Summary:
echo Successfully built: %BUILD_COUNT% files
if %ERROR_COUNT% gtr 0 (
    echo Failed: %ERROR_COUNT% files
    exit /b 1
) else (
    echo All builds successful
)

:end
echo Build process complete.
endlocal
