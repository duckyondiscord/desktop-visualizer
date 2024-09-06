#!/usr/bin/env bash

# Copy files to the needed locations and make needed directories
echo "Building project..."
echo "If this fails, install fftw version 3"
sleep 3
cmake . > /dev/null 2<&1
make -j$(nproc --all) > /dev/null 2<&1
mkdir -vp ~/.local/bin/
mkdir -vp ~/.local/share/applications/
if [ -a ${HOME}/.config/deskvis.ini ];
then
    echo "Config file exists, will not overwrite!"
else
    cp install/deskvis.ini ~/.config/deskvis.ini
fi
rm -rvf ~/.local/bin/tblvis
cp -vf ./tblvis ~/.local/bin
sed "s,~,${HOME}," ./install/desktop-visualizer.desktop > $HOME/.local/share/applications/desktop-visualizer.desktop

#echo "Do you want to disable the \"application not responding\" dialog that appears on GNOME sometimes with this app? [y/n]"
#read yesno

if [ $yesno == "y" ]; then
	gsettings set org.gnome.mutter check-alive-timeout 60000
	echo "No man is powerful enough to figure out how GNOME tells if an app is responding or not, so I've disabled the dialog for you lmao"
fi
