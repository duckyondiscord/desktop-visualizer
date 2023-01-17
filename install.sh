#!/bin/bash

# Copy files to the needed locations and make needed directories
cmake .
make -j$(nproc --all)
mkdir -p ~/.local/bin/
mkdir -p ~/.local/share/applications/
cp install/deskvis.ini ~/.config/deskvis.ini
cp -f ./tblvis ~/.local/bin
sed "s,~,${HOME}," ./install/desktop-visualizer.desktop > $HOME/.local/share/applications/desktop-visualizer.desktop
