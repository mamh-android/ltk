#!/bin/bash
lines=29 #total lines + 1 in this script
STAF local shutdown shutdown
~/ltk_cloud/rcc_utilc  sysctl exit
if [ -d "~/ltk_cloud" ];then rm -rf ~/ltk_cloud;fi
mkdir ~/ltk_cloud;chmod 777 ~/ltk_cloud
sed -n ''$lines',$p' $0  > ~/ltk_cloud/__tmp__.tgz
cd ~/ltk_cloud;tar xzvf __tmp__.tgz;rm *.tgz
cp ltk_cloud /usr/bin
if [ `cat /etc/issue | grep -q "10\." ; echo $?` -eq 0 -o `cat /etc/issue | grep -q "9\." ; echo $?` -eq 0 ];then
cp rcc_utils.v151 rcc_utils
else
cp rcc_utils.v183 rcc_utils
fi
echo "install STAF"
awk 'BEGIN { cmd="cp -ir STAF/bin/* /usr/bin/" ; print "n" | cmd ;}' > /dev/null
awk 'BEGIN { cmd="cp -ir STAF/lib/* /usr/lib/" ; print "n" | cmd ;}' > /dev/null
echo "install STAF Finished "
mkdir -p /usr/local/staf ; cp -r STAF/codepage /usr/local/staf
echo "###Now you can use ltk_cloud command !!!!###"
ltk_cloud -h
#echo "Do you want add ltk_cloud into starting up? [Y/N]"
#read answer
#if [ $answer = "Y" -o $answer = "y" ];then
#ltk_cloud -a
#fi
echo "SETUP Finished @`pwd`!!!"
exit 0
