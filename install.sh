#!/bin/bash

# Copy files to the needed locations and make needed directories
echo "Building project..."
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

echo "Do you want to disable the \"application not responding\" dialog that appears on GNOME sometimes with this app? [y/n]"
read yesno

if [ $yesno == "y" ]; then
    gsettings set org.gnome.desktop.wm.preferences auto-raise-delay 0

fi