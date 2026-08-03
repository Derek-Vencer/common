./tools/com_sdk/cust/build_pro_1306p_0015.sh

sed -i 's/\r$//' tools/com_sdk/cust/build_pro_1306p_0015.sh

./tools/build_compressed_ota.sh  out/best1306p/best1306p.bin  out/best1306p/best1306p_ota_0730_v_0_9_4_10_yamaki10.mp3


static const uint8_t g_ntt_local_fw_version
    [NTT_EARBUD_FW_VERSION_LEN] =
{
    0x00,
    0x09,
    0x06,
};

static const uint8_t local_version[NTT_EARBUD_FW_VERSION_LEN] =
{
    0x00,
    0x09,
    0x06,
};