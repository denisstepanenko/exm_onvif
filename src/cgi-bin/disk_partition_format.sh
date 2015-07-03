#!/bin/sh
## do something here
process_cgi()
{
#echo "debug3"
if [ "$REQUEST_METHOD" = "POST" ] ;then
QUERY_STRING=`cat -`
fi
#echo $QUERY_STRING
if [ $QUERY_STRING = 0 ]; then
	echo "you input is wrong" && exit 1
fi

if [ -f /dev/sda$QUERY_STRING ] ;then
echo "you input is wrong" && exit 1
else
/sbin/mke2fs -T ext5 /dev/sda$QUERY_STRING -b 4096 -j -L hqdata$QUERY_STRING -m 1 -i 1048576 1>/dev/null 2>/dev/null
if [ $? = 0 ] ;then
echo "格式化成功!!"
else
echo "格式化失败!!"
fi
fi

}







#程序从这开始运行
echo "Context-type:text/html; charset=gb2312"
echo ""
echo "<html>"
echo "<body>"

LOCKFILE=/tmp/$(basename $0)_lockfile
#判断函数是否在运行
if [ -f $LOCKFILE  ];then
#在运行则退出
        process_cgi "The script backup.sh is running" && exit 1
	else 
        echo $$ > $LOCKFILE
fi



#echo "debug1"
#没有运行
if [ -e /dev/sda ];then
#echo "debug2"
killall -9 watch_proc>/dev/null
killall -15 hdmodule>/dev/null
killall -9 diskman>>/dev/null
killall -9 ftpd>>/dev/null
killall -9 e2fsck>>/dev/null
killall -9 encbox>>/dev/null
killall -9 rtimage>>/dev/null
umount /hqdata/sd*
process_cgi "start format"
else
echo  "no disk" 
fi

rm -rf $LOCKFILE 
/sbin/reboot
echo ""
echo "</body>"
echo "</html>"
echo ""
