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

#ifndef	__H_WORD_ANALYSE_H__
#define	__H_WORD_ANALYSE_H__

/***************************************************************************************/
typedef enum word_type
{
	WORD_TYPE_NULL = 0,
	WORD_TYPE_STRING,
	WORD_TYPE_NUM,
	WORD_TYPE_SEPARATOR
}WORD_TYPE;


#ifdef __cplusplus
extern "C" {
#endif


int  dq_string_get(char *word_buf,unsigned int word_buf_len, char *input_buf,unsigned int *offset);

BOOL is_char(char ch);
BOOL is_num(char ch);
BOOL is_separator(char ch);
BOOL is_ip_address(const char * address);
BOOL is_integer(char * p_str);

BOOL GetLineText(char *buf,int cur_line_offset,int max_len, int * len,int * next_line_offset);
BOOL GetSipLine(char *p_buf, int max_len, int * len, BOOL * bHaveNextLine);
BOOL GetLineWord(char *line, int cur_word_offset, int line_max_len, char *word_buf, int buf_len, int *next_word_offset, WORD_TYPE w_t);
BOOL GetNameValuePair(char * text_buf, int text_len, const char * name, char *value, int value_len);

#ifdef __cplusplus
}
#endif

#endif	//	__H_WORD_ANALYSE_H__



