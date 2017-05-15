#!/bin/bash

###############################################################################
#Variables

OUTPATH=$1
CM4_BINARY_NAME=$2
N9_BINARY_NAME="WIFI_RAM_CODE_MT76X7_in_flash.bin"
HASH_OUT="hash_check"
IMAGE_CM4_OUT="image_cm4"
IMAGE_N9_OUT="image_n9"
A_POSTFIX="_A"
B_POSTFIX="_B"
HASH_POSTFIX="_hash"
FOTA_DUAL_FOLDER="fota_dual_folder"
###############################################################################
#Begin here
echo $OUTPATH
echo $CM4_BINARY_NAME

if [ -e "$OUTPATH/$FOTA_DUAL_FOLDER" ]; then
	echo "remove old dual_image_fota folder."
	rm -rf $OUTPATH/$FOTA_DUAL_FOLDER
fi
mkdir -p $OUTPATH/$FOTA_DUAL_FOLDER

## gen bianry A hash
./hash_gen "$OUTPATH/$N9_BINARY_NAME" "$N9_BINARY_NAME$A_POSTFIX$HASH_POSTFIX"
./hash_gen "$OUTPATH/$CM4_BINARY_NAME" "$CM4_BINARY_NAME$A_POSTFIX$HASH_POSTFIX"

## gen binary B hash
./hash_gen "$OUTPATH/binary_B/$N9_BINARY_NAME" "$N9_BINARY_NAME$B_POSTFIX$HASH_POSTFIX"
./hash_gen "$OUTPATH/binary_B/$CM4_BINARY_NAME" "$CM4_BINARY_NAME$B_POSTFIX$HASH_POSTFIX"

## cat
cat "$N9_BINARY_NAME$A_POSTFIX$HASH_POSTFIX" \
    "$CM4_BINARY_NAME$A_POSTFIX$HASH_POSTFIX" \
    "$N9_BINARY_NAME$B_POSTFIX$HASH_POSTFIX" \
    "$CM4_BINARY_NAME$B_POSTFIX$HASH_POSTFIX" \
    > $OUTPATH/$FOTA_DUAL_FOLDER/$HASH_OUT

rm  "$N9_BINARY_NAME$A_POSTFIX$HASH_POSTFIX" \
    "$CM4_BINARY_NAME$A_POSTFIX$HASH_POSTFIX" \
    "$N9_BINARY_NAME$B_POSTFIX$HASH_POSTFIX" \
    "$CM4_BINARY_NAME$B_POSTFIX$HASH_POSTFIX"

## rename binary
cp "$OUTPATH/$N9_BINARY_NAME" "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_N9_OUT$A_POSTFIX"
cp "$OUTPATH/$CM4_BINARY_NAME" "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_CM4_OUT$A_POSTFIX"
cp "$OUTPATH/binary_B/$N9_BINARY_NAME" "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_N9_OUT$B_POSTFIX"
cp "$OUTPATH/binary_B/$CM4_BINARY_NAME" "$OUTPATH/$FOTA_DUAL_FOLDER/$IMAGE_CM4_OUT$B_POSTFIX"

echo "hash gen done."