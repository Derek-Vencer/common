#!/bin/bash

COMMAND=$1
set -e

if [[ "$COMMAND" == "clean" ]];
then
rm -rf out/*
else
chmod 777 tools/com_sdk/*
chmod 777 tools/com_sdk/cust/*
./tools/com_sdk/build_cust_cmd.sh best1306p
./tools/com_sdk/build_cust_cmd.sh best1502p
./tools/com_sdk/build_cust_cmd.sh best1503
./tools/com_sdk/cust/build_pro_1306p_0001.sh
./tools/com_sdk/cust/build_pro_1306p_0002.sh
./tools/com_sdk/cust/build_pro_1306p_0003.sh
./tools/com_sdk/cust/build_pro_1306p_0004.sh
./tools/com_sdk/cust/build_pro_1306p_0005.sh
./tools/com_sdk/cust/build_pro_1306p_0006.sh
./tools/com_sdk/cust/build_pro_1306p_0008.sh
./tools/com_sdk/cust/build_pro_1306p_0009.sh
./tools/com_sdk/cust/build_pro_1306p_0010.sh
./tools/com_sdk/cust/build_pro_1306p_0011.sh
./tools/com_sdk/cust/build_pro_1306p_0012.sh
./tools/com_sdk/cust/build_pro_1306p_0013.sh
./tools/com_sdk/cust/build_pro_1306p_0014.sh
./tools/com_sdk/cust/build_pro_1306p_0015.sh
./tools/com_sdk/cust/build_pro_1306p_0016.sh
./tools/com_sdk/cust/build_pro_1306p_0017.sh
./tools/com_sdk/cust/build_pro_1502p_0001.sh
./tools/com_sdk/cust/build_pro_1502p_0003.sh
./tools/com_sdk/cust/build_pro_1502p_0004.sh
./tools/com_sdk/cust/build_pro_1502p_0006.sh
./tools/com_sdk/cust/build_pro_1502p_0007.sh
./tools/com_sdk/cust/build_pro_1502p_0008.sh
./tools/com_sdk/cust/build_pro_1502p_0009.sh
./tools/com_sdk/cust/build_pro_1502p_0010.sh
./tools/com_sdk/cust/build_pro_1502p_0011.sh
./tools/com_sdk/cust/build_pro_1503_0001.sh
./tools/com_sdk/cust/build_pro_1503_0002.sh
./tools/com_sdk/cust/build_pro_1503_0003.sh
./tools/com_sdk/cust/build_pro_1503_0004.sh
fi
# ./tools/besui_tool/build.sh TWSPRO OTA
# ./tools/besui_tool/build.sh NOTWS OTA