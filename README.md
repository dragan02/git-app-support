# git-app-support

These files represent additional support for various interfaces:
- chardev_app: folder containing program aimed to run under QEMU with GPIO character device approach
- sysfs_app: folder containing program aimed to run under QEMU with deprecated sysfs approach
- qt-app: Qt project containing neccessary GUI functionality (qt-app run file added from build folder)
- led-designer-plugin: LED plugin needed to instantiate LED widget in Qt Designer
- tools: folder with various scripts which were used for system design (only most neccessary are included)
- sd.tar.gz: compressed SD image

Program within SD image, i.e. within rootfs is located in /home directory (/home/sysfs_app, /home/chardev_app).
