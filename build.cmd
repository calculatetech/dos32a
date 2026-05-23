@echo off
setlocal

call "%~dp0build-env.cmd" || exit /b 1
set "OUT=%DOS32A%\binw"

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
wcl %WCLFLAGS% -lr -fm=dos32a -fe=dos32a -"disable 2083" -"option packcode=1,packdata=1" dos32a.obj kernel.obj || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
copy /y dos32a.exe "%OUT%\dos32a.exe" >nul || goto fail_popd
call :clean_current
popd
exit /b 0

:stub32a
echo.
echo Creating STUB/32A and STUB/32C
pushd "%DOS32A%\src\stub32a" || exit /b 1
tasm32 -dEXEC_TYPE=0 %TASMFLAGS% stub32a.asm || goto fail_popd
tasm32 -dEXEC_TYPE=0 %TASMFLAGS% stub32c.asm || goto fail_popd
wcl %WCLFLAGS% -lr -fe=stub32a stub32a.obj || goto fail_popd
wcl %WCLFLAGS% -lr -fe=stub32c stub32c.obj || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
copy /y stub32a.exe "%OUT%\stub32a.exe" >nul || goto fail_popd
copy /y stub32c.exe "%OUT%\stub32c.exe" >nul || goto fail_popd
call :clean_current
popd
exit /b 0

:sb
echo.
echo Creating SUNSYS Bind Utility
pushd "%DOS32A%\src\sb" || exit /b 1
tasm32 %TASMFLAGS% sbind.asm || goto fail_popd
wcl386 %WCLFLAGS% -l=dos32a -fe=sb -k65536 sbind.obj main.c || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
copy /y sb.exe "%OUT%\sb.exe" >nul || goto fail_popd
call :clean_current
popd
exit /b 0

:sc
echo.
echo Creating SUNSYS Compress Utility
pushd "%DOS32A%\src\sc" || exit /b 1
tasm32 %TASMFLAGS% scomp.asm || goto fail_popd
tasm32 %TASMFLAGS% sload.asm || goto fail_popd
wcl386 %WCLFLAGS% -l=dos32a -fe=sc -k65536 scomp.obj sload.obj encode.c main.c || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
copy /y sc.exe "%OUT%\sc.exe" >nul || goto fail_popd
call :clean_current
popd
exit /b 0

:ss
echo.
echo Creating SUNSYS Setup Utility
pushd "%DOS32A%\src\ss" || exit /b 1
tasm32 %TASMFLAGS% setup.asm || goto fail_popd
wcl386 %WCLFLAGS% -l=dos32a -fe=ss -k65536 -"option nocaseexact" setup.obj main.c || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
copy /y ss.exe "%OUT%\ss.exe" >nul || goto fail_popd
call :clean_current
popd
exit /b 0

:sver
echo.
echo Creating DOS/32 SVER
pushd "%DOS32A%\src\sver" || exit /b 1
wcl %WCLFLAGS% -lr -fe=sver main.c || goto fail_popd
if not exist "%OUT%" mkdir "%OUT%" || goto fail_popd
copy /y sver.exe "%OUT%\sver.exe" >nul || goto fail_popd
call :clean_current
popd
exit /b 0

:clean
if exist "%OUT%" (
    del /q "%OUT%\dos32a.exe" "%OUT%\stub32a.exe" "%OUT%\stub32c.exe" "%OUT%\sb.exe" "%OUT%\sc.exe" "%OUT%\ss.exe" "%OUT%\sver.exe" >nul 2>nul
)
for %%D in (dos32a stub32a sb sc ss sver) do (
    if exist "%DOS32A%\src\%%D" (
        pushd "%DOS32A%\src\%%D" >nul
        call :clean_current
        popd >nul
    )
)
echo Clean complete.
exit /b 0

:clean_current
del /q *.obj *.o *.lst *.err *.crf *.xrf *.sym *.map *.exe >nul 2>nul
exit /b 0

:fail_popd
set "BUILD_RC=%ERRORLEVEL%"
popd
exit /b %BUILD_RC%
