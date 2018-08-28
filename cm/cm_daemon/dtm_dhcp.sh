#!/bin/sh
#
ifconfig eth0 up

if [ `/usr/bin/bsi_dump dhcp` == "1" ]
then
  subnet_repl="s/_SUBNET_/`/usr/bin/bsi_dump subnet`/g"
  netma_repl="s/_NETMASK_/`/usr/bin/bsi_dump netmask`/g"
  ipbeg_repl="s/_IP_BEGIN_/`/usr/bin/bsi_dump ipbegin`/g"
  ipend_repl="s/_IP_END_/`/usr/bin/bsi_dump ipend`/g"
  gateway_repl="s/_GATEWAY_/`/usr/bin/bsi_dump gateway`/g"
  sed -e $subnet_repl -e $netma_repl -e $ipbeg_repl -e $ipend_repl -e $gateway_repl /etc/dhcpd.dtm.conf >& /tmp/dhcpd.conf
  echo >> /etc/leases.txt
  echo "dtm_dhcp starting server..."
  dhcpd -cf /tmp/dhcpd.conf -lf /etc/leases.txt -d -f eth0
else
  echo "dtm_dhcp starting client..."
  systemctl start dhcpcd
fi
