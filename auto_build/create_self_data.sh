#!/bin/bash

android_main_version=`cat $_PSDK_DIR_/build/core/version_defaults.mk | grep "PLATFORM_VERSION :=" | head -n 1 | awk -F":=" '{print $2}' | awk -F"." '{print $1}'`
DESTFILE=setup_data.exe

create_self()
{
    [ $# = "0" ] && echo "Usage: $0 package_name" && exit 0
    pad=4096
    tmp_file="__tmp_data__.tgz"

    package_size=`ls -l $1 | awk '{ print $5 }'`
    package_padding=$(($package_size % $pad))
    if [ "$package_padding" != "0"  ];then
        package_padding=$(($pad - $package_padding))
    fi
    package_padding_size=$(($package_size + $package_padding))
    package_padding_block=$(($package_padding_size / $pad))

    head_lines=$(($head_lines_end - $head_lines_start + 1))
    echo "head_lines=$head_lines"
    #ICS version remove busybox from root folder
    date_string=`date +%Y-%m-%d`
    if [ $android_main_version -lt 4 ];then
        echo '#!/busybox/bin/busybox sh' > $DESTFILE
    fi
    echo "if [ \$\# -gt 0 -a \$1 = \"-v\" ]; then echo \"Build date: $date_string\";exit 0;fi" >>$DESTFILE
    echo "export PATH=/data/bin:\$PATH" >> $DESTFILE
    echo "if [ ! -d /data/test2 ];then mkdir /data/test2;fi" >> $DESTFILE 
    echo "MMC_DEVICES=\$(/data/bin/busybox ls -d /sys/bus/mmc/drivers/mmcblk/mmc*);g_storage_device_node=\"unknown\"" >> $DESTFILE
    echo "for mmc_device in \$MMC_DEVICES" >> $DESTFILE
    echo "do if [ \$(cat \$mmc_device/type) = \"SD\" ];then" >> $DESTFILE
    echo "g_storage_device_node=\"/dev/block/\"\$(ls \$mmc_device/block/)" >> $DESTFILE
    echo "if [ -b \"\$g_storage_device_node\"\"p1\" ];then g_storage_device_node=\"\$g_storage_device_node\"\"p1\"" >> $DESTFILE
    echo "fi;fi;done" >> $DESTFILE
    echo "if [ \"\$g_storage_device_node\" != \"unknown\" ];then" >> $DESTFILE
    echo "if [ -n \"\$(mount | /data/bin/busybox grep \$g_storage_device_node | /data/bin/busybox grep \"/data/test2\")\" ];then /data/bin/busybox mount \$g_storage_device_node /data/test2;fi;fi" >> $DESTFILE
    echo "echo \"extracting package...\"" >> $DESTFILE
    echo "if [ ! -d /data/tmp ];then mkdir /data/tmp;fi" >> $DESTFILE
    echo "mounted=\$(mount | /data/bin/busybox grep \"/data/tmp\");if [ -n \"\$mounted\" ];then umount /data/tmp;fi">> $DESTFILE
    echo "mount -t tmpfs none /data/tmp" >> $DESTFILE
    echo "free_size=\`/data/bin/busybox df -k | /data/bin/busybox grep /data/tmp | /data/bin/busybox head -n 1 | /data/bin/busybox awk '{print \$4
}'\`" >>  $DESTFILE
echo "if [ \$free_size -gt 102400 -a 1 -eq 0 ];then echo \"extract image into ramdisk!\";tmp_file=\"/data/tmp/$tmp_file\";else echo \"less than 100M in ramdisk, try to switch to Flash!\";free_size=\`/data/bin/busybox df -k | /data/bin/busybox grep \"/data\" | /data/bin/busybox head -n 1 | /data/bin/busybox awk '{print \$(NF-2)}'\`;if [  \$free_size -lt 102400 ];then echo \"Error:no enough free space on flash less than 100M!!!\";exit 1;else tmp_file=$tmp_file;fi;fi" >> $DESTFILE
echo "dd if=\$0 of=\$tmp_file.p bs=$pad count=$package_padding_block seek=0 skip=$(($busybox_padding_block + 1))" >> $DESTFILE
echo "dd if=\$tmp_file.p of=\$tmp_file bs=$package_size count=1 skip=0 seek=0" >> $DESTFILE
echo "if [ ! -d /data/test2 ];then mkdir /data/test2;fi" >> $DESTFILE
echo "busybox tar -xzvf \$tmp_file -C /data/test2;rm \$tmp_file \$tmp_file.p" >> $DESTFILE
echo "if [ -d /data/test2/bsp_test_data/vmin ];then /data/bin/busybox cp -r /data/test2/bsp_test_data/vmin/* /data/test/vmin/worst_case/;fi" >> $DESTFILE
echo "echo \"test data package has been extracted into /data/test2\"" >> $DESTFILE
echo "exit 0" >> $DESTFILE
head_size=`ls -l $DESTFILE | awk '{ print $5 }'`;left_size=`expr $pad - $head_size`
echo "size=$head_size,left_size=$left_size"
dd if=/dev/zero of=$DESTFILE bs=1 count=$left_size skip=0 seek=$head_size
chmod 777 $DESTFILE 
cat $1  >> $DESTFILE
if [ ! $package_padding -eq 0 ];then
    echo "package is padded $package_padding bytes for $pad alignment!"
    dd if=/dev/zero of=$DESTFILE bs=1 count=$package_padding skip=0 seek=$(($pad + $busybox_padding_size + $package_size))
fi
}


[ $# = "0" ] && echo "Usage: $0 data_dir" && exit 0
data_dir=$1
[ ! -d $data_dir/bsp_test_data ] && echo "invalid test data directory!!!" && exit 0
chmod 777 $data_dir -R
if [ -f $DESTFILE ];then rm $DESTFILE;fi

echo "create data package from $data_dir ..."
cd $data_dir
if [ -f bsp_test_data.tgz ];then rm "bsp_test_data.tgz";fi
if [ -f setup_data.exe ];then rm "setup_data.exe";fi
tar zcvf bsp_test_data.tgz bsp_test_data/
echo "create package done"
cd -

head_lines_start=`awk '/#!\/busybox/{print FNR}' $0 | tail -n 1` 
head_lines_end=`awk '/"exit 0"/{print FNR}' $0 | tail -n 1`
create_self $data_dir/bsp_test_data.tgz
echo "setup_data.exe create done"
if [ -f $data_dir/bsp_test_data.tgz ];then rm $data_dir/bsp_test_data.tgz;fi

