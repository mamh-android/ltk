#!/bin/bash


base_path=`pwd`
cd ./GUI/src

cd $base_path/GUI/src/cloud_client
make 2>$base_path/cloud_client.error.log 1>$base_path/cloud_client.log
cloud_client_build_result=$?
make install

cd $base_path/GUI/src/cloud_server
make 2>$base_path/cloud_server.error.log 1>$base_path/cloud_server.log
cloud_server_build_result=$?
make install

cd $base_path

mkdir -p ltk_cloud_client_build_out
cp -r ./GUI/bin/linux/cloud_client/* ./ltk_cloud_client_build_out
cp -r ./auto_build/package/STAF ./ltk_cloud_client_build_out/
chmod -R 777 ./ltk_cloud_client_build_out

mkdir -p ltk_cloud_server_build_out
cp -r ./GUI/bin/linux/cloud_server/* ./ltk_cloud_server_build_out
cp -r ./auto_build/package/STAF ./ltk_cloud_server_build_out/
chmod -R 777 ./ltk_cloud_server_build_out


if [ $cloud_client_build_result -ne 0 -o $cloud_server_build_result -ne 0 ];then
echo "cloud build fail !!!!!"
exit 1
fi
