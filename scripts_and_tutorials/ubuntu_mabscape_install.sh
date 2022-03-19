#!/bin/bash

echo "vagabond is needed for  this programm to work. Do you want to install vagabond? [yes/NO]"
read vagabond
vagabond="$(tr [:upper:] [:lower:] <<<"$vagabond")"

# Saving the old working directory in order to may install vagabond.
old_pwd=$PWD

# Change into the user specified folder.
if [ -n "$1" ]
then
	if [ -d $1 ]
	then
		cd $1
	fi
fi
# Check if a vagabond should be installed.
if [ -n "$vagabond" ]
then
	if [[ $vagabond = "yes" || $vagabond = "ye" || $vagabond = "y" ]]
	then
		# Install vagabond into the same folder as mabscape
		bash $old_pwd/ubuntu_vagabond_install.sh $PWD
	fi
fi

# Check if there is an mabscape installation present in the folder.
if [ -d mabscape ]
then
	# If there is an installation, update the codebase.
	cd mabscape
	git checkout master
	git pull
else
	# If there is no installation present, get the codebase and update it.
	git clone https://github.com/helenginn/mabscape.git
	cd mabscape
	git checkout master
	git pull
fi

# Check if the installation should be rebuild completely.
if [ -n "$2" ] && [ -n "$1" ]
then
	if [ "$2" = "rebuild" ]
	then
		# If a rebuild is wanted, remove the build folder.
		rm -r build
	fi
elif [ -n "$1" ]
then
	if [ "$1" = "rebuild" ]
	then
		rm -r build
	fi
fi

# If there is a current build, recompile neccessary files.
if [ -d build ]
then
	ninja -C build/current
else
	# If there is no build, create it and compile all the files.
	PKG_CONFIG_PATH=~/install/lib/x86_64-linux-gnu/pkgconfig/ meson --buildtype=release build/current
	ninja -C build/current
	# Add mabscape link to the .bashrc
	echo "export "'PATH'"=\"`pwd`/build/current\"":'$PATH' >> ~/.bashrc
fi
