@echo off
setlocal

echo ========================================================
echo   Launching VesperOS in QEMU (x86_64 UEFI)
echo ========================================================

:: Check if QEMU is installed
where qemu-system-x86_64 >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] QEMU was not found on your system PATH!
    echo.
    echo To install QEMU on Windows, run this in PowerShell or Command Prompt:
    echo     winget install SoftwareFreedomConservancy.QEMU
    echo.
    pause
    exit /b 1
)

:: Check if disk image exists
if not exist "bin\vesper-os.img" (
    if exist "vesper-os.img" (
        set "IMG=vesper-os.img"
    ) else (
        echo [ERROR] Could not find vesper-os.img!
        echo Please download vesper-os.img from your GitHub Releases into this folder.
        pause
        exit /b 1
    )
) else (
    set "IMG=bin\vesper-os.img"
)

:: Locate OVMF (UEFI firmware) from QEMU directory
set "OVMF="
for /f "delims=" %%I in ('where qemu-system-x86_64') do (
    set "QEMU_DIR=%%~dpI"
)

if exist "%QEMU_DIR%share\edk2-x86_64-code.fd" (
    set "OVMF=%QEMU_DIR%share\edk2-x86_64-code.fd"
) else if exist "%QEMU_DIR%edk2-x86_64-code.fd" (
    set "OVMF=%QEMU_DIR%edk2-x86_64-code.fd"
) else if exist "OVMF.fd" (
    set "OVMF=OVMF.fd"
)

if "%OVMF%"=="" (
    echo [INFO] Running QEMU with default bios...
    qemu-system-x86_64 -m 512M -drive file=%IMG%,format=raw
) else (
    echo [INFO] Booting with UEFI firmware: %OVMF%
    qemu-system-x86_64 -bios "%OVMF%" -m 512M -drive file=%IMG%,format=raw
)

endlocal
