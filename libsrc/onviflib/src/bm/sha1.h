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

#ifndef _SHA1_H_
#define _SHA1_H_

#ifndef uint8
#define uint8 unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

typedef struct
{
  uint32 total[2];
  uint32 state[5];
  uint8 buffer[64];
} sha1_context;

#ifdef __cplusplus
extern "C" {
#endif

void sha1_starts(sha1_context *ctx);
void sha1_update(sha1_context *ctx, uint8 *input, uint32 length);
void sha1_finish(sha1_context *ctx, uint8 digest[20]);


#ifdef __cplusplus
}
#endif

#endif

