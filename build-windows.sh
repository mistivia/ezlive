#!/bin/bash

rm ezlive-windows-build.zip
rm -rf ezlive-windows-build
make clean
make
if [ $? -ne 0 ]; then exit $? ; fi
mkdir ezlive-windows-build
cp $(ldd ezlive | grep -v '/c/Windows' | awk '{ print $3 }') ezlive-windows-build/
cp ezlive ezlive-windows-build/
cp config.example.txt ezlive-windows-build/
zip ezlive-windows-build.zip $(find ezlive-windows-build -name '*')