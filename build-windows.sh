#!/bin/bash

rm -rf winpack
make
if [ $? -ne 0 ]; then exit $? ; fi
mkdir winpack
cp $(ldd ezlive | grep -v '/c/Windows' | awk '{ print $3 }') winpack/
cp ezlive winpack/
cp config.example.txt winpack/