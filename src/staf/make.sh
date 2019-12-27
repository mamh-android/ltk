#!/bin/bash
#make OS_NAME=linux PROJECTS=staf BUILD_TYPE=retail STAF_USE_IPV6=  OPENSSL_ROOT=./ OPENSSL_VERSION=0.9.8 ALL_LIB_LIST="-lSTAF -lpthread -lcrypt -ldl"

make OS_NAME=linux PROJECTS=staf BUILD_TYPE=retail STAF_USE_IPV6=  OPENSSL_ROOT=./ OPENSSL_VERSION=0.9.8 ALLMAKEFILES=stafif/makefile.staf 
make OS_NAME=linux PROJECTS=staf BUILD_TYPE=retail STAF_USE_IPV6=  OPENSSL_ROOT=./ OPENSSL_VERSION=0.9.8 ALLMAKEFILES=services/ltk_test/makefile.staf ALL_LIB_LIST="-lSTAF -lpthread -lcrypt -ldl"


#for test libarary
#make  OS_NAME=linux PROJECTS=staf BUILD_TYPE=retail STAF_USE_IPV6=  OPENSSL_ROOT=/usr/local/staf OPENSSL_VERSION=0.9.8 ALLMAKEFILES=/home/lbzhu/STAF/src/staf/test/makefile.staf ALL_LIB_LIST="-lSTAF -lpthread -lcrypt -ldl"

#the failed process to generate ucm2bin
#g++ -o /home/lbzhu/STAF/obj/linux/staf/retail/codepage/ucm2bin -D_XOPEN_SOURCE -D_BSD_SOURCE -DSTAF_OS_NAME_LINUX -DSTAF_GETHOSTBYNAME_R_6PARM -DSTAF_GETHOSTBYADDR_R_8PARM -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64  -fPIC -O2 -DNDEBUG -DSTAF_SHARED_LIB_PREFIX=lib -DSTAF_SHARED_LIB_SUFFIX=.so -D_PTHREADS -DSTAF_NATIVE_COMPILER   -I/usr/local/staf/include -I/home/lbzhu/STAF/src/staf/codepage -I/home/lbzhu/STAF/src/staf/codepage/unix -I/home/lbzhu/STAF/src/staf/stafif/unix -I/home/lbzhu/STAF/src/staf/stafif -L/home/lbzhu/STAF/rel/linux/staf/retail/lib -L/usr/local/staf/lib -lpthread -lcrypt -ldl -lSTAF /home/lbzhu/STAF/obj/linux/staf/retail/codepage/ucm2bin.o

#for cleanup
#make OS_NAME=linux PROJECTS=staf BUILD_TYPE=retail STAF_USE_IPV6=  OPENSSL_ROOT=./ OPENSSL_VERSION=0.9.8   cleanup
#bug caused by the gcc -l searching library issue, the -lSTAF should be put at the very beginning at least before -ldl
#g++ -o /home/lbzhu/STAF/obj/linux/staf/retail/codepage/ucm2bin -D__cplusplus -D_XOPEN_SOURCE -D_BSD_SOURCE -DSTAF_OS_NAME_LINUX -DSTAF_GETHOSTBYNAME_R_6PARM -DSTAF_GETHOSTBYADDR_R_8PARM -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64  -fPIC -O2 -DNDEBUG -DSTAF_SHARED_LIB_PREFIX=lib -DSTAF_SHARED_LIB_SUFFIX=.so -D_PTHREADS -DSTAF_NATIVE_COMPILER   -I/usr/local/staf/include -I/home/lbzhu/STAF/src/staf/codepage -I/home/lbzhu/STAF/src/staf/codepage/unix -I/home/lbzhu/STAF/src/staf/stafif/unix -I/home/lbzhu/STAF/src/staf/stafif -L/home/lbzhu/STAF/rel/linux/staf/retail/lib -L/usr/local/staf/lib  /home/lbzhu/STAF/obj/linux/staf/retail/codepage/ucm2bin.o -lSTAF -lpthread -lcrypt -ldl
