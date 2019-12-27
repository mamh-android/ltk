#!/bin/bash
cd package
tar -cvzf ../release.tgz ./
cat ../install.sh ../release.tgz > ../ltk_cloud.bin
sudo chmod -R 777 ../ltk_cloud.bin
