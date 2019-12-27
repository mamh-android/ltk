#!/bin/bash

staf_ip="10.38.36.178"

board_loop=0
while [ $board_loop -le 100 ];do
board_id="FAKE_Board_"`date +%s`
./rcc_utilc faketest $board_id add_devices
let board_loop=$board_loop+1
sleep 1
done


execution_time=143200
time=0
loop=0
while [ $time -le $execution_time ];do
	start_time=`date +%s`
board_number=`wc -l board_list.txt | awk '{print $1}'`
let random_board_number=$RANDOM%$board_number+1
random_board=`cat board_list.txt | awk -F ":" '{print $1}' | sed -n "$random_board_number"p`

let random_option=$RANDOM%100
let random_bool_option=$RANDOM%2

echo "LOOP: $loop $random_board"

if [ $random_option -le 50 ];then
echo "#### RANDOM for usb plug in/out ####"
if [ $random_bool_option -eq 1 ];then
./rcc_utilc faketest $random_board usb_online yes
else
./rcc_utilc faketest $random_board usb_online no
fi
fi

if [ $random_option -ge 50 -a $random_option -le 80 ];then
echo "#### RANDOM for serial plug in/out ####"
if [ $random_bool_option -eq 1 ];then
./rcc_utilc faketest $random_board serial_online yes
else
./rcc_utilc faketest $random_board serial_online no
fi
fi

if [ $random_option -ge 80 -a $random_option -le 90 ];then
echo "#### RANDOM for rcc plug in/out ####"
if [ $random_bool_option -eq 1 ];then
./rcc_utilc faketest $random_board rcc_online yes
else
./rcc_utilc faketest $random_board rcc_online no
fi

if [ $random_option -ge 90 -a $random_option -le 100 ];then
echo "#### RANDOM for new board register ####"
board_id="FAKE_Board_"`date +%s`
./rcc_utilc faketest $board_id add_devices
fi
fi

sleep $(($RANDOM%5))
	end_time=`date +%s`
let time=$time+$end_time-$start_time
let loop=$loop+1
done
