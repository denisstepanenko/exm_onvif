/*
**************************************************************************
domain_socket.h
@Author: duanjigang @2006-4-11
@Desp: declaratin of methods used for unix-domain-socket communication 
**************************************************************************
*/
#ifndef _H_
#define _H_
#include <stdio.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#define MSG_SIZE 1024
int init_send_socket(struct sockaddr_un * addr,char * path)
{
        int sockfd; 	//,len;
        sockfd=socket(AF_UNIX,SOCK_DGRAM,0);
        if(sockfd<0)
        {
                return sockfd;	// lsk return -1 -> return sockfd
        }
        bzero(addr,sizeof(struct sockaddr_un));
        addr->sun_family=AF_UNIX;
        strcpy(addr->sun_path,path);
        return sockfd;
}
int init_recv_socket(char * path)
{
        int sockfd,len; 
        struct sockaddr_un addr; 
        sockfd=socket(AF_UNIX,SOCK_DGRAM,0); 
        if(sockfd<0) 
        { 
           return sockfd;		// lsk return -1 -> return sockfd
        } 
        bzero(&addr,sizeof(struct sockaddr_un)); 
        addr.sun_family = AF_UNIX; 
        strcpy(addr.sun_path, path);
        unlink(path);
        len = strlen(addr.sun_path) + sizeof(addr.sun_family);
        if(bind(sockfd,(struct sockaddr *)&addr,len)<0) 
         { 
	          return -1;
         } 
        return sockfd;

}

int receive_from_socket(int sockfd, char* msg , int Lenth, const struct sockaddr_un * addr)
{
	  int n , len; 
	  memset(msg, 0, Lenth);
	  len = strlen(addr->sun_path)+sizeof(addr->sun_family);
	  n=recvfrom(sockfd, msg, Lenth, 0, (struct sockaddr*)addr, &len); 
	  if(n<=0)
	  {
	   return -1;
	  }
//	  msg[n]=0;   // lsk 2006-6-15
	  return n;
}

int send_to_socket(int sockfd, char* msg, int Lenth, const struct sockaddr_un * addr)
{
        int len; 
        len = strlen(addr->sun_path)+sizeof(addr->sun_family);
        sendto(sockfd, msg, Lenth, 0, (struct sockaddr*)addr, len); 
        return 0;	// lsk return 1 -> return 0
}
#endif

