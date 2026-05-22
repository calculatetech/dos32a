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

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0smoke-dosbox.ps1"
exit /b %ERRORLEVEL%
