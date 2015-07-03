#!/bin/sh
echo "Context-type:text/html; charset=gb2312"
echo ""
echo "<html>"
echo "<body>"
echo "debug  "
#1、改devinfo.ini中的lfc_flag标志位
sed -i 's/\(lfc_flag\).*/\1                       = 1/' /conf/devinfo.ini
#2、调用initdisk程序
/ip1004/initdisk 1>/dev/null&
echo $?
echo "success init"

echo ""
echo "</body>"
echo "</html>"

