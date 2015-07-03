/*
 * FrameQueue.h
 *
 *  Created on: 2013-11-14
 *      Author: yangkun
 */

#ifndef FRAMEQUEUE_H_
#define FRAMEQUEUE_H_
#include <pthread.h>
#include <queue>
#include "Blocks.h"
using std::queue;
namespace gtsda
{

template <typename T>
class FrameQueue
{
public:
	FrameQueue();
	void clear();
	void push(T);
	//type=0 «£…÷°, type=1 «£–÷°
	void push(char *buf, unsigned int size, int type=1);
	unsigned int size(){return frame.size();};
	T pop();
	virtual ~FrameQueue();
private:

	queue<T> frame;
	pthread_mutex_t counter_mutex;
};







template <typename T>
FrameQueue<T>::FrameQueue()
{
	 counter_mutex = PTHREAD_MUTEX_INITIALIZER;
}

template <typename T>
FrameQueue<T>::~FrameQueue()
{
	clear();
}

template <typename T>
void FrameQueue<T>::push(T bs)
{
	pthread_mutex_lock(&counter_mutex);
	frame.push(bs);
	pthread_mutex_unlock(&counter_mutex);
}


template <typename T>
void FrameQueue<T>::push(char *buf, unsigned int size, int type)
{
	Blocks *bs;
	if(type==0)
		bs = new Blocks(size,0,noblock);
	else
		bs = new Blocks(size);
	memcpy((char *)bs->GetBuf(), buf, size);
	push(bs);
}
template <typename T>
T FrameQueue<T>::pop()
{
	pthread_mutex_lock(&counter_mutex);
	T bs = frame.front();
	frame.pop();
	pthread_mutex_unlock(&counter_mutex);
	return bs;
}

template <typename T>
void FrameQueue<T>::clear()
{
	while(frame.size()>0)
	{
		delete pop();
	};
}


} /* namespace gtsda */
#endif /* FRAMEQUEUE_H_ */
