#!/bin/bash
  
# run in subshell
(

# error handling
set -e

err_report() {
    echo "---------------------------------------------------------------------"
    echo "                            FAILED!"
    echo "Script failed while executing"
    echo ""
    echo "line $1 : ${@:2}."
    echo ""
}

trap 'err_report $LINENO $BASH_COMMAND' ERR

# copy rootfs SD card
echo "-------------------------------------------------------------------------"
echo " Copying rootfs to SD card image ..."
echo "-------------------------------------------------------------------------"
cd $STAGING
sudo cp -ra * /mnt/rootfs
sudo sync
sudo chown root:root /mnt/rootfs
sudo chmod 755 /mnt/rootfs
echo "-------------------------------------------------------------------------"
echo "                       ... done!"
echo "-------------------------------------------------------------------------"

)
