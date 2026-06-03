#!/bin/bash

# set -euo pipefail

DIRS=(
    "nn/beco"
    "nn/cmsis-nn"
    "nn/core-driver"
    "nn/core-platform"
    "nn/core-software"
    "nn/tflite-micro")

# Get all files in DIRS
FILE_LIST=$(find "${DIRS[@]}" -maxdepth 1 ! -name ".*" 2>/dev/null)

# Remove DIRS from file list
FILTERED_LIST="nn/Makefile"$'\n'

while IFS= read -r line; do
    in_dir=false
    for dir in "${DIRS[@]}"; do
        if [[ "$line" == "$dir" ]]; then
            in_dir=true
            break
        fi
    done
    if ! $in_dir; then
        FILTERED_LIST+="$line"$'\n'
    fi
done <<< "$FILE_LIST"

# echo "$FILTERED_LIST"

EXPORT_FILE_DIR="$EXPORT_FILE_DIR
$FILTERED_LIST"
