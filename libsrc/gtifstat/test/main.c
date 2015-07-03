#include<stdio.h>




int main(void)
{
        int i=0;
        unsigned long snd_pkts;

	printf("¿ªÊ¼²âÊÔ...\n");
        init_gtifstat();        
        for(i=0;i<40;i++)
	while(1)
	{
		snd_pkts=get_send_pkts();
		printf("zw-test ---->pkts_send=%ld\n",snd_pkts);
		sleep(1);
	}
	close_gtifstat();
                                                                                                
}
                                                                                             
