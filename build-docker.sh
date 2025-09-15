sudo rm -rf rootfs
mkdir rootfs
mkdir rootfs/etc
mkdir rootfs/etc/ezlive/
create_rootfs.sh ezlive rootfs/
cp ezlive rootfs/
cp -L -r /etc/ssl rootfs/etc/
sudo podman build -t ezlive .
sudo podman save ezlive | gzip > ezlive-docker-image.tar.gz
