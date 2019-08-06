@echo off

set MINIMA_LOAD_USE_ATT=false
if /I "%CUR_BSP%" == "glv" set MINIMA_LOAD_USE_ATT=true
set	LOCAL_CFLAGS=-I%GCC_DIR%\lib -Wformat -Wformat-security -DBSP_$(CUR_BSP) -D%PLATFORM%
if /I "%TEST_BUILD%" == "true" set LOCAL_CFLAGS=-I%GCC_DIR%\lib  -DBSP_$(CUR_BSP) -DKERNEL_TEST -DENABLE_ULT -D%PLATFORM%
if /I "%ISH_USER_APP%" == "kernel_test_app" set LOCAL_CFLAGS=%LOCAL_CFLAGS% -DKERNEL_TEST
if /I "%TEST_BUILD%" == "validation" set LOCAL_CFLAGS=-I%GCC_DIR%\lib  -DBSP_$(CUR_BSP) -DKERNEL_TEST -DVALIDATION=1 -D%PLATFORM%
if /I "%TEST_BUILD%" == "demo" set LOCAL_CFLAGS=-I%GCC_DIR%\lib -DDEMO -DBSP_%CUR_BSP% -D%PLATFORM%
if "%DEBUG_MODE%"=="1" set LOCAL_CFLAGS=%LOCAL_CFLAGS% -DDEBUG
if /I "%CUR_BSP%" == "chv" set LOCAL_CFLAGS=%LOCAL_CFLAGS% -DFIXED_MATH
set	LOCAL_CSRC=
set	LOCAL_ASRC=
set	PROJ_MK=%1

rem =================================
rem running free-RTOS make: (all flags are optional)
rem make all 
rem        PROJECT_CSRC="<list of extra c files to compile, defaults to none]>" 
rem        PROJECT_ASRC="<list of extra assembly files to compile, defaults to none>">
rem        PROJECT_CFLAGS="<list of extra flags, including toolchain location, defaults to none>"
rem        TARGET_NAME=<name of target, defaults to out> 
rem        OUT_FOLDER=<output folder, defaults to bin>
rem =================================
make all TEST=%TEST_BUILD% TARGET_NAME=%OPENRTOS_IMAGE_NAME% OUT_FOLDER=%OUTPUT_FOLDER_MINIMA% LOCAL_CFLAGS="%LOCAL_CFLAGS%" LOCAL_CSRC="%LOCAL_CSRC%" LOCAL_ASRC="%LOCAL_ASRC%" PROJ_MK="%PROJ_MK%" TARGET_PLATFORM=%CUR_BSP%
if %errorlevel% neq 0 goto error
call %GCC_DIR%/bin/i486-elf-size.exe %OUTPUT_FOLDER_MINIMA%\%OPENRTOS_IMAGE_NAME%.elf
python generate_metrics.py %OUTPUT_FOLDER_MINIMA% %OPENRTOS_IMAGE_NAME%

rem =================================
rem This is ISH specific logic. TODO: Should be removed from here.
rem =================================
Setlocal EnableDelayedExpansion

if not exist "%OUTPUT_FOLDER_MINIMA%" mkdir %OUTPUT_FOLDER_MINIMA%
if not exist "%OUTPUT_FOLDER_BUP%" mkdir %OUTPUT_FOLDER_BUP%


goto :eof

:error
echo error building minima
exit /b 1
