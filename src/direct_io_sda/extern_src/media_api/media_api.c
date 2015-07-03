#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "typedefine.h"
#include <media_api.h>
#include <pthread.h>

//int posix_memalign(void **memptr, size_t alignment, size_t size);
int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	*memptr=(void *)malloc(size*alignment);
	if(*memptr==NULL)
	{
		perror("error in malloc and it will exit\n");
		return -1;
		exit(1);
	}
	return 0;
}


/****************************************************************************/
/*******************************读写都用到的操作接口**************************/
/****************************************************************************/

/**********************************************************************************************
 * 函数名	:init_media_rw()
 * 功能	:初始化读写媒体信息用的结构
 * 输入	:type:媒体格式,音频还是视频 MEDIA_TYPE_VIDEO,MEDIA_TYPE_AUDIO
 *			 no:资源编号
 *			 buflen:要分配用于临时读写的缓冲区(media->temp_buf)的大小
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int init_media_rw(OUT media_source_t *media,IN int type,IN int no,IN int buflen)
{
	int ret;
	if(media==NULL)
	{
		return -EINVAL;
	}
	pthread_mutex_init(&media->mutex,NULL);
	media->media_type=type;
	media->no=no;
	media->attrib=NULL;
	media->thread_id=-1;
	media->dev_stat=-1;
	media->temp_buf=NULL;
	media->buflen=buflen;
	if(buflen>0)
	{
		ret=posix_memalign((void**)&media->temp_buf,sizeof(DWORD ),buflen);
		if(ret==0)
		{
			media->buflen=buflen;
		}
		else
		{
			media->buflen=0;//内存不足
			return -ENOMEM;
		}
	}
	return 0;
}

/**********************************************************************************************
 * 函数名	:get_media_attrib()
 * 功能	:获取描述媒体属性的结构指针
 * 输入	:media:描述媒体信息的结构指针
 * 返回值	:描述媒体属性结构的指针,NULL表示参数为NULL,或者还未连接
 **********************************************************************************************/
void *get_media_attrib(IN media_source_t *media)
{
	if((media==NULL))
		return NULL;
	else
		return media->attrib;
}

/****************************************************************************/
/*******************************读操作的接口**************************/
/****************************************************************************/

/**************************************************************************
  *函数名	:AttachVEncDevice
  *功能	:连接到一个已经打开的缓冲池
  *参数	:name: 创建设备的程序名
  *			 Enc:已经初始化好的描述编码器信息的结构
  *			 Pool:一个没有初始化的描述缓冲池信息的结构
  *			 bytes:希望分配的共享缓冲区大小 0表示自动选择大小
  *返回值	:0表示正常连接设备缓冲区
  *			 -EINVAL:参数错误
  *			 -EAGAIN:设备正在初始化返回
  *			 --ENODEV:设备故障
  *			-
  *注:本函数应该由从缓冲区中读取数据的程序调用
  *************************************************************************/
static int attach_media_device(int key,media_source_t *media,char *usr_name,int type)
{
	int Ret;
	MSHM_POOL *pool=NULL;
	key_t EncKey=(key_t)key;
	if((media==NULL)||(usr_name==NULL))
		return -EINVAL;
	pool=&media->mpool;
	if(key<0)
	{
//		showbug();
		return -EINVAL;
	}
	if((type!=MSHMPOOL_LOCAL_USR)&&(type!=MSHMPOOL_NET_USR))
		return -EINVAL;
	
	Ret=MShmPoolReq(pool,EncKey,usr_name,type);
	if(Ret<0)
		return Ret;
	return 0;
	
}
/**********************************************************************************************
 * 函数名	:move_media_place()
 * 功能	:将当前用户的读取位置移动place个位置
 *			
 * 输入	:media:秒数媒体的指针
 *			 place:要移动的数量,
 *				负值表示向前移动,正值表示向后移动
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int move_media_place(IO media_source_t *media,IN int place)
{
	if(media==NULL)
		return -EINVAL;
	return MShmPoolMvUsrPlace(&media->mpool,place);
}
/**********************************************************************************************
 * 函数名	:connect_media_read()
 * 功能	:连接到指定key的媒体缓冲池(读取数据的程序使用)
 * 输入	:key:要连接的媒体缓冲池的key
 *			 name:连接者的名字字符串
 *			 usr_type:连接者的用户类型 MSHMPOOL_LOCAL_USR和MSHMPOOL_NET_USR
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int connect_media_read(OUT media_source_t *media,IN int key,IN char *name,IN int usr_type)
{
	int ret=0;
	if((media==NULL)||(name==NULL))
		return -EINVAL;
	if(media->dev_stat<0)
	{			
			
			ret=attach_media_device(key,media,name,usr_type);			
			if(ret>=0)
			{
				printf("ConnectVEncDev call AttachEncDevice%d  Ret=%d:%s usrno=%d name=%s\n",media->no,ret,strerror(-ret),media->mpool.userno,name);
				pthread_mutex_lock(&media->mutex);			
				media->attrib=MShmPoolGetInfo(&media->mpool);
				media->dev_stat=0;
				pthread_mutex_unlock(&media->mutex);	
				ret=0;
			}
			
			return ret;
	}
	return ret;	
}

/**********************************************************************************************
 * 函数名	:disconnect_media_read()
 * 功能	:从已连接的媒体缓冲池断开(读取数据的程序使用)
 * 输入	:meida:描述已连接的媒体缓冲池结构指针
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功负值表示失败
 **********************************************************************************************/
int disconnect_media_read(IO media_source_t *media)
{
	int ret;
	if(media==NULL)
		return -EINVAL;
	pthread_mutex_lock(&media->mutex);	
	media->dev_stat=-1;
	media->attrib=NULL;
	pthread_mutex_unlock(&media->mutex);	
	ret=MShmPoolReqFree(&media->mpool);
	
	return ret;	
}

/**********************************************************************************************
 * 函数名	:reactive_media_usr()
 * 功能	:重新激活媒体服务(读取数据的程序使用)
 * 输入	:meida:描述已连接的媒体缓冲池结构指针
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示正常返回, 1表示已经重新激活激活 
 **********************************************************************************************/
int reactive_media_usr(IO media_source_t *media)
{
	return MShmPoolReActiveUsr(&media->mpool);
}




/****************************************************************************/
/*******************************写操作的接口**************************/
/****************************************************************************/

/**********************************************************************************************
 * 函数名	:create_media_write()
 * 功能	:创建共享缓冲池(对缓冲池进行写操作的程序调用)
 * 输入	:key:共享内存缓冲池的key
 *			 name:创建者的名字
 *			 size:准备创建的缓冲池大小(byte)
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:0表示成功,负值表示失败
 **********************************************************************************************/
int create_media_write(OUT media_source_t *media,IN int key,char *name,int size)
{
	int ret=0;
	if((name==NULL)||(key<0)||(size<=0))
		return -EINVAL;

	ret=MShmPoolCreate(name,&media->mpool,key,size);
	printf("call MShmPoolCreate(key=%x) ret=%d\n",key,ret);	
	if(ret!=0)
	{
		printf("create_media_write key:0x%x err ret=%d ",key,ret);
	}
	else
	{
		media->attrib=MShmPoolGetInfo(&media->mpool);
		media->dev_stat=0;
		media->attrib->media_type=media->media_type;
		media->attrib->stat=ENC_NO_INIT;
		ret=0;
	}
	return ret;
}

/**********************************************************************************************
 * 函数名	:set_media_attrib()
 * 功能	:设置媒体资源属性
 * 输入	:media:描述媒体信息的结构指针
 *			 attrib:要设置的属性信息缓冲区
 *			 att_len:attrib中的有效字节数
 * 输出	 media:返回时填充好相应的信息
 * 返回值	:att_len表示成功,负值表示失败
 **********************************************************************************************/
int set_media_attrib(IO media_source_t *media,IN void *attrib,IN int att_len)
{
	if((media==NULL)||(attrib==NULL)||(att_len<=0))
		return -EINVAL;
	memcpy(media->attrib,attrib,att_len);
	return att_len;
}





