#!/bin/bash


#build_branch=$1
git_folder=`pwd`
work_space=`pwd`"/jk_test"
rm -r $work_space
mkdir -p $work_space
cd $work_space
#export DEFCONFIG_3_10=pxa1U88_defconfig

server_ip="10.38.34.91"
#task_name=

case_id_str="casename TC_EXTRATEST_GENERAL_01"
#case_id_str="casename TC_CACHE_04"
#ltk_version="build_log_2014-07-22"
#ltk_branch="pxa1088dkb-kk4.4"

if [ $UCONFIG = "pxa1928_concord" ];then
	platform="pxa1928"
	build_branch="pxa1928"
	board_id="PXA1928_EMCP_V11_057"
fi
#burning_image_version="2014-07-22_pxa988-kk4.4"
#burning_image_type="pxa1U88dkb_def"
#burning_image_blf="HLN2_Nontrusted_LPDDR3_2G_Hynix.blf"
image_address_ip="10.38.164.23"
image="u-boot.bin"

echo "############ get test config  ###############"
wget -T 20 ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt
wget -T 20 ftp://10.38.164.23/Daily_Build/daily_branch_map.txt -O ./daily_branch_map.txt
if [ $platform = "pxa1928" ];then
	burning_image_blf=`cat daily_image_map.txt | grep "pxa1928-lp5.1=" | tail -1 | sed s/".*pxa1928dkb_tz:"/""/ | sed s/";.*"/""/ | sed s/"+"/";"/g`
	burning_image_version=`cat daily_image_map.txt | grep "pxa1928-lp5.1=" | awk -F "=" '{print $1}' | grep 'lp5.1$' | tail -1`
	ltk_version=`cat daily_branch_map.txt | grep "pxa1928dkb" | tail -1 | awk -F ":" '{print $1}'`
	burning_image_type="pxa1928dkb_tz/flash"
	ltk_branch="pxa1928dkb-lp5.1"
fi


CURRENT_DATE="`date '+%Y-%m-%d-%H-%M'`"
cd $git_folder
git log > git_log.txt
commit=`cat git_log.txt | sed -n 1p | awk '{print $2}'`

echo "############ upload u-boot.img  ###############"


curl --max-time 300 --retry 3 --retry-delay 5 --ftp-create-dirs -T ./u-boot.bin ftp://10.38.164.23/Daily_Build/Blf_Update/jk_image/$build_branch/u-boot_$commit/
update_uboot_success=$?
image_address_path="/Daily_Build/Blf_Update/jk_image/$build_branch/u-boot_$commit/"
echo "############ get jenkins info  ###############"

cd $work_space

task_name="$build_branch""_""$CURRENT_DATE""_""$board_id"_"$commit"



if [ $update_uboot_success -eq 0 ];then

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


	staf $server_ip ltktest list jobs | grep "$board_id"_"" -A 3 | grep `date '+%Y%m%d'`
	already_burn_basic_image=$?
	if [ $already_burn_basic_image -eq 0 ];then
		integration_str="General update_ltk=no ImageConfig skip_image=\"system.img;cache.img;userdata_8g.img\" "
	echo "###`date '+%Y%m%d'` image has been burning ####"
	fi

	#update_ltk>yes

	echo "staf $server_ip LTKTEST REQUEST TASKNAME $task_name ENTRY board_id=$board_id&&ignore_offline=1&&lock_task=1 $case_id_str LTKConfig version=$ltk_version LTKConfig branch=$ltk_branch General auto_reboot=no General runcase_mode=serialonly General logcat_cap=yes  General console_cap=yes General log_autoupload=yes General update_image=yes ImageConfig platform=$platform ImageConfig version=$burning_image_version ImageConfig image=\"$burning_image_type\" ImageConfig blf=$burning_image_blf ImageConfig eraseall=no ImageConfig modified=yes ImageConfig location2=$image_address_ip ImageConfig path2=$image_address_path ImageConfig image2=$image $integration_str"


	sleep 3
	staf $server_ip LTKTEST REQUEST TASKNAME $task_name ENTRY "board_id=$board_id&&ignore_offline=1&&lock_task=1" $case_id_str LTKConfig version=$ltk_version LTKConfig branch=$ltk_branch General auto_reboot=no General runcase_mode=serialonly General logcat_cap=yes  General console_cap=yes General log_autoupload=yes General update_image=yes ImageConfig platform=$platform ImageConfig version=$burning_image_version ImageConfig image="$burning_image_type" ImageConfig blf="$burning_image_blf" ImageConfig eraseall=no ImageConfig modified=yes ImageConfig location2=$image_address_ip ImageConfig path2=$image_address_path ImageConfig image2=$image $integration_str
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
		staf_test_result=1
	fi
	staf $server_ip ltktest lock board_id $board_id
else
	build_result_str="Imgae_build_fail"
	staf_test_result=1
fi

echo "give commits for  $commit $build_result_str"
ssh -p 29418 shgit.marvell.com gerrit review -m \'$build_result_str\'  $commit
exit $staf_test_result


