#include "upnpd.h"
#include "gtlog.h"
#include "iniparser.h"
#include "csvparser.h"
#include "commonlib.h"
#include "gtthread.h"
#include "send_state.h"
#include "mod_cmd.h"
#include "mod_socket.h"
#include "stdio.h"
port_t	main_ports[]=  //GTVM/S服务端口的描述列表,与配置文件保持统一,程序会把这些端口按内外一致的规则映射
{
	{"port:cmd_port"},
	{"port:image_port"},
	{"port:audio_port"},
	{"port:com0_port"},
	{"port:com1_port"},
	{"port:ftp_port"},
	{"port:telnet_port"},
	{"port:web_port"}
};

static UpnpClient_Handle hnd = -1; 	//upnp的操作句柄

static char current_router_addr[100];	//记录当前路由器的地址


char * ctrlurl;			//记录当前路由器的控制url

static int current_state = UPNPD_STATE_INIT; //记录当前状态,分为UPNPD_STATE_INIT,UPNPD_STATE_GETROUTER,UPNPD_STATE_OK,UPNPD_STATE_FAILED

/********************************************************************************
 * 函数名:get_current_state()
 * 功能: 返回当前状态
 ********************************************************************************/
int get_current_state(void)
{
	return current_state;
}

/********************************************************************************
 * 函数名:GetServiceList()
 * 功能: 在给定的servlistnodelist中找到第index个servicelist元素
 *		 并解析出它带的service元素列表
 *
 * 返回值:service元素列表指针
 ********************************************************************************/
IXML_NodeList *GetServiceList( IN IXML_NodeList *servlistnodelist ,int index)
{
    IXML_NodeList *ServiceList = NULL;
    IXML_Node *servlistnode = NULL;

    if( servlistnodelist && ixmlNodeList_length( servlistnodelist ) ) 
    {

        servlistnode = ixmlNodeList_item( servlistnodelist, index );

        /*
           create as list of DOM nodes 
         */
        ServiceList =  ixmlElement_getElementsByTagName( ( IXML_Element * )
                                              servlistnode, "service" );
    }

    return ServiceList;
}


/********************************************************************************
 * GetFirstDocumentItem
 *
 * Description: 
 *       Given a document node, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *       String must be freed by caller using free.
 * Parameters:
 *   doc -- The DOM document from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char * GetFirstDocumentItem( IN IXML_Document * doc, IN const char *item )
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;

    nodeList = ixmlDocument_getElementsByTagName( doc, ( char * )item );

    if( nodeList ) 
    {
        if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) ) 
        {
            textNode = ixmlNode_getFirstChild( tmpNode );
            ret = strdup( ixmlNode_getNodeValue( textNode ) );
        }
    }

    if( nodeList )
        ixmlNodeList_free( nodeList );
    return ret;
}

/********************************************************************************
 * GetFirstElementItem
 *
 * Description: 
 *       Given a DOM element, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *       The string must be freed using free.
 * Parameters:
 *   element -- The DOM element from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char * GetFirstElementItem( IN IXML_Element * element, IN const char *item )
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;

    nodeList = ixmlElement_getElementsByTagName( element, ( char * )item );

    if( nodeList == NULL ) 
    {
        printf( "Error finding %s in XML Node\n", item );
        return NULL;
    }

    if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) == NULL ) 
    {
        printf( "Error finding %s value in XML Node\n", item );
        ixmlNodeList_free( nodeList );
        return NULL;
    }

    textNode = ixmlNode_getFirstChild( tmpNode );

    ret = strdup( ixmlNode_getNodeValue( textNode ) );

    if( !ret ) 
    {
        printf( "Error allocating memory for %s in XML Node\n",item );
        ixmlNodeList_free( nodeList );
        return NULL;
    }

    ixmlNodeList_free( nodeList );

    return ret;
}
/********************************************************************************
 * 函数名:	FindAndParseService()
 *
 * 功能:	在给定的xml文件中找指定的服务类型，并输出其控制url
 *
 * 输入		DescDoc ,xml文件
 *   		location, 其所在的地址
 *   		serviceType, 需要找的服务名称 
 * 输出:	controlURL,控制服务的url
 * 返回值:	找到返回1，没找到返回0
 ********************************************************************************/
int FindAndParseService( IN IXML_Document * DescDoc,IN char *location,IN char *serviceType, OUT char **controlURL )
{
    int i,length,found = 0;
    int ret;
    char *tempServiceType = NULL;
    char *base;
    char *relcontrolURL = NULL;
    IXML_NodeList *serviceList = NULL;
    IXML_Element *service = NULL;
    int index=0;
    IXML_NodeList *servlistnodelist = NULL;


    base = location;
    servlistnodelist =ixmlDocument_getElementsByTagName(DescDoc, "serviceList" );

	for(index=0;index < ixmlNodeList_length(servlistnodelist); index++)
	{
		serviceList = GetServiceList( servlistnodelist, index );
    	length = ixmlNodeList_length( serviceList );
   		for( i = 0; i < length; i++ ) 
    	{
	        service = ( IXML_Element * ) ixmlNodeList_item( serviceList, i );
	        tempServiceType = GetFirstElementItem( ( IXML_Element * ) service, "serviceType" );
	      	if( strcmp( tempServiceType, serviceType ) == 0 ) {
             	relcontrolURL = GetFirstElementItem( service, "controlURL" );
         
            *controlURL = malloc( strlen( base ) + strlen( relcontrolURL ) + 1 );
            if( *controlURL ) 
            {
                ret = UpnpResolveURL( base, relcontrolURL, *controlURL );
                if( ret != UPNP_E_SUCCESS )
                    printf ( "Error generating controlURL from %s + %s\n",  base, relcontrolURL );
            }

            if( relcontrolURL )
                free( relcontrolURL );
      
            found = 1;
            break;
        }

        if( tempServiceType )
            free( tempServiceType );
        tempServiceType = NULL;
    	}
	}
    if( tempServiceType )
        free( tempServiceType );
    if( serviceList )
        ixmlNodeList_free( serviceList );
    if( servlistnodelist )
    	ixmlNodeList_free( servlistnodelist);

    return ( found );
}



/**********************************************************************
*	函数名: get_random_port()
*	功能:	得出一个1025-9024之间的随机端口号
*	输入:	
*	返回值:	得出的端口号
***********************************************************************/
int get_random_port(void)
{
	return 1025+(int)(rand()/(RAND_MAX+1.0)*8000);
}



/**********************************************************************
*	函数名: is_current_router()
*	功能:	判断输入地址是否符合当前在用的router地址
*	输入:	router_addr:需要判断的router地址
*	返回值:	TRUE or FALSE
***********************************************************************/
BOOL is_current_router(char *router_addr)
{
	if(strncmp(router_addr,current_router_addr,strlen(current_router_addr))==0)
		return TRUE;
	else
		return FALSE;
}

/**********************************************************************
*	函数名: parse_port_mapping_doc()
*	功能:	解析路由器发来的端口查询响应文档,判断端口是否被当前服务占用
*	说明:	只有在端口已被映射时才会调此函数
*	
*	输入:	RespDocument:	路由器发来的端口查询响应文档
*			intport:		打算映射的内部端口	
*			description:	端口的描述	
*	返回值:	PORT_OCCUPIED-------端口被其他服务占据
*			PORTMAP_SUCCESSFUL--当前端口和服务已被映射成功,无需再作映射
***********************************************************************/

int parse_port_mapping_doc(IN IXML_Document *RespDocument, IN int intport, IN const char* description)
{
	if (intport!=atoi(GetFirstDocumentItem(RespDocument, "NewInternalPort")))
		return PORT_OCCUPIED;
	if (strcmp(UpnpGetServerIpAddress(),GetFirstDocumentItem(RespDocument, "NewInternalClient"))!=0)
		return PORT_OCCUPIED;
	if (strcmp(description, GetFirstDocumentItem(RespDocument, "NewPortMappingDescription"))!=0)
		return PORT_OCCUPIED;
	else
		return PORTMAP_SUCCESSFUL;
}



/**********************************************************************
*	函数名: query_extport_status()
*	功能:	查询准备映射的外部端口的占用情况
*	
*	输入:	extport:		打算映射的外部端口		
*			intport:		打算映射的内部端口	
*			description:	端口的描述	
*			ctrlurl:		路由器的控制地址		
*	返回值:	PORT_AVAILABLE------端口空闲
*			PORT_OCCUPIED-------端口被其他服务占据
*			PORTMAP_SUCCESSFUL--当前端口和服务已被映射成功,无需再作映射
***********************************************************************/

int query_extport_status(IN int extport, IN int intport, IN const char* descrption, IN char* ctrlurl)
{
	int ret=PORTMAP_SUCCESSFUL;
	IXML_Document *ActionDocument= NULL;
	IXML_Document *RespDocument = NULL;
	
	char extportstr[10];
	
	
	sprintf(extportstr,"%d",extport);
	ActionDocument = UpnpMakeAction("GetSpecificPortMappingEntry", IP_SERVICE_TYPE, 0, NULL);

	UpnpAddToAction(&ActionDocument,"GetSpecificPortMappingEntry", IP_SERVICE_TYPE,	"NewRemoteHost",			NULL);
	UpnpAddToAction(&ActionDocument,"GetSpecificPortMappingEntry", IP_SERVICE_TYPE,	"NewExternalPort",			extportstr);
	UpnpAddToAction(&ActionDocument,"GetSpecificPortMappingEntry", IP_SERVICE_TYPE, "NewProtocol",				"TCP");
	UpnpAddToAction(&ActionDocument,"GetSpecificPortMappingEntry", IP_SERVICE_TYPE, "NewInternalPort",			NULL);
	UpnpAddToAction(&ActionDocument,"GetSpecificPortMappingEntry", IP_SERVICE_TYPE, "NewInternalClient",		NULL);
	UpnpAddToAction(&ActionDocument,"GetSpecificPortMappingEntry", IP_SERVICE_TYPE, "NewEnabled",				NULL);
	UpnpAddToAction(&ActionDocument,"GetSpecificPortMappingEntry", IP_SERVICE_TYPE, "NewPortMappingDescription",NULL);
	UpnpAddToAction(&ActionDocument,"GetSpecificPortMappingEntry", IP_SERVICE_TYPE, "NewLeaseDuration",			NULL);

	ret = UpnpSendAction(hnd,ctrlurl,IP_SERVICE_TYPE,NULL,ActionDocument,&RespDocument);
	if( ret != UPNP_E_SUCCESS )
	{
		if(ret == UPNP_SOAP_E_INVALID_ARGS)//表示端口未被映射
			ret = PORT_AVAILABLE;
		else
			ret = PORT_OCCUPIED;
	}
	else
	{
		ret = parse_port_mapping_doc(RespDocument,intport,descrption);
	}
	
	ixmlDocument_free(RespDocument);
	ixmlDocument_free(ActionDocument);
	return ret;
}


/**********************************************************************
*	函数名: send_mapport_action()
*	功能:	发送要求端口映射的命令
*	
*	输入:	intportstr:		要求映射的内部端口字符串		
*			extportstr:		要求映射的外部端口字符串	
*			port_dscrp:		端口的描述	
*			ctrlurl:		路由器的控制地址	
*	返回值:	0表示成功其他表示失败
***********************************************************************/
int send_mapport_action(IN char * intportstr, IN char * extportstr, IN char *port_dscrp, IN char* ctrlurl)
{
	IXML_Document *ActionDocument= NULL;
	IXML_Document *RespDocument = NULL;  
	int ret;
	
	ActionDocument = UpnpMakeAction("AddPortMapping", IP_SERVICE_TYPE, 0, NULL);

	UpnpAddToAction(&ActionDocument,"AddPortMapping", IP_SERVICE_TYPE,	"NewRemoteHost",			NULL);
	UpnpAddToAction(&ActionDocument,"AddPortMapping", IP_SERVICE_TYPE,	"NewExternalPort",			extportstr);
	UpnpAddToAction(&ActionDocument,"AddPortMapping", IP_SERVICE_TYPE, 	"NewProtocol",				"TCP");
	UpnpAddToAction(&ActionDocument,"AddPortMapping", IP_SERVICE_TYPE, 	"NewInternalPort",			intportstr);
	UpnpAddToAction(&ActionDocument,"AddPortMapping", IP_SERVICE_TYPE, 	"NewInternalClient",		UpnpGetServerIpAddress());
	UpnpAddToAction(&ActionDocument,"AddPortMapping", IP_SERVICE_TYPE,	"NewEnabled",				"1");
	UpnpAddToAction(&ActionDocument,"AddPortMapping", IP_SERVICE_TYPE,	"NewPortMappingDescription",port_dscrp);
	UpnpAddToAction(&ActionDocument,"AddPortMapping", IP_SERVICE_TYPE,	"NewLeaseDuration",			0);

	
	ret = UpnpSendAction(hnd,ctrlurl,IP_SERVICE_TYPE,NULL,ActionDocument,&RespDocument);
	ixmlDocument_free(ActionDocument);
	ixmlDocument_free(RespDocument);
	return ret;
}

//为xvsport作映射，规则是:
//a.内端口保持不变，寻找可用的外端口映射
//b.优先考虑配置文件中写的上次的外端口，以减少不必要的端口变化
int map_xvs_port (dictionary *ini, int intport, int tgtport, char* ctrlurl)
{
	int ret;
	int extport;
	
	char extportstr[10];
	char intportstr[10];

	char port_dscrp[] = "xvs port mapping";

	if((ini==NULL)||(intport==0))
		return PORTMAP_FAILED;

	if(tgtport>1024)
		extport = tgtport;
	else
		extport = intport;
	
	ret = query_extport_status(extport,intport,port_dscrp, ctrlurl);
	if(ret == PORTMAP_SUCCESSFUL)	//已经映射好了
		return extport;
	
	while(ret == PORT_OCCUPIED)
	{			
		//随机取一个端口
		
		extport = get_random_port();
		ret = query_extport_status(extport,intport,port_dscrp,ctrlurl);
	}

	sprintf(extportstr,"%d",extport);
	sprintf(intportstr,"%d",intport);
	
	ret = send_mapport_action(intportstr, extportstr, port_dscrp,ctrlurl);
	if(ret == UPNP_E_SUCCESS)
	{
		if(extport != tgtport)
		{
			printf("upnp将xvs端口%5d映射到网关的%5d上\n",intport,extport);
			gtloginfo("upnp将xvs端口%5d映射到网关的%5d上\n",intport,extport);
		}
		return extport;
	}
	else
		return PORTMAP_FAILED;
}



//为指定的描述，进行端口映射,规则是:
//a.内外端口保持一致;
//b.ftp不能使用21端口
//c.优先使用配置文件中的原端口，若不成功，则随机选取1025-9026之间的端口
int map_main_port(dictionary *ini, char* port_dscrp, char *ctrlurl)
{
	int ret=PORT_OCCUPIED;
	int extport;//最终映射到的外部端口号
	int tgtport;//配置文件中相对于该服务的原端口
	char cmd[100];
	char portname[100]; //端口名称，形如"ftp_port"
	char *lp;
	char extportstr[10];

	tgtport = iniparser_getint(ini,port_dscrp,0);
	if(tgtport == 0)     //配置文件中没有相应的记录，不映射                                  
		return 0;

	lp = index(port_dscrp,':');
	if(lp!= NULL)
		strncpy(portname,lp+1,90);
	else
		strncpy(portname,port_dscrp,strlen(port_dscrp)+1);
		
	if((strncmp(port_dscrp,"port:ftp_port",strlen("port:ftp_port"))== 0)&&(tgtport == 21)) //不允许ftp端口是21	
		goto test_avail_ports;
		
	ret = query_extport_status(tgtport,tgtport,portname,ctrlurl);
	if(ret == PORTMAP_SUCCESSFUL)	//已经映射好了,无需更新
		return 0;
	
	extport = tgtport ;
	
test_avail_ports:
	
	while(ret == PORT_OCCUPIED)
	{			
		extport = get_random_port();//随机取一个端口
		ret = query_extport_status(extport,extport,portname, ctrlurl);//查询占用情况
	}
	
	if(ret == PORT_AVAILABLE)
	{
		sprintf(extportstr,"%d",extport);
		ret = send_mapport_action(extportstr, extportstr, portname, ctrlurl);	
	}

	if(extport != tgtport)
	{
		//printf("upnp将主要服务%s改在端口%d上映射成功\n",port_dscrp,extport);
		//gtloginfo("upnp将主要服务%s改在端口%d上映射成功\n",port_dscrp,extport);
		iniparser_setint(ini,port_dscrp,extport);
		if(strncmp(port_dscrp,"port:telnet_port",strlen("port:telnet_port"))== 0)//telnet变换端口了
		{
			printf("新增telnet监听端口%d\n",extport);
			gtloginfo("新增telnet监听端口%d\n",extport);
			sprintf(cmd,"telnetd -p %d &",extport);
			system(cmd);
		}
		return 1;
	}
	return 0;

}

/**********************************************************************
*	函数名: parse_and_map_xvsport()
*	功能:	将配置文件中的intsec节解析，并逐一映射，写到extsec节
*	
*	输入:	ini:			配置文件结构		
*			intsec:			需要映射的内部端口(csv格式)	
*			extsec:			映射到的外部端口字符串(csv格式)
*			ctrlurl:		路由器的控制地址
*	返回值:	0表示成功
***********************************************************************/
int parse_and_map_xvsport(IN dictionary *ini, IN char * intsec, IN char *extsec, IN char* ctrlurl)
{
	char *portstr;
	CSV_T *csvin;
	CSV_T *csvout;
	int entry_num;
	int i;
	int extport;
	char *extportstr;
	int ctrlport;
	int intport;
	
	portstr = iniparser_getstr(ini,intsec);
	if(portstr == NULL)
		return 0;

	extportstr = iniparser_getstring(ini,extsec,portstr);

	csvin = csv_create();
	csvout = csv_create();
	csv_parse_string(csvin,portstr);
	csv_parse_string(csvout,extportstr);

	ctrlport = iniparser_getint(ini,"xvsinfo0:port",0);
	
	entry_num = csv_get_var_num(csvin);
	for(i=1; i<=entry_num; i++)	//for every xvs port to be mapped
	{	
		intport = csv_get_int(csvin, i, PORTMAP_FAILED);
		extport = map_xvs_port(ini,intport,csv_get_int(csvout, i, PORTMAP_FAILED), ctrlurl);
		csv_set_int(csvout,i,extport);
		if(intport == ctrlport)
		{
			iniparser_setint(ini,"xvsinfo0:port_upnp",extport);
		}
	}
	iniparser_setstr(ini,extsec,csv_get_string(csvout));
	
	csv_destroy(csvin);
	csv_destroy(csvout);
	return 0;
}

/**********************************************************************
*	函数名: get_ext_ipaddr()
*	功能:	获取路由器的外网ip

*	输入:	ctrlurl: 控制地址
*	返回值: 外网ip地址，失败返回null
***********************************************************************/
char* get_ext_ipaddr(char *ctrlurl)
{
	int ret;
	IXML_Document *ActionDocument= NULL;
	IXML_Document *RespDocument = NULL;
	char *extip;
	
	ActionDocument = UpnpMakeAction("GetExternalIPAddress", IP_SERVICE_TYPE, 0, NULL);

	UpnpAddToAction(&ActionDocument,"GetExternalIPAddress", IP_SERVICE_TYPE,	"NewExternalIPAddress",			NULL);
	
	ret = UpnpSendAction(hnd,ctrlurl,IP_SERVICE_TYPE,NULL,ActionDocument,&RespDocument);
	if( ret != UPNP_E_SUCCESS )
	{
		printf("failed to get the extipaddr, ret %d,%s\n",ret,UpnpGetErrorMessage(ret));
		return NULL;
	}
	else
	{
		extip = GetFirstDocumentItem(RespDocument, "NewExternalIPAddress");
	}
	ixmlDocument_free(ActionDocument);
	ixmlDocument_free(RespDocument);
	return extip;
}

/*在找到符合要求的路由后调用，进行端口映射工作
  输入: ctrlurl,路由器的控制地址
 */
int port_mapping_all(char *ctrlurl)
{
	FILE *fp = NULL;
	dictionary *ini = NULL;
	int i;
	char cmd[100];
	srand(132144);

	sprintf(cmd,"cp %s %s -f",UPNPD_PARA_FILE,UPNPD_SAVE_FILE);
	system(cmd);
	ini = iniparser_load_lockfile(UPNPD_PARA_FILE,1,&fp);

	//for every main port
	for(i=0; i< sizeof(main_ports)/sizeof(port_t); i++)
	{
		map_main_port(ini,main_ports[i].port_dscrp, ctrlurl);
	}

	//for xvs port
	parse_and_map_xvsport(ini,"xvsinfo0:map_port","xvsinfo0:map_port_upnp",ctrlurl);
	save_inidict_file(UPNPD_PARA_FILE,ini,&fp);
	iniparser_freedict(ini);

	current_state = UPNPD_SUCCESS;
	send_state(UPNPD_SUCCESS);	
	
	if(ini_diff(UPNPD_SAVE_FILE,UPNPD_PARA_FILE)>0)
	{
		printf("端口有变动,更新到其它配置文件,硬重起!\n");
		gtloginfo("端口有变动,更新到其它配置文件,硬重起!\n");
		system("/gt1000/para_conv -s");
		system("/gt1000/hwrbt");
	}
	else
	{
		printf("完成端口映射,端口无变动\n");
		gtloginfo("完成端口映射,端口无变动\n");
	}
	
	return 0;
}

/* 注册的回调函数，网关发来的命令都到这里处理*/
int upnp_callback( Upnp_EventType EventType,void *Event, void *Cookie )
{
	struct Upnp_Discovery *d_event;
	IXML_Document *DescDoc = NULL;
	int ret;
	char *extip;

	d_event=( struct Upnp_Discovery * )Event;  	
	switch ( EventType ) 
	{
		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE://收到广播
			break; //不理他!
			
        case UPNP_DISCOVERY_SEARCH_RESULT://查询返回
        
	       	if((current_state != UPNPD_STATE_INIT)&&(current_state != UPNPD_STATE_FAILED)) //不需要新router
	    		return 0;

	    	 if( d_event->ErrCode == UPNP_E_SUCCESS ) 
	    	 {
				if(is_current_router(d_event->Location))
				{
					current_state = UPNPD_STATE_GETROUTER;
	    	 		strncpy(current_router_addr,d_event->Location,strlen(d_event->Location));
	    	 	}
	    	 	else
	    	 		return 0;
	    	 	
	    	 	if(( ret =UpnpDownloadXmlDoc( d_event->Location,&DescDoc ) ) !=UPNP_E_SUCCESS ) 
	    	 	{
	               	current_state = UPNPD_STATE_INIT;
	               	return 0;
	    	 	} 
	    	 	else 
	    	 	{
	    	 		gtloginfo("连接到地址为%s的路由器%s\n",d_event->Location,GetFirstDocumentItem( DescDoc, "friendlyName"));
					printf("连接到地址为%s的路由器%s\n",d_event->Location,GetFirstDocumentItem( DescDoc, "friendlyName"));
					ret = FindAndParseService(DescDoc,d_event->Location,IP_SERVICE_TYPE, &ctrlurl);
					if(ret == 1) //found service
					{
						extip = get_ext_ipaddr(ctrlurl);
						printf("服务器外网地址%s. 开始端口映射..\n",extip);
						gtloginfo("服务器外网地址%s. 开始端口映射..\n",extip);
						port_mapping_all(ctrlurl);
					}
					else
						current_state =UPNPD_STATE_INIT ;
	             }

	            if( DescDoc )
	                ixmlDocument_free( DescDoc );
	         }
			 else
			 {
				printf( "Error in Discovery Callback -- %s",UpnpGetErrorMessage(d_event->ErrCode));
			 }
		break;
         
		case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE://路由要走了~
			if(current_state == UPNPD_STATE_OK)
			{
				if( is_current_router(d_event->Location) == TRUE) //是我们用的路由
		       	{
		       		printf("The router %s said goodbye\n",d_event->Location);
					gtloginfo("路由器%s宣告服务中止，重新寻找可用路由\n",d_event->Location);
		        	current_state = UPNPD_STATE_INIT;
		        	
				}
			}
		break;

        default:
        break;
	}
	return 0;

}

void *check_upnp_status_thread (void *data)
{
	int status_cnt =0;
	while(1)
		{
			sleep(1);
		
			if( current_state == UPNPD_SUCCESS)
			{
				status_cnt = 0;
				continue;
			}

			if((current_state!= UPNPD_STATE_FAILED)&&(++status_cnt >=SENDSTATE_INTERVAL))
			{
				current_state = UPNPD_STATE_FAILED; 
				send_state();
				status_cnt = 0;
			}
		}
}

/*获取当前系统的缺省路由,并把它写到current_router_addr里*/
int get_def_route(void)
{
	char tempbuf[300];
	char dfgateway[50];
	char residue[100];
	FILE *fp;
	while(1)
	{
		system("route   |   grep   UG>   /var/rtname") ;  
	  	fp   =   fopen("/var/rtname","r");   
	  	if   (!fp)
	  	{   
	        return   -1;   
	  	}   
	  	fgets(tempbuf,200,fp);   
	  	fclose(fp);
	   	sscanf(tempbuf,"default\t%s\t%s",dfgateway,residue);
  		if(dfgateway[0]!='\0')
  			break;
  		else
  		{
  			sleep(5); //等待dhcpcd找到可用路由
  		}
  	}
  	sprintf(current_router_addr,"http://%s",dfgateway);
	return 0;

}

//查询目前映射的端口是否健在，返回1表示正常
int is_mapport_alive(void)
{
	int imageport;
	int ret;
	dictionary *ini;
	
	ini = iniparser_load(UPNPD_PARA_FILE);
	if(ini == NULL)
		return 	0;
		
	imageport = iniparser_getint(ini,"port:image_port",0);
	iniparser_freedict(ini);
	
	if(imageport == 0)
	{
		return 0;
	}	
	
	
	ret = query_extport_status(imageport,imageport,"image_port", ctrlurl);
	if(ret == PORTMAP_SUCCESSFUL)
		return 1;
	else
		return 0;
}

int main(void)
{
 	int rc;
 	char ip_address[50];
 	char *addr = ip_address;
 	int port;
 	int dhcp;
 	int keepalive_cnt = 0;
 	int search_cnt=SEARCH_ROUTER_INTERVAL -1;
 	char *route = NULL;
 	dictionary *ini = NULL;
 	pthread_t recv_modcom_id = -1;
 	pthread_t check_status_id = -1;

	gtopenlog("upnpd");
	if(create_lockfile_save_version(UPNPD_LOCKFILE,VERSION)<0)
	{
		printf("trying to start upnpd but it's already running, exit!..\n");
		gtloginfo("trying to start upnpd but it's already running,exit!..\n");
		
		exit(1);
	}
	printf("upnpd [v%s] start running..\n",VERSION);
	gtloginfo("upnpd [v%s] start running..\n",VERSION);

	rc = system("route add -net 239.0.0.0 netmask 255.0.0.0 eth0");

	if(init_com_channel() <0)
	{
		printf("init com channel failed, exit!..\n");
		gtloginfo("init com channel failed, exit!..\n");
		exit(1);
	}
	
	gt_create_thread(&recv_modcom_id, recv_modcom_thread,NULL);
	create_recv_modsocket_thread();

	ini=iniparser_load(UPNPD_PARA_FILE);
	if(ini==NULL)
	{
		printf("upnpd load parafile %s failed,exit!\n",UPNPD_PARA_FILE);
		gtlogerr("load parafile %s failed,exit!\n",UPNPD_PARA_FILE);
		exit(1);
	}

	dhcp = iniparser_getint(ini,"net_config:use_dhcp",0);
	if((iniparser_getint(ini,"net_config:internet_mode",0)!= 1)||((iniparser_getint(ini,"net_config:use_upnp",0)!= 1)&&(dhcp!= 1)))
	{
		printf("upnpd: internet_mode != 1, or use_upnp/dhcp != 1, exit!\n");
		gtlogerr("internet_mode != 1, or use_upnp/dhcp != 1,exit\n");
		exit(1);
	}
	if(dhcp == 1)
	{
		get_def_route();
		printf("upnpd: dhcp方式下取得当前缺省路由 %s\n",current_router_addr);
		gtloginfo("dhcp方式下取得当前缺省路由 %s\n",current_router_addr);
	}
	else
	{
		route = iniparser_getstring(ini,"net_config:route_default",NULL);
		if(route == NULL)
		{
			printf("upnpd:配置文件中没有指定路由,退出\n");
			gtloginfo("配置文件中没有指定路由,退出!\n");
			exit(1);
		}
		else
		{
			printf("upnpd:配置文件中指定路由%s\n",route);
			gtloginfo("配置文件中指定路由%s\n",route);
			sprintf(current_router_addr,"http://%s",route);
		}
		iniparser_freedict(ini);
	}
	
    rc = UpnpInit( NULL, 0);
    if(rc != UPNP_E_SUCCESS ) 
    {
       	printf("upnpd初始化upnp sdk失败: %s,退出\n", UpnpGetErrorMessage(rc));
       	gtlogerr("upnpd初始化upnp sdk失败: %s,退出\n", UpnpGetErrorMessage(rc));
       	UpnpFinish();
       	send_state(UPNPD_FAILURE);
        exit(1);
    }
	addr = UpnpGetServerIpAddress(  );
    port = UpnpGetServerPort(  );
   	printf("UPnP Initialized (%s:%d)\n", addr, port );
	gtloginfo("UPnP Initialized (%s:%d)\n", addr, port );
	
 	rc = UpnpRegisterClient( upnp_callback,&hnd, &hnd );
	if(rc!= UPNP_E_SUCCESS)
	{
		printf("upnpd注册监控点失败:%s,退出\n",UpnpGetErrorMessage(rc));
		gtlogerr("upnpd注册监控点失败:%s,退出\n",UpnpGetErrorMessage(rc));
		UpnpFinish();
		send_state(UPNPD_FAILURE);
		exit(1);
	}

	gt_create_thread(&check_status_id, check_upnp_status_thread,NULL);		
	
	while(1)
    {
    
    	
    	if(current_state!= UPNPD_STATE_OK)  //还没有成功映射
    	{
    		if(++search_cnt >= SEARCH_ROUTER_INTERVAL)
    		{
	    		search_cnt = 0;
	    		UpnpSearchAsync(hnd,SEARCH_ROUTER_INTERVAL, IP_SERVICE_TYPE, NULL);
    		}
    	}
	    else //已经成功映射,需要检测是否掉线 
	    {
	   
			if(++keepalive_cnt >= KEEPALIVE_INTERVAL)
			{
				keepalive_cnt = 0;
				//查询端口映射
				if(is_mapport_alive()!=1) //failed
				{
					printf("The router %s stopped service\n",current_router_addr);
					gtloginfo("路由器%s可能断线或重起，重新进行映射\n",current_router_addr);
			        current_state = UPNPD_STATE_INIT;
			        UpnpSearchAsync(hnd,SEARCH_ROUTER_INTERVAL, IP_SERVICE_TYPE, NULL);
				}
			}
		}	
	   sleep(1);	
    }
	return 0;
}
