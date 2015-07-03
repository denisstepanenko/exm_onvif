#include "blocks.h"
#define BUFFER_SIZE 400*1024 //最大4Mbit，但个帧应该不会超过100K
static char frame_buffer[BUFFER_SIZE];
//./sg_dd if=sda.img of=/dev/sda oflag=sgio seek=1m bs=1024 count=1


#if 1
#include "pthread.h"

pthread_t ntid;
void *aa()
{
	while(1)
	{
		sleep(1);
		printf("ok\n\n\n");
	}
}

int
main(int argc, char * argv[])
{
	int ret;
	ret = pthread_create(&ntid, NULL, aa, "new thread: ");
	if (ret != 0) {
		fprintf(stderr, "can't create thread: %s\n", strerror(ret));
		exit(1);
	}
//sleep(10);


	   init_sda();


		/*初始化年块，是否要格式化*/
		if(0!=year_init())
		{
			printf("year init error\n");
			exit(1);
		}
		DP("year over \n\n\n\n\n");



		/*初始化天*/
		if( 0!=day_init())
		{
			printf("day init error\n");
			exit(1);
		}

		DP("day over \n\n\n\n");



		sec_write_data();




		free_sda();
}

#else

int main()
{
	struct hd_frame
	{
		char data_head[8];			/*帧数据头的标志位 0x5345434fa55a5aa5*/
		short frontIframe;			/*前一个I帧相对于本帧的偏移地址   如果前一帧在硬盘的最后面，这个值可能是负值*/
		short is_I;		/*本帧是不是I帧*/
		unsigned int size;			/*本帧视频数据块的大小*/
	};
	printf("::size:%d",sizeof(struct hd_frame));
}


#endif
