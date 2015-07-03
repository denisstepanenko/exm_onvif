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
//»º´æ10Ö¡£¬
class FrameQueue
{
public:
	FrameQueue();
	void push(Blocks *bs);
	void push(char *buf, unsigned int size);
	unsigned int size(){return frame.size();};
	Blocks *pop();
	virtual ~FrameQueue();
private:

	queue< Blocks *> frame;
	pthread_mutex_t counter_mutex;
};

} /* namespace gtsda */
#endif /* FRAMEQUEUE_H_ */
