@echo off


set GCC_DIR=%cd%\..\..\..\BuildTools\ISH_Toolchain_04_2019
set OPENRTOS_IMAGE_NAME=ish_minima_FPGA
set path= %path%;%GCC_DIR%\bin
set BASE_DIR=%cd%\..\..\..\

set CUR_BSP=clanton
set MINIMA_LOAD_TYPE=patch
set	LOCAL_CFLAGS=-I %GCC_DIR%\lib -DDEMO -DBSP_%CUR_BSP%
set	LOCAL_CSRC=
set	LOCAL_ASRC=
set PYTHON_DIR=%cd%\..\..\..\BuildTools\Python.2.7



set OUTPUT_FOLDER_MINIMA=..\bin\IA32_flat_GCC_Galileo_Gen_2
set PROJ_MK=%cd%\..\openRTOS_kernel\Demo\IA32_flat_GCC_Galileo_Gen_2\sources.mk
make all TARGET_NAME=%OPENRTOS_IMAGE_NAME% OUT_FOLDER=%OUTPUT_FOLDER_MINIMA% PROJECT_CFLAGS="%LOCAL_CFLAGS%" PROJECT_CSRC="%LOCAL_CSRC%" PROJECT_ASRC="%LOCAL_ASRC%" PROJ_MK="%PROJ_MK%" TARGET_PLATFORM=%CUR_BSP%
if %errorlevel% neq 0 goto error
call %GCC_DIR%/bin/i486-elf-size.exe %OUTPUT_FOLDER_MINIMA%\%OPENRTOS_IMAGE_NAME%.elf
%PYTHON_DIR%\python generate_metrics.py %OUTPUT_FOLDER_MINIMA% %OPENRTOS_IMAGE_NAME%


goto :eof

:error
echo error building minima
