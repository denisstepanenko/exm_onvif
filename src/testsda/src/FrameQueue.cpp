/*
 * FrameQueue.cpp
 *
 *  Created on: 2013-11-14
 *      Author: yangkun
 */

#include "FrameQueue.h"

namespace gtsda
{

FrameQueue::FrameQueue()
{
	 counter_mutex = PTHREAD_MUTEX_INITIALIZER;
}

FrameQueue::~FrameQueue()
{
	do
	{
		pthread_mutex_lock(&counter_mutex);
		delete pop();
		pthread_mutex_unlock(&counter_mutex);
	}while(frame.size()>0);
}

void FrameQueue::push(Blocks *bs)
{
	pthread_mutex_lock(&counter_mutex);
	frame.push(bs);
	pthread_mutex_unlock(&counter_mutex);
}

void FrameQueue::push(char *buf, unsigned int size)
{
	Blocks *bs = new Blocks(size);
	memcpy((char *)bs->GetBuf(), buf, size);
	push(bs);
}
Blocks *FrameQueue::pop()
{
	pthread_mutex_lock(&counter_mutex);
	Blocks *bs = frame.front();
	frame.pop();
	pthread_mutex_unlock(&counter_mutex);
	return bs;
}
} /* namespace gtsda */
