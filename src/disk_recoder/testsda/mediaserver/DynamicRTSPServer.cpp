/**********
 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the
 Free Software Foundation; either version 2.1 of the License, or (at your
 option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this library; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 **********/
// Copyright (c) 1996-2013, Live Networks, Inc.  All rights reserved
// A subclass of "RTSPServer" that creates "ServerMediaSession"s on demand,
// based on whether or not the specified stream name exists as a file
// Implementation
#include "DynamicRTSPServer.hh"
#include <liveMedia.hh>
#include <string.h>

DynamicRTSPServer*
DynamicRTSPServer::createNew(UsageEnvironment& env, Port ourPort,
		UserAuthenticationDatabase* authDatabase,
		unsigned reclamationTestSeconds)
{
	int ourSocket = setUpOurSocket(env, ourPort);
	if (ourSocket == -1)
		return NULL;

	return new DynamicRTSPServer(env, ourSocket, ourPort, authDatabase,
			reclamationTestSeconds);
}

DynamicRTSPServer::DynamicRTSPServer(UsageEnvironment& env, int ourSocket,
		Port ourPort, UserAuthenticationDatabase* authDatabase,
		unsigned reclamationTestSeconds) :
		RTSPServerSupportingHTTPStreaming(env, ourSocket, ourPort, authDatabase,
				reclamationTestSeconds)
{
}

DynamicRTSPServer::~DynamicRTSPServer()
{
}

static ServerMediaSession* createNewSMS(UsageEnvironment& env,
		char const* fileName, FILE* fid); // forward
/*************************************************************************************/
#include <iostream>
#include "H264VideoServerMediaSubsession.h"
#include "PCMAudioServerMediaSubsession.h"
#include "RWlogic.h"
extern gtsda::RWlogic *rdl;
extern gtsda::RWlogic::ReadDisk *rd[5];
/*************************************************************************************/
ServerMediaSession*
DynamicRTSPServer::lookupServerMediaSession(char const* streamName)
{
/*************************************************************************************/
	//点播264的第一路,以887为开始时间，播放时长885
	//rtsp://192.168.11.21/video?type=realtime&channel=1&starttime=887&length=885
	//type = realtime&record
	//channel = 0,1,2,3
	//if(type==record) starttime=?,length=?
	int pthread_channel;
	cout << "debug str: " << streamName << endl<< endl;;
	if(strstr(streamName, "video?type="))
	{
		char *p;
		int channel=0,starttime=0,length=0;
		//查找点播的类型
		if(strstr(streamName,"video?type=realtime"))
		{
			cout << "realtime" << endl;
		}
		else if(strstr(streamName,"video?type=record"))
		{
			cout << "record" << endl;
		}
		else
		{
			cout << "no type" <<  endl;
			return NULL;
		}
		//查找通道
		if( p = strstr((char *)streamName,"channel=") )
		{
			sscanf(p, "channel=%d&%*s",&channel);
			cout << "channel= " << channel << endl;
		}
		else
		{
			cout << "no channel" << endl;
			return NULL;
		}
		channel=0;//目前只有0通道

		//播放开始时间
		if( p = strstr((char *)streamName, "starttime=") )
		{
			sscanf(p, "starttime=%d&%*s",&starttime);
			if(starttime==0)
			{
				cout << "starttime=0" << endl;
				return NULL;
			}
			cout << "starttime= " << starttime << endl;
		}
		else
		{
			cout << "no starttime" << endl;
			return NULL;
		}
		//播放时长
		if( p = strstr((char *)streamName,"length="))
		{
			sscanf(p, "length=%d&%*s",&length);
			cout << "length=" << length << endl;
		}
		else
		{
			cout << "no length" << endl;
			return NULL;
		}

		long long seek;
		if( ( seek=rdl->is_in(starttime) ) == 0)
		{
			cout << "starttime is wrong" << endl;
			return NULL;
		}
		cout << "get seek= " << seek << endl;
		int i;
		for(i=0; i<5; i++)
		{
			if(!rd[i]->isRead)
				break;
		}
		cout << "d1111" << endl;
		//rd[i]->clear();
		cout << "d1112" << endl;
		rd[i]->setread(seek );
		//rd[i]->isRead = true;
		cout << "pthread_channel: " << i << endl;
		cout << "isRead: " <<  rd[i]->isRead << endl;
		pthread_channel = i;

		cout << endl << endl;
		{

			cout << "debug 12" <<endl<<endl;;
			char  streamName1[100] = {0};
			sprintf(streamName1,"video%d",pthread_channel);
			ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName1);
			if (sms != NULL)
			{
				// "sms" was created for a file that no longer exists. Remove it:
				//removeServerMediaSession(sms);
				return sms;
			}
			//else
			{
				Boolean reuseFirstSource = False;
				char const* descriptionString
					= "Session streamed by \"testOnDemandRTSPServer\"";


				sms	= ServerMediaSession::createNew(envir(), streamName1, streamName1,
									descriptionString);
				OutPacketBuffer::maxSize = 300000;
				cout << "debug 13" <<endl;
				//视频
				sms->addSubsession(H264VideoServerMediaSubsession\
							::createNew(envir(), pthread_channel, reuseFirstSource));
				cout << "debug 14" <<endl;
				//音频
					sms->addSubsession(PCMAudioServerMediaSubsession::createNew(envir(),pthread_channel));
				addServerMediaSession(sms);
				cout << "debug 15" <<endl;
				return sms;
			}
		    //announceStream(rtspServer, sms, streamName, inputFileName);

		}
	}




	{
		cout << "debug 112" <<endl<<endl;;
		ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
		if (sms != NULL)
		{
			// "sms" was created for a file that no longer exists. Remove it:
			//removeServerMediaSession(sms);
			return sms;
		}
	}
/*************************************************************************************/
cout << "no video play" << endl;








	//以下是以前的代码
	// First, check whether the specified "streamName" exists as a local file:
	FILE* fid = fopen(streamName, "rb");
	Boolean fileExists = fid != NULL;

	// Next, check whether we already have a "ServerMediaSession" for this file:
	ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
	Boolean smsExists = sms != NULL;

	// Handle the four possibilities for "fileExists" and "smsExists":
	if (!fileExists)
	{
		if (smsExists)
		{
			// "sms" was created for a file that no longer exists. Remove it:
			removeServerMediaSession(sms);
		}
		return NULL;
	}
	else
	{
		if (!smsExists)
		{
			// Create a new "ServerMediaSession" object for streaming from the named file.
			sms = createNewSMS(envir(), streamName, fid);
			addServerMediaSession(sms);
		}
		fclose(fid);
		return sms;
	}
}

// Special code for handling Matroska files:
static char newMatroskaDemuxWatchVariable;
static MatroskaFileServerDemux* demux;
static void onMatroskaDemuxCreation(MatroskaFileServerDemux* newDemux,
		void* /*clientData*/)
{
	demux = newDemux;
	newMatroskaDemuxWatchVariable = 1;
}
// END Special code for handling Matroska files:

#define NEW_SMS(description) do {\
char const* descStr = description\
    ", streamed by the LIVE555 Media Server";\
sms = ServerMediaSession::createNew(env, fileName, fileName, descStr);\
} while(0)

static ServerMediaSession* createNewSMS(UsageEnvironment& env,
		char const* fileName, FILE* /*fid*/)
{
	// Use the file name extension to determine the type of "ServerMediaSession":
	char const* extension = strrchr(fileName, '.');
	if (extension == NULL)
		return NULL;

	ServerMediaSession* sms = NULL;
	Boolean const reuseSource = False;
	if (strcmp(extension, ".aac") == 0)
	{
		// Assumed to be an AAC Audio (ADTS format) file:
		NEW_SMS("AAC Audio");
		sms->addSubsession(
				ADTSAudioFileServerMediaSubsession::createNew(env, fileName,
						reuseSource));
	}
	else if (strcmp(extension, ".amr") == 0)
	{
		// Assumed to be an AMR Audio file:
		NEW_SMS("AMR Audio");
		sms->addSubsession(
				AMRAudioFileServerMediaSubsession::createNew(env, fileName,
						reuseSource));
	}
	else if (strcmp(extension, ".ac3") == 0)
	{
		// Assumed to be an AC-3 Audio file:
		NEW_SMS("AC-3 Audio");
		sms->addSubsession(
				AC3AudioFileServerMediaSubsession::createNew(env, fileName,
						reuseSource));
	}
	else if (strcmp(extension, ".m4e") == 0)
	{
		// Assumed to be a MPEG-4 Video Elementary Stream file:
		NEW_SMS("MPEG-4 Video");
		sms->addSubsession(
				MPEG4VideoFileServerMediaSubsession::createNew(env, fileName,
						reuseSource));
	}
	else if (strcmp(extension, ".264") == 0)
	{
		// Assumed to be a H.264 Video Elementary Stream file:
		NEW_SMS("H.264 Video");
		OutPacketBuffer::maxSize = 600000; // allow for some possibly large H.264 frames
		sms->addSubsession(
				H264VideoFileServerMediaSubsession::createNew(env, fileName,
						reuseSource));
	}
	else if (strcmp(extension, ".mp3") == 0)
	{
		// Assumed to be a MPEG-1 or 2 Audio file:
		NEW_SMS("MPEG-1 or 2 Audio");
		// To stream using 'ADUs' rather than raw MP3 frames, uncomment the following:
//#define STREAM_USING_ADUS 1
		// To also reorder ADUs before streaming, uncomment the following:
//#define INTERLEAVE_ADUS 1
		// (For more information about ADUs and interleaving,
		//  see <http://www.live555.com/rtp-mp3/>)
		Boolean useADUs = False;
		Interleaving* interleaving = NULL;
#ifdef STREAM_USING_ADUS
		useADUs = True;
#ifdef INTERLEAVE_ADUS
		unsigned char interleaveCycle[] =
		{	0,2,1,3}; // or choose your own...
		unsigned const interleaveCycleSize
		= (sizeof interleaveCycle)/(sizeof (unsigned char));
		interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
#endif
#endif
		sms->addSubsession(
				MP3AudioFileServerMediaSubsession::createNew(env, fileName,
						reuseSource, useADUs, interleaving));
	}
	else if (strcmp(extension, ".mpg") == 0)
	{
		// Assumed to be a MPEG-1 or 2 Program Stream (audio+video) file:
		NEW_SMS("MPEG-1 or 2 Program Stream");
		MPEG1or2FileServerDemux* demux = MPEG1or2FileServerDemux::createNew(env,
				fileName, reuseSource);
		sms->addSubsession(demux->newVideoServerMediaSubsession());
		sms->addSubsession(demux->newAudioServerMediaSubsession());
	}
	else if (strcmp(extension, ".vob") == 0)
	{
		// Assumed to be a VOB (MPEG-2 Program Stream, with AC-3 audio) file:
		NEW_SMS("VOB (MPEG-2 video with AC-3 audio)");
		MPEG1or2FileServerDemux* demux = MPEG1or2FileServerDemux::createNew(env,
				fileName, reuseSource);
		sms->addSubsession(demux->newVideoServerMediaSubsession());
		sms->addSubsession(demux->newAC3AudioServerMediaSubsession());
	}
	else if (strcmp(extension, ".ts") == 0)
	{
		// Assumed to be a MPEG Transport Stream file:
		// Use an index file name that's the same as the TS file name, except with ".tsx":
		unsigned indexFileNameLen = strlen(fileName) + 2; // allow for trailing "x\0"
		char* indexFileName = new char[indexFileNameLen];
		sprintf(indexFileName, "%sx", fileName);
		NEW_SMS("MPEG Transport Stream");
		sms->addSubsession(
				MPEG2TransportFileServerMediaSubsession::createNew(env,
						fileName, indexFileName, reuseSource));
		delete[] indexFileName;
	}
	else if (strcmp(extension, ".wav") == 0)
	{
		// Assumed to be a WAV Audio file:
		NEW_SMS("WAV Audio Stream");
		// To convert 16-bit PCM data to 8-bit u-law, prior to streaming,
		// change the following to True:
		Boolean convertToULaw = False;
		sms->addSubsession(
				WAVAudioFileServerMediaSubsession::createNew(env, fileName,
						reuseSource, convertToULaw));
	}
	else if (strcmp(extension, ".dv") == 0)
	{
		// Assumed to be a DV Video file
		// First, make sure that the RTPSinks' buffers will be large enough to handle the huge size of DV frames (as big as 288000).
		OutPacketBuffer::maxSize = 300000;

		NEW_SMS("DV Video");
		sms->addSubsession(
				DVVideoFileServerMediaSubsession::createNew(env, fileName,
						reuseSource));
	}
	else if (strcmp(extension, ".mkv") == 0 || strcmp(extension, ".webm") == 0)
	{
		// Assumed to be a Matroska file (note that WebM ('.webm') files are also Matroska files)
		NEW_SMS("Matroska video+audio+(optional)subtitles");

		// Create a Matroska file server demultiplexor for the specified file.  (We enter the event loop to wait for this to complete.)
		newMatroskaDemuxWatchVariable = 0;
		MatroskaFileServerDemux::createNew(env, fileName,
				onMatroskaDemuxCreation, NULL);
		env.taskScheduler().doEventLoop(&newMatroskaDemuxWatchVariable);

		ServerMediaSubsession* smss;
		while ((smss = demux->newServerMediaSubsession()) != NULL)
		{
			sms->addSubsession(smss);
		}
	}

	return sms;
}
