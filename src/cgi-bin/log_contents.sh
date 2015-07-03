#!/bin/sh
echo "Context-type:text/html; charset=gb2312"
echo ""

echo "<html>"
echo "<body>"



lines=`ls -al /log/gtlog*|wc -l`

#循环输出
lines=$(($lines-2))
while [ $lines -gt 0 ];do
	echo "<a href =\"/log/gtlog$lines.txt\" target =\"showframe\">"
	ls -al /log/gtlog.txt.$(($lines-1))|awk '{printf("%s %2d %s %s",$6,$7,$8,"-")}'
	ls -al /log/gtlog.txt.$lines|awk '{print $6,$7,$8}'
	echo "</a><br />"

	lines=$(($lines-1))
done

#输出第一行
current_time=`date|awk '{print $2,$3,$4}'`
echo "<a href =\"/log/gtlog.txt\" target =\"showframe\">"
ls -al /log/gtlog.txt|awk '{print $6,$7,$8,"-"}'
date "+%b %d %H:%M"
echo "</a><br />"

echo "</body>"
echo "</html>"
