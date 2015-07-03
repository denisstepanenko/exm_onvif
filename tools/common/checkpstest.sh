#!/bin/bash

VAL=`source /checkps.sh diskman 3`
RET_VAL=$?

echo "before-ret_val=$RET_VAL"

if [ "$RET_VAL" == "1" ];then
	echo "ret_val=0"
	echo "ok,done"
	
else
	echo "not ok"
	echo "after-ret_val=$RET_VAL"	
fi




