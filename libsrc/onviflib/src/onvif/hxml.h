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

#ifndef	__H_LT_XML_H__
#define	__H_LT_XML_H__

#define LTXML_MAX_STACK_DEPTH	1024
#define LTXML_MAX_ATTR_NUM	128

typedef struct ltxd_xmlparser 
{
	char *	xmlstart;
	char *	xmlend;
	char *	ptr;		// pointer to current character
	int		xmlsize;

	char *	e_stack[LTXML_MAX_STACK_DEPTH];
	int		e_stack_index;					

	char *	attr[LTXML_MAX_ATTR_NUM];

	void *	userdata;
	void (*startElement)(void * userdata, const char * name, const char ** attr);
	void (*endElement)(void * userdata, const char * name);
	void (*charData)(void * userdata, const char * str, int len);
}LTXMLPRS;

#ifdef __cplusplus
extern "C" {
#endif

int hxml_parse_header(LTXMLPRS * parse);

int hxml_parse_attr(LTXMLPRS * parse);
int hxml_parse_element_end(LTXMLPRS * parse);
int hxml_parse_element_start(LTXMLPRS * parse);

int hxml_parse_element(LTXMLPRS * parse);

int hxml_parse(LTXMLPRS * parse);

void xml_startElement(void * userdata, const char * name, const char ** attr);
void xml_endElement(void * userdata, const char * name);
void xml_charData(void * userdata, const char * str, int len);

void hxml_test();

#ifdef __cplusplus
}
#endif

#endif	//	__H_LT_XML_H__
