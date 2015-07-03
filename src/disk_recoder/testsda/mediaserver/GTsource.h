/*
 * GTsource.h
 *
 *  Created on: 2013-12-9
 *      Author: yangkun
 */

#ifndef GTSOURCE_H_
#define GTSOURCE_H_

#include "FramedSource.hh"

#include "Blocks.h"
#include "FrameQueue.h"
#include "RWlogic.h"
extern gtsda::RWlogic::ReadDisk *rd[5];
// The following class can be used to define specific encoder parameters
class GTParameters {
  //%%% TO BE WRITTEN %%%
public:
	unsigned channel;
	int type;/*0视频 1音频*/
	long long seek;
};

class GTsource: public FramedSource {
public:
  static GTsource* createNew(UsageEnvironment& env,
				 GTParameters params);

public:
  static EventTriggerId eventTriggerId;
  // Note that this is defined here to be a static class variable, because this code is intended to illustrate how to
  // encapsulate a *single* device - not a set of devices.
  // You can, however, redefine this to be a non-static member variable.

protected:
  GTsource(UsageEnvironment& env, GTParameters params);
  // called only by createNew(), or by subclass constructors
  virtual ~GTsource();

private:
  // redefined virtual functions:
  virtual void doGetNextFrame();
  //virtual void doStopGettingFrames(); // optional

private:
  static void deliverFrame0(void* clientData);
  void deliverFrame();
  gtsda::FrameQueue<gtsda::Blocks *> *video_queue;
  gtsda::FrameQueue<gtsda::Blocks *> *audio_queue;
private:
  static unsigned referenceCount; // used to count how many instances of this class currently exist
  GTParameters fParams;


  gtsda::Blocks *bs;

  u_int8_t* newFrameDataStart ;
  unsigned newFrameSize ;
  bool is_over;
  unsigned left_value;
  unsigned ftime;
  unsigned long long fDurationtime;


  timeval oldtime,newtime;
  /*0为视频 1为音频*/
  void Computing_time(int type);

  //处理视频时间戳,复制视频等
  void copyvideo();
};

#endif
