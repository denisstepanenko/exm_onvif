/*
 * GTsource.cpp
 *
 *  Created on: 2013-12-9
 *      Author: yangkun
 */

#include "GTsource.h"
#include "InputFile.hh"
#include "GroupsockHelper.hh"

#include "FramedSource.hh"
GTsource*
GTsource::createNew(UsageEnvironment& env,
			GTParameters params) {
  return new GTsource(env, params);
}

EventTriggerId GTsource::eventTriggerId = 0;
unsigned GTsource::referenceCount = 0;



GTsource::GTsource(UsageEnvironment& env,
			   GTParameters params)
  : FramedSource(env), fParams(params),is_over(true),bs(NULL){
	is_over = true;
  if (referenceCount == 0) {
    // Any global initialization of the device would be done here:
    //%%% TO BE WRITTEN %%%

  }
  ++referenceCount;
  // Any instance-specific initialization of the device would be done here:
  //%%% TO BE WRITTEN %%%

  // We arrange here for our "deliverFrame" member function to be called
  // whenever the next frame of data becomes available from the device.
  //
  // If the device can be accessed as a readable socket, then one easy way to do this is using a call to
  //     envir().taskScheduler().turnOnBackgroundReadHandling( ... )
  // (See examples of this call in the "liveMedia" directory.)
  //
  // If, however, the device *cannot* be accessed as a readable socket, then instead we can implement it using 'event triggers':
  // Create an 'event trigger' for this device (if it hasn't already been done):
//  if (eventTriggerId == 0) {
//    eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
//  }
	//cout << "debug !!!!gtsource"<< endl;
	if(fParams.type == 0)
	{
		cout << "video channel:" << fParams.channel << endl;
		rd[fParams.channel]->isRead = true;
	}
	cout << "audio " << endl;
}

GTsource::~GTsource() {
  // Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:
  //%%% TO BE WRITTEN %%%


  --referenceCount;
  //cout << "debug !!! ~GTsource channel :"<<fParams.channel << " isRead : " << rd[fParams.channel]->isRead<< endl;
	//视频
if(fParams.type == 0)
{
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~video" << endl;
	rd[fParams.channel]->clear();
	rd[fParams.channel]->isRead = false;
}
else
{
	cout << "~audio " << endl;
}

//  if (referenceCount == 0) {
    // Any global 'destruction' (i.e., resetting) of the device would be done here:
    //%%% TO BE WRITTEN %%%

    // Reclaim our 'event trigger'
//    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
//    eventTriggerId = 0;
//  }

}

void GTsource::doGetNextFrame() {
  // This function is called (by our 'downstream' object) when it asks for new data.

  // Note: If, for some reason, the source device stops being readable (e.g., it gets closed), then you do the following:
  if (0 /* the source stops being readable */ /*%%% TO BE WRITTEN %%%*/) {
    handleClosure(this);
    return;
  }

  // If a new frame of data is immediately available to be delivered, then do this now:
//  if (0 /* a new frame of data is immediately available to be delivered*/ /*%%% TO BE WRITTEN %%%*/) {
    deliverFrame();
//  }

  // No new data is immediately available to be delivered.  We don't do anything more here.
  // Instead, our event trigger must be called (e.g., from a separate thread) when new data becomes available.
}

void GTsource::deliverFrame0(void* clientData) {
  ((GTsource*)clientData)->deliverFrame();
}

void GTsource::deliverFrame() {
	// This function is called when new frame data is available from the device.
	// We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
	// 'in' parameters (these should *not* be modified by this function):
	//     fTo: The frame data is copied to this address.
	//         (Note that the variable "fTo" is *not* modified.  Instead,
	//          the frame data is copied to the address pointed to by "fTo".)
	//     fMaxSize: This is the maximum number of bytes that can be copied
	//         (If the actual frame is larger than this, then it should
	//          be truncated, and "fNumTruncatedBytes" set accordingly.)
	// 'out' parameters (these are modified by this function):
	//     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
	//     fNumTruncatedBytes: Should be set iff the delivered frame would have been
	//         bigger than "fMaxSize", in which case it's set to the number of bytes
	//         that have been omitted.
	//     fPresentationTime: Should be set to the frame's presentation time
	//         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
	//         by calling "gettimeofday()".
	//     fDurationInMicroseconds: Should be set to the frame's duration, if known.
	//         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
	//         to set this variable, because - in this case - data will never arrive 'early'.
	// Note the code below.
//cout << "debug 1" << endl;
//cout <<"channel : " <<  fParams.channel << endl;
	if (!isCurrentlyAwaitingData()) return; // we're not ready for the data yet


	int size;
	//视频
	if(fParams.type == 0)
	{
		video_queue = rd[fParams.channel]->get_video();
		//cout << "video size: " << video_queue->size() << endl;
		while(video_queue->size()<10)
		{
			//cout << "no enough video data!,size: " << video_queue->size()<< endl;
			rd[fParams.channel]->set_notful();
		}

#if 1
		copyvideo();
#else
		bs= video_queue->pop();
		size = bs->GetSize();
		memmove(fTo, bs->GetBuf(), size<fMaxSize?size:fMaxSize);
		if(size>fMaxSize)
		{
			fNumTruncatedBytes = size-fMaxSize ;
			fFrameSize = fMaxSize;
		}
		else
		{
			fFrameSize = size;
		}

		gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
		fDurationInMicroseconds = 40000;
		delete bs;
#endif
	}
	//音频
	else
	{
		audio_queue = rd[fParams.channel]->get_audio();
		//cout << "audio size:  " << audio_queue->size() << endl;

		gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
		fDurationInMicroseconds = 20000;


		while(audio_queue->size()<=10)
		{
			cout << "no audio data!!!" << endl;
			//rd[fParams.channel]->set_notful();
			goto no_data;
		}
		bs= audio_queue->pop();
		size = bs->GetSize();
		memmove(fTo, bs->GetBuf(), size<fMaxSize?size:fMaxSize);
		if(size>fMaxSize)
		{
			fNumTruncatedBytes = size-fMaxSize ;
			fFrameSize = fMaxSize;
		}
		else
		{
			fFrameSize = size;
		}
		delete bs;
	}





//cout << "debug 2" << endl;
	// After delivering the data, inform the reader that it is now available:
no_data:
	FramedSource::afterGetting(this);
}


// The following code would be called to signal that a new frame of data has become available.
// This (unlike other "LIVE555 Streaming Media" library code) may be called from a separate thread.
// (Note, however, that "triggerEvent()" cannot be called with the same 'event trigger id' from different threads.
// Also, if you want to have multiple device threads, each one using a different 'event trigger id', then you will need
// to make "eventTriggerId" a non-static member variable of "GTsource".)
void signalNewFrameData() {
  TaskScheduler* ourScheduler = NULL; //%%% TO BE WRITTEN %%%
  GTsource* ourDevice  = NULL; //%%% TO BE WRITTEN %%%

  if (ourScheduler != NULL) { // sanity check
    ourScheduler->triggerEvent(GTsource::eventTriggerId, ourDevice);
  }
}
/*0为视频 1为音频*/
void GTsource::Computing_time(int type)
{
	if( 0 == fPresentationTime.tv_sec&&0==fPresentationTime.tv_usec)
	{
		std::cout << "debug1!!" << std::endl;
		gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
		memcpy(&oldtime, &fPresentationTime, sizeof(fPresentationTime));
		fDurationtime = fPresentationTime.tv_usec + fPresentationTime.tv_sec*1000000;
		if(type==0)
			fDurationInMicroseconds = 40000;
		else
			fDurationInMicroseconds = 20000;
	}
	else
	{
		//视频
		if(type==0)
		{
			fDurationInMicroseconds = 40000 ;
			fDurationtime +=  fDurationInMicroseconds;
			fPresentationTime.tv_sec  = fDurationtime / 1000000;
			fPresentationTime.tv_usec = fDurationtime % 1000000;
		}
		//音频
		else
		{
			fDurationInMicroseconds = 20000;
			fDurationtime +=  fDurationInMicroseconds;
			fPresentationTime.tv_sec  = fDurationtime / 1000000;
			fPresentationTime.tv_usec = fDurationtime % 1000000;
		}

	}
}
//处理视频时间戳,复制视频等
void GTsource::copyvideo()
{
	//上次没读完
	if(!is_over)
	{
		is_over = true;
		if(left_value>fMaxSize)
		{
			memmove(fTo, newFrameDataStart + fFrameSize, fMaxSize);
			fDurationInMicroseconds = 40000*((float)fMaxSize/newFrameSize);
			gettimeofday(&fPresentationTime, NULL);
			fFrameSize += (left_value-fMaxSize);
			left_value -= fMaxSize;
			is_over = false;
			cout << "this is impossibility!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl << endl;
		}
		else
		{
			fDurationInMicroseconds = 40000 - ftime;
			//cout << "debug fduration2: " << fDurationInMicroseconds << endl;
			gettimeofday(&fPresentationTime, NULL);
			memmove(fTo, newFrameDataStart + fFrameSize, left_value);
			fFrameSize = left_value;
			//cout << "debug11111"<< endl;
		}
	}
	//上次读完了
	else
	{
		//cout << "debug3333"<< endl;
		if(bs)
		{
		//	cout << "debug4444"<< endl;
			delete bs;
			bs =NULL;
		}
		bs= video_queue->pop();
		newFrameDataStart = (u_int8_t*)( bs->GetBuf()); //%%% TO BE WRITTEN %%%
		newFrameSize =  bs->GetSize(); //%%% TO BE WRITTEN %%%

		// Deliver the data here:
		//buf存不完，要剩下了
		if (newFrameSize > fMaxSize)
		{
			is_over = false;
			left_value = newFrameSize - fMaxSize;
			fFrameSize = fMaxSize;
			fNumTruncatedBytes = newFrameSize - fMaxSize;
			ftime=fDurationInMicroseconds = 40000*((float)fMaxSize/newFrameSize);
			//cout << "debug fduration1: " << fDurationInMicroseconds << endl;
			memmove(fTo, newFrameDataStart, fMaxSize);
		}
		//buf足够多，不会剩下
		else
		{
			fDurationInMicroseconds = 40000;
			is_over = true;
			fFrameSize = newFrameSize;
			memmove(fTo, newFrameDataStart, fFrameSize);
		}

		if( 0 == fPresentationTime.tv_sec&&0==fPresentationTime.tv_usec)
		{
			std::cout << "debug1!!" << std::endl;
			gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
			fDurationtime = fPresentationTime.tv_usec + fPresentationTime.tv_sec*1000000;
			fDurationInMicroseconds = 40000;
		}
		else
		{
			gettimeofday(&fPresentationTime, NULL);
		}
	}


}
