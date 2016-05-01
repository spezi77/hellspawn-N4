#!/bin/bash

# Bash Color
green='\033[01;32m'
red='\033[01;31m'
blink_red='\033[05;31m'
restore='\033[0m'

clear

# Resources
THREAD="-j6"
KERNEL="zImage"
DEFCONFIG="hellspawn_mako_defconfig"

# Kernel Details
BASE_HC_VER="hellspawn-N4-mm-6.0"
VER="-r10"
HC_VER="$BASE_HC_VER$VER"

# Vars
export ARCH=arm
export SUBARCH=arm

# Paths
KERNEL_DIR=`pwd`
REPACK_DIR="${HOME}/android/Kernel/anykernel_msm"
ZIP_MOVE="${HOME}/android/Kernel/Releases/hellspawn-N4"
ZIMAGE_DIR="${HOME}/android/Kernel/hellspawn-N4/arch/arm/boot"
DB_FOLDER="${HOME}/Dropbox/Kernel-Betas/hellspawn-N4"

# Functions
function clean_all {
		rm -rf $REPACK_DIR/tmp/anykernel/zImage
		make clean && make mrproper
}

function make_kernel {
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function make_bs_kernel_TC5 {
		HC_VER="$BASE_HC_VER$VER-BS-UBERTC-5.3"
		echo "[....Building `echo $HC_VER`....]"
		export CROSS_COMPILE=/home/spezi77/android/prebuilts/gcc/linux-x86/arm/arm-eabi-5.3/bin/arm-eabi-
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function make_cm_kernel_TC5 {
		HC_VER="$BASE_HC_VER$VER-CM-UBERTC-5.3"
		echo "[....Building `echo $HC_VER`....]"
		export CROSS_COMPILE=/home/spezi77/android/prebuilts/gcc/linux-x86/arm/arm-eabi-5.3/bin/arm-eabi-
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function make_aosp_kernel_TC5 {
		HC_VER="$BASE_HC_VER$VER-AOSP-UBERTC-5.3"
		echo "[....Building `echo $HC_VER`....]"
		export CROSS_COMPILE=/home/spezi77/android/prebuilts/gcc/linux-x86/arm/arm-eabi-5.3/bin/arm-eabi-
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function make_bs_kernel_TC7 {
		HC_VER="$BASE_HC_VER$VER-BS-UBERTC-7.0"
		echo "[....Building `echo $HC_VER`....]"
		export CROSS_COMPILE=/home/spezi77/android/prebuilts/gcc/linux-x86/arm/arm-eabi-7.0/bin/arm-eabi-
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function make_cm_kernel_TC7 {
		HC_VER="$BASE_HC_VER$VER-CM-UBERTC-7.0"
		echo "[....Building `echo $HC_VER`....]"
		export CROSS_COMPILE=/home/spezi77/android/prebuilts/gcc/linux-x86/arm/arm-eabi-7.0/bin/arm-eabi-
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function make_aosp_kernel_TC7 {
		HC_VER="$BASE_HC_VER$VER-AOSP-UBERTC-7.0"
		echo "[....Building `echo $HC_VER`....]"
		export CROSS_COMPILE=/home/spezi77/android/prebuilts/gcc/linux-x86/arm/arm-eabi-7.0/bin/arm-eabi-
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function git_revert_cm_commits {
		branch_name=$(git symbolic-ref -q HEAD)
		branch_name=${branch_name##refs/heads/}
		branch_name=${branch_name:-HEAD}
		git checkout -b temp-for-making-aosp-builds
		git revert 091183726158125d1f72f82c7ea8326dda08a86a --no-edit
		git revert feda27448d5dad766dd06066f9f318f94a938ac3 --no-edit
}

function git_revert_to_cm_comp_gamma {
		branch_name=$(git symbolic-ref -q HEAD)
		branch_name=${branch_name##refs/heads/}
		branch_name=${branch_name:-HEAD}
		git checkout -b temp-for-making-cm-builds
		git revert aec312d0288f709f809a9744760b1055704aa421 --no-edit
		git revert da1972c44274be088f676cbb5a2953538a855d90 --no-edit
		git revert f7acaa7cb32396c76eb7c8054474cbaa30c8e759 --no-edit
		git revert 48a60002eb5f1838a01efdc28e040b7b617ee756 --no-edit
		git revert 244ddc67e3da111b31f017e76c84ddcd85493150 --no-edit
}

function git_switch_to_previous_branch {
		git checkout $branch_name
		git branch -D temp-for-making-aosp-builds
		git branch -D temp-for-making-cm-builds
}

function make_zip {
		cd $REPACK_DIR
		zip -9 -r --exclude='*.git*' `echo $HC_VER`.zip .
		mv  `echo $HC_VER`.zip $ZIP_MOVE
		cd $KERNEL_DIR
}

function copy_dropbox {
		cd $ZIP_MOVE
		cp -vr  `echo $HC_VER`.zip $DB_FOLDER
		cd $KERNEL_DIR
}

DATE_START=$(date +"%s")

echo -e "${green}"
echo "hellspawn-N4 Kernel Creation Script:"
echo

echo "---------------"
echo "Kernel Version:"
echo "---------------"

echo -e "${red}"; echo -e "${blink_red}"; echo "$HC_VER"; echo -e "${restore}";

echo -e "${green}"
echo "-----------------"
echo "Making Kernel:"
echo "-----------------"
echo -e "${restore}"

echo "----------------------------"
echo "Please choose your option:"
echo "----------------------------"
while read -p " [1]clean-build / [2]dirty-build / [3]batch-build / [4]abort " cchoice
do
case "$cchoice" in
	1 )
		HC_VER="$BASE_HC_VER$VER"
		echo -e "${green}"
		echo
		echo "[..........Cleaning up..........]"
		echo
		echo -e "${restore}"
		clean_all
		echo -e "${green}"
		echo
		echo "[....Building `echo $HC_VER`....]"
		echo
		echo -e "${restore}"
		make_kernel
		echo -e "${green}"
		echo
		echo "[....Make `echo $HC_VER`.zip....]"
		echo
		echo -e "${restore}"
		make_zip
		echo -e "${green}"
		echo
		echo "[.....Moving `echo $HC_VER`.....]"
		echo
		echo -e "${restore}"
		copy_dropbox
		break
		;;
	2 )
		HC_VER="$BASE_HC_VER$VER"
		echo -e "${green}"
		echo
		echo "[....Building `echo $HC_VER`....]"
		echo
		echo -e "${restore}"
		make_kernel
		echo -e "${green}"
		echo
		echo "[....Make `echo $HC_VER`.zip....]"
		echo
		echo -e "${restore}"
		make_zip
		echo -e "${green}"
		echo
		echo "[.....Moving `echo $HC_VER`.....]"
		echo
		echo -e "${restore}"
		copy_dropbox
		break
		;;
	3 )
		HC_VER="$BASE_HC_VER$VER"
		echo -e "${green}"
		echo
		echo "[..........Cleaning up..........]"
		echo
		echo -e "${restore}"
		clean_all
		echo -e "${green}"
		echo
		make_bs_kernel_TC5
		echo
		echo -e "${restore}"
		echo -e "${green}"
		echo
		echo "[....Make `echo $HC_VER`.zip....]"
		echo
		echo -e "${restore}"
		make_zip
		echo -e "${green}"
		echo
		echo "[.....Moving `echo $HC_VER`.....]"
		echo
		echo -e "${restore}"
		copy_dropbox

		HC_VER="$BASE_HC_VER$VER"
		echo -e "${green}"
		echo
		echo "[..........Cleaning up..........]"
		echo
		echo -e "${restore}"
		clean_all
		echo -e "${green}"
		echo
		make_bs_kernel_TC7
		echo
		echo -e "${restore}"
		echo -e "${green}"
		echo
		echo "[....Make `echo $HC_VER`.zip....]"
		echo
		echo -e "${restore}"
		make_zip
		echo -e "${green}"
		echo
		echo "[.....Moving `echo $HC_VER`.....]"
		echo
		echo -e "${restore}"
		copy_dropbox

		HC_VER="$BASE_HC_VER$VER"
		echo -e "${green}"
		echo
		echo "[..........Cleaning up..........]"
		echo
		echo -e "${restore}"
		clean_all
		echo -e "${green}"
		echo
		git_revert_to_cm_comp_gamma
		make_cm_kernel_TC5
		echo
		echo -e "${restore}"
		echo -e "${green}"
		echo
		echo "[....Make `echo $HC_VER`.zip....]"
		echo
		echo -e "${restore}"
		make_zip
		echo -e "${green}"
		echo
		echo "[.....Moving `echo $HC_VER`.....]"
		echo
		echo -e "${restore}"
		copy_dropbox

		HC_VER="$BASE_HC_VER$VER"
		echo -e "${green}"
		echo
		echo "[..........Cleaning up..........]"
		echo
		echo -e "${restore}"
		clean_all
		echo -e "${green}"
		echo
		make_cm_kernel_TC7
		echo
		echo -e "${restore}"
		echo -e "${green}"
		echo
		echo "[....Make `echo $HC_VER`.zip....]"
		echo
		echo -e "${restore}"
		make_zip
		echo -e "${green}"
		echo
		echo "[.....Moving `echo $HC_VER`.....]"
		echo
		echo -e "${restore}"
		copy_dropbox
		git_switch_to_previous_branch

		HC_VER="$BASE_HC_VER$VER"
		echo -e "${green}"
		echo
		echo "[..........Cleaning up..........]"
		echo
		echo -e "${restore}"
		clean_all
		echo -e "${green}"
		echo
		git_revert_cm_commits
		make_aosp_kernel_TC5
		echo
		echo -e "${restore}"
		echo -e "${green}"
		echo
		echo "[....Make `echo $HC_VER`.zip....]"
		echo
		echo -e "${restore}"
		make_zip
		echo -e "${green}"
		echo
		echo "[.....Moving `echo $HC_VER`.....]"
		echo
		echo -e "${restore}"
		copy_dropbox

		HC_VER="$BASE_HC_VER$VER"
		echo -e "${green}"
		echo
		echo "[..........Cleaning up..........]"
		echo
		echo -e "${restore}"
		clean_all
		echo -e "${green}"
		echo
		make_aosp_kernel_TC7
		echo
		echo -e "${restore}"
		echo -e "${green}"
		echo
		echo "[....Make `echo $HC_VER`.zip....]"
		echo
		echo -e "${restore}"
		make_zip
		echo -e "${green}"
		echo
		echo "[.....Moving `echo $HC_VER`.....]"
		echo
		echo -e "${restore}"
		copy_dropbox
		git_switch_to_previous_branch

		break
		;;
	4 )
		break
		;;
	* )
		echo -e "${red}"
		echo
		echo "Invalid try again!"
		echo
		echo -e "${restore}"
		;;
esac
done

echo -e "${green}"
echo "-------------------"
echo "Build Completed in:"
echo "-------------------"
echo -e "${restore}"

DATE_END=$(date +"%s")
DIFF=$(($DATE_END - $DATE_START))
echo "Time: $(($DIFF / 60)) minute(s) and $(($DIFF % 60)) seconds."
echo

