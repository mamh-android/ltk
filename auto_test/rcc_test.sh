#!/bin/bash

staf_ip="10.38.34.91"


platform=$1
time_out=43200
free_board_found=0
time_taken=0
case_id_str="casename TC_EXTRATEST_01  "
while [ $time_taken -le $time_out -a $free_board_found -eq 0 ];do

	unset board_free_list
	unset board_pending_list
	board_list=`staf $staf_ip ltktest list | grep "(EEPROM)board_id" | awk '{print $3}'`
	echo $board_list
	for  board_id in $board_list 
	do
		job_status=`staf $staf_ip ltktest list | awk "/$board_id/,/job_running/"  | grep "(AUTO_DETECT)job_running" | awk '{print $3}'`
		board_online=`staf $staf_ip ltktest list | awk "/$board_id/,/job_running/"  | grep "(AUTO_DETECT)BoardConnection" | awk '{print $3}'`
		board_lock=`staf $staf_ip ltktest list | awk "/$board_id/,/job_running/"  | grep "(AUTO_DETECT)task_lock" | awk '{print $3}'`
		board_platform=`staf $staf_ip ltktest list | awk "/$board_id/,/job_running/"  | grep "(EEPROM)chip_name" | awk '{print $3}' | tr '[A-Z]''[a-z]'`

		echo "$board_id:$job_status:$board_lock:$board_platform:$board_online"
		if [ $job_status -eq 0 -a $board_online == "Online" -a $board_lock -eq 0 -a $board_platform == $platform ];then
			test_board_id=$board_id
			free_board_found=1
		fi
	done 
done

wget -T 20 ftp://10.38.164.23/Daily_Build/daily_branch_map.txt -O ./daily_branch_map.txt
wget ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt
if [ $platform = "pxa1928" ];then
burning_image_blf=`cat daily_image_map.txt | grep "pxa1928-lp5.0=" | tail -1 | sed s/".*pxa1928dkb_tz:"/""/ | sed s/";.*"/""/ | sed s/"+"/";"/g`
burning_image_version=`cat daily_image_map.txt | grep "pxa1928-lp5.0=" | awk -F "=" '{print $1}' | grep 'lp5.0$' | tail -1`
	ltk_version=`cat daily_branch_map.txt | grep "pxa1928dkb-lp5.0;" | tail -1 | awk -F ":" '{print $1}'`
	burning_image_type="pxa1928dkb_def"
	ltk_branch="pxa1928dkb-lp5.0"
fi

staf $server_ip LTKTEST REQUEST TASKNAME $task_name ENTRY "chip_name=$platform" $case_id_str LTKConfig version=$ltk_version LTKConfig branch=$ltk_branch General auto_reboot=no General runcase_mode=serialonly General logcat_cap=yes  General console_cap=yes General log_autoupload=yes General update_image=yes ImageConfig platform=$platform ImageConfig version=$burning_image_version ImageConfig image="$burning_image_type" ImageConfig blf="$burning_image_blf" ImageConfig eraseall=no 

