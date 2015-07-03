#!/bin/bash
#	program: rm_dir_file.sh
#	arg1   : path
#	arg2   : filename	
#	Delet all the files named FILENAME under the PATH.
#	data   :2007.03.21



RM_DIR=$1
RM_FILE=$2

if [ "$RM_DIR" == "" ];then
	echo "path error"
	exit 1
fi 

if [ "$RM_FILE" == "" ];then
	echo "filename error"
	exit 1
fi


#echo "RM_DIR=$RM_DIR"
#echo "RM_FILE=$RM_FILE"

du $RM_DIR | cut -d '/' -f 2- > 01_rm_dir.txt

chmod 777 01_rm_dir.txt

while read FILEE 
do 
#	echo "FILEE : $FILEE" 
	rm -fr /$FILEE/$2
done < 01_rm_dir.txt 
#done < `du $RM_DIR | cut -d '/' -f 2-`
      
rm -f 01_rm_dir.txt

