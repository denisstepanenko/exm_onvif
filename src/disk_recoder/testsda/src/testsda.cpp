//============================================================================
// Name        : testsda.cpp
// Author      : yangkun
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <signal.h>
#include <iostream>
#include "Blocks.h"
#include "DataWR.h"
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
using namespace std;
using namespace gtsda;
#ifdef MEMIN
#include "Test.h"
static TestInMem *tim ;
#else
#include "RWlogic.h"
RWlogic *wt;
#endif


void sig_handler(int signum)
{
    cout << "Receive signal. " <<  signum << endl;
    if(signum==2)
    {
    	wt->Stop();
    	sleep(1);
    	exit(-1);
    }
}

void* sigmgr_thread(void *arg)
{
    sigset_t   waitset, oset;
    int        sig;
    int        rc;
    pthread_t  ppid = pthread_self();

    pthread_detach(ppid);

    sigemptyset(&waitset);
    sigaddset(&waitset, SIGINT);

cout << "this is sigmr_thread" << endl;
    while (1)  {
    	cout <<"sigmr_thread" << endl;
        rc = sigwait(&waitset, &sig);
        if (rc != -1) {
            sig_handler(sig);
        } else {
            cout << "sigwaitinfo() returned err:  " << errno << "; "<< strerror(errno) << endl;
        }
    }
}
void daemonize(void)
{
	pid_t  pid;

	/*
	 * Become a session leader to lose controlling TTY.
	 */
	if ((pid = fork()) < 0) {
		perror("fork");
		exit(1);
	} else if (pid != 0) /* parent */
		exit(0);
	setsid();

	/*
	 * Change the current working directory to the root.
	 */
	if (chdir("/") < 0) {
		perror("chdir");
		exit(1);
	}

	/*
	 * Attach file descriptors 0, 1, and 2 to /dev/null.
	 */
	close(0);
	open("/dev/null", O_RDWR);
	dup2(0, 1);
	dup2(0, 2);
}
/********************************************************
 * 前期版本并没有考虑以后扩充问题，发布时，请考虑后期版本与前期版本
 * 兼容问题
 *
 ********************************************************/
int main() {
	cout << "Verion : " << Version << endl;
	//daemonize();
	gtopenlog("sda_write");
    sigset_t bset, oset;
    int             i;
    pid_t           pid = getpid();
    pthread_t       ppid;
    sigemptyset(&bset);
    sigaddset(&bset, SIGINT);


    if (pthread_sigmask(SIG_BLOCK, &bset, NULL) != 0)
        printf("!! Set pthread mask failed\n");
    // Create the dedicated thread sigmgr_thread() which will handle signals synchronously
    pthread_create(&ppid, NULL, sigmgr_thread, NULL);


    cout << "debug" << endl<< endl;
	wt = RWlogic::newRW();
	wt->Init();

	wt->start();

	while(1)
	{
		sleep(1);
		if(!wt->IsRun())break;
		//wt->printids("main pthread\n");
	}
	delete wt;
	pthread_join(ppid,NULL);
	cout << "bye bye " << endl;

	exit(-1);
	//return 0;
}

