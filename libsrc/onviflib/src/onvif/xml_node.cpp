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

#include "sys_inc.h"
#include "hxml.h"
#include "xml_node.h"

/***************************************************************************************
 *
 * XML operational functions
 *
***************************************************************************************/
XMLN * xml_node_add(XMLN * parent, char * name)
{
	XMLN * p_node = (XMLN *)malloc(sizeof(XMLN));
	if (p_node == NULL)
	{
		log_print("xml_node_add::memory alloc fail!!!\r\n");
		return NULL;
	}

	memset(p_node, 0, sizeof(XMLN));

	p_node->type = NTYPE_TAG;
	p_node->name = name;	//strdup(name);

	if (parent != NULL)
	{
		p_node->parent = parent;
		
		if (parent->f_child == NULL)
		{
			parent->f_child = p_node;
			parent->l_child = p_node;
		}
		else
		{
			parent->l_child->next = p_node;
			p_node->prev = parent->l_child;
			parent->l_child = p_node;
		}
	}

	return p_node;
}

XMLN * xml_node_add_by_struct_at_last(XMLN * parent, XMLN * pdest)
{
	if ((parent == NULL)||(pdest == NULL))
	{
		log_print("xml_node_add_by_struct_at_last:: parent or pdest is NULL\r\n");
		return NULL;
	}
	
	pdest->parent = parent;
	
	if (parent->f_child == NULL)
	{
		parent->f_child = pdest;
		parent->l_child = pdest;
	}
	else
	{
		parent->l_child->next = pdest;
		pdest->prev = parent->l_child;
		parent->l_child = pdest;
	}
	
	return pdest;
}

void xml_node_del(XMLN * p_node)
{
    XMLN * p_attr;
    XMLN * p_child;
    
	if (p_node == NULL) return;

	p_attr = p_node->f_attrib;
	while (p_attr)
	{
		XMLN * p_next = p_attr->next;
	//	if(p_attr->data) free(p_attr->data);
	//	if(p_attr->name) free(p_attr->name);

		free(p_attr);

		p_attr = p_next;
	}

	p_child = p_node->f_child;
	while (p_child)
	{
		XMLN * p_next = p_child->next;
		xml_node_del(p_child);
		p_child = p_next;
	}

	if (p_node->prev) p_node->prev->next = p_node->next;
	if (p_node->next) p_node->next->prev = p_node->prev;

	if (p_node->parent)
	{
		if (p_node->parent->f_child == p_node)
			p_node->parent->f_child = p_node->next;
		if (p_node->parent->l_child == p_node)
			p_node->parent->l_child = p_node->prev;
	}

//	if(p_node->name) free(p_node->name);
//	if(p_node->data) free(p_node->data);

	free(p_node);
}

XMLN * xml_node_get(XMLN * parent, const char * name)
{
    XMLN * p_node;
    
	if (parent == NULL || name == NULL)
		return NULL;

	p_node = parent->f_child;
	while (p_node != NULL)
	{
		if (strcasecmp(p_node->name, name) == 0)
			return p_node;

		p_node = p_node->next;
	}

	return NULL;
}

int soap_strcmp(const char * str1, const char * str2)
{
    const char * ptr1;
    const char * ptr2;
    
	if (strcasecmp(str1, str2) == 0)
		return 0;

	ptr1 = strchr(str1, ':');
	ptr2 = strchr(str2, ':');
	
	if (ptr1 && ptr2)
		return strcasecmp(ptr1+1, ptr2+1);
	else if (ptr1)
		return strcasecmp(ptr1+1, str2);
	else if (ptr2)
		return strcasecmp(str1, ptr2+1);
	else
		return -1;
}

XMLN * xml_node_soap_get(XMLN * parent, const char * name)
{
    XMLN * p_node;
    
	if (parent == NULL || name == NULL)
		return NULL;

	p_node = parent->f_child;
	while (p_node != NULL)
	{
		if (soap_strcmp(p_node->name, name) == 0)
			return p_node;

		p_node = p_node->next;
	}

	return NULL;
}

/***************************************************************************************/
XMLN * xml_attr_add(XMLN * p_node, const char * name, const char * value)
{
    XMLN * p_attr;
    
	if (p_node == NULL || name == NULL || value == NULL)
		return NULL;

	p_attr = (XMLN *)malloc(sizeof(XMLN));
	if (p_attr == NULL)
	{
		log_print("xml_attr_add::memory alloc fail!!!\r\n");
		return NULL;
	}

	memset(p_attr, 0, sizeof(XMLN));

	p_attr->type = NTYPE_ATTRIB;
	p_attr->name = name;	//strdup(name);
	p_attr->data = value;	//strdup(value);
	p_attr->dlen = strlen(value);

	if (p_node->f_attrib == NULL)
	{
		p_node->f_attrib = p_attr;
		p_node->l_attrib = p_attr;
	}
	else
	{
		p_attr->prev = p_node->l_attrib;
		p_node->l_attrib->next = p_attr;
		p_node->l_attrib = p_attr;
	}

	return p_attr;
}

void xml_attr_del(XMLN * p_node, const char * name)
{
    XMLN * p_attr;
    
	if (p_node == NULL || name == NULL)
		return;

	p_attr = p_node->f_attrib;
	while (p_attr != NULL)
	{
		if (strcasecmp(p_attr->name, name) == 0)
		{
			xml_node_del(p_attr);
			return;
        }

		p_attr = p_attr->next;
	}
}

const char * xml_attr_get(XMLN * p_node, const char * name)
{
    XMLN * p_attr;
    
	if (p_node == NULL || name == NULL)
		return NULL;

	p_attr = p_node->f_attrib;
	while (p_attr != NULL)
	{
		if ((NTYPE_ATTRIB == p_attr->type) && (0 == strcasecmp(p_attr->name, name)))
			return p_attr->data;

		p_attr = p_attr->next;
	}

	return NULL;
}

XMLN * xml_attr_node_get(XMLN * p_node, const char * name)
{
    XMLN * p_attr;
    
	if (p_node == NULL || name == NULL)
		return NULL;

	p_attr = p_node->f_attrib;
	while (p_attr != NULL)
	{
		if ((NTYPE_ATTRIB == p_attr->type) && (0 == strcasecmp(p_attr->name, name)))
			return p_attr;

		p_attr = p_attr->next;
	}

	return NULL;
}

/***************************************************************************************/
void xml_cdata_set(XMLN * p_node, const char * value, int len)
{
	if (p_node == NULL || value == NULL || len <= 0)
		return;

	p_node->data = value;
	p_node->dlen = len;
}

/***************************************************************************************/
int xml_calc_buf_len(XMLN * p_node)
{
	int xml_len = 0;
	XMLN * p_attr;
	
	xml_len += 1 + strlen(p_node->name);	//sprintf(xml_buf+xml_len, "<%s",p_node->name);

	p_attr = p_node->f_attrib;
	while (p_attr)
	{
		if (p_attr->type == NTYPE_ATTRIB)
			xml_len += strlen(p_attr->name) + 4 + strlen(p_attr->data);	//sprintf(xml_buf+xml_len," %s=\"%s\"",p_attr->name,p_attr->data);
		else if (p_attr->type == NTYPE_CDATA)
		{
			xml_len += 1 + strlen(p_attr->data) + 2 + strlen(p_node->name) + 1;	//sprintf(xml_buf+xml_len,">%s</%s>",p_attr->data,p_node->name);
			return xml_len;
		}
		else
			;

		p_attr = p_attr->next;
	}

	if (p_node->f_child)
	{
	    XMLN * p_child;
	    
		xml_len += 1;	//sprintf(xml_buf+xml_len, ">");
		
		p_child = p_node->f_child;
		while (p_child)
		{
			xml_len += xml_calc_buf_len(p_child);	//xml_write_buf(p_child,xml_buf+xml_len);
			p_child = p_child->next;
		}

		xml_len += 2 + strlen(p_node->name) + 1;	//sprintf(xml_buf+xml_len, "</%s>",p_node->name);
	}
	else
	{
		xml_len += 2;	//sprintf(xml_buf+xml_len, "/>");
	}

	return xml_len;
}

/***************************************************************************************/
int xml_write_buf(XMLN * p_node, char * xml_buf)
{
	int xml_len = 0;
    XMLN * p_attr;
    
	xml_len += sprintf(xml_buf+xml_len, "<%s", p_node->name);

	p_attr = p_node->f_attrib;
	while (p_attr)
	{
		if (p_attr->type == NTYPE_ATTRIB)
			xml_len += sprintf(xml_buf+xml_len, " %s=\"%s\"", p_attr->name, p_attr->data);
		else if(p_attr->type == NTYPE_CDATA)
		{
			xml_len += sprintf(xml_buf+xml_len, ">%s</%s>", p_attr->data, p_node->name);
			return xml_len;
		}
		else
			;

		p_attr = p_attr->next;
	}

	if (p_node->f_child)
	{
	    XMLN * p_child;
	    
		xml_len += sprintf(xml_buf+xml_len, ">");
		
		p_child = p_node->f_child;
		while (p_child)
		{
			xml_len += xml_write_buf(p_child, xml_buf+xml_len);
			p_child = p_child->next;
		}

		xml_len += sprintf(xml_buf+xml_len, "</%s>", p_node->name);
	}
	else
	{
		xml_len += sprintf(xml_buf+xml_len, "/>");
	}

	return xml_len;
}

/***************************************************************************************/
int xml_nwrite_buf(XMLN * p_node, char * xml_buf, int buf_len)
{
    int ret = 0;
	int xml_len = 0;
    XMLN * p_attr;
    
	if ((NULL == p_node) || (NULL == p_node->name))
		return -1;

	if (strlen(p_node->name) >= (size_t)buf_len)
		return -1;

	xml_len += sprintf(xml_buf+xml_len, "<%s", p_node->name);

	p_attr = p_node->f_attrib;
	while (p_attr)
	{
		if (p_attr->type == NTYPE_ATTRIB)
		{
			if ((strlen(p_attr->name) + strlen(p_attr->data) + xml_len) > (size_t)buf_len)
				return -1;
			xml_len += sprintf(xml_buf+xml_len, " %s=\"%s\"", p_attr->name, p_attr->data);
		}
		else if (p_attr->type == NTYPE_CDATA)
		{
			if (0x0a == (*p_attr->data))
			{
				p_attr = p_attr->next;
				continue;
			}
			if ((strlen(p_attr->data) + strlen(p_node->name) + xml_len) >= (size_t)buf_len)
				return -1;
			xml_len += sprintf(xml_buf+xml_len, ">%s</%s>", p_attr->data, p_node->name);
			return xml_len;
		}
		else
			;

		p_attr = p_attr->next;
	}

	if (p_node->f_child)
	{
	    XMLN * p_child;
	    
		xml_len += sprintf(xml_buf+xml_len, ">");
		
		p_child = p_node->f_child;
		while (p_child)
		{
			ret = xml_nwrite_buf(p_child, xml_buf+xml_len, buf_len-xml_len);
			if (ret < 0)
				return ret;
			xml_len += ret;
			p_child = p_child->next;
		}

		xml_len += sprintf(xml_buf+xml_len, "</%s>", p_node->name);
	}
	else
	{
		xml_len += sprintf(xml_buf+xml_len, "/>");
	}

	return xml_len;
}

void stream_startElement(void * userdata, const char * name, const char ** atts)
{
    XMLN * parent;
    XMLN * p_node;
	XMLN ** pp_node = (XMLN **)userdata;
	if (pp_node == NULL)
	{
		return;
	}

	parent = *pp_node;
	p_node = xml_node_add(parent,(char *)name);
	if (atts)
	{
		int i=0;
		while (atts[i] != NULL)
		{
			if (atts[i+1] == NULL)
				break;

			xml_attr_add(p_node,atts[i],atts[i+1]);	//(XMLN *)malloc(sizeof(XMLN));

			i += 2;
		}
	}

	*pp_node = p_node;
}

void stream_endElement(void * userdata, const char * name)
{
    XMLN * p_node;
	XMLN ** pp_node = (XMLN **)userdata;
	if (pp_node == NULL)
	{
		return;
	}

	p_node = *pp_node;
	if (p_node == NULL)
	{
		return;
	}

	p_node->finish = 1;

	if (p_node->type == NTYPE_TAG && p_node->parent == NULL)
	{
		// parse finish
	}
	else
	{
		*pp_node = p_node->parent;	// back up a level
	}
}

void stream_charData(void* userdata, const char* s, int len)
{
    const char *pchar = NULL;
    XMLN * p_node;
	XMLN ** pp_node = (XMLN **)userdata;
	if (pp_node == NULL)
	{
		return;
	}
	
	p_node = *pp_node;
	if (p_node == NULL)
	{
	//	log_user_print((void *)p_user,"stream_charData::cur_node is null,s'addr=0x%x,len=%d!!!\r\n",s,len);
		return;
	}
	
	p_node->data = s;
	p_node->dlen = len;
}

XMLN * xxx_hxml_parse(char * p_xml, int len)
{
    int status;
	XMLN * p_root = NULL;
	LTXMLPRS parse;
	
	memset(&parse, 0, sizeof(parse));

	parse.userdata = &p_root;
	parse.startElement = stream_startElement;
	parse.endElement = stream_endElement;
	parse.charData = stream_charData;

	parse.xmlstart = p_xml;
	parse.xmlend = p_xml + len;
	parse.ptr = parse.xmlstart;

	status = hxml_parse(&parse);
	if (status < 0)
	{
		log_print("xxx_hxml_parse::err[%d]\r\n", status);

		xml_node_del(p_root);
		p_root = NULL;
	}

	return p_root;
}


