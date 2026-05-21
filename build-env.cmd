@echo off
rem Configure the historical DOS/32A build tools for this checkout.

set "DOS32A=%~dp0"
if "%DOS32A:~-1%"=="\" set "DOS32A=%DOS32A:~0,-1%"

if not defined WATCOM set "WATCOM=C:\WATCOM"
if not defined DOSBOX_X set "DOSBOX_X=C:\DOSBox-X\dosbox-x.exe"
set "TASM32=%DOS32A%\tasm5\PATCHES\53_WIN\TASM32.EXE"

if not exist "%WATCOM%\BINNT\wcl.exe" (
    echo Open Watcom was not found at "%WATCOM%".
    echo Set WATCOM before calling this script, or install it at C:\WATCOM.
    exit /b 1
)

if not exist "%TASM32%" (
    echo TASM32 5.3 was not found at "%TASM32%".
    echo Place the TASM 5.3 setup/patch tree under "%DOS32A%\tasm5".
    exit /b 1
)

set "PATH=%DOS32A%\tasm5\PATCHES\53_WIN;%DOS32A%\binw;%WATCOM%\BINNT;%WATCOM%\BINW;%PATH%"
set "EDPATH=%WATCOM%\EDDAT"
set "INCLUDE=%WATCOM%\H;%WATCOM%\H\NT;.;%DOS32A%\dos32a_800\h32;%DOS32A%\dos32a_800\src\sutils\misc"
set "TASMFLAGS=-r -ml -m -q -zn -w2 -m5 -i%DOS32A%\dos32a_800\src\sutils\misc"
set "WCLFLAGS=-oneatx -ohirbk -ei -zp16 -6 -fp6 -fpi87 -bt=dos"

exit /b 0
