#!/bin/bash

staf_ip="10.38.36.178"


execution_time=443200
time=0
while [ $time -le $execution_time ] ;do
	unset board_free_list
	unset board_pending_list
	start_time=`date +%s`
	board_list=`staf $staf_ip ltktest list | grep "(EEPROM)board_id" | awk '{print $3}'`
	echo $board_list
	pending_board_num=0
	free_board_num=0
	for  board_id in $board_list 
	do
		#echo "board id:$board_id"
		job_status=`staf $staf_ip ltktest list | awk "/$board_id/,/job_running/"  | grep "(AUTO_DETECT)job_running" | awk '{print $3}'`
		board_online=`staf $staf_ip ltktest list | awk "/$board_id/,/job_running/"  | grep "(AUTO_DETECT)BoardConnection" | awk '{print $3}'`
		echo "$board_id:$job_status:$board_online"
		if [ $job_status -eq 0 -a $board_online == "Online" ];then
			board_free_list="$board_free_list $board_id"
		else
			board_pending_list="$board_pending_list $board_id"
			let pending_board_num=$pending_board_num+1
		fi
	done 

	echo "Free Board list : $board_free_list"
	echo "Pending Board list : $board_pending_list"
	#board_free_list=`echo $board_free_list | sed s/"^;"/""/`
	#board_pending_list=`echo $board_pending_list | sed s/"^;"/""/`

	for free_board in $board_free_list;do
		task_name=FAKE_TEST_`date '+%m-%d-%H-%M-%S'`
		staf 10.38.36.178 LTKTEST REQUEST TASKNAME $task_name ENTRY "board_id=$free_board" casename TC_LCD_01 LTKConfig version=build_log_2014-09-14 LTKConfig branch=pxa1928dkb_64bit-kk4.4 General auto_emmd=yes  General logcat_cap=yes  General console_cap=yes  General log_autoupload=yes 
		sleep 1
	done
	let random_sleep=$RANDOM/1000+1
	echo "sleep $random_sleep to list again"
	sleep $random_sleep

	task_name=FAKE_TEST_`date '+%m-%d-%H-%M-%S'`
	staf 10.38.36.178 LTKTEST REQUEST TASKNAME $task_name ENTRY "chip_name=PXA1U88" casename TC_LCD_01 LTKConfig version=build_log_2014-09-14 LTKConfig branch=pxa1928dkb_64bit-kk4.4 General auto_emmd=yes  General logcat_cap=yes  General console_cap=yes  General log_autoupload=yes 
	end_time=`date +%s`
	let time=$time+$end_time-$start_time
done
