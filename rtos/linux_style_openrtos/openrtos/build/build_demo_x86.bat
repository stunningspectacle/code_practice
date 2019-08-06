@echo off



set GCC_DIR=%cd%\..\..\..\BuildTools\ISH_Toolchain_04_2019
set OUTPUT_FOLDER_MINIMA=..\bin\x86
set OPENRTOS_IMAGE_NAME=ish_minima_FPGA
set path= %path%;%GCC_DIR%\bin
set CUR_BSP=glv
set BASE_DIR=%cd%\..\..\..\

if not "%1" == "" set CUR_BSP=%1
if not "%2" == "" set OUTPUT_FOLDER_MINIMA=%2

set MINIMA_LOAD_TYPE=patch
set	LOCAL_CFLAGS=-I %GCC_DIR%\lib -DDEMO -DBSP_%CUR_BSP%
set	LOCAL_CSRC=
set	LOCAL_ASRC=
set	PROJ_MK=%cd%\..\openRTOS_kernel\Demo\x86\sources.mk
set PYTHON_DIR=%cd%\..\..\..\BuildTools\Python.2.7

rem =================================
rem running free-RTOS make: (all flags are optional)
rem make all 
rem        PROJECT_CSRC="<list of extra c files to compile, defaults to none]>" 
rem        PROJECT_ASRC="<list of extra assembly files to compile, defaults to none>">
rem        PROJECT_CFLAGS="<list of extra flags, including toolchain location, defaults to none>"
rem        TARGET_NAME=<name of target, defaults to out> 
rem        OUT_FOLDER=<output folder, defaults to bin>
rem        TARGET_PLATFORM=<IA32 or Galileo>
rem =================================
make all TARGET_NAME=%OPENRTOS_IMAGE_NAME% OUT_FOLDER=%OUTPUT_FOLDER_MINIMA% PROJECT_CFLAGS="%LOCAL_CFLAGS%" PROJECT_CSRC="%LOCAL_CSRC%" PROJECT_ASRC="%LOCAL_ASRC%" PROJ_MK="%PROJ_MK%" TARGET_PLATFORM=%CUR_BSP% 
if %errorlevel% neq 0 goto error
call %GCC_DIR%/bin/i486-elf-size.exe %OUTPUT_FOLDER_MINIMA%\%OPENRTOS_IMAGE_NAME%.elf
%PYTHON_DIR%\python generate_metrics.py %OUTPUT_FOLDER_MINIMA% %OPENRTOS_IMAGE_NAME%

rem =================================
rem This is ISH specific logic. TODO: Should be removed from here.
rem =================================
if /I "%MINIMA_LOAD_TYPE%" == "patch" (
if not exist "%OUTPUT_FOLDER_MINIMA%" mkdir %OUTPUT_FOLDER_MINIMA%
call %cd%\..\..\bringup\bin2patch.exe -t %CUR_BSP% -i %OUTPUT_FOLDER_MINIMA%\%OPENRTOS_IMAGE_NAME%.bin -o %OUTPUT_FOLDER_MINIMA%\ish_bup.bin -a 0xff000000 -e 0xff000000 -m sram -b 0xf -h %BASE_DIR%\Src\include\build.h > nul
)


goto :eof

:error
echo error building minima
pause
