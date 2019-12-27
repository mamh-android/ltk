#!/bin/bash

base_dir=`pwd`
data_map=./auto_test_data_map.txt
data_folder_all=$1
data_standalone="`pwd`/data_standalone"
data_auto=$2
rm -r $data_auto
rm -r $data_standalone
rm data_size.log
mkdir -p $data_standalone
mkdir -p $data_auto

while read line;do
	copy_loop=1
	test_data=`echo $line | awk -F "|" '{print $2}'`
	test_id=`echo $line | awk -F "|" '{print $1}'`
	if [ "$test_data" != "NO_DATA" ];then
		data_number=`echo $line | awk -F ";" '{print NF}'`
		while [ $copy_loop -le $data_number ];do
			test_data_cp=`echo $test_data | awk -F ";" '{print $'$copy_loop'}'`
			test_data_foler=`echo ${test_data_cp%/*}`
			echo "$test_id: $test_data_cp -> $test_data_foler"
			mkdir -p $data_standalone/$test_id/$test_data_foler
			cd $data_folder_all
			cp -r $test_data_cp  $data_standalone/$test_id/$test_data_foler
			cd - > /dev/null
			let copy_loop=$copy_loop+1
		done
		find $data_standalone -name ".svn" | xargs rm -r 2>&1 | grep nothing > /dev/null
		data_size=`du -hsk $data_standalone/$test_id | awk '{print $1}'`
		echo "$test_id|$data_size|$test_data" >> data_size.log
	fi
done < $data_map


merge_list=`ls $data_standalone`
for merge_data in $merge_list ; do 
	cp -r $data_standalone/$merge_data/* $data_auto
done


data_size=`du -hsk $data_auto | awk '{print $1}'`
echo "Summary_Auto|$data_size|All_data_auto" >> data_size.log
data_size=`du -hsk $data_folder_all | awk '{print $1}'`
let data_size=$data_size/2
echo "Summary_All|$data_size|All_data" >> data_size.log
#cd $data_standalone 
#ls | xargs du -khs | sort -rn | sed s/"\t"/"|"/g
#cd -
