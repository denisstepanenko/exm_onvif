/*
 * H264VideoServerMediaSubsession.h
 *
 *  Created on: 2013-12-9
 *      Author: yangkun
 */

#ifndef H264VIDEOSERVERMEDIASUBSESSION_H_
#define H264VIDEOSERVERMEDIASUBSESSION_H_

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif
#include "GTsource.h"
class H264VideoServerMediaSubsession:public OnDemandServerMediaSubsession
{
public:
	static H264VideoServerMediaSubsession*
	createNew(UsageEnvironment& env, int channel, Boolean reuseFirstSource);

	// Used to implement "getAuxSDPLine()":
	void checkForAuxSDPLine1();
	void afterPlayingDummy1();

protected:
	H264VideoServerMediaSubsession(UsageEnvironment& env, int channel,
			    Boolean reuseFirstSource);
      // called only by createNew();
  virtual ~H264VideoServerMediaSubsession();

  void setDoneFlag() { fDoneFlag = ~0; }

protected: // redefined virtual functions
  virtual char const* getAuxSDPLine(RTPSink* rtpSink,
				    FramedSource* inputSource);
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);

private:
  char* fAuxSDPLine;
  char fDoneFlag; // used when setting up "fAuxSDPLine"
  RTPSink* fDummyRTPSink; // ditto

  GTParameters tmp;

};

#endif /* H264VIDEOSERVERMEDIASUBSESSION_H_ */
