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
BASE_HC_VER="HellSpawn-N4-Nougat"
VER="-R06"
HC_VER="$BASE_HC_VER$VER"

# Vars
export ARCH=arm
export SUBARCH=arm
export CROSS_COMPILE=/home/spezi77/android/uber-tc/arm-eabi-6.x/bin/arm-eabi-
export LOCALVERSION="-$HC_VER"

# Paths
KERNEL_DIR=`pwd`
REPACK_DIR="${HOME}/android/Kernel/anykernel_msm"
ZIP_MOVE="${HOME}/android/Kernel/Releases/hellspawn-N4"
ZIMAGE_DIR="${HOME}/android/Kernel/android_kernel_lge_mako/arch/arm/boot"
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

function make_cm_kernel {
		HC_VER="$BASE_HC_VER$VER-CM-UBERTC-6.x"
		echo "[....Building `echo $HC_VER`....]"
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function make_aosp_kernel {
		HC_VER="$BASE_HC_VER$VER-AOSP-UBERTC-6.x"
		echo "[....Building `echo $HC_VER`....]"
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function make_special_aosp_kernel {
		HC_VER="$BASE_HC_VER$VER-AOSP-UBERTC-6.x-CPUSET"
		echo "[....Building `echo $HC_VER`....]"
		make $DEFCONFIG
		make $THREAD
		cp -vr $ZIMAGE_DIR/$KERNEL $REPACK_DIR/tmp/anykernel
}

function git_addback_cm_patch {
		branch_name=$(git symbolic-ref -q HEAD)
		branch_name=${branch_name##refs/heads/}
		branch_name=${branch_name:-HEAD}
		git checkout -b temp-for-making-cm-build
		git revert fe4d203c80f7cb2d491af21ea19fe1a2bf69265c --no-edit
}

function git_addback_cpuset_commits {
		branch_name=$(git symbolic-ref -q HEAD)
		branch_name=${branch_name##refs/heads/}
		branch_name=${branch_name:-HEAD}
		git checkout -b temp-for-making-special-build
		git revert 9474793a59bf888faa783d8556e56dc12b2b5831 --no-edit
		git revert d09522ae74613f29543a669a9c051dcb2037c331 --no-edit
}

function git_switch_to_previous_branch {
		git checkout $branch_name
		git branch -D temp-for-making-cm-build
		git branch -D temp-for-making-special-build
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
echo "HellSpawn-N4 Nougat Kernel Creation Script:"
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
		make_aosp_kernel
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
		git_addback_cm_patch
		make_cm_kernel
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
		git_addback_cpuset_commits
		make_special_aosp_kernel
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


