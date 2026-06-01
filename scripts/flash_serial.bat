@echo off
setlocal enabledelayedexpansion

:: Check if we are running in the correct directory (scripts/ folder or parent)
set "FW_DIR=%~dp0..\firmware"
if not exist "%FW_DIR%\platformio.ini" (
    if exist "%~dp0platformio.ini" (
        set "FW_DIR=%~dp0"
    ) else if exist "%~dp0..\platformio.ini" (
        set "FW_DIR=%~dp0.."
    ) else (
        echo [ERROR] platformio.ini nicht gefunden! Bitte das Skript aus dem GrowTower-Verzeichnis ausfuehren.
        pause
        exit /b 1
    )
)

:: Set standard default to Auto-Detect
set "DEFAULT_PORT=Auto-Detect"

:: Parse platformio.ini for upload_port in [env:main]
set "in_main=0"
for /f "usebackq tokens=*" %%i in ("%FW_DIR%\platformio.ini") do (
    set "line=%%i"
    set "clean_line=!line: =!"
    if "!clean_line!"=="[env:main]" (
        set "in_main=1"
    ) else if "!clean_line:~0,5!"=="[env:" (
        set "in_main=0"
    )
    
    if !in_main! equ 1 (
        for /f "tokens=1,2 delims==" %%a in ("%%i") do (
            set "key=%%a"
            set "val=%%b"
            set "key_clean=!key: =!"
            if "!key_clean!"=="upload_port" (
                :: Clean leading/trailing spaces from value
                set "val_clean=!val!"
                for /f "tokens=*" %%g in ("!val_clean!") do set "val_clean=%%g"
                set "DEFAULT_PORT=!val_clean!"
            )
        )
    )
)

echo ==================================================
echo  GrowTower Serial Flash Tool
echo ==================================================
echo.
echo Standard-Port aus platformio.ini: %DEFAULT_PORT%
echo.

set /p "UPLOAD_PORT=Bitte COM-Port eingeben (Enter fuer %DEFAULT_PORT%): "

if "%UPLOAD_PORT%"=="" (
    set "UPLOAD_PORT=%DEFAULT_PORT%"
)

echo.
if "%UPLOAD_PORT%"=="Auto-Detect" (
    echo Starte seriellen Upload mit automatischer Port-Erkennung...
    echo.
    pushd "%FW_DIR%"
    call pio run -t upload -e main
    set "EXIT_CODE=%ERRORLEVEL%"
    popd
) else (
    echo Starte seriellen Upload zu: %UPLOAD_PORT%...
    echo.
    pushd "%FW_DIR%"
    call pio run -t upload -e main --upload-port %UPLOAD_PORT%
    set "EXIT_CODE=%ERRORLEVEL%"
    popd
)

echo.
if %EXIT_CODE% equ 0 (
    echo ==================================================
    echo [SUCCESS] Firmware erfolgreich via Serial geflasht!
    echo ==================================================
) else (
    echo ==================================================
    echo [ERROR] Fehler beim seriellen Flash (Code: %EXIT_CODE%).
    echo ==================================================
)

echo.
pause
