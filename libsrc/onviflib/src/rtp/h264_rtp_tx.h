/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2010-2014, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#ifndef	__H_H264_RTP_TX_H__
#define	__H_H264_RTP_TX_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "h264_def.h"


unsigned char * h264_split_nalu_tx(unsigned char * e_buf, int e_len, int * d_len);

int h264_rtp_build_tcp(int vfd, UA_RTP_INFO * p_uari, unsigned char * p_data, int len, unsigned char v_chan);

int h264_rtp_build(int vfd, UA_RTP_INFO * p_uari, unsigned char * p_data, int len, 
		unsigned int rip, unsigned short rport, int is_tcp, unsigned char v_chan);

int h264_fu_rtp_build(int vfd, UA_RTP_INFO * p_uari, unsigned char * p_data, int len, 
		unsigned int rip, unsigned short rport, int is_tcp, unsigned char v_chan);

int v_rtp_tx(int vfd, unsigned int rip, unsigned short rport, char * p_rtp, int len);

int encode_sps_pps_base64(H264SPS * p_avcc);
BOOL decode_sps_pps_base64(char * p_sps_str, H264SPS * p_avcc);

int h264_tx_avcc(H264SPS * p_avcc, unsigned char * p_data, int len);
int sps_pps_decodec(H264SPS * p_avcc);
void print_avcc(H264SPS * p_avcc);

#ifdef __cplusplus
}
#endif

#endif	//	__H_H264_RTP_TX_H__

