#!/bin/bash
set -e

rm /mud/data/pid
cd /mud/bin
./aberd

/usr/sbin/apache2 -D FOREGROUND
