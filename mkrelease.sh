#!/bin/bash

# Script used for making releases

# Make project
cmake .
make -j$(nproc --all)

# Create release tarball
mkdir -p desktop-visualizer/install
cp tblvis desktop-visualizer/
cp -r install/* desktop-visualizer/install
cp install.sh desktop-visualizer/
tar -caf $(uname)-$(uname -m).tar.gz desktop-visualizer/*
rm -rf desktop-visualizer/
