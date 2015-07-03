#include <limits>
#include "onviflib.h"

#include "gtest/gtest.h"

#define MAX_DEV_NUM 20
device_info_t *g_dev_index[MAX_DEV_NUM];
video_mediainfo_t g_videomedia_info[MAX_DEV_NUM*2];
audio_mediainfo_t g_audiomedia_info[MAX_DEV_NUM*2];

void probe_cb(dev_handler_t dev_index,device_info_t* pinfo)
{
    g_dev_index[dev_index] = pinfo;
}
    
void video_cb(unsigned char* data,int len,unsigned int ts,unsigned short seq)
{
}

void audio_cb(unsigned char* data,int len)
{
}

TEST(INIT_TEST,init_call)
{
    EXPECT_EQ(0,onvif_lib_init());
}

TEST(UNINIT_TEST,uninit_call)
{
    EXPECT_EQ(0,onvif_lib_uninit());
}

TEST(PROBE_TEST,succ_probe)
{
    EXPECT_EQ(0,onvif_lib_start_probe(probe_cb,NULL));
}

TEST(PROBE_TEST,fail_probe)
{
    EXPECT_GT(0,onvif_lib_start_probe(NULL,NULL));
}

TEST(AUTH_TEST,succ_auth)
{
    //EXPECT_EQ(0,onvif_lib_set_auth(0,"admin","admin"));
}

TEST(AUTH_TEST,fail_auth)
{
    EXPECT_GT(0,onvif_lib_set_auth(10,"admin","admin"));
    EXPECT_GT(0,onvif_lib_set_auth(0,"admin","xxxxx"));
}

TEST(GETMEDIA_TEST,succ_getmedia)
{
    //EXPECT_EQ(0,onvif_lib_get_av_mediainfo(0,STREAM_MAIN,&g_videomedia_info[0],&g_audiomedia_info[0]));
    //EXPECT_EQ(0,onvif_lib_get_av_mediainfo(0,STREAM_SEC,&g_videomedia_info[1],&g_audiomedia_info[1]));
}

TEST(AUTH_TEST,fail_getmedia)
{
    EXPECT_GT(0,onvif_lib_get_av_mediainfo(0,STREAM_MAIN,NULL,&g_audiomedia_info[0]));
    EXPECT_GT(0,onvif_lib_get_av_mediainfo(0,STREAM_SEC,&g_videomedia_info[1],NULL));
}

TEST(STREAM_TEST,succ_stream_start)
{
//    EXPECT_EQ(0,onvif_lib_rtsp_start(0,STREAM_MAIN,video_cb,audio_cb,NULL));
//    EXPECT_EQ(0,onvif_lib_rtsp_start(0,STREAM_SEC,video_cb,audio_cb,NULL));
}

TEST(STREAM_TEST,fail_stream_start)
{
    EXPECT_GT(0,onvif_lib_rtsp_start(0,STREAM_MAIN,NULL,NULL,NULL,NULL));
    EXPECT_GT(0,onvif_lib_rtsp_start(0,STREAM_SEC,NULL,NULL,NULL,NULL));
}

TEST(STREAM_TEST,succ_stream_stop)
{
//  EXPECT_EQ(0,onvif_lib_rtsp_stop(0,STREAM_MAIN));
   // EXPECT_EQ(0,onvif_lib_rtsp_stop(0,STREAM_SEC));
}

// stream stop always succ
/*
TEST(STREAM_TEST,fail_stream_stop)
{
    EXPECT_GT(0,onvif_lib_rtsp_stop(10,STREAM_MAIN));
    EXPECT_GT(0,onvif_lib_rtsp_stop(10,STREAM_SEC));
}
*/


int main(int argc,char* argv[])
{
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
