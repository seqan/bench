#!/bin/bash

WD=$(dirname $0)

# $Id: $
#rsync -aLv $WD/* $NETDIR

J=$(qsub -t 1-192  $WD/run.sh)
J=${J%%.*};echo $J
sleep 2
