#!/bin/sh

########################################################################
#
# This script sets up the wand network in a niave way.
#
#

if [ -f wand.$(hostname -s).conf ]; then
	. wand.$(hostname -s).conf
elif [ -f wand.conf ]; then
	. wand.conf
else
	echo Could not find configuration file
	exit
fi

if [ "x$INTERFACE" = "x" ]; then
	INTERFACE=tap0
fi

if [ "x$ETHERNET" = "x" ]; then
	ETHERNET=$(/sbin/ip link show eth0 | grep "link/eth" | cut -b 16-33)
fi

if [ "x$ETHERNET" = "x" ]; then
	echo Could not determine a valid MAC address.
	exit;
fi

if [ "x$BROADCAST" = "x" ]; then
	BROADCAST="192.168.255.255"
fi

if [ "x$BITLENGTH" = "x" ]; then
	BITLENGTH="16"
fi

if [ "x$SERVERS" = "x" ]; then
	echo No listed wand servers.
	exit;
fi

######
#
# Everything below here shouldn't need changing
#

function find_program() {
	local PROG
	PROG=$1
	export PROG
	PROG=$(which $PROG)
	echo $PROG
}

# Find the IP program
IP=$( find_program "ip" )

if [ ! -x $IP ]; then
	echo ip: not found
	exit;
fi

# Start up the tunnel.  
echo " * Starting Ethernet over IP driver"
Ethernet-Over-UDP/Etud

echo " * Configuring link layer."
$IP link set $INTERFACE \
	arp on \
	multicast off \
	address $ETHERNET \
	mtu 576

echo " * Configuring ipv4 layer."
echo "  * Removing stale entries"
$IP addr flush dev $INTERFACE
echo "  * Adding $IFADDR"
$IP addr add $IFADDR/$BITLENGTH broadcast $BROADCAST dev $INTERFACE

echo -n " * Starting wand"
for i in $SERVERS; do
	echo " ${i}"
	./wand/wand $i $ETHERNET
done
echo

echo " * Bringing the networking interface up."
$IP link set tap0 up

echo " * Done."

