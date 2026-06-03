@echo off
color 0A
cls
cd %~dp0
:AGAIN
set /p pc_addr=input PC_Addrress:
D:\BES2700\Tools\GCC\gcc-arm-none-eabi-9-2019-q4-major-win32\bin\arm-none-eabi-addr2line.exe  -f -C -i -e out/best1306p/best1306p.elf  %pc_addr%
goto AGAIN
: