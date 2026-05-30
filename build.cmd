@echo off
setlocal

call "%~dp0build-env.cmd" || exit /b 1
set "OUT=%DOS32A%\BINW"

if /i "%~1"=="clean" goto clean
if /i "%~1"=="dos32a" goto one_dos32a
if /i "%~1"=="stub32a" goto one_stub32a
if /i "%~1"=="sb" goto one_sb
if /i "%~1"=="sc" goto one_sc
if /i "%~1"=="ss" goto one_ss
if /i "%~1"=="sver" goto one_sver
if not "%~1"=="" goto usage

if not exist "%OUT%" mkdir "%OUT%" || exit /b 1

call :dos32a || exit /b 1
call :stub32a || exit /b 1
call :sb || exit /b 1
call :sc || exit /b 1
call :ss || exit /b 1
call :sver || exit /b 1

echo.
echo Build complete. Outputs are in "%OUT%".
exit /b 0

:usage
echo Usage: build.cmd [clean^|dos32a^|stub32a^|sb^|sc^|ss^|sver]
exit /b 1

:one_dos32a
call :dos32a
exit /b %ERRORLEVEL%

:one_stub32a
call :stub32a
exit /b %ERRORLEVEL%

:one_sb
call :sb
exit /b %ERRORLEVEL%

:one_sc
call :sc
exit /b %ERRORLEVEL%

:one_ss
call :ss
exit /b %ERRORLEVEL%

:one_sver
call :sver
exit /b %ERRORLEVEL%

:dos32a
echo.
echo Creating DOS/32 Advanced DOS Extender
pushd "%DOS32A%\src\dos32a" || exit /b 1
tasm32 -dEXEC_TYPE=0 %TASMFLAGS% -c kernel.asm || goto fail_popd
tasm32 -dEXEC_TYPE=0 %TASMFLAGS% -c dos32a.asm || goto fail_popd
wcl %WCLFLAGS% -lr -fm=DOS32A -fe=DOS32A -"disable 2083" -"option packcode=1,packdata=1" dos32a.obj kernel.obj || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
del /q "%OUT%\DOS32A.EXE" >nul 2>nul
copy /y DOS32A.EXE "%OUT%\DOS32A.EXE" >nul || goto fail_popd
del /q *.OBJ *.O *.LST *.ERR *.CRF *.XRF *.SYM *.MAP *.EXE >nul 2>nul
popd
exit /b 0

:stub32a
echo.
echo Creating STUB/32A and STUB/32C
pushd "%DOS32A%\src\stub32a" || exit /b 1
tasm32 -dEXEC_TYPE=0 %TASMFLAGS% stub32a.asm || goto fail_popd
tasm32 -dEXEC_TYPE=0 %TASMFLAGS% stub32c.asm || goto fail_popd
wcl %WCLFLAGS% -lr -fe=STUB32A stub32a.obj || goto fail_popd
wcl %WCLFLAGS% -lr -fe=STUB32C stub32c.obj || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
del /q "%OUT%\STUB32A.EXE" "%OUT%\STUB32C.EXE" >nul 2>nul
copy /y STUB32A.EXE "%OUT%\STUB32A.EXE" >nul || goto fail_popd
copy /y STUB32C.EXE "%OUT%\STUB32C.EXE" >nul || goto fail_popd
del /q *.OBJ *.O *.LST *.ERR *.CRF *.XRF *.SYM *.MAP *.EXE >nul 2>nul
popd
exit /b 0

:sb
echo.
echo Creating SUNSYS Bind Utility
pushd "%DOS32A%\src\sb" || exit /b 1
tasm32 %TASMFLAGS% sbind.asm || goto fail_popd
wcl386 %WCLFLAGS% -l=dos32a -fe=SB -k65536 sbind.obj main.c || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
del /q "%OUT%\SB.EXE" >nul 2>nul
copy /y SB.EXE "%OUT%\SB.EXE" >nul || goto fail_popd
del /q *.OBJ *.O *.LST *.ERR *.CRF *.XRF *.SYM *.MAP *.EXE >nul 2>nul
popd
exit /b 0

:sc
echo.
echo Creating SUNSYS Compress Utility
pushd "%DOS32A%\src\sc" || exit /b 1
tasm32 %TASMFLAGS% scomp.asm || goto fail_popd
tasm32 %TASMFLAGS% sload.asm || goto fail_popd
wcl386 %WCLFLAGS% -l=dos32a -fe=SC -k65536 scomp.obj sload.obj encode.c main.c || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
del /q "%OUT%\SC.EXE" >nul 2>nul
copy /y SC.EXE "%OUT%\SC.EXE" >nul || goto fail_popd
del /q *.OBJ *.O *.LST *.ERR *.CRF *.XRF *.SYM *.MAP *.EXE >nul 2>nul
popd
exit /b 0

:ss
echo.
echo Creating SUNSYS Setup Utility
pushd "%DOS32A%\src\ss" || exit /b 1
tasm32 %TASMFLAGS% setup.asm || goto fail_popd
wcl386 %WCLFLAGS% -l=dos32a -fe=SS -k65536 -"option nocaseexact" setup.obj main.c || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
del /q "%OUT%\SS.EXE" >nul 2>nul
copy /y SS.EXE "%OUT%\SS.EXE" >nul || goto fail_popd
del /q *.OBJ *.O *.LST *.ERR *.CRF *.XRF *.SYM *.MAP *.EXE >nul 2>nul
popd
exit /b 0

:sver
echo.
echo Creating DOS/32 SVER
pushd "%DOS32A%\src\sver" || exit /b 1
wcl %WCLFLAGS% -lr -fe=SVER main.c || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
del /q "%OUT%\SVER.EXE" >nul 2>nul
copy /y SVER.EXE "%OUT%\SVER.EXE" >nul || goto fail_popd
del /q *.OBJ *.O *.LST *.ERR *.CRF *.XRF *.SYM *.MAP *.EXE >nul 2>nul
popd
exit /b 0

:clean
if exist "%OUT%" (
    del /q "%OUT%\DOS32A.EXE" "%OUT%\STUB32A.EXE" "%OUT%\STUB32C.EXE" "%OUT%\SB.EXE" "%OUT%\SC.EXE" "%OUT%\SS.EXE" "%OUT%\SVER.EXE" >nul 2>nul
)
for %%D in (dos32a stub32a sb sc ss sver) do (
    if exist "%DOS32A%\src\%%D" (
        pushd "%DOS32A%\src\%%D" >nul
        del /q *.OBJ *.O *.LST *.ERR *.CRF *.XRF *.SYM *.MAP *.EXE >nul 2>nul
        popd >nul
    )
)
echo Clean complete.
exit /b 0

:fail_popd
set "BUILD_RC=%ERRORLEVEL%"
popd
exit /b %BUILD_RC%
