#!/bin/bash

sudo apt-get install --no-install-recommends git cmake ninja-build gperf \
                     ccache doxygen dfu-util device-tree-compiler \
                     python3-ply python3-pip python3-setuptools python3-wheel xz-utils file \
                     make gcc-multilib g++-multilib autoconf automake libtool librsvg2-bin \
		     texlive-latex-base texlive-latex-extra latexmk texlive-fonts-recommended

sudo apt-get install --no-install-recommends git cmake ninja-build gperf \

sudo apt-get install libczmq-dev libczmq-dev:i386

wget https://download.qt.io/archive/qt/5.10/5.10.1/qt-opensource-linux-x64-5.10.1.run
./qt-opensource-linux-x64-5.10.1.run

sudo apt-get install mesa-common-dev libglu1-mesa-dev freeglut3-dev
# edit ~/.bashrc, add below item
export PATH=~/Qt/Qt5.10.1/5.10.1/gcc_64/bin:$PATH

sudo apt-get install libopencv-dev

git clone git://android.intel.com/jellyfish_simulation

cd jellyfish_simulation/modules/rtos/zephyr 
pip3 install --user -r scripts/requirements.txt

cd jellyfish_simulation
source env.sh

qmake --version

cd jellyfish_simulation && ./build_sdv.sh config
cd jellyfish_simulation && ./build_sdv.sh all
#cd jellyfish_simulation && ./build_sdv.sh clean
cd jellyfish_simulation && ./build_sdv.sh run

