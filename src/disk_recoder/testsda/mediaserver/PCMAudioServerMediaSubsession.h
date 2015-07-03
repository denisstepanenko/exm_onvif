/*
 * PCMAudioServerMediaSubsession.h
 *
 *  Created on: 2013-12-13
 *      Author: yangkun
 */

#ifndef PCMAUDIOSERVERMEDIASUBSESSION_H_
#define PCMAUDIOSERVERMEDIASUBSESSION_H_
#include <OnDemandServerMediaSubsession.hh>
#include "GTsource.h"
class PCMAudioServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
	static PCMAudioServerMediaSubsession*
	createNew(UsageEnvironment& env, int channel);

protected:
	PCMAudioServerMediaSubsession(UsageEnvironment& env, int channel);
	virtual ~PCMAudioServerMediaSubsession();
private: // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);

private:
  unsigned fSamplingFrequency;
  unsigned fNumChannels;
	GTParameters tmp;
};

#endif /* PCMAUDIOSERVERMEDIASUBSESSION_H_ */
