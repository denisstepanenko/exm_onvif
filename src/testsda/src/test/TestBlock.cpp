/*
 * TestBlock.cpp
 *
 *  Created on: 2013-10-24
 *      Author: yangkun
 */
#include "Test.h"
#include "../Blocks.h"
using namespace gtsda;

int TestBlocks()
{


	Blocks b(10,20,audio);
	cout <<"block size:" << b.GetSize()<< endl;
	cout <<"b:"<< b << endl;

	char p[]="afasdfaf";
	Blocks c(p,sizeof(p),50);
	cout << "c:" << c << endl;

	//Blocks d=c; // d(c) //不可复制对象
	cout << "bye" << endl;
	return 0;
}

