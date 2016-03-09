#!/bin/sh

echo "Redémarrage programmé dans une minutes."
/bin/su -l -c "/sbin/shutdown -r +1"
