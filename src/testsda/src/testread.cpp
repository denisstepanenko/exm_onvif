/*
 * testread.cpp
 *
 *  Created on: 2013-11-7
 *      Author: yangkun
 */
#if 0
#include <signal.h>
#include <iostream>
#include "Blocks.h"
#include "DataWR.h"
#include "DayBlocks.h"
using namespace std;
using namespace gtsda;
#include "RWlogic.h"
static RWlogic *rd;

pthread_t ntid;
void *thr_send(void *arg)
{
	sleep(3);
	while(1)
	{
		cout << "thr send!!\n";
		usleep(40000);
		rd->write_for_rtsp(0 );
	}
	return NULL;
}
static void HandleSig(int signo)
{

	cout << "signo:" << signo << endl;
   // if (SIGINT == signo || SIGTSTP == signo)
    //{
    	rd->Stop();
        cout << "\033[0;31mprogram termination abnormally!\033[0;39m\n" ;
    //}
    exit(-1);
}

int main(int argc,char **argv) {
	  if (already_running(LOCKFILE))
	                return 0;
	int type;
	if(argc!=2)
	{
		cout << "usage: %s usetype" << endl;
		cout << "ustype 0   : only read ";
		cout << "       time: only write";
		return 0;
	}
	type=atoi(argv[1]);
	gtopenlog("sda_read");
	if(type!=0)
	{
		/*捕捉信号*/
		signal(SIGINT, HandleSig);
		signal(SIGTERM, HandleSig);
		signal(SIGTSTP, HandleSig);

		int err;
		char p[]="new thread: ";
		cout << p << endl;
		err = pthread_create(&ntid, NULL, &thr_send, p);
		if (err != 0)
		{
			cout << " can't create thread: ";
			exit(1);
		}
	}


#if 1
    int ret;
    int starttime;
    if(type==0)cout << "<!--"<< endl; //html 注示
	rd = RWlogic::newRW(false/*read*/);
	rd->Init();
	if(type==0)cout << "-->"<< endl; //html 注示
	if(type==0)
	{
		cout << "<p>" << endl;
		if( ( ret = rd->read_disk_print_record_time()) < 0 )
		{
			cout << "read print error!! " << endl;
			goto err;
		}
		cout << "<p><br>"<< endl;
	}
	else
	{
		//cout << "please in put your want time of begin:" << endl;
		//cin >> starttime;
		//cout << "you input time is :" << ctime((const time_t *)&starttime);
		starttime = type;
		if(starttime<=0)
		{
			cout << "you input time is wrong !!" << endl;
			goto err;
		}
		rd->start(starttime);
	}
err:
if(type==0)cout << "<!--"<< endl; //html 注示
	delete rd;
if(type==0)cout << "-->"<< endl; //html 注示







#else//测试复制天块到备份天块逻辑正确性
	if(init_sda()<0)
		cout<< "init err" << endl;
	DayBlocks db;
	RWlogic::printOneDay((const struct day_block *)(db.GetBuf()), 0 );
	delete rd;
#endif



	cout << "bye bye " << endl;
	return 0;
}
#endif













#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "wrap.h"
#include "RWlogic.h"

#define MAXLINE 80

using namespace std;
using namespace gtsda;


int main(int argc, char **argv)
{

	//if (already_running(LOCKFILE))
	//                return 0;
	int type;
	if(argc!=2)
	{
		cout << "usage: %s usetype" << endl;
		cout << "ustype 0   : only read ";
		cout << "       time: only write";
		return 0;
	}
	type=atoi(argv[1]);


	struct sockaddr_in servaddr;
	char buf[MAXLINE];
	int sockfd, n;

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(CMD_PORT);

	Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	char *msg_content=NULL;//消息内容
	msg_st msg;
	msg.msg_head = MSG_HEAD;
	if(type == 0)
	{
		msg.msg_len = 0;
		msg.msgtype = request_time;
	}
	else
	{
		msg.msg_len = type;
		msg.msgtype = cmd_play;
	}


	Write(sockfd, &msg, sizeof(struct msg_st));






	//读数据头
	memset(&msg, 0, sizeof(struct msg_st));
	n = Read(sockfd, &msg, sizeof(struct msg_st));
	if (n == 0)
		printf("the other side has been closed.\n");
	if(msg.msg_head != MSG_HEAD)
	{
		cout << "msg error!!"<< endl;
		exit(-1);
	}
	if(msg.msgtype == cmd_ret_ok)
	{
		cout << " cmd return ok"<< endl;
	}
	else
		cout << " cmd return error"<< endl;
	if(msg.msg_len != 0)
	{
		cout << "msg len: " << msg.msg_len << endl;
		msg_content = new char[msg.msg_len+1];
		memset(msg_content, '\0' , msg.msg_len+1);
		n = Read(sockfd, msg_content, msg.msg_len);
		if (n == 0)
			printf("the other side has been closed.\n");
		cout << "msg_content: " << msg_content << endl;
		delete []msg_content;
	}

cout << "debug" << endl;
	Close(sockfd);


	return 0;
}
