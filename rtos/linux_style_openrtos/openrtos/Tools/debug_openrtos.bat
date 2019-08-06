@echo on

rem usage: run_gdb <s=[1|0]> <i=IP> <p=PORT>
rem	 	s: start debug service (when running on FPGA with utag) default is do not start
rem 	i: set gdb remote IP to FPGA IP (e.g. 11.0.0.2) default is 127.0.0.1
rem	 	p: set gdb port. (i.e -p 9998 when using debug agent) default is 12345
rem

set GDB_IP=127.0.0.1
set GDB_PORT=12345
set USE_DBG_SERVICE=
set TARGET=simics

:scan

if "%1" EQU "" goto scan_end

set argName=%1
set argValue=%2

if /i "%argName%"=="s" (
	if "%argValue%"=="1" (
		set USE_DBG_SERVICE=%argValue%
	)
    goto scan_next
)

if /i "%argName%"=="i" (
	set GDB_IP=%argValue%
    goto scan_next
)

if /i "%argName%"=="p" (
	set GDB_PORT=%argValue%
    goto scan_next
)

if /i "%argName%"=="t" (
	set TARGET=%argValue%
    goto scan_next
)

rem None of the above... must be a typo!
echo usage: run_gdb ^<s=[1^|0]^> ^<i=IP^> ^<p=PORT^>
echo 	s: start debug service (when running on FPGA with utag) default is do not start
echo 	i: set gdb remote IP to FPGA IP (e.g. 11.0.0.2) default is 127.0.0.1
echo 	p: set gdb port. (e.g. 9998 when using debug agent) default is 12345
echo 	t: set target platform. (simics\galileo) default is simics

exit /b 1

:scan_next
shift
shift
goto scan

:scan_end

set GDB_REL_PATH=BuildTools\gdb
set BASE_DIR=%cd%\..\..\..
pushd %~dp0

if "%TARGET%" == "simics" goto debug_simics
if "%TARGET%" == "galileo" goto debug_galileo

:debug_simics
set OUTPUT_FOLDER=%BASE_DIR%\Src\openrtos\bin\x86
goto run_gdb

:debug_galileo
set OUTPUT_FOLDER=%BASE_DIR%\Src\openrtos\bin\IA32_flat_GCC_Galileo_Gen_2
set USE_DBG_SERVICE=1
rem TODO: Remove these calls once we have galileo_freertos bsp
pushd .
cd %BASE_DIR%
call setenv_CLANTON.bat 
call setenv.bat 
popd

goto run_gdb

:run_gdb
:: creating gdbinit
copy /y %BASE_DIR%\%GDB_REL_PATH%\template.gdbinit %BASE_DIR%\%GDB_REL_PATH%\.gdbinit 

>> %BASE_DIR%\%GDB_REL_PATH%\.gdbinit ( 


echo target extended-remote %GDB_IP%:%GDB_PORT%

echo.
echo python execfile^("python_scripts\scriptsManager.py"^)
echo.

echo symbol-file "%OUTPUT_FOLDER:\=/%/ish_minima_FPGA.elf"

)

::if running FPGA or SI, need to run debug service first
if not "%USE_DBG_SERVICE%"=="" (
	echo starting debug service on port %GDB_PORT%
	cd %BASE_DIR%\FPGA\UTAG_debug
	start  "debug service" start_debug.bat
	echo waiting 10 seconds
	ping -n 10 127.0.0.1 > nul
)

::running gdb
cd %BASE_DIR%\%GDB_REL_PATH%
echo starting GDB on %GDB_IP%:%GDB_PORT%
start "gdb-python" ..\ISH_Toolchain_04_2019\bin\i486-elf-gdb.exe

popd