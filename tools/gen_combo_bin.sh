#!/bin/bash

# Check the number of arguments
if [ $# -ne 2 ]; then
    echo "Usage: $0 <upgrade mode: copy|remap> <target name: best1501|best1603_ibrt_anc|ohos_best1700...>"
    exit 1
fi

MODE="$1"
TARGET="$2"

# Validate upgrade mode
if [[ "$MODE" != "copy" && "$MODE" != "remap" ]]; then
    echo "Error: upgrade mode must be 'copy' or 'remap'"
    exit 1
fi

# Check if required files exist
if [ ! -f "out/arm_cmse/arm_cmse.bin" ]; then
    echo "Error: File out/arm_cmse/arm_cmse.bin not found."
    exit 1
fi

if [ ! -f "out/$TARGET/${TARGET}.bin" ]; then
    echo "Error: File out/$TARGET/${TARGET}.bin not found."
    exit 1
fi

echo ">>> Mode: $MODE"
echo ">>> Target: $TARGET"

# Step 1: Modify flag in Python script
PYTHON_FILE="tools/prompt_bin_generate/prompt_bin_generate.py"
echo "Updating IS_GENERATE_COMBO_BIN_FLAG to 0x01..."
sed -i 's/IS_GENERATE_COMBO_BIN_FLAG *= *0x00/IS_GENERATE_COMBO_BIN_FLAG = 0x01/' "$PYTHON_FILE"

# Step 2: Modify CSV file
CSV_FILE="tools/prompt_bin_generate/bin_gen.csv"
echo "Generating bin_gen.csv..."
cat > "$CSV_FILE" <<EOF
;  sub_id,  file_name
261,${TARGET}_enc.bin
260,arm_cmse_enc.bin
EOF

# Step 3: Execute commands based on the upgrade mode
BIN_DIR="tools/prompt_bin_generate/bin"
mkdir -p "$BIN_DIR"

if [ "$MODE" == "copy" ]; then
    echo ">>> Running in copy mode..."

    ./tools/build_compressed_ota.sh out/arm_cmse/arm_cmse.bin arm_cmse_cmp.bin
    python2 tools/generate_crc32_of_image.py arm_cmse_cmp.bin

    ./tools/build_compressed_ota.sh out/$TARGET/${TARGET}.bin ${TARGET}_cmp.bin
    python2 tools/generate_crc32_of_image.py ${TARGET}_cmp.bin

    openssl enc -aes-128-cbc -in arm_cmse_cmp.bin.converted.bin -out arm_cmse_encrypted.bin \
        -K 1F8199004CB7FBAF070C4864495399A0 -iv 3FA8DC149F54D369F71E7386BD37EE50 -p -nosalt

    mv arm_cmse_encrypted.bin $BIN_DIR/arm_cmse_enc.bin
    mv ${TARGET}_cmp.bin.converted.bin $BIN_DIR/${TARGET}_enc.bin

else
    echo ">>> Running in remap mode..."

    openssl enc -aes-128-cbc -in out/arm_cmse/arm_cmse.bin -out arm_cmse_encrypted.bin \
        -K 1F8199004CB7FBAF070C4864495399A0 -iv 3FA8DC149F54D369F71E7386BD37EE50 -p -nosalt

    mv arm_cmse_encrypted.bin $BIN_DIR/arm_cmse_enc.bin
    cp out/$TARGET/${TARGET}.bin $BIN_DIR/${TARGET}_enc.bin
fi

# Final step: Generate combo bin
echo ">>> Generating combo bin..."
python2 tools/prompt_bin_generate/prompt_bin_generate.py \
    -i 0x00000200 -v 0x00000001 \
    -c $CSV_FILE \
    -d $BIN_DIR

# Clean up temporary files if they exist
echo ">>> Cleaning up temporary files..."
rm -f arm_cmse_cmp.bin \
      arm_cmse_cmp.bin.converted.bin \
      crc.bin \
      magic.bin \
      ${TARGET}_cmp.bin \

mv ../combined_bin.bin combined_bin.bin
echo ">>> Upgrade package generated successfully. File name: combined_bin.bin ✅"
