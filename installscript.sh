#!/bin/bash
if (( $EUID > 0 )); then
	echo "Please run \"make install\" as root.";
	exit 1;
fi
if [ ! -f $1 ]; then
	echo "You must compile wadcli first before you can install it.";
	exit 1;
elif [ -d $2 ]; then
	if [ -f $2$1 ]; then
		rm $2$1;
		echo "Removed old wadcli.";
	fi
	if mv $1 $2; then
		echo "Succesfully installed wadcli into your /usr/local/bin/ folder.";
		exit 0;
	fi
fi