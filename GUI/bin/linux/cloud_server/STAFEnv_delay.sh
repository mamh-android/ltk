#!/bin/bash

echo $*
delay_time=$1
shift 1
echo $*
sleep $delay_time && ./STAFEnv.sh $* &
