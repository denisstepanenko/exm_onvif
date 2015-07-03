#!/bin/bash
# Program: checkps.sh
#    There is not the PROCESS return 0 or check it some time(=TIMEOUT) and return 1 if the PROCESS always exist.
#
# arg1: process-name
# arg2: timeout 
# Usage: checkps [arg1] [arg2] 
# 
# date 2007.3.20 
#
VER=1.1

PRONAME=$1
WAITTIME=$2
#echo "VER=$VER"

if [ "$1" == "" ];then
	echo "arg1 error"
	exit 0
fi

if [ "$2" == "" ];then
	echo "arg2 error"
	exit 0
fi

if [ $2 -le 0 ];then
	echo "arg2 <= 0,error"
	exit 0
fi

echo "the proc : $PRONAME"
echo "the time : $WAITTIME"

RET_VAL=`ps -aux | grep "\<$PRONAME\>" | grep -v "checkps" | grep -v "grep"`
VAL=$?

#echo "val="
#echo $VAL
#echo "RET_VAL="
#echo "$RET_VAL"

WTME=$(($WAITTIME-1))

if [ "$VAL" == "0" ];then
#	echo "ok"
#	echo "i will sleep"
#	echo "before val=$VAL"	
	i=0
	
	while [ $i -ne $WTME ]
	do
#		echo "i will sleep i=$i"
		sleep 1
#		DATE=`date`
#		echo $DATE
		SUB_RET_VAL=`ps -aux | grep "\<$PRONAME\>" | grep -v "checkps" | grep -v "grep"`
		SUB_VAL=$?
#		echo "sub_val=$SUB_VAL"
		if [ "$SUB_VAL" != "0" ];then
			exit 0
		fi
		i=$(($i+1))
	done
	
else	
#	echo "not ok"
	exit 0	
fi

exit 1

