#!/bin/bash

# Copy files to the needed locations and make needed directories
cmake .
make -j$(nproc --all)
mkdir -p ~/.local/bin/tblvis/
mkdir -p ~/.local/share/applications/
cp ./install/run.sh ~/.local/bin/tblvis/
cp ./tblvis ~/.local/bin/tblvis/
sed "s,~,${HOME}," ./install/desktop-visualizer.desktop > $HOME/.local/share/applications/desktop-visualizer.desktop
