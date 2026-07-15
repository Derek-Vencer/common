#!/bin/sh
#input:    ./tools/besui_tool/base64_hex.sh d3TZcGEXIy7VGhgj2yLnIG76MtSQ74uqqML9J2iKnh8=
input_val=$1

echo ' '
echo "|-----------------------------------------------------------------------------------|"
echo "|                              base64 to hex                                        |"
echo "|-----------------------------------------------------------------------------------|"

file_path=tools/besui_tool/base64.txt
echo base64_val=$input_val

rm -rf $file_path

# base64 to string
base64_val=`echo -n "$input_val" | base64 -d`

#string to hex
base64_hex=`echo -n "$base64_val" | od -A n -t x1`
echo base64_hex=$base64_hex | tr a-z A-Z

# write hex data to file
echo -n "{0x"  >> $file_path
# echo $base64_hex >> $file_path
echo $base64_hex | tr a-z A-Z >> $file_path

keyword1=' '
keyword2=', 0x'
sed -i 's/'"$keyword1"'/'"$keyword2"'/g' $file_path

echo " "
while read line
do
    echo -n $line
done < $file_path
echo -n "}"

rm -rf $file_path

echo " "
echo " "
echo "|-----------------------------------------------------------------------------------|"
echo "|                              base64 process end                                   |"
echo "|-----------------------------------------------------------------------------------|"

exit 0
