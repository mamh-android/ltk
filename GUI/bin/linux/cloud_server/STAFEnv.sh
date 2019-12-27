#!/bin/sh
# STAF environment variables
PATH=~/ltk_cloud_server/STAF/bin:${PATH:-}
LD_LIBRARY_PATH=~/ltk_cloud_server/STAF/lib:${LD_LIBRARY_PATH:-}
CLASSPATH=~/ltk_cloud_server/STAF/JSTAF.jar:~/ltk_cloud_server/STAF/samples/demo/STAFDemo.jar:${CLASSPATH:-}
STAFCONVDIR=~/ltk_cloud_server/STAF/codepage
$*
