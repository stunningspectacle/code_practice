#!/bin/bash
#fastboot -t 192.168.42.1 oem write_osip_header
#fastboot -t 192.168.42.1 flash boot boot.img
#fastboot -t 192.168.42.1 flash recovery recovery.img 
#fastboot -t 192.168.42.1 flash fastboot droidboot.img
#fastboot -t 192.168.42.1 oem start_partitioning
#fastboot -t 192.168.42.1 flash /tmp/partition.tbl partition.tbl 
#fastboot -t 192.168.42.1 oem partition /tmp/partition.tbl
#fastboot -t 192.168.42.1 erase factory
#fastboot -t 192.168.42.1 erase cache
#fastboot -t 192.168.42.1 erase system
#fastboot -t 192.168.42.1 erase config
#fastboot -t 192.168.42.1 erase logs
#fastboot -t 192.168.42.1 erase spare
#fastboot -t 192.168.42.1 erase data
#fastboot -t 192.168.42.1 oem stop_partitioning
#fastboot -t 192.168.42.1 flash system system.img
#fastboot -t 192.168.42.1 continue

fastboot oem start_partitioning
fastboot flash /tmp/partition.tbl partition.tbl 
fastboot oem partition /tmp/partition.tbl
fastboot erase factory
fastboot erase cache
fastboot erase system
fastboot erase config
fastboot erase logs
fastboot erase spare
fastboot erase data
fastboot oem stop_partitioning

fastboot oem write_osip_header
fastboot flash recovery recovery.img 
fastboot flash fastboot droidboot.img

fastboot flash boot boot.img
fastboot flash system system.img
#fastboot continue
