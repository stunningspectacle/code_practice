#!/bin/bash
WORKDIR=$(pwd)
KERNEL_DIR=/home/leo/gingerbread/hardware/intel/linux-2.6
SOURCE_DIR=${KERNEL_DIR}/$1
SOURCE_DIR=$(echo $SOURCE_DIR | sed -e 's/\/$//')

if [ $# -ne 1 ]; then
    echo $0 driver_dir output_dir
    exit
fi
if [ ! -d $SOURCE_DIR ]; then
    echo "$SOURCE_DIR not exist"
    exit
fi

adb pull /sys/kernel/debug/gcov${SOURCE_DIR} .
cp *.gcda ${SOURCE_DIR}
rm *.gcda
infoname=$(echo $1 | sed -e 's/\//_/g')
lcov -b ${KERNEL_DIR} --directory ${SOURCE_DIR} --capture -output-file ${infoname}.info
genhtml $infoname.info -o ${infoname}_html/
