sudo rm -rf rootfs
mkdir rootfs
mkdir rootfs/etc
mkdir rootfs/etc/ezlive/
sh ./scripts/create_rootfs.sh ezlive rootfs/
cp ezlive rootfs/
cp -L -r /etc/ssl rootfs/etc/
sudo docker build -t localhost/ezlive .
sudo docker save localhost/ezlive | gzip > ezlive-docker-image.tar.gz
sudo docker rmi localhost/ezlive
