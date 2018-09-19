#!/bin/bash

mkdir device/intel/sofia3g/flashloader
DIR=device/intel/sofia3g/flashloader
cp ../modem/umts_fw_dev/tools/build_system/tools/rvds/cust/psi_ram.fls $DIR/
cp android/out/target/product/sofia3g_xges2_0_ages2_svb/flashloader/ebl.fls $DIR/
