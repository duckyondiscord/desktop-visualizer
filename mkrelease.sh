#!/bin/bash

# Script used for making releases

# Make project
echo "Building project..."
cmake . > /dev/null
make -j$(nproc --all) > /dev/null

# Create release tarball
echo "Making release tarball..."
mkdir -p desktop-visualizer/install
cp tblvis desktop-visualizer/
cp -r install/* desktop-visualizer/install
cp install.sh desktop-visualizer/
tar -cvaf $(uname)-$(uname -m).tar.gz desktop-visualizer/*
rm -rf desktop-visualizer/
