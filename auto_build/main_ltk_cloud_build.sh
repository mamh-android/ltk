#!/bin/bash

CURRENT_DATE="`date '+%Y-%m-%d'`"
build_dir="/root/daily_build/ltk_cloud/ltk_cloud_build-$CURRENT_DATE"
build_dir_cloud=$build_dir"/ltk_cloud"
build_dir_rcc=$build_dir"/remote_control"
build_dir_script="/root/tc_daily_build/ltk_cloud_build"
PREVIOUS_DAY=`date '+%Y-%m-%d' -d "1 day ago"`
umount /root/daily_build/ltk_cloud/ltk_cloud_build_out
rm -r /root/daily_build/ltk_cloud/ltk_cloud-*
mkdir -p $build_dir
mkdir -p $build_dir/log
rm -r ltk_cloud
rm -r remote_control
./acquire_atd_git.sh "yq" "shgit.marvell.com/git/ose/linux" "ltk_cloud.git" "marvell88"
./acquire_atd_git.sh "yq" "shgit.marvell.com/git/ose/linux" "remote_control" "marvell88"
cp -r ltk_cloud.sh $build_dir/
cp -r remote_control $build_dir/
cp -r rcc_report.html $build_dir/
build_test_data=0



cd $build_dir_script/ltk_cloud
commit_cloud=`git show HEAD | sed -n 1p | awk '{print $2}'`
cd $build_dir_script/remote_control
commit_remote=`git show HEAD | sed -n 1p | awk '{print $2}'`

sed -i s/build_date_NA/$CURRENT_DATE/ $build_dir/rcc_report.html
sed -i s/"last_link_NA"/"\\\\\\\\10.38.164.22\\\\Daily_Build2\\\\Build_History_ltk_cloud"/ $build_dir/rcc_report.html
sed -i s/commit_id_cloud/$commit/ $build_dir/rcc_report.html
sed -i s/commit_id_remote/$commit/ $build_dir/rcc_report.html


echo "############ build cloud ############ "
#cd $build_dir_cloud/src/staf
#./make.sh 2>ltk_cloud.error 1>ltk_cloud.log
cd $build_dir_cloud
./make_cloud.sh
if [ $? -eq 0 ];then
	rm *.error.log
else
	sed -i s/color\:green\'\>cloud_PASS/color\:red\'\>FAIL/ $build_dir/rcc_report.html
fi
cp *.log $build_dir/log/
echo "############ build cloud ############ "

echo "############ build remote control ############ "
#cd $build_dir_cloud/src/staf
#./make.sh 2>ltk_cloud.error 1>ltk_cloud.log
cd $build_dir_rcc
./make_rcc.sh
if [ $? -eq 0 ];then
	rm *.error.log
else
	sed -i s/color\:green\'\>remote_PASS/color\:red\'\>FAIL/ $build_dir/rcc_report.html
fi
cp *.log $build_dir/log/
echo "############ build remote control ############ "


cd $build_dir
mkdir -p package_release/package
cd package_release
cp $build_dir_script/cloud_package.sh $build_dir_script/install.sh $build_dir_script/ltk_cloud ./
cp ./ltk_cloud.sh ./package/ltk_cloud
cp $build_dir_rcc/rcc_build_out/* ./package
cp $build_dir_cloud/ltk_clout_build_out/* ./package
cp $build_dir_script/../Case_information.xml ./package
chmod -R 777 ./package
./cloud_package.sh
chmod -R ltk_cloud.bin



echo "############ build_test_data ############ "
if [ $build_test_data -eq 1 ];then
	cd /root/tc_daily_build/ltk_cloud_build/
	#perl ./tools/auto_build/Case_information.pl -input_scp_folder=./scp -input_excel_folder=./tools
	cp ../Case_information.xml ./
	cp ../bsp_test_case.xls ./
	#perl ./TCMS_case_infor_export.pl -folder_for_excel=./
	perl ./test_data_filter.pl -i=./bsp_test_case.xls | tee ./auto_test_data_map_ori.txt
	perl ./merge_case_by_module.pl -input=Case_information.xml -data_map=auto_test_data_map_ori.txt | tee auto_test_data_map.txt
	DAILY_BUILD_SCRIPT_FOLDER=/root/tc_daily_build
	./merge_auto_test_data.sh  $DAILY_BUILD_SCRIPT_FOLDER/SVN/10.38.34.11/LTK/test_data $DAILY_BUILD_SCRIPT_FOLDER/bsp_auto_test_data/bsp_test_data/

	mkdir -p temp_folder/bsp_test_data
	mkdir module_package

	package_list=`ls data_standalone`
	for package in $package_list
	{
	package=`echo $package | sed s'|/||'`
	rm -r ./temp_folder/bsp_test_data/*
	cp -r ./data_standalone/$package/* ./temp_folder/bsp_test_data/
	./create_self_data.sh ./temp_folder
	mv setup_data.exe  module_package/$package".exe"
	}
fi
echo "############ build_test_data ############ "

. /root/tc_daily_build/ltk_cloud_build/config_ltk_cloud_build.sh


cd  $build_dir
mkdir -p $build_dir/../ltk_cloud_build_out
umount $build_dir/../ltk_cloud_build_out
mount -t smbfs -o username=admin,password=wsunhhuan10 -l //10.38.164.22/Daily_Build2  $build_dir/../ltk_cloud_build_out
cd $build_dir/../ltk_cloud_build_out
rm -r ltk_cloud_build-*
build_out_folder="ltk_cloud_build-$CURRENT_DATE"
mkdir -p $build_out_folder
#cp -r $DAILY_BUILD_SCRIPT_FOLDER/ltk_cloud_build/module_package $build_out_folder/
#cp -r $build_dir/ltk_cloud/rel/linux/staf/retail/lib $build_out_folder/

cp -r $build_dir/log $build_out_folder/
cp $build_dir/rcc_report.html $build_out_folder/

mkdir -p Build_History_ltk_cloud
cp -r $build_out_folder Build_History_ltk_cloud/
sleep 1
cd  $build_dir
umount $build_dir/../ltk_cloud_build_out



echo send_mail
cd  $build_dir
#cat $build_dir/rcc_report.html | formail -I "From: wsun@marvell.com" -I "To:wsun@marvell.com" -I "Cc:$MAIL_LIST" -I "MIME-Version:1.0" -I "Content-type:text/html;charset=gb2312" -I "Subject:BSP remort control auto build report" | /usr/sbin/sendmail -t



