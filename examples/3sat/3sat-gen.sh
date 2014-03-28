#!/bin/sh

if [ $# -ne 2 ]; then
	echo $0 nr-of-terms nr-of-vars
	exit 1
fi

TERMNR=$1
VARNR=$2

X=1
while [ $X -lt $TERMNR ]; do
	echo AND
	X=`expr $X + 1`
done

X=1
while [ $X -le $TERMNR ]; do
	echo OR
	Y=1
	while [ $Y -le 3 ]; do
		RAND=`openssl rand 4 | od -D -A n`
		N=`expr $RAND % $VARNR + 1`
		RAND=`openssl rand 1 | od -D -A n`
		NEG=`expr $RAND % 2`
		if [ $Y -eq 2 ]; then
			echo OR
		fi
		if [ $NEG -eq 0 ]; then
			echo NOT
		fi
		echo -n x
		echo $N
		Y=`expr $Y + 1`
	done
	X=`expr $X + 1`
done

exit 0
