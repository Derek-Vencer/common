#!/bin/bash

chip_target=""
rom_target=""

for arg in "$@"; do
    if [[ $arg == chip=* ]]; then
        chip_target="${arg#chip=}"
        break
    fi
done

for arg in "$@"; do
    if [[ $arg == rom=* ]]; then
        rom_target="${arg#rom=}"
        break
    fi
done

if [[ $1 == "clean" ]]; then
    if [[ -z "$chip_target" ]]; then
        echo "No chip specified"
        echo "Usage: build_bth_rom_auto_test.sh  clean chip=xxx"
        exit 1
    fi
else
    if [[ -z "$chip_target" ]] || [[ -z "$rom_target" ]]; then
        echo "No chip specified"
        echo "Usage: build_bth_rom_auto_test.sh  rom_make chip=xxx rom=xxx"
        exit 1
    fi
fi


if [[ $chip_target =~ best1307 ]]; then
    rom_dir="1307"
elif [[ $chip_target =~ best1502 ]]; then
    rom_dir="1502"
elif [[ $chip_target =~ best1503 ]]; then
    rom_dir="1503"
elif [[ $chip_target =~ best1306 ]]; then
    rom_dir="1306"
else
    echo "No valid chip specified"
    exit 1
fi

function gen_rom_bin()
{
    cd ./out/bth_rom/
    arm-none-eabi-objcopy -O binary bth_rom.elf -j .rom_text -j .rodata -j .rom_libc_text bth_rom.bin
    arm-none-eabi-objcopy -O binary bth_rom.elf -j .data bth_ram.bin
    arm-none-eabi-objcopy -j .rom_text -j .data -j .bss bth_rom.elf
    mkdir ./../../bthost/rom/"$rom_dir"
    mkdir ./../../bthost/rom/"$rom_dir"/lib/
    ./../../tools/bin2ascii.pl bth_rom.bin bth_rom.rcf -flash 4
    cp -r ./bth_rom.elf ./../../bthost/rom/"$rom_dir"/lib/
    cp -r ./bth_rom.bin ./../../bthost/rom/"$rom_dir"/lib/
    cp -r ./bth_ram.bin ./../../bthost/rom/"$rom_dir"/lib/
    cp -r ./bth_rom.map ./../../bthost/rom/"$rom_dir"/lib/
    cp -r ./bth_rom.rcf ./../../bthost/rom/"$rom_dir"/lib/
    cp -r ./bth_rom.lst ./../../bthost/rom/"$rom_dir"/lib/
    cd ../../
}

build_bth_rom_cmd="make T=bth_rom CHIP=$rom_target DEBUG=1 IMAGE_IN_FLASH=0 NOSTD=0 BT_STACK_LOG_DISABLE=0 BTHOST_ROM_TEST=1 OPT_LEVEL=s -j8 all lst"
build_target_with_rom_cmd="make T=$chip_target -j APP_RX_API_ENABLE=1 APP_TRACE_RX_ENABLE=1 GPIO_WAKEUP_ENABLE=1 BTH_IN_ROM=1 BTHOST_ROM_TEST=1 all lst"

if [[ $1 == "clean" ]];
then
    make T=$chip_target clean
elif [[ $1 == rom_make ]];
then
    rm -rf ./out
    $build_bth_rom_cmd
    gen_rom_bin
    $build_target_with_rom_cmd
elif [[ $1 == 'gen_lib' ]];
then
    $build_target_with_rom_cmd GEN_LIB=1
else
    $build_target_with_rom_cmd
fi