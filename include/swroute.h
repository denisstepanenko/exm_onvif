#ifndef _SWROUTE_H
#define _SWROUTE_H

#define PARA_ERR				4000
#define DNAT_ERR		 	4001
#define SNAT_ERR				4002
#define FORWARD_ERR		4004
#define NO_MODULES			4005
#define NO_LIBS				4006


/*库函数必须要有iptables的支持 */
/*
************************************************
sw_route_tcp_NAT
简要描述：tcp协议网络地址和端口映射。
输入：	
		 d_ip		数据包的目的 IP地址，如"10.1.4.189"
		 d_port	数据包的目的端口号， 如8000
		 to_ip	数据包的映射到的IP地址,如"192.168.1.134"
		 to_port	数据包的映射到的端口号，如 9000
		 s_ip		需要将数据包的源地址更改为一个能够返回的ip
输出：	无
返回值:	0 成功	负值 失败	 错误原因在上面定义
修改日志：
************************************************
*/
int sw_route_tcp_NAT(char* d_ip, unsigned int d_port, char* to_ip, unsigned  int to_port, char* s_ip);

/*
使用例子: 虚拟机有两个网口 
	eth0 	10.1.1.100	255.0.0.0 
	eth1		192.168.4.22	255.255.255.0

	eth1与的网口设备相连，设备ip 192.168.4.88 	255.255.255.0

	服务器与虚拟机eth0相连 服务器ip 10.1.1.1	255.0.0.0 

	服务器需要通过虚拟机登陆设备。
	因此，我们要在虚拟机上做端口映射，将设备的23端口，映射到eth0的8999端口。
	23端口是telnet的服务端口。这样一来，我们从服务器telnet到虚拟机的8999端口，
	实际上就是登陆到设备的23端口了。
	利用这个函数来实现这一目的。
	sw_route_tcp_NAT( "10.1.1.100" , 8999 , "192.168.4.88" , 23 , "192.168.4.22" );
*/

#endif

