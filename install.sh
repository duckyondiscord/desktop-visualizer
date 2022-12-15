#!/bin/bash

# Copy files to the needed locations and make needed directories

mkdir -p ~/.local/bin/tblvis/
mkdir -p ~/.local/share/applications/
cp ./install/run.sh ~/.local/bin/tblvis/
cp ./install/tblvis ~/.local/bin/tblvis/
cp ./install/desktop-visualizer.desktop ~/.local/share/applications/

