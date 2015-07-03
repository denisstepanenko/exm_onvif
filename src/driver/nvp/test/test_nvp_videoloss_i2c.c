#include "gpio_i2c.h"

int main()
{
	int fd,ret;
	if((fd=open("/dev/gpioi2c",0))<0)
			perror("open");

	unsigned int value=0;
	unsigned short reg_val=0x01;
	value |= (0x60)<<24;//i2cµØÖ·
	value |= (0xff)<<16;//nvp¼Ä´æÆ÷µØÖ·
	value |= reg_val;//nvp¼Ä´æÆ÷ÒªÐ´µÄÖµ
	ret = ioctl(fd, GPIO_I2C_WRITE, &value);
	if(ret<0)
		perror("write error\n");
	printf("value:%#X\n",value);

	value =0;
	value |= (0x60)<<24;
	value |= (0xd8)<<16;
	ret = ioctl(fd, GPIO_I2C_READ, &value);
	if(ret<0)
		perror("read error\n");
	reg_val=0;
	reg_val = value&0xf;
	printf("value:%#X\n",reg_val);


}
