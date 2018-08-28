#! /bin/sh
# This script can clean up leftover cable locks for a XILINX dongle
#
# It typically needs to be run as root in /tmp to work right

impact -batch <<EOF
setmode -bs
cleancablelock
setcable -p auto
exit
EOF
