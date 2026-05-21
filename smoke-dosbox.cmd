@echo off
setlocal

call "%~dp0build-env.cmd" || exit /b 1

if not exist "%DOSBOX_X%" (
    echo DOSBox-X was not found at "%DOSBOX_X%".
    echo Set DOSBOX_X before calling this script, or install DOSBox-X at C:\DOSBox-X.
    exit /b 1
)

if not exist "%DOS32A%\binw\sver.exe" call "%~dp0build.cmd" sver || exit /b 1
if not exist "%DOS32A%\binw\dos32a.exe" call "%~dp0build.cmd" dos32a || exit /b 1

set "SMOKE_OUT=%DOS32A%\sver.out"
set "SMOKE_LOG=%DOS32A%\dosbox-smoke.log"
del /q "%SMOKE_OUT%" "%SMOKE_LOG%" >nul 2>nul

"%DOSBOX_X%" -c "mount c %DOS32A%" -c "c:" -c "binw\sver.exe binw\dos32a.exe ^> sver.out" -c "exit" > "%SMOKE_LOG%" 2>&1

if exist "%SMOKE_OUT%" (
    type "%SMOKE_OUT%"
    del /q "%SMOKE_OUT%" "%SMOKE_LOG%" >nul 2>nul
    exit /b 0
)

findstr /c:"OS-Free build" "%SMOKE_LOG%" >nul 2>nul
if "%ERRORLEVEL%"=="0" (
    echo DOSBox-X started, but this installed binary reports an OS-Free build.
    echo That build cannot run mounted DOS programs without booting a separate DOS image.
    echo Install a DOSBox-X build with integrated DOS, or configure this script for a bootable DOS image.
    exit /b 1
)

echo DOSBox-X did not create "%SMOKE_OUT%".
echo See "%SMOKE_LOG%" for the emulator log.
exit /b 1
