/** @file	       net_avstream.h
 *   @brief 	接收并处理客户端发来的音视频上行命令
 *   @date 	2007.03
 */
#ifndef NET_AVSTREAM_H_20070306
#define NET_AVSTREAM_H_20070306

int init_net_avstream(void);
int create_rtnet_av_servers(void);
/** 
 *   @brief     返回音视频上行服务的响应信息
 *   @param  fd 目的描述符
 *   @param  result 返回的错误代码
 *   @param  需要挂在返回信息answer_data后面的信息
 *   @param  datalen buf中有效数据的个数
 *   @return   负值表示出错,非负表示成功
 */ 
int send_rtstream_ack_pkt(int fd,WORD result,char* buf,int datalen);
#endif
