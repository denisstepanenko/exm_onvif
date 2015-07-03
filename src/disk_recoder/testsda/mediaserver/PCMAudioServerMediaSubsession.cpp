/*
 * PCMAudioServerMediaSubsession.cpp
 *
 *  Created on: 2013-12-13
 *      Author: yangkun
 */

#include "PCMAudioServerMediaSubsession.h"
#include "SimpleRTPSink.hh"
//#include "AudioSource.h"
PCMAudioServerMediaSubsession*
PCMAudioServerMediaSubsession::createNew(UsageEnvironment& env, int channel)
{
	  return new PCMAudioServerMediaSubsession(env,channel);
}
PCMAudioServerMediaSubsession::PCMAudioServerMediaSubsession(UsageEnvironment& env, int channel)
:OnDemandServerMediaSubsession(env, True /*reuse the first source*/)
{
	tmp.channel = channel;
	// TODO Auto-generated constructor stub

}

PCMAudioServerMediaSubsession::~PCMAudioServerMediaSubsession()
{
	// TODO Auto-generated destructor stub
}
FramedSource* PCMAudioServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate)
{
	//“Ù∆µ
	tmp.type = 1;
	return GTsource::createNew(envir(), tmp);
}
RTPSink* PCMAudioServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                  unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource)
{
	unsigned audioSamplingFrequency = 8000;
	unsigned audioNumChannels = 1;
	return SimpleRTPSink::createNew(envir(), rtpGroupsock, /*96*/0,
						 audioSamplingFrequency, "audio", "PCMU", audioNumChannels);
}
