#!/bin/bash


#build_branch=$1
work_space=`pwd`"/jk_test"
rm -r $work_space
mkdir -p $work_space
cd $work_space

server_ip="10.38.34.91"

#case_id_str="casename TC_LCD_03 casename TC_INTEGRATION_01  "
case_id_str="casename TC_LCD_03 "
UCONFIG=$1
MAIL_FROM="wsun@marvell.com"
#MAIL_LIST="wsun@marvell.com,fswu@marvell.com,wuqm@marvell.com,lllu@marvell.com,wwang27@marvell.com,xjian@marvell.com,zhangwn@marvell.com,lianglx@marvell.com,wanghc@marvell.com,leeying@marvell.com,zhsx@marvell.com"
MAIL_LIST="wsun@marvell.com"

if [ $UCONFIG = "pxa1908" ];then
	platform="pxa1908"
	build_branch="pxa1908"
	board_id="PXA1908_DKB_V10_219"
#board_id=PXA1908_DKB_V10_417
fi
image_address_ip="10.38.164.23"
image_address_path="/Daily_Build/Blf_Update/debug_image/$build_branch/"
image="boot.img"

echo "############ get test config  ###############"
wget -T 20 ftp://10.38.164.23/Daily_Build/daily_branch_map.txt -O ./daily_branch_map.txt
wget ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt

if [ $platform = "pxa1908" ];then
	burning_image_blf=`cat daily_image_map.txt | grep "pxa1908-kk4.4=" | tail -1 | sed s/".*pxa1908dkb_tz:"/""/ | sed s/";.*"/""/ | sed s/"+"/";"/g`
	burning_image_version=`cat daily_image_map.txt | grep "pxa1908-kk4.4=" | awk -F "=" '{print $1}' | grep 'kk4.4$' | tail -1`
	ltk_version=`cat daily_branch_map.txt | grep "pxa1908dkb" | tail -1 | awk -F ":" '{print $1}'`
	burning_image_type="pxa1908dkb_tz/flash"
	ltk_branch="pxa1908dkb-kk4.4"
fi

echo "############ get boot.img ###############"

CURRENT_DATE="`date '+%Y-%m-%d-%H-%M'`"

if [ $platform = "pxa1908" ];then
	basic_link="http://10.38.116.40/autobuild/android/pxa1908/"
	def_link=$burning_image_version"/pxa1908dkb_tz/debug/debug_kernel_img/"
	debug_image="boot-debug.img"
	wget --http-user=wsun --http-passwd=marvell789 $basic_link/$def_link/$debug_image -O boot.img
	image_exist=$?
fi

curl --max-time 300 --retry 3 --retry-delay 5 --ftp-create-dirs -T ./boot.img ftp://10.38.164.23/Daily_Build/Blf_Update/debug_image/$build_branch/$burning_image_version/
image_address_path="$image_address_path$burning_image_version/"
echo "############ get jenkins info  ###############"

task_name="kernel_debug_test_""$burning_image_version""_""$CURRENT_DATE""_""$board_id"



if [ $image_exist -eq 0 ];then

	echo "############ Push task to STAF  ###############"

	export PATH=~/ltk_cloud_server/STAF/bin:$PATH
	export LD_LIBRARY_PATH=~/ltk_cloud_server/STAF/lib:$LD_LIBRARY_PATH
	export CLASSPATH=~/ltk_cloud_server/STAF/JSTAF.jar:~/ltk_cloud_server/STAF/samples/demo/STAFDemo.jar:$CLASSPATH
	export STAFCONVDIR=~/ltk_cloud_server/STAF/codepage

	if [ `ps -e | grep -q STAFProc; echo $?` -ne 0 ];then
		STAFProc &
		sleep 5
	fi

	staf $server_ip ltktest unlock board_id $board_id

	echo "staf $server_ip LTKTEST REQUEST TASKNAME $task_name ENTRY board_id=$board_id&&ignore_offline=1&&lock_task=1 $case_id_str LTKConfig version=$ltk_version LTKConfig branch=$ltk_branch General auto_reboot=no General runcase_mode=serialonly General logcat_cap=yes General auto_emmd=yes General console_cap=yes General log_autoupload=yes General update_image=yes ImageConfig platform=$platform ImageConfig version=$burning_image_version ImageConfig image="$burning_image_type" ImageConfig blf="$burning_image_blf" ImageConfig eraseall=no ImageConfig modified=yes ImageConfig location2=$image_address_ip ImageConfig path2=$image_address_path ImageConfig image2=$image ExtraAct preset=ftp://10.38.164.23//Daily_Build/Blf_Update/debug_image/debug_preset.sh"

	sleep 3
	staf $server_ip LTKTEST REQUEST TASKNAME $task_name ENTRY "board_id=$board_id&&ignore_offline=1&&lock_task=1" $case_id_str LTKConfig version=$ltk_version LTKConfig branch=$ltk_branch General auto_reboot=no General runcase_mode=serialonly General logcat_cap=yes General auto_emmd=yes General console_cap=yes General log_autoupload=yes General update_image=yes ImageConfig platform=$platform ImageConfig version=$burning_image_version ImageConfig image="$burning_image_type" ImageConfig blf="$burning_image_blf" ImageConfig eraseall=no ImageConfig modified=yes ImageConfig location2=$image_address_ip ImageConfig path2=$image_address_path ImageConfig image2=$image ExtraAct preset="ftp://10.38.164.23//Daily_Build/Blf_Update/debug_image/debug_preset.sh"
	request_result=$?
	sleep 1
	if [ $request_result -eq 0 ];then
		staf $server_ip ltktest list taskname $task_name > staf_task_status.log
		time_out=3600
		time_count=0

		while [ `grep -q "========Job Report========" staf_task_status.log ; echo $? ` -ne 0 -a $time_count -le $time_out ];do
			let check_time=$time_count%60
			if [ $check_time -eq 0 ];then
				staf 10.38.34.91 ltktest list taskname $task_name | tee staf_task_status.log
			else
				staf 10.38.34.91 ltktest list taskname $task_name > staf_task_status.log
			fi
			#echo "============sleep 10 to check task $task_name=============="
			sleep 10
			let time_count=$time_count+10
		done

		staf_test_result=`grep "Result:" staf_task_status.log | grep -q "Success" ; echo $? `
		build_result_str=`grep "Result:" staf_task_status.log`
		if [ `grep -q "Result:" staf_task_status.log;echo $?` -ne 0 ];then
			build_result_str="Regression_Time_out"
		fi
	else
		build_result_str="task request fail or pending"
		MAIL_LIST="wsun@marvell.com"
		staf_test_result=1
	fi
	staf $server_ip ltktest lock board_id $board_id
else
	build_result_str="image not find in $basic_link/$def_link/$debug_image"
	staf_test_result=1
fi


#echo "test result:"$build_result_str | tee result.html

special_image_link=$image_address_path
test_log_link=$task_name
#"\\\\\\\\10.38.164.23\\\\Daily_Build2\\\\Blf_Update\debug_image\pxa1908"

cp ../ltk_cloud_test_report_ori.html ltk_cloud_test_report.html
sed -i s/Task_Name_NA/"$task_name"/ ./ltk_cloud_test_report.html
sed -i 's|Test_Image_NA|'$burning_image_version-$burning_image_type'|' ./ltk_cloud_test_report.html
sed -i s/Special_Image_NA/"$image"/ ./ltk_cloud_test_report.html
sed -i s/Special_Image_Link_NA/"\\\\\\\\10.38.164.23\\\\Daily_Build\\\\Blf_Update\\\\debug_image\\\\pxa1908\\\\$burning_image_version"/ ./ltk_cloud_test_report.html
sed -i s/Test_Case_Version_NA/""$ltk_version"-"$ltk_branch""/ ./ltk_cloud_test_report.html
sed -i s/Test_Case_NA/"$case_id_str"/ ./ltk_cloud_test_report.html
sed -i s/Test_Result_NA/"$build_result_str"/ ./ltk_cloud_test_report.html
if [ $staf_test_result -ne 0 ];then
	sed -i s/"color:green"/"color:red"/ ./ltk_cloud_test_report.html
fi
sed -i s/Test_Log_Link_NA/"\\\\\\\\10.38.164.23\\\\Daily_Build\\\\Cloud_Test\\\\$test_log_link"/ ./ltk_cloud_test_report.html

cat ltk_cloud_test_report.html | formail -I "From: $MAIL_FROM" -I "To: $MAIL_LIST" -I "MIME-Version:1.0" -I "Content-type:text/html;charset=gb2312" -I "Subject:Debug Kernel Test Result" | /usr/sbin/sendmail -t



exit $staf_test_result


