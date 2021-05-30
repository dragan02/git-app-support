#!/bin/bash

# run in subshell
(

BUSYBOX_VER=1.30.1

# error handling
set -e

err_report() {
    echo "---------------------------------------------------------------------"
    echo "                            FAILED!"
    echo "Script failed while executing"
    echo ""
    echo "line $1 : ${@:2}."
    echo ""
    echo "---------------------------------------------------------------------"
}

trap 'err_report $LINENO $BASH_COMMAND' ERR

source ~/aenv.sh

# prepare staging directory
echo "-------------------------------------------------------------------------"
echo " Preparing staging directory ..."
echo "-------------------------------------------------------------------------"
mkdir rootfs
cd rootfs
export STAGING=$PWD
mkdir bin dev etc home lib proc sbin sys tmp usr var root
mkdir etc/init.d
mkdir -p etc/network/if-pre-up.d
mkdir -p etc/network/if-up.d
mkdir usr/bin usr/lib usr/sbin
mkdir var/log var/run
cd ..
echo "-------------------------------------------------------------------------"
echo "                             ... done!"
echo "-------------------------------------------------------------------------"

# download busybox source and unpack
echo "-------------------------------------------------------------------------"
echo " Downloading Busybox source ..."
echo "-------------------------------------------------------------------------"
wget -c  http://busybox.net/downloads/busybox-${BUSYBOX_VER}.tar.bz2
tar xf busybox-${BUSYBOX_VER}.tar.bz2
echo "-------------------------------------------------------------------------"
echo "                            ... done!"
echo "-------------------------------------------------------------------------"

# compile
echo "-------------------------------------------------------------------------"
echo " Compiling Busybox ..."
echo "-------------------------------------------------------------------------"
pushd busybox-${BUSYBOX_VER}
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- defconfig
sed -i "s|^\(CONFIG_PREFIX=\"\).*\"|\1$STAGING\"|g" .config
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
echo "-------------------------------------------------------------------------"
echo "                   ... done!"
echo "-------------------------------------------------------------------------"


# debugging prerequisities
echo "-------------------------------------------------------------------------"
echo " Installing debugging prerequisities..."
echo "-------------------------------------------------------------------------"
sudo apt-get install libtinfo5
sudo apt-get install libncursesw5

echo "-------------------------------------------------------------------------"
echo " Populate rootfs ..."
echo "-------------------------------------------------------------------------"
# busybox
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- install
popd
# libs
export SYSROOT=$(arm-linux-gnueabihf-gcc -print-sysroot)
cp -a $SYSROOT/lib/ld-linux-armhf.so.3 $STAGING/lib
cp -a $SYSROOT/lib/ld-2.28.so $STAGING/lib
cp -a $SYSROOT/lib/libc.so.6 $STAGING/lib
cp -a $SYSROOT/lib/libc-2.28.so $STAGING/lib
cp -a $SYSROOT/lib/libm.so.6 $STAGING/lib
cp -a $SYSROOT/lib/libm-2.28.so $STAGING/lib
cp -a $SYSROOT/lib/libresolv* $STAGING/lib
cp -a $SYSROOT/lib/libnss* $STAGING/lib
# multi-tread
cp -a $SYSROOT/lib/libpthread.so.0 $STAGING/lib
cp -a $SYSROOT/lib/libpthread-2.28.so $STAGING/lib
# debugging
cp -a $SYSROOT/lib/libdl.so.2  $STAGING/lib
cp -a $SYSROOT/lib/libdl-2.28.so $STAGING/lib
cp -a $SYSROOT/usr/lib/libgcc_s.so* $STAGING/usr/lib
cp -a $SYSROOT/usr/lib/libstdc++.so* $STAGING/usr/lib
cp -a $SYSROOT/usr/bin/gdbserver $STAGING/usr/bin
# mknod
sudo mknod -m 666 $STAGING/dev/null c 1 3
sudo mknod -m 600 $STAGING/dev/console c 5 1
# inittab
tee $STAGING/etc/inittab << EOF
::sysinit:/etc/init.d/rcS
::respawn:/sbin/getty 115200 console
::respawn:/sbin/syslogd -n
EOF
# rcS
tee $STAGING/etc/init.d/rcS << EOF
#!/bin/sh
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev
EOF
chmod +x $STAGING/etc/init.d/rcS
# user
tee $STAGING/etc/passwd << EOF
root:x:0:0:root:/root:/bin/sh
daemon:x:1:1:daemon:/usr/sbin:/bin/false
EOF
# shadow
tee $STAGING/etc/shadow << EOF
root::10933:0:99999:7:::
daemon:*:10933:0:99999:7:::
EOF
chmod 0600 $STAGING/etc/shadow
# group
tee $STAGING/etc/group << EOF
root:x:0:
daemon:x:1:
EOF
#network
tee $STAGING/etc/network/interfaces << EOF
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet static
    address 192.168.10.101
    netmask 255.255.255.0
    network 192.168.10.0
    gateway 192.168.10.1
EOF
cp -a /etc/networks $STAGING/etc/
cp -a /etc/protocols $STAGING/etc/
cp -a /etc/services $STAGING/etc/
tee $STAGING/etc/nsswitch.conf << EOF
passwd:     files
group:      files
shadow:     files
hosts:      files dns
networks:   files
protocols:  files
services:   files
EOF
tee $STAGING/etc/hosts << EOF
127.0.0.1 localhost
EOF

echo "-------------------------------------------------------------------------"
echo "                 ... done!"
echo "-------------------------------------------------------------------------"

# libgpiod
echo "-------------------------------------------------------------------------"
echo " Downloading libgpiod source ..."
echo "-------------------------------------------------------------------------"
wget -c https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/snapshot/libgpiod-1.6.3.tar.gz
tar xf libgpiod-1.6.3.tar.gz
echo "-------------------------------------------------------------------------"
echo "                            ... done!"
echo "-------------------------------------------------------------------------"

# compile
echo "-------------------------------------------------------------------------"
echo " Compiling libgpiod ..."
echo "-------------------------------------------------------------------------"
pushd libgpiod-1.6.3
CC=arm-linux-gnueabihf-gcc ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes ./autogen.sh --enable-tools=yes --prefix=/usr --host=arm-linux-gnueabihf
make
make DESTDIR=$STAGING install
popd
echo "-------------------------------------------------------------------------"
echo "                            ... done!"
echo "-------------------------------------------------------------------------"
echo "-------------------------------------------------------------------------"
echo "                    Finished building busybox!"
echo "-------------------------------------------------------------------------"

)
