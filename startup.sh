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
	echo iproute: not found
	exit;
fi

# Start up the tunnel.  The tunnel programs dumb at the moment, so let it
# do its thing in the background. 
Ethernet-Over-UDP/Etud &

# Because it's dumb, we're going to give it 5s to sort itself out.
sleep 5

echo Configuring link layer.
$IP link set $INTERFACE \
	arp on \
	multicast off \
	address $ETHERNET \
	mtu 576

echo Configuring ipv4 layer.
echo " Removing stale entries"
$IP addr flush dev $INTERFACE
echo " Adding $IFADDR"
$IP addr add $IFADDR/8 broadcast 10.255.255.255 dev $INTERFACE

echo Configuring peers.
while read interface ip comment; do
	echo peer $interface is on ip $ip
	clientsrc/client "add $interface $ip"
done < ./membership.txt

echo Bringing the networking interface up.
$IP link set tap0 up

echo Done.

