/*
 * H264VideoServerMediaSubsession.cpp
 *
 *  Created on: 2013-12-9
 *      Author: yangkun
 */

#include "H264VideoServerMediaSubsession.h"
#include "H264VideoRTPSink.hh"
#include "H264VideoStreamFramer.hh"
#include "H264VideoStreamDiscreteFramer.hh"

H264VideoServerMediaSubsession::H264VideoServerMediaSubsession(UsageEnvironment& env,  int channel,
	    Boolean reuseFirstSource):OnDemandServerMediaSubsession(env, reuseFirstSource)
{
	// TODO Auto-generated constructor stub
	 tmp.channel = channel;
}

H264VideoServerMediaSubsession::~H264VideoServerMediaSubsession()
{
	// TODO Auto-generated destructor stub
}
H264VideoServerMediaSubsession*
H264VideoServerMediaSubsession::createNew(UsageEnvironment& env, int channel, Boolean reuseFirstSource)
{
	  return new H264VideoServerMediaSubsession(env, channel, reuseFirstSource);
}
static void afterPlayingDummy(void* clientData) {
  H264VideoServerMediaSubsession* subsess = (H264VideoServerMediaSubsession*)clientData;
  subsess->afterPlayingDummy1();
}
void H264VideoServerMediaSubsession::afterPlayingDummy1() {
  // Unschedule any pending 'checking' task:
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  // Signal the event loop that we're done:
  setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
  H264VideoServerMediaSubsession* subsess = (H264VideoServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}
void H264VideoServerMediaSubsession::checkForAuxSDPLine1() {
  char const* dasl;

  if (fAuxSDPLine != NULL) {
    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
    fAuxSDPLine = strDup(dasl);
    fDummyRTPSink = NULL;

    // Signal the event loop that we're done:
    setDoneFlag();
  } else {
    // try again after a brief delay:
    int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)checkForAuxSDPLine, this);
  }
}

char const* H264VideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
  if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

  if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
    // Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
    // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
    // and we need to start reading data from our file until this changes.
    fDummyRTPSink = rtpSink;

    // Start reading the file:
    fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

    // Check whether the sink's 'auxSDPLine()' is ready:
    checkForAuxSDPLine(this);
  }

  envir().taskScheduler().doEventLoop(&fDoneFlag);

  return fAuxSDPLine;
}
FramedSource* H264VideoServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = 500; // kbps, estimate

#if 1 //yk TODO
  tmp.type = 0;
  // Create the video source:
  GTsource* videoSource = GTsource::createNew(envir(), tmp);
  if (videoSource == NULL) return NULL;
  // TODO fFileSize = videoSource->fileSize();

  // Create a framer for the Video Elementary Stream:
  return H264VideoStreamFramer::createNew(envir(), videoSource);
  //return H264VideoStreamDiscreteFramer::createNew(envir(), videoSource);

#endif
}

RTPSink* H264VideoServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) {
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}


