#!/bin/sh

pwd
cd `dirname $0`
pwd
while [ 1 ] ; do
	./MyHomeEvents
	sleep 30
done
