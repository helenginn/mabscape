#!/bin/bash

echo "Do you want to install all necessary dependencies? [yes/NO]"
# Request user input if installation of dependencies is wanted
read dependencies
# Change all characters to lowercase
dependencies="$(tr [:upper:] [:lower:] <<< "$dependencies")"
# Check if the input is not empty
if [ -n "$dependencies" ]
then
	# Check if input is some variation of yes.
	if [[ $dependencies = "yes" || $dependencies = "ye" || $dependencies = "y" ]]
	then
		# Install dependencies
		apt install meson qt3d5-dev libfftw3-dev libpng-dev libboost-all-dev build-essential cmake pkg-config
	fi
fi
# Check if an installation folder was specified 
if [ -n "$1" ]
then
	if [ -d $1 ]
	then
		cd $1
	fi
fi

echo "installing to"
pwd
# Check if there is an existing installation in the specified folder
if [ -d vagabond ]
then
	cd vagabond
else
	git clone https://github.com/helenginn/vagabond.git
	cd vagabond
fi
# Save target folder in a variable for later convenience
target_folder=$PWD
# Checkout the branch with the latest version and update all submodules
echo `pwd`
git checkout edge
git pull
git submodule update --init --recursive
cd subprojects/helen3d
echo `pwd`
git checkout master
git pull
cd h3dsrc/shaders/
echo `pwd`
git checkout master
git pull
if [ -d $target_folder/subprojects/helencore ]
then
	cd $target_folder/subprojects/helencore
	echo `pwd`
	git checkout master
	git pull
	cd $target_folder
	echo `pwd`
else
	cd $target_folder
fi
# If a complete rebuild is wanted, the current build folder is deleted
if [ -n "$2" ] && [ -n "$1" ]
then
	if [ "$2" = "rebuild" ]
	then
		rm -r build
	fi
elif [ -n "$1" ]
then
	if [ "$1" = "rebuild" ]
	then
		rm -r build
	fi
fi
# If a build folder is present, the build will be compiled.
if [ -d build ]
then
	ninja -C build/current install
# Otherwise a build is generated and then compiled.
else
	PKG_CONFIG_PATH=~/install/lib/x86_64-linux-gnu/pkgconfig meson --buildtype=release build/current
	meson configure -Dprefix=~/install build/current
	ninja -C build/current install
	# Link the path to vagabond into .bashrc
	echo "export "'PATH'"=\"`pwd`/build/current\"":'$PATH' >> ~/.bashrc
fi
