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

case_id_str="casename TC_LCD_03 casename TC_INTEGRATION_01  "
#case_id_str="casename TC_CACHE_04"
#ltk_version="build_log_2014-07-22"
#ltk_branch="pxa1088dkb-kk4.4"
if [ $DEFCONFIG_3_10 = "pxa1U88_defconfig" ];then
	platform="pxa1U88"
	build_branch="pxa1U88"
	board_id="PXA1U88_DKB_V10_033"
fi
if [ $UCONFIG = "pxa1928_concord" ];then
	export DEFCONFIG_ARM64=defconfig
	platform="pxa1928"
	build_branch="pxa1928"
	board_id="PXA1928_EMCP_V11_057"
	if [ $BUILD_BRANCH = "beta2" ];then
		board_id="PXA1928_EMCP_V40_033"
	else
		board_id="PXA1928_EMCP_V11_057"
	fi
fi
#burning_image_version="2014-07-22_pxa988-kk4.4"
#burning_image_type="pxa1U88dkb_def"
#burning_image_blf="HLN2_Nontrusted_LPDDR3_2G_Hynix.blf"
image_address_ip="10.38.164.23"
image_address_path="/Daily_Build/Blf_Update/jk_image/$build_branch/"
image="boot.img"

echo "############ get test config  ###############"
wget -T 20 ftp://10.38.164.23/Daily_Build/daily_branch_map.txt -O ./daily_branch_map.txt
wget ftp://10.38.164.23/Daily_Build/daily_image_map.txt -O ./daily_image_map.txt
if [ $platform = "pxa1U88" ];then
	burning_image_blf=`cat daily_image_map.txt | grep -v pxa1908 | grep pxa1U88 | tail -1 | awk -F ":" '{print $NF}' | awk -F ";" '{print $1}' | sed s/"+"/";"/g`
	burning_image_version=`cat daily_image_map.txt | grep -v pxa1908 | grep pxa1U88 | awk -F "=" '{print $1}' | grep 'kk4.4$' | tail -1`
	ltk_version=`cat daily_branch_map.txt | grep "pxa1U88dkb-kk4.4;" | tail -1 | awk -F ":" '{print $1}'`
	burning_image_type="pxa1U88dkb_def"
	ltk_branch="pxa1U88dkb-kk4.4"
fi
if [ $platform = "pxa1928" ];then
	#burning_image_blf=`cat daily_image_map.txt | grep "pxa1928-kk4.4=" | tail -1 | awk -F ":" '{print $NF}' | sed s/"+"/";"/g`
	burning_image_blf=`cat daily_image_map.txt | grep "pxa1928-kk4.4=" | tail -1 | sed s/".*pxa1928dkb_tz:"/""/ | sed s/";.*"/""/ | sed s/"+"/";"/g`
	burning_image_version=`cat daily_image_map.txt | grep "pxa1928-kk4.4=" | awk -F "=" '{print $1}' | grep 'kk4.4$' | tail -1`
	ltk_version=`cat daily_branch_map.txt | grep "pxa1928dkb" | tail -1 | awk -F ":" '{print $1}'`
	burning_image_type="pxa1928dkb_tz/flash"
	ltk_branch="pxa1928dkb-kk4.4"
fi
echo "############ get uImage , ramdisk and dtb  ###############"

CURRENT_DATE="`date '+%Y-%m-%d-%H-%M'`"
if [ $platform = "pxa1U88" ];then
	basic_link="http://10.38.116.40/autobuild/android/pxa1U88/"
	def_link=$burning_image_version"/pxa1U88dkb_def/"
	ramdisk_link="ramdisk.img"
	dtb_link="dtb/pxa1U88-dkb.dtb"
	wget --http-user=wsun --http-passwd=marvell789 $basic_link/$def_link/$ramdisk_link -O $ramdisk_link
	wget --http-user=wsun --http-passwd=marvell789 $basic_link/$def_link/$dtb_link -O pxa1U88-dkb.dtb
fi

if [ $platform = "pxa1928" ];then
	basic_link="http://10.38.116.40/autobuild/android/pxa1928/"
	def_link=$burning_image_version"/pxa1928dkb_tz/debug/"
	ramdisk_link="ramdisk.img"
	dtb_link1="pxa1928concord.dtb"
	dtb_link2="pxa1928concord-discrete.dtb"
	wget --http-user=wsun --http-passwd=marvell789 $basic_link/$def_link/$ramdisk_link -O $ramdisk_link
	wget --http-user=wsun --http-passwd=marvell789 $basic_link/$def_link/$dtb_link1 -O pxa1928-concord1.dtb
	wget --http-user=wsun --http-passwd=marvell789 $basic_link/$def_link/$dtb_link2 -O pxa1928-concord2.dtb
fi



echo "############ make boot.img  ###############"

cd $git_folder
build_image_result=0

if [ $platform = "pxa1U88" ];then
	export PATH=$PATH:/home/jenkins/Integration/:/home/jenkins/toolchain/arm-eabi-4.7/bin
	#export DEFCONFIG_3_10=pxa1U88_defconfig
	export INTEGRATION_HOME=/home/jenkins/Integration
	kIntegration_cloud.sh 
	if [ $? -ne $build_image_result ];then
		build_image_result=1
	fi
	cd $work_space
	cp $git_folder/arch/arm/boot/uImage ./


	#These lines pad the uImage to 2K align
	len=`ls -l uImage | awk -F' ' '{print $5}'`                                                                                 
	mod=$(($len%2048))
	mod=$((2048-$mod+$len))
	cat uImage /dev/zero | head -c $mod > uImage_pad

	# these patches pad the DTB files to 128K size and combine with uImage
	cat pxa1U88-dkb.dtb /dev/zero | head -c 131072 > 10.dtb.pad
	cat  uImage_pad 10.dtb.pad >  uImage_new
	mv uImage_new uImage

	#create new boot.img with uImage and ramdisk.img
	mkbootimg --kernel ./uImage --ramdisk ./ramdisk.img -o boot.img


	#mkbootimg --kernel $git_folder/arch/arm/boot/uImage --ramdisk ./ramdisk.img -o boot.img
	#cat boot.img /dev/zero | head -c 15728640 > boot.img.pad                                                                     
	#cat pxa1U88-dkb.dtb /dev/zero | head -c  262144 > 10.dtb.pad
	#cat boot.img.pad 10.dtb.pad> boot.img.new
	#cp boot.img.new boot.img
fi

if [ $platform = "pxa1928" ];then
	#export PATH=$PATH:/home/jenkins/Integration/:/home/jenkins/toolchain/aarch64-linux-android-4.8/bin
	#export INTEGRATION_HOME=/home/jenkins/Integration
	#kIntegration_cloud.sh 

	export ARCH=arm64                                                                                                                          
	export CROSS_COMPILE=aarch64-linux-android-
	export PATH=$PATH:/toolchain/aarch64-linux-android-4.8/bin/
	#make -j16

	if [ $? -ne $build_image_result ];then
		build_image_result=1
	fi

	#/home/jenkins/Integration/mkimage -A arm64 -O linux -C gzip -a 0x01280000 -e 0x01280000 -n "edenA0 fpga linux" -d arch/arm64/boot/Image.gz uImage
	/home/jenkins/Integration/mkimage -A arm64 -O linux -C gzip -a 0x01080000 -e 0x01080000 -n "edenA0 fpga linux" -d arch/arm64/boot/Image.gz uImage

	cd $work_space
	cp $git_folder/uImage ./


	len=`ls -l uImage | awk -F' ' '{print $5}'`                                                                                 
	mod=$(($len%2048))
	mod=$((2048-$mod+$len))
	cat uImage /dev/zero | head -c $mod > uImage_pad

	cat pxa1928-concord1.dtb /dev/zero | head -c 131072 > 10.dtb.pad
	cat pxa1928-concord2.dtb /dev/zero | head -c 131072 > 20.dtb.pad
	cat  uImage_pad 10.dtb.pad 20.dtb.pad >  uImage_new
	mv uImage_new uImage

	mkbootimg --kernel ./uImage --ramdisk ./ramdisk.img -o boot.img
fi



curl --max-time 300 --retry 3 --retry-delay 5 --ftp-create-dirs -T ./boot.img ftp://10.38.164.23/Daily_Build/Blf_Update/jk_image/$build_branch/
echo "############ get jenkins info  ###############"

cd $work_space
git log > git_log.txt
commit=`cat git_log.txt | sed -n 1p | awk '{print $2}'`
task_name="$build_branch""_""$CURRENT_DATE""_""$board_id"_"$commit"



if [ $build_image_result -eq 0 ];then

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


