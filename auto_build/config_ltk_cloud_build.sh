#!/bin/bash
#PLATFORM INFORMATION
HWS="HELAN_LTE HELAN ADIR EDEN HELAN_LTE HELAN EMEI"
PDTS="pxa1L88dkb pxa1088dkb pxa1986sdk pxa1928dkb pxa1L88dkb pxa1088dkb pxa988dkb"
BRANCH="kk4.4 kk4.4 kk4.4 kk4.4 jb4.3 jb4.2_beta3 jb4.2_beta3"
#HWS="TTC988"
#PDTS="pxa988dkb"
#BRANCH="ics"
#PLATFORM[9]="camera_engine"
BDLOG_PREFIX=build_result.log

ondemand=no
on_demand_module=concurrency_stress

#Daily Build folder 
DAILYBUILDFOLDER=/root
DAILY_BUILD_SCRIPT_FOLDER=/root/tc_daily_build

AUTO_TEST_DATA="/root/tc_daily_build/bsp_test_data"
#Autobuild(BuildFarm) information
BUILD_FARM="//10.38.116.40/autobuild/android"
BUILD_FARM_LINK="\\\\\\\\10.38.116.40\\\\autobuild\\\\android"
BF_USER="qa"
BF_PASSWD="marvellqa"
#BSP Git information
GIT_SERVER="shgit.marvell.com/git/qae/"
GIT_USER="yq"
GIT_REPO="ATD_testsuite"
GIT_PASSWORD="marvell88"
COMMIT_ID_FILE=commitid
#Archive server information


#date
CURRENT_DATE="`date '+%Y-%m-%d'`"

#kernel version issue
MMP2_KERN_VER="\"2.6.35.7+ preempt mod_unload ARMv7 \""
TTC_KERN_VER="\"2.6.35.7+ preempt mod_unload ARMv5 \""
MG1_KERN_VER="\"2.6.35.7+ preempt mod_unload ARMv7 \""
MMP3_KERN_VER="\"2.6.35.7+ preempt mod_unload ARMv7 \""
#output information
DB_VER_INFO=daily_build_ver.log
IMAGE_INFO=image.info
OUTPUT_INFO=output.info

#MISC
SCRIPT=$0
TOTAL_ROLL_BACK=90
LOCAL_IP=10.38.34.178

#ARCHIVE 
ARCHIVE_BASE=/archive/
#REMOTE_ARCHIVE_BASE=//sh-fs03/APSE_QAE/BSP_validation/automation/auto_build
#REMOTE_ARCHIVE_BASE=//10.38.164.147/Daily_Build
#NAS_IP=10.38.164.147

REMOTE_ARCHIVE_BASE=//10.38.164.23/Daily_Build
NAS_IP=10.38.164.23

#Monthly Release
RELEASE_NAME=ATD_12R10

#Sendmail switch
SENDMAIL=Y

#Grablog switch
GRABLOG=Y

#MAIL Information
MAIL_FROM="yq@marvell.com"
#MAIL_LIST="yq@marvell.com"
MAIL_LIST="wsun@marvell.com,hhuan10@marvell.com,lbzhu@marvell.com,liuqw@marvell.com,chengwei@marvell.com"
