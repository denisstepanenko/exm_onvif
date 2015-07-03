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
#include "onvif_pkt.h"
#include "sha1.h"
#include "onvif_utils.h"


/***************************************************************************************/
static const char xml_hdr[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

static const char onvif_xmlns[] = 
	"<s:Envelope "
	"xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" " 
	"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" " 
	"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " 
	"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" " 
	"xmlns:wsa=\"http://www.w3.org/2005/08/addressing\" "	
	"xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" " 
	"xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\" " 
	"xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" " 
	"xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\" "
	"xmlns:tt=\"http://www.onvif.org/ver10/schema\" " 
	"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
	"xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" " 
	"xmlns:tev=\"http://www.onvif.org/ver10/events/wsdl\" "
    "xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\" "
    "xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\" "
    "xmlns:trp=\"http://www.onvif.org/ver10/replay/wsdl\" "
    "xmlns:tse=\"http://www.onvif.org/ver10/search/wsdl\" "
    "xmlns:trc=\"http://www.onvif.org/ver10/recording/wsdl\" "
	"xmlns:ter=\"http://www.onvif.org/ver10/error\" >";

static const char soap_body[] = 
    "<s:Body>";

static const char soap_tailer[] =
    "</s:Body></s:Envelope>";

    
/***************************************************************************************/
#define NONCELEN	20
#define SHA1_SIZE	20

void onvif_calc_nonce(char nonce[NONCELEN])
{ 
	static int count = 0xCA53;
  	char buf[NONCELEN + 1];
  	
  	/* we could have used raw binary instead of hex as below */
  	sprintf(buf, "%8.8x%4.4hx%8.8x", (int)time(NULL), count++, (int)rand());
  	memcpy(nonce, buf, NONCELEN);
}

void onvif_calc_digest(const char *created, const char *nonce, int noncelen, const char *password, char hash[SHA1_SIZE])
{
	sha1_context ctx;
	
	sha1_starts(&ctx);
	sha1_update(&ctx, (unsigned char *)nonce, noncelen);
	sha1_update(&ctx, (unsigned char *)created, strlen(created));
	sha1_update(&ctx, (unsigned char *)password, strlen(password));
	sha1_finish(&ctx, (unsigned char *)hash);
}

int build_onvif_req_header(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, const char * to)
{
	int offset = 0;
	
	if (p_dev->username[0] != '\0' && p_dev->password[0] != '\0')
	{
		char HA[SHA1_SIZE], HABase64[100];
		char nonce[NONCELEN], nonceBase64[100];		

		const char *created = onvif_format_datetime_str(time(NULL), 1, "%Y-%m-%dT%H:%M:%SZ");

		onvif_calc_nonce(nonce);

		base64_encode((unsigned char*)nonce, NONCELEN, nonceBase64, sizeof(nonceBase64));

		onvif_calc_digest(created, nonce, NONCELEN, p_dev->password, HA);

		base64_encode((unsigned char*)HA, SHA1_SIZE, HABase64, sizeof(HABase64));

        offset += snprintf(p_buf+offset, mlen-offset, "<s:Header>");
        if (to != NULL)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<wsa:To>%s</wsa:To>", to);
        }
		offset += snprintf(p_buf+offset, mlen-offset, "<wsse:Security>"
        	"<wsse:UsernameToken>"
        	"<wsse:Username>%s</wsse:Username>"
        	"<wsse:Password "
        	"Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">"
        	"%s</wsse:Password>"
        	"<wsse:Nonce>%s</wsse:Nonce>"
        	"<wsu:Created>%s</wsu:Created>"
        	"</wsse:UsernameToken>"
        	"</wsse:Security>", p_dev->username, HABase64, nonceBase64, created);
		offset += snprintf(p_buf+offset, mlen-offset, "</s:Header>");
	}
	else if (to != NULL)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<s:Header>");
	    offset += snprintf(p_buf+offset, mlen-offset, "<wsa:To>%s</wsa:To>", to);
	    offset += snprintf(p_buf+offset, mlen-offset, "</s:Header>");
	}

	return offset;
}


/***************************************************************************************/
int build_GetCapabilities_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	GetCapabilities_REQ * p_req = (GetCapabilities_REQ *) argv;
	
    int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetCapabilities>"
			"<tds:Category>%s</tds:Category>"
		"</tds:GetCapabilities>", 
	    p_req ? onvif_get_cap_str(p_req->Category) : "All");
	    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetServices_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
    GetServices_REQ * p_req = (GetServices_REQ *) argv;
	
    int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetServices>"
			"<tds:IncludeCapability>%s</tds:IncludeCapability>"
		"</tds:GetServices>", 
	    p_req ? (p_req->IncludeCapability ? "true" : "false") : "false");
	    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetDeviceInformation_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetDeviceInformation />");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetNetworkInterfaces_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkInterfaces />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_SetNetworkInterfaces_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetNetworkInterfaces_REQ * p_req = (SetNetworkInterfaces_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:SetNetworkInterfaces>"
    	"<tds:InterfaceToken>%s</tds:InterfaceToken>", 
    	p_req->net_inf.token);

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:NetworkInterface>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Enabled>%s</tt:Enabled>", p_req->net_inf.enabled ? "true" : "false");

	if (p_req->net_inf.mtu > 60)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:MTU>%d</tt:MTU>", p_req->net_inf.mtu);
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4><tt:Enabled>%s</tt:Enabled>", p_req->net_inf.ipv4_enabled ? "true" : "false");

	if (p_req->net_inf.fromdhcp)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:DHCP>true</tt:DHCP>");
	}
	else
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:Manual>"
				"<tt:Address>%s</tt:Address>"
	       		"<tt:PrefixLength>%d</tt:PrefixLength>"
       		"</tt:Manual>", 
       		p_req->net_inf.ipv4_addr, 
       		p_req->net_inf.prefix_len);
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:IPv4>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkInterface>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:SetNetworkInterfaces>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetNTP_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNTP />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetNTP_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetNTP_REQ * p_req = (SetNTP_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNTP>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:FromDHCP>%s</tds:FromDHCP>", p_req->ntp.fromdhcp ? "true" : "false");

	if (p_req->ntp.fromdhcp == FALSE)
	{
		int i;
		for (i = 0; i < MAX_NTP_SERVER; i++)
		{
			if (p_req->ntp.server[i][0] == '\0')
			{
				continue;
			}

			if (is_ip_address(p_req->ntp.server[i]))
			{
    			offset += snprintf(p_buf+offset, mlen-offset, "<tds:NTPManual><tt:Type>IPv4</tt:Type>"
    				"<tt:IPv4Address>%s</tt:IPv4Address></tds:NTPManual>", p_req->ntp.server[i]);
			}
			else
			{
			    offset += snprintf(p_buf+offset, mlen-offset, "<tds:NTPManual><tt:Type>DNS</tt:Type>"
    				"<tt:DNSName>%s</tt:DNSName></tds:NTPManual>", p_req->ntp.server[i]);
			}
		}	
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:SetNTP>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetHostname_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetHostname />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetHostname_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetHostname_REQ * p_req = (SetHostname_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetHostname><tds:Name>%s</tds:Name></tds:SetHostname>", p_req->Name);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetHostnameFromDHCP_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetHostnameFromDHCP_REQ * p_req = (SetHostnameFromDHCP_REQ *) argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:SetHostnameFromDHCP>"
			"<tds:FromDHCP>%s</tds:FromDHCP>"
	    "</tds:SetHostnameFromDHCP>", 
	    p_req ? (p_req->FromDHCP ? "true" : "false") : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;	
}

int build_GetDNS_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetDNS />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetDNS_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int i;
	int offset;
	SetDNS_REQ * p_req = (SetDNS_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetDNS>");

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:FromDHCP>%s</tds:FromDHCP>", p_req->dns.fromdhcp ? "true" : "false");

	for (i = 0; i < MAX_SEARCHDOMAIN; i++)
	{
		if (p_req->dns.searchdomain[i][0] == '\0')
		{
			continue;
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:SearchDomain>%s</tds:SearchDomain>", p_req->dns.searchdomain[i]);
	}

	if (p_req->dns.fromdhcp == FALSE)
	{
		for (i = 0; i < MAX_DNS_SERVER; i++)
		{
			if (p_req->dns.server[i][0] == '\0')
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:DNSManual><tt:Type>IPv4</tt:Type>"
				"<tt:IPv4Address>%s</tt:IPv4Address></tds:DNSManual>", p_req->dns.server[i]);
		}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:SetDNS>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetDynamicDNS_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetDynamicDNS />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;		
}

int build_SetDynamicDNS_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetDynamicDNS_REQ * p_req = (SetDynamicDNS_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:SetDynamicDNS>"
			"<tds:Type>%s</tds:Type>"
        	"<tds:Name>%s</tds:Name>"
        	"<tds:TTL>PT%dS</tds:TTL>"
        "</tds:SetDynamicDNS>", 
        onvif_get_ddns_type_str(p_req->ddns.type), p_req->ddns.name, p_req->ddns.ttl);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetNetworkProtocols_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkProtocols />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetNetworkProtocols_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int i;	
	int offset;
	SetNetworkProtocols_REQ * p_req = (SetNetworkProtocols_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNetworkProtocols>");

	if (p_req->protocols.http_support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:NetworkProtocols><tt:Name>HTTP</tt:Name>"
			"<tt:Enabled>%s</tt:Enabled>", p_req->protocols.http_enable ? "true" : "false");

		for (i = 0; i < MAX_SERVER_PORT; i++)
		{
			if (p_req->protocols.http_port[i] != 0)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:Port>%d</tt:Port>", p_req->protocols.http_port[i]);
			}
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
	}

	if (p_req->protocols.https_support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:NetworkProtocols><tt:Name>HTTPS</tt:Name>"
			"<tt:Enabled>%s</tt:Enabled>", p_req->protocols.https_enable ? "true" : "false");

		for (i = 0; i < MAX_SERVER_PORT; i++)
		{
			if (p_req->protocols.https_port[i] != 0)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:Port>%d</tt:Port>", p_req->protocols.https_port[i]);
			}
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
	}

	if (p_req->protocols.rtsp_support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:NetworkProtocols><tt:Name>RTSP</tt:Name>"
			"<tt:Enabled>%s</tt:Enabled>", p_req->protocols.rtsp_enable ? "true" : "false");

		for (i = 0; i < MAX_SERVER_PORT; i++)
		{
			if (p_req->protocols.rtsp_port[i] != 0)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:Port>%d</tt:Port>", p_req->protocols.rtsp_port[i]);
			}
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:SetNetworkProtocols>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetDiscoveryMode_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetDiscoveryMode />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetDiscoveryMode_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	SetDiscoveryMode_REQ * p_req = (SetDiscoveryMode_REQ *) argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:SetDiscoveryMode>"
    		"<tds:DiscoveryMode>%s</tds:DiscoveryMode>"
   		"</tds:SetDiscoveryMode>",
   		p_req ? (p_req->DiscoveryMode ? "Discoverable" : "NonDiscoverable") : "Discoverable");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetNetworkDefaultGateway_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkDefaultGateway />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetNetworkDefaultGateway_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int i;
	int offset;
	SetNetworkDefaultGateway_REQ * p_req = (SetNetworkDefaultGateway_REQ *) argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNetworkDefaultGateway>");

	for (i = 0; i < MAX_GATEWAY && p_req; i++)
	{
		if (p_req->IPv4Address[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:IPv4Address>%s</tds:IPv4Address>", p_req->IPv4Address[i]);
		}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:SetNetworkDefaultGateway>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetSystemDateAndTime_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetSystemDateAndTime />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetSystemDateAndTime_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetSystemDateAndTime_REQ * p_req = (SetSystemDateAndTime_REQ *) argv;	
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetSystemDateAndTime>");

	if (p_req->type == 1)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:DateTimeType>NTP</tds:DateTimeType>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:DaylightSavings>%s</tds:DaylightSavings>", p_req->DaylightSavings ? "true" : "false");
	}
	else
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:DateTimeType>Manual</tds:DateTimeType>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:DaylightSavings>%s</tds:DaylightSavings>", p_req->DaylightSavings ? "true" : "false");

        if (p_req->TZ[0] != '\0')
        {
		    offset += snprintf(p_buf+offset, mlen-offset, "<tds:TimeZone><tt:TZ>%s</tt:TZ></tds:TimeZone>", p_req->TZ);
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:UTCDateTime>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Time><tt:Hour>%d</tt:Hour><tt:Minute>%d</tt:Minute><tt:Second>%d</tt:Second></tt:Time>",
			p_req->datetime.hour, p_req->datetime.minute, p_req->datetime.second);	
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Date><tt:Year>%d</tt:Year><tt:Month>%d</tt:Month><tt:Day>%d</tt:Day></tt:Date>",
			p_req->datetime.year, p_req->datetime.month, p_req->datetime.day);		
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:UTCDateTime>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:SetSystemDateAndTime>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SystemReboot_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SystemReboot />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetSystemFactoryDefault_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	SetSystemFactoryDefault_REQ * p_req = (SetSystemFactoryDefault_REQ *) argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:SetSystemFactoryDefault>"
			"<tds:FactoryDefault>%s</tds:FactoryDefault>"
		"</tds:SetSystemFactoryDefault>", 
		p_req ? (p_req->type == 1 ? "Hard" : "Soft") : "Soft");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer); 
	return offset;
}

int build_GetSystemLog_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
    GetSystemLog_REQ * p_req = (GetSystemLog_REQ *) argv;
    
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetSystemLog>"
        	"<tds:LogType>%s</tds:LogType>"
        "</tds:GetSystemLog>", 
        p_req ? ((p_req->LogType == 1) ? "Access" : "System") : "System");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}
		
int build_GetScopes_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	return 0;
}
		
int build_SetScopes_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	return 0;
}
		
int build_AddScopes_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	return 0;
}
		
int build_RemoveScopes_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	return 0;
}

int build_GetVideoSources_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSources />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}
		
int build_GetAudioSources_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSources />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_CreateProfile_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
    CreateProfile_REQ * p_req = (CreateProfile_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:CreateProfile><trt:Name>%s</trt:Name>", p_req->Name);
	if (p_req->Token[0] != '\0')
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<trt:Token>%s</trt:Token>", p_req->Token);
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:CreateProfile>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}
		
int build_GetProfile_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetProfile_REQ * p_req = (GetProfile_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:GetProfile>"
			"<trt:ProfileToken>%s</trt:ProfileToken>"
	    "</trt:GetProfile>", 
	    p_req->ProfileToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetProfiles_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetProfiles />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_AddVideoEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	AddVideoEncoderConfiguration_REQ * p_req = (AddVideoEncoderConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:AddVideoEncoderConfiguration>"
       	 	"<trt:ProfileToken>%s</trt:ProfileToken>"
       	 	"<trt:ConfigurationToken>%s</trt:ConfigurationToken>"
        "</trt:AddVideoEncoderConfiguration>",
        p_req->ProfileToken, p_req->ConfigurationToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

		
int build_AddVideoSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	AddVideoSourceConfiguration_REQ * p_req = (AddVideoSourceConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:AddVideoSourceConfiguration>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        	"<trt:ConfigurationToken>%s</trt:ConfigurationToken>"
        "</trt:AddVideoSourceConfiguration>", 
        p_req->ProfileToken, p_req->ConfigurationToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_AddAudioEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	AddAudioEncoderConfiguration_REQ * p_req = (AddAudioEncoderConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:AddAudioEncoderConfiguration>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        	"<trt:ConfigurationToken>%s</trt:ConfigurationToken>"
        "</trt:AddAudioEncoderConfiguration>", 
        p_req->ProfileToken, p_req->ConfigurationToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}
		
int build_AddAudioSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	AddAudioSourceConfiguration_REQ * p_req = (AddAudioSourceConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:AddAudioSourceConfiguration>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        	"<trt:ConfigurationToken>%s</trt:ConfigurationToken>"
        "</trt:AddAudioSourceConfiguration>", 
        p_req->ProfileToken, p_req->ConfigurationToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}
		
int build_AddPTZConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	AddPTZConfiguration_REQ * p_req = (AddPTZConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:AddPTZConfiguration>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        	"<trt:ConfigurationToken>%s</trt:ConfigurationToken>"
        "</trt:AddPTZConfiguration>", 
        p_req->ProfileToken, p_req->ConfigurationToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_RemoveVideoEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	RemoveVideoEncoderConfiguration_REQ * p_req = (RemoveVideoEncoderConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:RemoveVideoEncoderConfiguration>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        "</trt:RemoveVideoEncoderConfiguration>",
        p_req->ProfileToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_RemoveVideoSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	RemoveVideoSourceConfiguration_REQ * p_req = (RemoveVideoSourceConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:RemoveVideoSourceConfiguration>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        "</trt:RemoveVideoSourceConfiguration>", 
        p_req->ProfileToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_RemoveAudioEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	RemoveAudioEncoderConfiguration_REQ * p_req = (RemoveAudioEncoderConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:RemoveAudioEncoderConfiguration>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        "</trt:RemoveAudioEncoderConfiguration>", 
        p_req->ProfileToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_RemoveAudioSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	RemoveAudioSourceConfiguration_REQ * p_req = (RemoveAudioSourceConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:RemoveAudioSourceConfiguration>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        "</trt:RemoveAudioSourceConfiguration>", 
        p_req->ProfileToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_RemovePTZConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	RemovePTZConfiguration_REQ * p_req = (RemovePTZConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:RemovePTZConfiguration>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        "</trt:RemovePTZConfiguration>", 
        p_req->ProfileToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_DeleteProfile_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	DeleteProfile_REQ * p_req = (DeleteProfile_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:DeleteProfile>"
        	"<trt:ProfileToken>%s</trt:ProfileToken>"
        "</trt:DeleteProfile>", 
        p_req->ProfileToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetVideoSourceConfigurations_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurations />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetVideoEncoderConfigurations_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurations />");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetAudioSourceConfigurations_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurations />");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetAudioEncoderConfigurations_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioEncoderConfigurations />");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}
	
int build_GetVideoSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
    GetVideoSourceConfiguration_REQ * p_req = (GetVideoSourceConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:GetVideoSourceConfiguration>"
        	"<trt:ConfigurationToken></trt:ConfigurationToken>"
        "</trt:GetVideoSourceConfiguration>", 
        p_req->ConfigurationToken);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetVideoEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetVideoEncoderConfiguration_REQ * p_req = (GetVideoEncoderConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:GetVideoEncoderConfiguration>"
        	"<trt:ConfigurationToken></trt:ConfigurationToken>"
        "</trt:GetVideoEncoderConfiguration>", 
        p_req->ConfigurationToken);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetAudioSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetAudioSourceConfiguration_REQ * p_req = (GetAudioSourceConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:GetAudioSourceConfiguration>"
        	"<trt:ConfigurationToken></trt:ConfigurationToken>"
        "</trt:GetAudioSourceConfiguration>", 
        p_req->ConfigurationToken);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetAudioEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetAudioEncoderConfiguration_REQ * p_req = (GetAudioEncoderConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:GetAudioEncoderConfiguration>"
        	"<trt:ConfigurationToken></trt:ConfigurationToken>"
        "</trt:GetAudioEncoderConfiguration>",
        p_req->ConfigurationToken);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_SetVideoSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetVideoSourceConfiguration_REQ * p_req = (SetVideoSourceConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoSourceConfiguration><trt:Configuration token=\"%s\">"
		"<tt:Name>%s</tt:Name><tt:UseCount>%d</tt:UseCount><tt:SourceToken>%s</tt:SourceToken>"
     	"<tt:Bounds x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\"></tt:Bounds>"
     	"<trt:ForcePersistence>%s</trt:ForcePersistence></trt:SetVideoSourceConfiguration>",
     	p_req->video_src_cfg.token, p_req->video_src_cfg.name, p_req->video_src_cfg.use_count, p_req->video_src_cfg.source_token,
     	p_req->video_src_cfg.x, p_req->video_src_cfg.y, p_req->video_src_cfg.width, p_req->video_src_cfg.height, 
     	p_req->persistence ? "true" : "false");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_SetVideoEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetVideoEncoderConfiguration_REQ * p_req = (SetVideoEncoderConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoEncoderConfiguration><trt:Configuration token=\"%s\">"
		"<tt:Name>%s</tt:Name><tt:UseCount>%d</tt:UseCount><tt:Encoding>%s</tt:Encoding>",
		p_req->video_enc.token, p_req->video_enc.name, p_req->video_enc.use_count, onvif_get_video_encoding_str(p_req->video_enc.encoding));
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution>",
		p_req->video_enc.width, p_req->video_enc.height);
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Quality>%d</tt:Quality>", p_req->video_enc.quality);
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit>"
      	"<tt:EncodingInterval>%d</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl>",
      	p_req->video_enc.framerate_limit, p_req->video_enc.encoding_interval, p_req->video_enc.bitrate_limit);

	if (VIDEO_ENCODING_H264 == p_req->video_enc.encoding)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264><tt:GovLength>%d</tt:GovLength>"
    		"<tt:H264Profile>%s</tt:H264Profile></tt:H264>", p_req->video_enc.gov_len,
	    	onvif_get_h264_profile_str((E_H264_PROFILE)p_req->video_enc.profile));
	}
	else if (VIDEO_ENCODING_MPEG4 == p_req->video_enc.encoding)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:MPEG4><tt:GovLength>%d</tt:GovLength>"
    		"<tt:Mpeg4Profile>%s</tt:Mpeg4Profile></tt:MPEG4>", p_req->video_enc.gov_len,
	    	onvif_get_mpeg4_profile_str((E_MPEG4_PROFILE)p_req->video_enc.profile));
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type>"
		"<tt:IPv4Address>%s</tt:IPv4Address></tt:Address><tt:Port>%d</tt:Port>"
		"<tt:TTL>%d</tt:TTL><tt:AutoStart>%s</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT%dS</tt:SessionTimeout>",
		p_req->video_enc.multicast.ip, p_req->video_enc.multicast.port, 
	    p_req->video_enc.multicast.ttl, p_req->video_enc.multicast.auto_start ? "true" : "false");   	

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration><trt:ForcePersistence>%s</trt:ForcePersistence>"
		"</trt:SetVideoEncoderConfiguration>", p_req->persistence ? "true" : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_SetAudioSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetAudioSourceConfiguration_REQ * p_req = (SetAudioSourceConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetAudioSourceConfiguration><trt:Configuration token=\"%s\">"
		"<tt:Name>%s</tt:Name><tt:UseCount>%d</tt:UseCount><tt:SourceToken>%s</tt:SourceToken>"     	
     	"<trt:ForcePersistence>%s</trt:ForcePersistence></trt:SetAudioSourceConfiguration>",
     	p_req->audio_src_cfg.token, p_req->audio_src_cfg.name, p_req->audio_src_cfg.use_count, p_req->audio_src_cfg.source_token,
     	p_req->persistence ? "true" : "false");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_SetAudioEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetAudioEncoderConfiguration_REQ * p_req = (SetAudioEncoderConfiguration_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetAudioEncoderConfiguration><trt:Configuration token=\"%s\">"
		"<tt:Name>%s</tt:Name><tt:UseCount>%d</tt:UseCount><tt:Encoding>%s</tt:Encoding><tt:Bitrate>%d</tt:Bitrate>"
     	"<tt:SampleRate>%d</tt:SampleRate>", p_req->audio_enc.token, p_req->audio_enc.name, p_req->audio_enc.use_count, 
     	onvif_get_audio_encoding_str(p_req->audio_enc.encoding), p_req->audio_enc.bitrate, p_req->audio_enc.sample_rate);	

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type>"
		"<tt:IPv4Address>%s</tt:IPv4Address></tt:Address><tt:Port>%d</tt:Port>"
		"<tt:TTL>%d</tt:TTL><tt:AutoStart>%s</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT%dS</tt:SessionTimeout>",
		p_req->audio_enc.multicast.ip, p_req->audio_enc.multicast.port, 
	    p_req->audio_enc.multicast.ttl, p_req->audio_enc.multicast.auto_start ? "true" : "false");   	

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:ForcePersistence>%s</trt:ForcePersistence>"
		"</trt:SetAudioEncoderConfiguration>", p_req->persistence ? "true" : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetVideoSourceConfigurationOptions_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
    GetVideoSourceConfigurationOptions_REQ * p_req = (GetVideoSourceConfigurationOptions_REQ *) argv;
        
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationOptions>");
	if (p_req && p_req->config_token[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:ConfigurationToken>%s</trt:ConfigurationToken>", p_req->config_token);
	}
	if (p_req && p_req->profile_token[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:ProfileToken>%s</trt:ProfileToken>", p_req->profile_token);
	}
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationOptions>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetVideoEncoderConfigurationOptions_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	GetVideoEncoderConfigurationOptions_REQ * p_req = (GetVideoEncoderConfigurationOptions_REQ *) argv;
        
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationOptions>");
	if (p_req && p_req->config_token[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:ConfigurationToken>%s</trt:ConfigurationToken>", p_req->config_token);
	}
	if (p_req && p_req->profile_token[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:ProfileToken>%s</trt:ProfileToken>", p_req->profile_token);
	}
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoEncoderConfigurationOptions>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetAudioSourceConfigurationOptions_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	GetAudioSourceConfigurationOptions_REQ * p_req = (GetAudioSourceConfigurationOptions_REQ *) argv;
        
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurationOptions>");
	if (p_req && p_req->config_token[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:ConfigurationToken>%s</trt:ConfigurationToken>", p_req->config_token);
	}
	if (p_req && p_req->profile_token[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:ProfileToken>%s</trt:ProfileToken>", p_req->profile_token);
	}
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourceConfigurationOptions>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetAudioEncoderConfigurationOptions_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	GetAudioEncoderConfigurationOptions_REQ * p_req = (GetAudioEncoderConfigurationOptions_REQ *) argv;
        
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioEncoderConfigurationOptions>");
	if (p_req && p_req->config_token[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:ConfigurationToken>%s</trt:ConfigurationToken>", p_req->config_token);
	}
	if (p_req && p_req->profile_token[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:ProfileToken>%s</trt:ProfileToken>", p_req->profile_token);
	}
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioEncoderConfigurationOptions>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetStreamUri_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetStreamUri_REQ * p_req = (GetStreamUri_REQ *) argv;
	assert(p_req);
	
    offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetStreamUri>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:StreamSetup>");

	if (p_req->stream_type == 1)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Stream>RTP-Multicast</tt:Stream>");
	}
	else 
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Stream>RTP-Unicast</tt:Stream>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Transport>");
	
	if (p_req->protocol == 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Protocol>UDP</tt:Protocol>");
	}
	else if (p_req->protocol == 1)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Protocol>TCP</tt:Protocol>");
	}
	else if (p_req->protocol == 3)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Protocol>HTTP</tt:Protocol>");
	}
	else
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Protocol>RTSP</tt:Protocol>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Transport>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:StreamSetup>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:ProfileToken>%s</trt:ProfileToken>", p_req->profile_token);
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetStreamUri>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_SetSynchronizationPoint_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetSynchronizationPoint_REQ * p_req = (SetSynchronizationPoint_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetSynchronizationPoint>"
        "<trt:ProfileToken></trt:ProfileToken></trt:SetSynchronizationPoint>", p_req->profile_token);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}
		
int build_GetSnapshotUri_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetSnapshotUri_REQ * p_req = (GetSnapshotUri_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetSnapshotUri>"
        "<trt:ProfileToken></trt:ProfileToken></trt:GetSnapshotUri>", p_req->profile_token);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetNodes_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetNodes />");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetNode_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetNode_REQ * p_req = (GetNode_REQ *) argv;
    assert(p_req);
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetNode>"
		"<tptz:NodeToken></tptz:NodeToken></trt:GetNode>", p_req->node_token);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetPresets_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetPresets_REQ * p_req = (GetPresets_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetPresets><tptz:ProfileToken>%s"
		"</tptz:ProfileToken></tptz:GetPresets>", p_req->profile_token);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetPreset_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetPreset_REQ * p_req = (SetPreset_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:SetPreset>"
		"<tptz:ProfileToken>%s</tptz:ProfileToken>", p_req->profile_token);

	if (p_req->name[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:PresetName>%s</tptz:PresetName>", p_req->name);
	}	 

	if (p_req->preset_token[0] != '\0')
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:PresetToken>%s</tptz:PresetToken>", p_req->preset_token);
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:SetPreset>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_RemovePreset_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	RemovePreset_REQ * p_req = (RemovePreset_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:RemovePreset><tptz:ProfileToken>%s</tptz:ProfileToken>"
		"<tptz:PresetToken>%s</tptz:PresetToken></tptz:RemovePreset>", p_req->profile_token, p_req->preset_token);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GotoPreset_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GotoPreset_REQ * p_req = (GotoPreset_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GotoPreset><tptz:ProfileToken>%s</tptz:ProfileToken>"
    	"<tptz:PresetToken>%s</tptz:PresetToken>", p_req->profile_token, p_req->preset_token);
    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Speed><tt:PanTilt x=\"%0.1f\" y=\"%0.1f\"></tt:PanTilt>"
        "<tt:Zoom x=\"%0.1f\"></tt:Zoom></tptz:Speed>", p_req->pantilt_speed_x, p_req->pantilt_speed_y, p_req->zoom_speed);
    offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GotoPreset>");
    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GotoHomePosition_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GotoHomePosition_REQ * p_req = (GotoHomePosition_REQ *) argv;	
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GotoHomePosition><tptz:ProfileToken>%s"
		"</tptz:ProfileToken>", p_req->profile_token);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Speed><tt:PanTilt x=\"%0.1f\" y=\"%0.1f\"></tt:PanTilt>"
        "<tt:Zoom x=\"%0.1f\"></tt:Zoom></tptz:Speed>", p_req->pantilt_speed_x, p_req->pantilt_speed_y, p_req->zoom_speed);
    offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GotoHomePosition>");	
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetHomePosition_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetHomePosition_REQ * p_req = (SetHomePosition_REQ *) argv;	
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:SetHomePosition><tptz:ProfileToken>%s"
		"</tptz:ProfileToken></tptz:SetHomePosition>", p_req->profile_token);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetStatus_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetStatus_REQ * p_req = (GetStatus_REQ *) argv;	
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetStatus><tptz:ProfileToken>%s"
		"</tptz:ProfileToken></tptz:GetStatus>", p_req->profile_token);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_ContinuousMove_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	ContinuousMove_REQ * p_req = (ContinuousMove_REQ *) argv;
	assert(p_req);
	
    offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:ContinuousMove>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:ProfileToken>%s</tptz:ProfileToken>", p_req->profile_token);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Velocity>");
   	offset += snprintf(p_buf+offset, mlen-offset, "<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\"></tt:PanTilt>", 
    	    p_req->pantilt_velocity_x, p_req->pantilt_velocity_y);
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Zoom x=\"%0.1f\"></tt:Zoom>", p_req->zoom_velocity);
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:Velocity>");
	if (p_req->timeout > 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Timeout>PT%dS</tptz:Timeout>", p_req->timeout);
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:ContinuousMove>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_RelativeMove_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	RelativeMove_REQ * p_req = (RelativeMove_REQ *) argv;
	assert(p_req);
	
    offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:RelativeMove>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:ProfileToken>%s</tptz:ProfileToken>", p_req->profile_token);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Translation>");
   	offset += snprintf(p_buf+offset, mlen-offset, "<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\"></tt:PanTilt>", 
    	    p_req->pantilt_translation_x, p_req->pantilt_translation_y);
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Zoom x=\"%0.1f\"></tt:Zoom>", p_req->zoom_translation);
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:Translation>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Speed>");
   	offset += snprintf(p_buf+offset, mlen-offset, "<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\"></tt:PanTilt>", 
    	    p_req->pantilt_speed_x, p_req->pantilt_speed_y);
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Zoom x=\"%0.1f\"></tt:Zoom>", p_req->zoom_speed);
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:Speed>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:RelativeMove>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}
		
int build_AbsoluteMove_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	AbsoluteMove_REQ * p_req = (AbsoluteMove_REQ *) argv;
	assert(p_req);
	
    offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:AbsoluteMove>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:ProfileToken>%s</tptz:ProfileToken>", p_req->profile_token);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Position>");
   	offset += snprintf(p_buf+offset, mlen-offset, "<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\"></tt:PanTilt>", 
    	    p_req->pantilt_position_x, p_req->pantilt_position_y);
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Zoom x=\"%0.1f\"></tt:Zoom>", p_req->zoom_position);
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:Position>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Speed>");
   	offset += snprintf(p_buf+offset, mlen-offset, "<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\"></tt:PanTilt>", 
    	    p_req->pantilt_speed_x, p_req->pantilt_speed_y);
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Zoom x=\"%0.1f\"></tt:Zoom>", p_req->zoom_speed);
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:Speed>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:AbsoluteMove>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_PTZ_Stop_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	PTZStop_REQ * p_req = (PTZStop_REQ *) argv;
	assert(p_req);
	
    offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Stop>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:ProfileToken>%s</tptz:ProfileToken>", p_req->profile_token);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:PanTilt>%s</tptz:PanTilt>", p_req->stop_pantile ? "true" : "false");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:Zoom>%s</tptz:Zoom>", p_req->stop_zoom ? "true" : "false");
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:Stop>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetConfigurations_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetConfigurations />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetConfiguration_REQ * p_req = (GetConfiguration_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetConfiguration><tptz:PTZConfigurationToken>%s"
		"</tptz:PTZConfigurationToken></tptz:GetConfiguration>", p_req->config_token);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetConfiguration_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	return 0;
}

int build_GetConfigurationOptions_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetConfigurationOptions_REQ * p_req = (GetConfigurationOptions_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetConfigurationOptions><tptz:ConfigurationToken>%s"
		"</tptz:ConfigurationToken></tptz:GetConfigurationOptions>", p_req->config_token);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetEventProperties_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tev:GetEventProperties />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}
    
int build_Renew_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	Renew_REQ * p_req = (Renew_REQ *) argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, p_dev->events.producter_addr);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<wsnt:Renew><wsnt:TerminationTime>PT%dS</wsnt:TerminationTime></wsnt:Renew>", 
		p_req ? (p_req->term_time) : 60);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_Unsubscribe_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, p_dev->events.producter_addr);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:Unsubscribe />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_Subscribe_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	Subscribe_REQ * p_req = (Subscribe_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:Subscribe>");
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<wsnt:ConsumerReference>"
	    "<wsa:Address>%s</wsa:Address>"
	    "</wsnt:ConsumerReference>", p_req->reference_addr);	    
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<wsnt:InitialTerminationTime>PT%dS</wsnt:InitialTerminationTime>", p_req->init_term_time);
    offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:Subscribe>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetImagingSettings_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetImagingSettings_REQ * p_req = (GetImagingSettings_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetImagingSettings><timg:VideoSourceToken>%s"
		"</timg:VideoSourceToken></timg:GetImagingSettings>", p_req->source_token);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_SetImagingSettings_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	SetImagingSettings_REQ * p_req = (SetImagingSettings_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:SetImagingSettings><timg:VideoSourceToken>%s"
		"</timg:VideoSourceToken>", p_req->source_token);

	offset += snprintf(p_buf+offset, mlen-offset, "<timg:ImagingSettings>");	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:BacklightCompensation><tt:Mode>%s</tt:Mode></tt:BacklightCompensation>",
		p_req->img_cfg.BacklightCompensation_Mode ? "ON" : "OFF");
		
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Brightness>%d</tt:Brightness><tt:ColorSaturation>%d</tt:ColorSaturation>"
		"<tt:Contrast>%d</tt:Contrast>", p_req->img_cfg.Brightness, p_req->img_cfg.ColorSaturation, p_req->img_cfg.Contrast);
		
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Exposure><tt:Mode>%s</tt:Mode>"
		"<tt:MinExposureTime>%d</tt:MinExposureTime><tt:MaxExposureTime>%d</tt:MaxExposureTime>"
		"<tt:MinGain>%d</tt:MinGain><tt:MaxGain>%d</tt:MaxGain></tt:Exposure>",
		p_req->img_cfg.Exposure_Mode ? "MANUAL" : "AUTO", p_req->img_cfg.MinExposureTime,
		p_req->img_cfg.MaxExposureTime, p_req->img_cfg.MinGain, p_req->img_cfg.MaxGain);
		
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilter>%s</tt:IrCutFilter><tt:Sharpness>%d</tt:Sharpness>", 
		p_req->img_cfg.IrCutFilter_Mode == 1 ? "ON" : (p_req->img_cfg.IrCutFilter_Mode == 2 ? "AUTO" : "OFF"),
		p_req->img_cfg.Sharpness);
		
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:WideDynamicRange><tt:Mode>%s</tt:Mode><tt:Level>%d</tt:Level></tt:WideDynamicRange>", 
		p_req->img_cfg.WideDynamicRange_Mode ? "ON" : "OFF", p_req->img_cfg.WideDynamicRange_Level);	
		
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:WhiteBalance><tt:Mode>%s</tt:Mode></tt:WhiteBalance>", 
		p_req->img_cfg.WhiteBalance_Mode ? "MANUAL" : "AUTO");	
	
	offset += snprintf(p_buf+offset, mlen-offset, "</timg:ImagingSettings>");
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:ForcePersistence>%s</timg:ForcePersistence></timg:SetImagingSettings>",
		p_req->persistence ? "true" : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetOptions_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetOptions_REQ * p_req = (GetOptions_REQ *) argv;
	assert(p_req);
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetOptions><timg:VideoSourceToken>%s"
		"</timg:VideoSourceToken></timg:GetOptions>", p_req->source_token);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetReplayUri_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
    GetReplayUri_REQ * p_req = (GetReplayUri_REQ *) argv;
	assert(p_req);
	
    offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trp:GetStreamUri><trp:StreamSetup>");

	if (p_req->StreamType == 1) // RTP-Multicast
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Stream>RTP-Multicast</tt:Stream>");
	}
	else // RTP-Unicast
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Stream>RTP-Unicast</tt:Stream>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Transport>");
	
	if (p_req->TransportProtocol == 0) // UDP
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Protocol>UDP</tt:Protocol>");
	}
	else if (p_req->TransportProtocol == 1) // TCP
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Protocol>TCP</tt:Protocol>");
	}
	else if (p_req->TransportProtocol == 3)  // HTTP
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Protocol>HTTP</tt:Protocol>");
	}
	else // RTSP
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Protocol>RTSP</tt:Protocol>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Transport></trp:StreamSetup>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trp:RecordingToken>%s</trp:RecordingToken>", p_req->RecordingToken);
	offset += snprintf(p_buf+offset, mlen-offset, "</trp:GetStreamUri>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}


int build_GetRecordingSummary_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetRecordingSummary />");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetRecordingInformation_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetRecordingInformation_REQ * p_req = (GetRecordingInformation_REQ *) argv;
	assert(p_req);

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:GetRecordingInformation>"
			"<tse:RecordingToken>%s</tse:RecordingToken>"
		"</tse:GetRecordingInformation>",
		p_req->RecordingToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetMediaAttributes_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int i;
	int offset;
	GetMediaAttributes_REQ * p_req = (GetMediaAttributes_REQ *) argv;
	assert(p_req);

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetMediaAttributes>");
	for (i = 0; i < 10; i++)
	{
		if (p_req->RecordingTokens[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tse:RecordingTokens>%s</tse:RecordingTokens>", p_req->RecordingTokens[i]);
		}
	}
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:Time>%s</tse:Time>", onvif_format_datetime_str(p_req->Time, 1, "%Y-%m-%dT%H:%M:%SZ"));
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:GetMediaAttributes>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_FindRecordings_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{	
	int i;
	int offset;
	FindRecordings_REQ * p_req = (FindRecordings_REQ *) argv;
	assert(p_req);

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:FindRecordings>");	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:Scope>");
	for (i = 0; i < 10; i++)
	{
		if (p_req->IncludedSources[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:IncludedSources><tt:Token>%s</tt:Token></tt:IncludedSources>", p_req->IncludedSources[i]);	
		}
	}
	for (i = 0; i < 10; i++)
	{
		if (p_req->IncludedRecordings[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:IncludedRecordings>%s</tt:IncludedRecordings>", p_req->IncludedRecordings[i]);	
		}
	}
	if (p_req->RecordingInformationFilter[0] != '\0')
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingInformationFilter>%s</tt:RecordingInformationFilter>", p_req->RecordingInformationFilter);
	}	
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:Scope>");
	if (p_req->MaxMatches != 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:MaxMatches>%d</tse:MaxMatches>", p_req->MaxMatches);
	}	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:KeepAliveTime>%d</tse:KeepAliveTime>", p_req->KeepAliveTime);
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:FindRecordings>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetRecordingSearchResults_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{	
	int offset;
	GetRecordingSearchResults_REQ * p_req = (GetRecordingSearchResults_REQ *) argv;
	assert(p_req);

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetRecordingSearchResults>");	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:SearchToken>%s</tse:SearchToken>", p_req->SearchToken);
	if (p_req->MinResults != 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:MinResults>%d</tse:MinResults>", p_req->MinResults);
	}
	if (p_req->MaxResults != 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:MaxResults>%d</tse:MaxResults>", p_req->MaxResults);
	}
	if (p_req->WaitTime != 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:WaitTime>%d</tse:WaitTime>", p_req->WaitTime);
	}		    
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:GetRecordingSearchResults>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_FindEvents_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{	
	int i;
	int offset;
	FindEvents_REQ * p_req = (FindEvents_REQ *) argv;
	assert(p_req);

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:FindEvents>");	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:StartPoint>%s</tse:StartPoint>", onvif_format_datetime_str(p_req->StartPoint, 1, "%Y-%m-%dT%H:%M:%SZ"));
	if (p_req->EndPoint != 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:EndPoint>%s</tse:EndPoint>", onvif_format_datetime_str(p_req->StartPoint, 1, "%Y-%m-%dT%H:%M:%SZ"));
	}
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:Scope>");
	for (i = 0; i < 10; i++)
	{
		if (p_req->IncludedSources[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:IncludedSources><tt:Token>%s</tt:Token></tt:IncludedSources>", p_req->IncludedSources[i]);	
		}
	}
	for (i = 0; i < 10; i++)
	{
		if (p_req->IncludedRecordings[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:IncludedRecordings>%s</tt:IncludedRecordings>", p_req->IncludedRecordings[i]);	
		}
	}
	if (p_req->RecordingInformationFilter[0] != '\0')
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingInformationFilter>%s</tt:RecordingInformationFilter>", p_req->RecordingInformationFilter);
	}	
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:Scope>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:SearchFilter></tse:SearchFilter>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:IncludeStartState>%s</tse:IncludeStartState>", p_req->IncludeStartState ? "true" : "false");
	if (p_req->MaxMatches != 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:MaxMatches>%d</tse:MaxMatches>", p_req->MaxMatches);
	}	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:KeepAliveTime>%d</tse:KeepAliveTime>", p_req->KeepAliveTime);
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:FindEvents>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetEventSearchResults_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{	
	int offset;
	GetEventSearchResults_REQ * p_req = (GetEventSearchResults_REQ *) argv;
	assert(p_req);

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetEventSearchResults>");	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:SearchToken>%s</tse:SearchToken>", p_req->SearchToken);
	if (p_req->MinResults != 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:MinResults>%d</tse:MinResults>", p_req->MinResults);
	}
	if (p_req->MaxResults != 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:MaxResults>%d</tse:MaxResults>", p_req->MaxResults);
	}
	if (p_req->WaitTime != 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:WaitTime>%d</tse:WaitTime>", p_req->WaitTime);
	}		    
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:GetEventSearchResults>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_GetSearchState_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	GetSearchState_REQ * p_req = (GetSearchState_REQ *) argv;
	assert(p_req);

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:GetSearchState>"
			"<tse:SearchToken>%s</tse:SearchToken>"
		"</tse:GetSearchState>",
		p_req->SearchToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}

int build_EndSearch_xml(char * p_buf, int mlen, ONVIF_DEVICE * p_dev, void * argv)
{
	int offset;
	EndSearch_REQ * p_req = (EndSearch_REQ *) argv;
	assert(p_req);

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += build_onvif_req_header(p_buf+offset, mlen-offset, p_dev, NULL);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:EndSearch>"
			"<tse:SearchToken>%s</tse:SearchToken>"
		"</tse:EndSearch>",
		p_req->SearchToken);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);	
	return offset;
}



int build_onvif_req_xml(char * p_buf, int mlen, eOnvifAction type, ONVIF_DEVICE * p_dev, void * p_req)
{
	int rlen = 0;

	switch(type)
	{
	case eGetCapabilities:
        rlen = build_GetCapabilities_xml(p_buf, mlen, p_dev, p_req);
        break;

    case eGetServices:
        rlen = build_GetServices_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eGetDeviceInformation:
		rlen = build_GetDeviceInformation_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eGetNetworkInterfaces:
        rlen = build_GetNetworkInterfaces_xml(p_buf, mlen, p_dev, p_req);
        break;

	case eSetNetworkInterfaces:
        rlen = build_SetNetworkInterfaces_xml(p_buf, mlen, p_dev, p_req);
        break;

	case eGetNTP:
		rlen = build_GetNTP_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eSetNTP:
		rlen = build_SetNTP_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eGetHostname:
		rlen = build_GetHostname_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eSetHostname:
		rlen = build_SetHostname_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eSetHostnameFromDHCP:
		rlen = build_SetHostnameFromDHCP_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetDNS:
		rlen = build_GetDNS_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetDNS:
		rlen = build_SetDNS_xml(p_buf, mlen, p_dev, p_req);
		break;	
		
	case eGetDynamicDNS:
		rlen = build_GetDynamicDNS_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetDynamicDNS:
		rlen = build_SetDynamicDNS_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eGetNetworkProtocols:
		rlen = build_GetNetworkProtocols_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eSetNetworkProtocols:
		rlen = build_SetNetworkProtocols_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetDiscoveryMode:
		rlen = build_GetDiscoveryMode_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eSetDiscoveryMode:
		rlen = build_SetDiscoveryMode_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetNetworkDefaultGateway:
		rlen = build_GetNetworkDefaultGateway_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eSetNetworkDefaultGateway:
		rlen = build_SetNetworkDefaultGateway_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetSystemDateAndTime:
		rlen = build_GetSystemDateAndTime_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetSystemDateAndTime:
		rlen = build_SetSystemDateAndTime_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSystemReboot:
		rlen = build_SystemReboot_xml(p_buf, mlen, p_dev, p_req);
		break;

	case eSetSystemFactoryDefault:
		rlen = build_SetSystemFactoryDefault_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetSystemLog:
		rlen = build_GetSystemLog_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetScopes:
		rlen = build_GetScopes_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetScopes:
		rlen = build_SetScopes_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eAddScopes:
		rlen = build_AddScopes_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eRemoveScopes:
		rlen = build_RemoveScopes_xml(p_buf, mlen, p_dev, p_req);
		break;
	
	case eGetVideoSources:
		rlen = build_GetVideoSources_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetAudioSources:
		rlen = build_GetAudioSources_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eCreateProfile:
		rlen = build_CreateProfile_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetProfile:
		rlen = build_GetProfile_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetProfiles:
		rlen = build_GetProfiles_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eAddVideoEncoderConfiguration:
		rlen = build_AddVideoEncoderConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eAddVideoSourceConfiguration:
		rlen = build_AddVideoSourceConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eAddAudioEncoderConfiguration:
		rlen = build_AddAudioEncoderConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eAddAudioSourceConfiguration:
		rlen = build_AddAudioSourceConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eAddPTZConfiguration:
		rlen = build_AddPTZConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eRemoveVideoEncoderConfiguration:
		rlen = build_RemoveVideoEncoderConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eRemoveVideoSourceConfiguration:
		rlen = build_RemoveVideoSourceConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eRemoveAudioEncoderConfiguration:
		rlen = build_RemoveAudioEncoderConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eRemoveAudioSourceConfiguration:
		rlen = build_RemoveAudioSourceConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eRemovePTZConfiguration:
		rlen = build_RemovePTZConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eDeleteProfile:
		rlen = build_DeleteProfile_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetVideoSourceConfigurations:
		rlen = build_GetVideoSourceConfigurations_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetVideoEncoderConfigurations:
		rlen = build_GetVideoEncoderConfigurations_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetAudioSourceConfigurations:
		rlen = build_GetAudioSourceConfigurations_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetAudioEncoderConfigurations:
		rlen = build_GetAudioEncoderConfigurations_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetVideoSourceConfiguration:
		rlen = build_GetVideoSourceConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetVideoEncoderConfiguration:
		rlen = build_GetVideoEncoderConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetAudioSourceConfiguration:
		rlen = build_GetAudioSourceConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetAudioEncoderConfiguration:
		rlen = build_GetAudioEncoderConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetVideoSourceConfiguration:
		rlen = build_SetVideoSourceConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetVideoEncoderConfiguration:
		rlen = build_SetVideoEncoderConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetAudioSourceConfiguration:
		rlen = build_SetAudioSourceConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetAudioEncoderConfiguration:
		rlen = build_SetAudioEncoderConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetVideoSourceConfigurationOptions:
		rlen = build_GetVideoSourceConfigurationOptions_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetVideoEncoderConfigurationOptions:
		rlen = build_GetVideoEncoderConfigurationOptions_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetAudioSourceConfigurationOptions:
		rlen = build_GetAudioSourceConfigurationOptions_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetAudioEncoderConfigurationOptions:
		rlen = build_GetAudioEncoderConfigurationOptions_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetStreamUri:
		rlen = build_GetStreamUri_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetSynchronizationPoint:
		rlen = build_SetSynchronizationPoint_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetSnapshotUri:
		rlen = build_GetSnapshotUri_xml(p_buf, mlen, p_dev, p_req);
		break;
		
    case eGetNodes:
    	rlen = build_GetNodes_xml(p_buf, mlen, p_dev, p_req);
    	break;
    	
	case eGetNode:
		rlen = build_GetNode_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetPresets:
		rlen = build_GetPresets_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetPreset:
		rlen = build_SetPreset_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eRemovePreset:
		rlen = build_RemovePreset_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGotoPreset:
		rlen = build_GotoPreset_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGotoHomePosition:
		rlen = build_GotoHomePosition_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetHomePosition:
		rlen = build_SetHomePosition_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eGetStatus:
		rlen = build_GetStatus_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eContinuousMove:
        rlen = build_ContinuousMove_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eRelativeMove:
		rlen = build_RelativeMove_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eAbsoluteMove:
		rlen = build_AbsoluteMove_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case ePTZStop:
        rlen = build_PTZ_Stop_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eGetConfigurations:
        rlen = build_GetConfigurations_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eGetConfiguration:
		rlen = build_GetConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
		
	case eSetConfiguration:
		rlen = build_SetConfiguration_xml(p_buf, mlen, p_dev, p_req);
		break;
			
	case eGetConfigurationOptions:
        rlen = build_GetConfigurationOptions_xml(p_buf, mlen, p_dev, p_req);
        break;
    
	case eGetEventProperties:
        rlen = build_GetEventProperties_xml(p_buf, mlen, p_dev, p_req);
        break;
    
	case eRenew:
        rlen = build_Renew_xml(p_buf, mlen, p_dev, p_req);
        break;
    
	case eUnsubscribe:
        rlen = build_Unsubscribe_xml(p_buf, mlen, p_dev, p_req);
        break;
    
	case eSubscribe:
        rlen = build_Subscribe_xml(p_buf, mlen, p_dev, p_req);
        break;
    	
	case eGetImagingSettings:
        rlen = build_GetImagingSettings_xml(p_buf, mlen, p_dev, p_req);
        break;
    
	case eSetImagingSettings:
        rlen = build_SetImagingSettings_xml(p_buf, mlen, p_dev, p_req);
        break;    

	case eGetOptions:
		rlen = build_GetOptions_xml(p_buf, mlen, p_dev, p_req);
        break;

    case eGetReplayUri:
		rlen = build_GetReplayUri_xml(p_buf, mlen, p_dev, p_req);
        break;

    case eGetRecordingSummary:
    	rlen = build_GetRecordingSummary_xml(p_buf, mlen, p_dev, p_req);
        break;

    case eGetRecordingInformation:
    	rlen = build_GetRecordingInformation_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eGetMediaAttributes:
    	rlen = build_GetMediaAttributes_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eFindRecordings:
    	rlen = build_FindRecordings_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eGetRecordingSearchResults:
    	rlen = build_GetRecordingSearchResults_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eFindEvents:
    	rlen = build_FindEvents_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eGetEventSearchResults:
    	rlen = build_GetEventSearchResults_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eGetSearchState:
    	rlen = build_GetSearchState_xml(p_buf, mlen, p_dev, p_req);
        break;
        
	case eEndSearch:
    	rlen = build_EndSearch_xml(p_buf, mlen, p_dev, p_req);
        break;        
	
	default:
		assert(FALSE);
	    break;
	}

	return rlen;
}




