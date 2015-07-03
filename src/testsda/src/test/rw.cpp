/*
 * rw.cpp
 *
 *  Created on: 2013-11-7
 *      Author: yangkun
 */
#include "Blocks.h"
#include "DataWR.h"
using namespace gtsda;
int main(int argc, char **argv)
{
	unsigned int channel=0;
	if(argc==2)
	{
		channel = atoi(argv[1]);
		cout << "channel is: " << channel << endl;
		if(channel >15 || channel <0)channel=0;
	}
	else
		cout << "usage: "<< argv[0]<<" channel[0-8]<< endl;";

	cout << "channel is: " << channel << endl;
	gtopenlog("rw");
	DataRead dr(0,block,channel);
	DataWrite dw;

	cout << "runing .."<<endl;
	while(1)
	{
		//ttt();
		dr.readpool();
		dw=dr;
		dw.writedata();
	}
	cout << "out .."<< endl;
}

