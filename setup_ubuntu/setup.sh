#!/bin/bash

sudo yes | apt-get update
sudo yes | apt-get upgrade

SETUP_ROOT=$(pwd)
cd ~/

rm -rf Desktop Documents Downloads Pictures Public Videos Templates \
	Music examples.desktop

sudo yes | apt-get install --no-install-recommends tmux vim cscope ctags git \
  ccache doxygen dfu-util device-tree-compiler \
  python3-ply python3-pip python3-setuptools python3-wheel xz-utils file \
  make gcc-multilib autoconf automake libtool libczmq-dev libczmq-dev:i386

sudo yes | apt-get install manpages manpages-de manpages-de-dev manpages-dev \
	manpages-posix manpages-posix-dev

cd $SETUP_ROOT
cp .vimrc ~/
cp -r .vim ~/
cp .tmux.conf ~/
cp .gitconf* ~/

if [ "$(grep bashrc_ext ~/.bashrc)" != "" ]; then
	echo "bashrc extension already exists"
else
	echo "bashrc extension doesn't exist, adding..."
	cat bashrc_ext >> ~/.bashrc
fi

source ~/.bashrc
