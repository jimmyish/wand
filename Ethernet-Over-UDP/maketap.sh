#!/usr/bin/env bash

for i in 0 1 2 3 4 5; do
	if [ ! -c /dev/tap${i} ]; then
		mknod /dev/tap${i} c 36 $[ 16 + $i ]
	fi
	chown root.root /dev/tap${i}
	chmod 700 /dev/tap${i}
done