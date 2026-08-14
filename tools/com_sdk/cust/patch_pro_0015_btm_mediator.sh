#!/bin/bash

set -e

PROJECT_ROOT="/mnt/c/Project/common"

LIB_DIR="${PROJECT_ROOT}/lib/bes/best1306p/PRO_0015"

TARGET_LIB="${LIB_DIR}/ibrt_libbt_profiles_sbc_enc_hfp_2m_dip_ble_gfps_anc.a"
PATCH_OBJ="${LIB_DIR}/patch/btm_mediator_old.o"

EXPECTED_MD5="f962be99a569c6a771c009153b8120bb"

TMP_DIR="/tmp/pro0015_btm_patch"

echo "=========================================="
echo "[BTM_PATCH] Start"
echo "=========================================="

if [ ! -f "${TARGET_LIB}" ]; then
    echo "[BTM_PATCH][ERROR] Target library not found"
    echo "${TARGET_LIB}"
    exit 1
fi

if [ ! -f "${PATCH_OBJ}" ]; then
    echo "[BTM_PATCH][ERROR] Patch object not found"
    echo "${PATCH_OBJ}"
    exit 1
fi

PATCH_MD5="$(md5sum "${PATCH_OBJ}" | awk '{print $1}')"

if [ "${PATCH_MD5}" != "${EXPECTED_MD5}" ]; then
    echo "[BTM_PATCH][ERROR] Invalid patch object"
    echo "[BTM_PATCH] Expected: ${EXPECTED_MD5}"
    echo "[BTM_PATCH] Actual  : ${PATCH_MD5}"
    exit 1
fi

rm -rf "${TMP_DIR}"
mkdir -p "${TMP_DIR}"

cp "${PATCH_OBJ}" "${TMP_DIR}/btm_mediator.o"

echo "[BTM_PATCH] Remove current btm_mediator.o"

arm-none-eabi-ar d \
    "${TARGET_LIB}" \
    btm_mediator.o

echo "[BTM_PATCH] Insert compatible btm_mediator.o"

cd "${TMP_DIR}"

arm-none-eabi-ar r \
    "${TARGET_LIB}" \
    btm_mediator.o

arm-none-eabi-ranlib \
    "${TARGET_LIB}"

rm -f "${TMP_DIR}/btm_mediator.o"

echo "[BTM_PATCH] Verify"

cd "${TMP_DIR}"

arm-none-eabi-ar x \
    "${TARGET_LIB}" \
    btm_mediator.o

FINAL_MD5="$(md5sum btm_mediator.o | awk '{print $1}')"

if [ "${FINAL_MD5}" != "${EXPECTED_MD5}" ]; then
    echo "[BTM_PATCH][ERROR] Verification failed"
    echo "[BTM_PATCH] Expected: ${EXPECTED_MD5}"
    echo "[BTM_PATCH] Actual  : ${FINAL_MD5}"
    exit 1
fi

echo "[BTM_PATCH] SUCCESS"
echo "[BTM_PATCH] btm_mediator.o MD5=${FINAL_MD5}"
echo "=========================================="

rm -rf "${TMP_DIR}"