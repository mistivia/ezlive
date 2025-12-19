#!/bin/bash

rm ezlive-docker-image.tar.gz
rm -rf rootfs
mkdir rootfs
bash ./scripts/create_rootfs.sh ./ezlive rootfs
cp ./ezlive ./rootfs/
mkdir rootfs/tmp
mkdir rootfs/etc
cp -rL /etc/ssl rootfs/etc/
sudo podman build -t ezlive .
sudo podman save ezlive | gzip > ezlive-docker-image.tar.gz
