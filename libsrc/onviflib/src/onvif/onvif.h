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

#ifndef _ONVIF_DEVICE_H_
#define _ONVIF_DEVICE_H_

#include "sys_inc.h"
#include "http.h"
#include "rtsp.h"

/***************************************************************************************/
#define ONVIF_TOKEN_LEN   	64
#define ONVIF_NAME_LEN    	100
#define ONVIF_URI_LEN     	512


#define MAX_PTZ_PRESETS     100 
#define MAX_DNS_SERVER		2
#define MAX_SEARCHDOMAIN	4
#define MAX_NTP_SERVER		2
#define MAX_SERVER_PORT		4
#define MAX_GATEWAY			2
#define MAX_RES_NUMS		8
#define MAX_SCOPE_NUMS		100


// device type
#define ODT_UNKNOWN     	0
#define ODT_NVT         	1
#define ODT_NVD         	2
#define ODT_NVS         	3
#define ODT_NVA         	4

// device flag
#define FLAG_MANUAL			(1 << 0)	/* manual added device, other auto discovery devices */


/***************************************************************************************/
typedef enum 
{   
    VIDEO_ENCODING_JPEG = 0, 
    VIDEO_ENCODING_MPEG4 = 1, 
    VIDEO_ENCODING_H264 = 2
} E_VIDEO_ENCODING;

typedef enum 
{   
    AUDIO_ENCODING_G711 = 0,
    AUDIO_ENCODING_G726,
    AUDIO_ENCODING_AAC
} E_AUDIO_ENCODING;

typedef enum 
{
    H264_PROFILE_Baseline = 0, 
    H264_PROFILE_Main = 1, 
    H264_PROFILE_Extended = 2, 
    H264_PROFILE_High = 3
} E_H264_PROFILE;

typedef enum 
{
    MPEG4_PROFILE_SP = 0, 
    MPEG4_PROFILE_ASP = 1
} E_MPEG4_PROFILE;

typedef enum 
{
	CAP_CATEGORY_INVALID = -1,
	CAP_CATEGORY_MEDIA,
	CAP_CATEGORY_DEVICE,
	CAP_CATEGORY_ANALYTICS,
	CAP_CATEGORY_EVENTS,
	CAP_CATEGORY_IMAGE,
	CAP_CATEGORY_PTZ,
	CAP_CATEGORY_ALL
} E_CAP_CATEGORY;

typedef enum
{
	PTZ_STA_IDLE,
	PTZ_STA_MOVING,
	PTZ_STA_UNKNOWN
} E_PTZ_STATUS;


/***************************************************************************************/
typedef struct
{
    int     port;           // onvif port
    char    host[24];       // ip of xaddrs
    char    url[128];       // /onvif/device_service
} ONVIF_XADDR;

typedef struct
{
    uint32	ip;             // msg recv from
    int		type;           // device type
    char	EndpointReference[100];
	
    ONVIF_XADDR xaddr;
} DEVICE_BINFO;

typedef struct
{
    char 	Manufacturer[32];
    char 	Model[32];
    char 	FirmwareVersion[32];
    char 	SerialNumber[32];
    char 	HardwareId[32];
} DEVICE_INFO;

/***************************************************************************************
 *
 * ONVIF capabilities structure
 *
****************************************************************************************/
typedef struct
{
    short  year;
    short  month;
    short  day;
    short  hour;
    short  minute;
    short  second;
} DATETIME;

typedef struct
{
	float min;
	float max;
} FLOAT_RANGE;

/* device capabilities */
typedef struct
{
	// network capabilities
	uint32	IPFilter 			: 1; // Indicates support for IP filtering
	uint32	ZeroConfiguration 	: 1; // Indicates support for zeroconf
	uint32	IPVersion6			: 1; // Indicates support for IPv6
	uint32 	DynDNS 				: 1; // Indicates support for dynamic DNS configuration
	uint32  Dot11Configuration  : 1; // Indicates support for IEEE 802.11 configuration
	uint32  HostnameFromDHCP    : 1; // Indicates support for retrieval of hostname from DHCP
	uint32  DHCPv6              : 1; // Indicates support for Stateful IPv6 DHCP

	// system capabilities
	uint32 	DiscoveryResolve 	: 1; // Indicates support for WS Discovery resolve requests
	uint32 	DiscoveryBye 		: 1; // Indicates support for WS-Discovery Bye
	uint32 	RemoteDiscovery 	: 1; // Indicates support for remote discovery
	uint32 	SystemBackup 		: 1; // Indicates support for system backup through MTOM
	uint32 	SystemLogging 		: 1; // Indicates support for retrieval of system logging through MTOM
	uint32 	FirmwareUpgrade 	: 1; // Indicates support for firmware upgrade through MTOM
	uint32  HttpFirmwareUpgrade : 1; // Indicates support for system backup through MTOM
	uint32  HttpSystemBackup    : 1; // Indicates support for system backup through HTTP
	uint32  HttpSystemLogging   : 1; // Indicates support for retrieval of system logging through HTTP
	uint32  HttpSupportInformation : 1; // Indicates support for retrieving support information through HTTP

    // scurity capabilities
    uint32  TLS10               : 1; // Indicates support for TLS 1.0
    uint32  TLS11               : 1; // Indicates support for TLS 1.1
    uint32  TLS12               : 1; // Indicates support for TLS 1.2
    uint32  OnboardKeyGeneration: 1; // Indicates support for onboard key generation
    uint32  AccessPolicyConfig  : 1; // Indicates support for access policy configuration
    uint32  DefaultAccessPolicy : 1; // Indicates support for the ONVIF default access policy
    uint32  Dot1X               : 1; // Indicates support for IEEE 802.1X configuration
    uint32  RemoteUserHandling  : 1; // Indicates support for remote user configuration. Used when accessing another device
    uint32  X509Token           : 1; // Indicates support for WS-Security X.509 token
    uint32  SAMLToken           : 1; // Indicates support for WS-Security SAML token
    uint32  KerberosToken       : 1; // Indicates support for WS-Security Kerberos token
    uint32  UsernameToken       : 1; // Indicates support for WS-Security Username token
    uint32  HttpDigest          : 1; // Indicates support for WS over HTTP digest authenticated communication layer
    uint32  RELToken            : 1; // Indicates support for WS-Security REL token
    
	uint32	reserved			: 1;
    
	int     Dot1XConfigurations;    // Indicates the maximum number of Dot1X configurations supported by the device
	int     NTP;                    // Maximum number of NTP servers supported by the devices SetNTP command
	int     SupportedEAPMethods;    // EAP Methods supported by the device. 
	                                // The int values refer to the <a href="http://www.iana.org/assignments/eap-numbers/eap-numbers.xhtml">IANA EAP Registry</a>
	int     MaxUsers;               // The maximum number of users that the device supports

	// misc capabilities
    char    AuxiliaryCommands[100]; // Lists of commands supported by SendAuxiliaryCommand
    
    ONVIF_XADDR xaddr;
} DEVICE_CAP;

/* media capabilities */
typedef struct
{
    uint32  SnapshotUri         : 1; // Indicates if GetSnapshotUri is supported
    uint32  Rotation            : 1; // Indicates whether or not Rotation feature is supported
    uint32  VideoSourceMode     : 1; // Indicates the support for changing video source mode
    uint32  OSD                 : 1; // Indicates if OSD is supported
	uint32	RTPMulticast 		: 1; // Indicates support for RTP multicast
	uint32	RTP_TCP				: 1; // Indicates support for RTP over TCP
	uint32	RTP_RTSP_TCP		: 1; // Indicates support for RTP/RTSP/TCP
	uint32  NonAggregateControl : 1; // Indicates support for non aggregate RTSP control
	uint32  NoRTSPStreaming     : 1; // Indicates the device does not support live media streaming via RTSP
	uint32  support				: 1; // Indication if the device supports media service
	uint32	reserved			: 22;

	int     MaximumNumberOfProfiles; // Maximum number of profiles supported
	
    ONVIF_XADDR xaddr;
} MEDIA_CAP;

/* PTZ capabilities */
typedef struct
{
    uint32  EFlip               : 1; // Indicates whether or not EFlip is supported
    uint32  Reverse             : 1; // Indicates whether or not reversing of PT control direction is supported
    uint32  GetCompatibleConfigurations : 1; // Indicates support for the GetCompatibleConfigurations command
    uint32  support				: 1; // Indication if the device supports ptz service
	uint32	reserved 			: 28;
	
    ONVIF_XADDR xaddr;
} PTZ_CAP;

/* event capabilities */
typedef struct
{
	uint32	WSSubscriptionPolicySupport	: 1; // Indicates that the WS Subscription policy is supported
	uint32	WSPullPointSupport 	: 1; // Indicates that the WS Pull Point is supported
	uint32	WSPausableSubscriptionManagerInterfaceSupport : 1; // Indicates that the WS Pausable Subscription Manager Interface is supported
	uint32  PersistentNotificationStorage : 1; // Indication if the device supports persistent notification storage
	uint32  support				: 1; // Indication if the device supports events service
	uint32	reserved 			: 27;

	int     MaxNotificationProducers; // Maximum number of supported notification producers as defined by WS-BaseNotification
	int     MaxPullPoints;            // Maximum supported number of notification pull points
	
    ONVIF_XADDR xaddr;
} EVENT_CAP;

/* image capabilities */
typedef struct
{
    uint32  ImageStabilization  : 1; // Indicates whether or not Image Stabilization feature is supported
	uint32  support				: 1; // Indication if the device supports image service
	uint32	reserved 			: 30;
	
    ONVIF_XADDR xaddr;
} IMAGE_CAP;

/* recording capabilities */
typedef struct
{
    uint32  ReceiverSource      : 1;
    uint32  MediaProfileSource  : 1;
    uint32  DynamicRecordings   : 1; // Indication if the device supports dynamic creation and deletion of recordings
    uint32  DynamicTracks       : 1; // Indication if the device supports dynamic creation and deletion of tracks
    uint32  Options             : 1; // Indication if the device supports the GetRecordingOptions command
    uint32  MetadataRecording   : 1; // Indication if the device supports recording metadata
    uint32  JPEG                : 1; // Indication if supports JPEG encoding
    uint32  MPEG4               : 1; // Indication if supports MPEG4 encoding
    uint32  H264                : 1; // Indication if supports H264 encoding
    uint32  G711                : 1; // Indication if supports G711 encoding
    uint32  G726                : 1; // Indication if supports G726 encoding
    uint32  AAC                 : 1; // Indication if supports AAC encoding
    uint32  support				: 1; // Indication if the device supports recording service
	uint32	reserved 			: 19;

	uint32  MaxStringLength;
	float   MaxRate;            // Maximum supported bit rate for all tracks of a recording in kBit/s
	float   MaxTotalRate;       // Maximum supported bit rate for all recordings in kBit/s.
    int     MaxRecordings;      // Maximum number of recordings supported.
    int     MaxRecordingJobs;   // Maximum total number of supported recording jobs by the device
	
	ONVIF_XADDR xaddr;
} RECORDING_CAP;

/* search capabilities */
typedef struct
{
    uint32  MetadataSearch      : 1;
    uint32  GeneralStartEvents  : 1; // Indicates support for general virtual property events in the FindEvents method
    uint32  support				: 1; // Indication if the device supports search service
	uint32	reserved 			: 29;
	
	ONVIF_XADDR xaddr;
} SEARCH_CAP;

/* replay capabilities */
typedef struct
{
    uint32  ReversePlayback     : 1; // Indicator that the Device supports reverse playback as defined in the ONVIF Streaming Specification
    uint32  RTP_RTSP_TCP        : 1; // Indicates support for RTP/RTSP/TCP
    uint32  support				: 1; // Indication if the device supports replay service
	uint32	reserved 			: 29;

	FLOAT_RANGE SessionTimeoutRange; // The minimum and maximum valid values supported as session timeout in seconds
	
	ONVIF_XADDR xaddr;
} REPLAY_CAP;

typedef struct
{
	DEVICE_CAP	    device;     // The capabilities for the device service is returned in the Capabilities element
    MEDIA_CAP	    media;      // The capabilities for the media service is returned in the Capabilities element
    PTZ_CAP		    ptz;        // The capabilities for the PTZ service is returned in the Capabilities element
	EVENT_CAP	    events;     // The capabilities for the event service is returned in the Capabilities element
	IMAGE_CAP	    image;      // The capabilities for the imaging service is returned in the Capabilities element
	RECORDING_CAP   recording;  // The capabilities for the recording service is returned in the Capabilities element
	SEARCH_CAP      search;     // The capabilities for the search service is returned in the Capabilities element
	REPLAY_CAP      replay;     // The capabilities for the replay service is returned in the Capabilities element
} ONVIF_CAP;

/***************************************************************************************/
typedef struct
{
	int w;
	int h;
} RESOLUTION;

typedef struct
{
	int frame_rate_min;
	int frame_rate_max;
	int encoding_interval_min;
	int encoding_interval_max;

	RESOLUTION	resolution[MAX_RES_NUMS];
} JPEG_CFG_OPT;

typedef struct
{
	int sp_profile 	: 1;
	int asp_profile : 1;
	int reserverd	: 30;
	
	int gov_length_min;
	int gov_length_max;
	int frame_rate_min;
	int frame_rate_max;
	int encoding_interval_min;
	int encoding_interval_max;

	RESOLUTION	resolution[MAX_RES_NUMS];
} MPEG4_CFG_OPT;

typedef struct
{
	int baseline_profile	: 1;
	int main_profile 		: 1;
	int extended_profile 	: 1;
	int high_profile 		: 1;
	int reserverd			: 28;
	
	int gov_length_min;
	int gov_length_max;
	int frame_rate_min;
	int frame_rate_max;
	int encoding_interval_min;
	int encoding_interval_max;

	RESOLUTION	resolution[MAX_RES_NUMS];	
} H264_CFG_OPT;

typedef struct 
{
	JPEG_CFG_OPT 	jpeg_opt;
	MPEG4_CFG_OPT	mpeg4_opt;
	H264_CFG_OPT	h264_opt;
	
	int				quality_min;
	int				quality_max;
} VIDEO_ENC_CFG;

typedef struct
{
	char	ip[32];
	int 	port;
	int 	ttl;
	BOOL 	auto_start;
} MULTICAST_CFG;

typedef struct
{
	int	BacklightCompensation_Mode;	// 0-OFF, 1-ON
	int Brightness;
	int ColorSaturation;
	int Contrast;
	int	Exposure_Mode;				// 0-AUTO, 1-MANUAL			
	int MinExposureTime;
	int MaxExposureTime;
	int MinGain;
	int MaxGain;
	int IrCutFilter_Mode;			// 0-OFF, 1-ON, 2-AUTO
	int Sharpness;
	int	WideDynamicRange_Mode;		// 0-OFF, 1-ON
	int WideDynamicRange_Level;
	int	WhiteBalance_Mode;			// 0-AUTO, 1-MANUAL
} IMAGE_CFG;

typedef struct
{
	unsigned int 	BacklightCompensation_ON	: 1;
	unsigned int 	BacklightCompensation_OFF	: 1;
	unsigned int    Exposure_AUTO				: 1;
	unsigned int    Exposure_MANUAL				: 1;
	unsigned int 	AutoFocus_AUTO				: 1;
	unsigned int 	AutoFocus_MANUAL			: 1;
	unsigned int 	IrCutFilter_ON				: 1;
	unsigned int 	IrCutFilter_OFF				: 1;
	unsigned int 	IrCutFilter_AUTO			: 1;
	unsigned int 	WideDynamicRange_ON			: 1;
	unsigned int 	WideDynamicRange_OFF		: 1;
	unsigned int 	WhiteBalance_AUTO			: 1;
	unsigned int 	WhiteBalance_MANUAL			: 1;
	
	unsigned int	reserved     				: 19;

	int				Brightness_min;
	int				Brightness_max;
	int				ColorSaturation_min;
	int				ColorSaturation_max;
	int				Contrast_min;
	int				Contrast_max;
	int				MinExposureTime_min;
	int				MinExposureTime_max;
	int				MaxExposureTime_min;
	int				MaxExposureTime_max;
	int				MinGain_min;
	int				MinGain_max;
	int				MaxGain_min;
	int				MaxGain_max;
	int				Sharpness_min;
	int				Sharpness_max;
	int				WideDynamicRange_Level_min;
	int				WideDynamicRange_Level_max;
} IMAGE_OPTIONS;

typedef struct _VIDEO_SRC
{
    struct _VIDEO_SRC * next;
	
    int     width;
    int     height;
    int     frame_rate;
    
    char    token[ONVIF_TOKEN_LEN];

    IMAGE_CFG img_cfg;
} VIDEO_SRC;

typedef struct _AUDIO_SRC
{
    struct _AUDIO_SRC * next;
	
    int     channels;
    
    char    token[ONVIF_TOKEN_LEN];
} AUDIO_SRC;

typedef struct _VIDEO_SRC_CFG
{
	struct _VIDEO_SRC_CFG * next;
	
    // Bounds   
    int  width;
    int  height;
    int  x;
    int  y;
    
    int  use_count;
    char token[ONVIF_TOKEN_LEN];
    char name[ONVIF_NAME_LEN];
    char source_token[ONVIF_TOKEN_LEN]; 
} VIDEO_SRC_CFG;

typedef struct _VIDEO_ENCODER
{    
	struct _VIDEO_ENCODER * next;	

	char name[ONVIF_NAME_LEN];
    char token[ONVIF_TOKEN_LEN];

	int  use_count;
	
    int  width;
    int  height;
    
	int  quality;
    int  session_timeout;
    int  framerate_limit;
	int  encoding_interval;
	int  bitrate_limit;
	
	E_VIDEO_ENCODING encoding;
	
	int  gov_len;
	int  profile;

	MULTICAST_CFG	 multicast;	
} VIDEO_ENCODER;

typedef struct _AUDIO_SRC_CFG
{    
	struct _AUDIO_SRC_CFG * next;
	
    int  use_count;
    char token[ONVIF_TOKEN_LEN];
	char name[ONVIF_NAME_LEN];
    char source_token[ONVIF_TOKEN_LEN]; 
} AUDIO_SRC_CFG;

typedef struct _AUDIO_ENCODER
{
	struct _AUDIO_ENCODER * next;
	
    int  use_count;
    int  session_timeout;
    
    char name[ONVIF_NAME_LEN];
    char token[ONVIF_TOKEN_LEN];
    
	int  sample_rate;
	int  bitrate;    
	
	E_AUDIO_ENCODING encoding;

	MULTICAST_CFG	 multicast;
} AUDIO_ENCODER;

typedef struct
{
    BOOL    used_flag;
    
    char	name[ONVIF_NAME_LEN];
    char	token[ONVIF_TOKEN_LEN];

    float 	pantilt_pos_x;
	float 	pantilt_pos_y;
	float   zoom_pos;
} PTZ_PRESET;

typedef struct
{
	FLOAT_RANGE abs_pantilt_x;
	FLOAT_RANGE abs_pantilt_y;
	FLOAT_RANGE abs_zoom;
	FLOAT_RANGE rel_pantilt_x;
	FLOAT_RANGE rel_pantilt_y;
	FLOAT_RANGE rel_zoom;
	FLOAT_RANGE con_pantilt_x;
	FLOAT_RANGE con_pantilt_y;
	FLOAT_RANGE con_zoom;
	FLOAT_RANGE pantile_speed;
	FLOAT_RANGE zoom_speed;
	FLOAT_RANGE timeout_range;
} PTZ_CFG_OPT;

typedef struct _PTZ_CFG
{
	struct _PTZ_CFG * next;

	int 	use_count;
	
	char	name[ONVIF_NAME_LEN];
    char	token[ONVIF_TOKEN_LEN];
	char	node_token[ONVIF_TOKEN_LEN];
	
	float 	def_pantilt_speed_x;
	float 	def_pantilt_speed_y;
	float 	def_zoom_speed;
	uint32 	def_timeout;

	FLOAT_RANGE pantilt_limits_x;
	FLOAT_RANGE pantilt_limits_y;
	FLOAT_RANGE zoom_limits;	

	PTZ_CFG_OPT	 ptz_cfg_opt;
} PTZ_CFG;

typedef struct _PTZ_NODE
{
	struct _PTZ_NODE * next;

	unsigned int fixed_home_pos 		: 1;
	unsigned int abs_pantilt_space 		: 1;
	unsigned int abs_zoom_space 		: 1;
	unsigned int rel_pantilt_space 		: 1;
	unsigned int rel_zoom_space 		: 1;	
	unsigned int con_pantilt_space 		: 1;
	unsigned int con_zoom_space 		: 1;
	unsigned int pantile_speed_space	: 1;
	unsigned int zoom_speed_space		: 1;
	unsigned int home_support			: 1;

	unsigned int reserved				: 23;

	char 		 name[ONVIF_NAME_LEN];
    char         token[ONVIF_TOKEN_LEN];

	FLOAT_RANGE abs_pantilt_x;
	FLOAT_RANGE abs_pantilt_y;
	FLOAT_RANGE abs_zoom;
	FLOAT_RANGE rel_pantilt_x;
	FLOAT_RANGE rel_pantilt_y;
	FLOAT_RANGE rel_zoom;
	FLOAT_RANGE con_pantilt_x;
	FLOAT_RANGE con_pantilt_y;
	FLOAT_RANGE con_zoom;
	FLOAT_RANGE pantile_speed;
	FLOAT_RANGE zoom_speed;	
} PTZ_NODE;

typedef struct
{
	float pantilt_pos_x;
	float pantilt_pos_y;
	float zoom_pos;
	
	E_PTZ_STATUS move_sta;
	E_PTZ_STATUS zoom_sta;

	char  error[200];
	char  utc_time[64];
} PTZ_STATUS;


typedef struct _ONVIF_PROFILE
{
    struct _ONVIF_PROFILE * next;
    
    VIDEO_SRC_CFG * video_src_cfg;
    VIDEO_ENCODER * video_enc;
	AUDIO_SRC_CFG * audio_src_cfg;
	AUDIO_ENCODER * audio_enc;
	PTZ_NODE      * ptz_node;
	PTZ_CFG	      * ptz_cfg;

	PTZ_PRESET 	  presets[MAX_PTZ_PRESETS];
	
	char 	name[ONVIF_NAME_LEN];
    char 	token[ONVIF_TOKEN_LEN];
    char 	stream_uri[ONVIF_URI_LEN];
    BOOL 	fixed;
    
} ONVIF_PROFILE;

typedef struct _ONVIF_NET_INF
{	
	struct _ONVIF_NET_INF * next;

	uint32	enabled 		: 1;
	uint32	ipv4_enabled 	: 1;
	uint32	fromdhcp		: 1;
	uint32  reserved		: 29;
	
	char 	name[ONVIF_NAME_LEN];
    char 	token[ONVIF_TOKEN_LEN];
	char 	hwaddr[32];
	char 	ipv4_addr[32];

	int	 	mtu;	
	int	 	prefix_len;
} ONVIF_NET_INF;

typedef struct
{
	int		type;	// 0 - NoUpdate, 1 - ClientUpdates, 2 - ServerUpdates
	int		ttl;
	char	name[ONVIF_NAME_LEN];
} DDNS_INFO;

typedef struct
{
	BOOL	fromdhcp;
	char	server[MAX_NTP_SERVER][32];
} NTP_INFO;

typedef struct
{
	BOOL	fromdhcp;
	char	searchdomain[MAX_SEARCHDOMAIN][64];
	char	server[MAX_DNS_SERVER][32];
} DNS_INFO;

typedef struct
{
	BOOL	fromdhcp;
	char    name[ONVIF_NAME_LEN];
} HOSTNAME;

typedef struct
{
	uint32 	http_support	: 1;
	uint32 	http_enable		: 1;
	uint32 	https_support	: 1;
	uint32 	https_enable	: 1;
	uint32 	rtsp_support	: 1;
	uint32 	rtsp_enable		: 1;
	uint32  reserved		: 26;
	
	int		http_port[MAX_SERVER_PORT];
	int		https_port[MAX_SERVER_PORT];
	int		rtsp_port[MAX_SERVER_PORT];
} NETPROTOCOL;

typedef struct 
{
    BOOL            discoverable;
	char			gateway[MAX_GATEWAY][32];
	
	DDNS_INFO	 	ddns;
	NTP_INFO	 	ntp;
	DNS_INFO	 	dns;
	HOSTNAME	 	hostname;
	NETPROTOCOL	 	protocols;

	ONVIF_NET_INF * interfaces;
} ONVIF_NET;

typedef struct _SIMPLE_ITEM
{
    struct _SIMPLE_ITEM * next;

    char    name[64];
    char    value[64];
} SIMPLE_ITEM;

typedef struct _ONVIF_NOTIFY
{
    struct _ONVIF_NOTIFY * next;
    
    char	reference_addr[256];
    char    producter_addr[256];
    char    topic[256];
    char    operation[100];
    char    utctime[64];
    time_t  arrival_time;
    
    SIMPLE_ITEM * source;
    SIMPLE_ITEM * key;
    SIMPLE_ITEM * data;
} ONVIF_NOTIFY;

typedef struct
{
    BOOL            subscribe;              // event subscribed flag
    BOOL			event_timer_run;		// event running flag
    
    HTTPSRV         http_srv;
    char            reference_addr[256];    // event comsumer address
    char            producter_addr[256];    // event producter address

    int             init_term_time;
    uint32 	   		timer_id;

    unsigned int    notify_nums;
    ONVIF_NOTIFY  * notify;
} ONVIF_EVENT;

typedef enum 
{
	RecordingStatus__Initiated = 0, 
	RecordingStatus__Recording = 1, 
	RecordingStatus__Stopped = 2, 
	RecordingStatus__Removing = 3, 
	RecordingStatus__Removed = 4, 
	RecordingStatus__Unknown = 5 
} RECORDINGSTATUS;

typedef enum 
{ 
	TrackType__Video = 0, 
	TrackType__Audio = 1, 
	TrackType__Metadata = 2, 
	TrackType__Extended = 3 
} TRACKTYPE;

typedef enum
{
	SearchState__Queued = 0, 							// The search is queued and not yet started.
	SearchState__Searching = 1,							// The search is underway and not yet completed
	SearchState__Completed = 2,							// The search has been completed and no new results will be found
	SearchState__Unknown = 3							// The state of the search is unknown. (This is not a valid response from GetSearchState.)
} SEARCHSTATUS;

typedef struct
{
	char 			TrackToken[ONVIF_TOKEN_LEN];		// required, 
	TRACKTYPE 		TrackType;							// required, Type of the track: "Video", "Audio" or "Metadata". The track shall only be able to hold data of that type
	char 			Description[100];					// required, Informative description of the contents of the track
	time_t 			DataFrom;							// required, The start date and time of the oldest recorded data in the track
	time_t 			DataTo;								// required, The stop date and time of the newest recorded data in the track
} TRACK;

typedef struct _TRACK_LIST
{
	struct _TRACK_LIST * next;
	TRACK 		    track;
} TRACK_LIST;

typedef struct
{
	char 			RecordingToken[ONVIF_TOKEN_LEN];	// required 

	char 			SourceId[128]; 						// required, Identifier for the source chosen by the client that creates the structure
	char			Name[20];							// required, Informative user readable name of the source, e.g. "Camera23". A device shall support at least 20 characters.
	char			Location[100];						// required, Informative description of the physical location of the source, e.g. the coordinates on a ma
	char			Description[100];					// required, Informative description of the source
	char			Address[256];						// required, URI provided by the service supplying data to be recorded. A device shall support at least 128 characters
		
	time_t 			EarliestRecording;					// optional  
	time_t 			LatestRecording;					// optional  
	
	char 			Content[100];						// required

	RECORDINGSTATUS	RecordingStatus;

	TRACK_LIST    * Track;								// Basic information about the track. Note that a track may represent a single contiguous time span or consist of multiple slices
} RECORDING;

typedef struct _RECORDING_LIST
{
	struct _RECORDING_LIST * next;
	
	RECORDING		RecordingInformation;
} RECORDING_LIST;

typedef struct 
{
	int 			Bitrate;							// optional, Average bitrate in kbps
	int 			Width;								// required, The width of the video in pixels
	int 			Height;								// required, The height of the video in pixels
	E_VIDEO_ENCODING Encoding;							// required, Used video codec, either Jpeg, H.264 or Mpeg4
	float 			Framerate;							// required, Average framerate in frames per second
} VIDEO_ATTR;

typedef struct
{
	int 			Bitrate;							// optional, The bitrate in kbps
	E_AUDIO_ENCODING Encoding;							// required, Audio codec used for encoding the audio (either G.711, G.726 or AAC)
	int 			Samplerate;							// required, The sample rate in kHz
} AUDIO_ATTR;

typedef struct
{
	BOOL 			CanContainPTZ;						// required, Indicates that there can be PTZ data in the metadata track in the specified time interval
	BOOL 			CanContainAnalytics;				// required, Indicates that there can be analytics data in the metadata track in the specified time interval
	BOOL 			CanContainNotifications;			// required, Indicates that there can be notifications in the metadata track in the specified time interval
} METADATA_ATTR;

typedef struct
{
	TRACK    		TrackInformation;					// required, The basic information about the track. Note that a track may represent a single contiguous time span or consist of multiple slices
	VIDEO_ATTR 		VideoAttributes;					// optional, If the track is a video track, exactly one of this structure shall be present and contain the video attributes
	AUDIO_ATTR 		AudioAttributes;					// optional, If the track is an audio track, exactly one of this structure shall be present and contain the audio attributes
	METADATA_ATTR 	MetadataAttributes;					// optional, If the track is an metadata track, exactly one of this structure shall be present and contain the metadata attributes
} TRACK_ATTR;

typedef struct _TRACK_ATTR_LIST
{
	struct _TRACK_ATTR_LIST * next;
	TRACK_ATTR		attr;
} TRACK_ATTR_LIST;

typedef struct 
{
	char 			RecordingToken[ONVIF_TOKEN_LEN];	// required, A reference to the recording that has these attributes
	TRACK_ATTR_LIST * TrackAttributes; 					// optional, A set of attributes for each track
	time_t 			From;								// required, The attributes are valid from this point in time in the recording
	time_t 			Until;								// required, The attributes are valid until this point in time in the recording. Can be equal to 'From' to indicate that the attributes are only known to be valid for this particular point in time
} MEDIA_ATTR;

typedef struct
{
	SEARCHSTATUS	SearchState;						// required, The state of the search when the result is returned. Indicates if there can be more results, or if the search is completed
	RECORDING_LIST* RecordingInformation;				// optional, A RecordingInformation structure for each found recording matching the search
} RECORDING_RESULT;

typedef struct
{
	int				flags;      						// FLAG_MANUAL mean added manual, other auto discovery device
	int				state;      						// 0 -- offline; 1 -- online
	int				no_res_nums;						// probe not response numbers, when > 2 means offline
	unsigned int    local_ip;   						// local ip address to connect to server, network byte order
	
    DEVICE_BINFO    binfo;
    DEVICE_INFO  	dev_info;

    ONVIF_CAP   	capablity;

	VIDEO_ENC_CFG	video_enc_cfg;

	VIDEO_SRC       * video_src;
	AUDIO_SRC       * audio_src;
    ONVIF_PROFILE   * profiles;
	VIDEO_SRC_CFG   * video_src_cfg;
	AUDIO_SRC_CFG   * audio_src_cfg;
	VIDEO_ENCODER   * video_enc;
	AUDIO_ENCODER   * audio_enc;
	PTZ_NODE        * ptznodes;
	PTZ_CFG		    * ptz_cfg;
	
	ONVIF_NET       network;
	ONVIF_EVENT     events;
	
	// request
	char 			username[32];
	char 			password[32];
    
    ONVIF_PROFILE * curProfile;
    void          * p_user;

    //lc add to manager the rtsp sessions
    CRtsp         * rtsp_main_session;
    CRtsp         * rtsp_sec_session;
} ONVIF_DEVICE;

/*************************************************************************/
typedef enum onvif_action_e
{
	eActionNull = 0,
		
	eGetCapabilities,
	eGetServices,
	eGetDeviceInformation,
	eGetNetworkInterfaces,
	eSetNetworkInterfaces,
	eGetNTP,
	eSetNTP,
	eGetHostname,
	eSetHostname,
	eSetHostnameFromDHCP,
	eGetDNS,
	eSetDNS,
	eGetDynamicDNS,
	eSetDynamicDNS,
	eGetNetworkProtocols,
	eSetNetworkProtocols,
	eGetDiscoveryMode,
	eSetDiscoveryMode,
	eGetNetworkDefaultGateway,
	eSetNetworkDefaultGateway,	
	eGetSystemDateAndTime,
	eSetSystemDateAndTime,
	eSystemReboot,
	eSetSystemFactoryDefault,
	eGetSystemLog,
	eGetScopes,
	eSetScopes,
	eAddScopes,
	eRemoveScopes,

	eGetVideoSources,
	eGetAudioSources,
	eCreateProfile,
	eGetProfile,
	eGetProfiles,
	eAddVideoEncoderConfiguration,
	eAddVideoSourceConfiguration,
	eAddAudioEncoderConfiguration,
	eAddAudioSourceConfiguration,
	eAddPTZConfiguration,
	eRemoveVideoEncoderConfiguration,
	eRemoveVideoSourceConfiguration,
	eRemoveAudioEncoderConfiguration,
	eRemoveAudioSourceConfiguration,
	eRemovePTZConfiguration,
	eDeleteProfile,
	eGetVideoSourceConfigurations,
	eGetVideoEncoderConfigurations,
	eGetAudioSourceConfigurations,
	eGetAudioEncoderConfigurations,
	eGetVideoSourceConfiguration,
	eGetVideoEncoderConfiguration,
	eGetAudioSourceConfiguration,
	eGetAudioEncoderConfiguration,
	eSetVideoSourceConfiguration,
	eSetVideoEncoderConfiguration,
	eSetAudioSourceConfiguration,
	eSetAudioEncoderConfiguration,
	eGetVideoSourceConfigurationOptions,
	eGetVideoEncoderConfigurationOptions,
	eGetAudioSourceConfigurationOptions,
	eGetAudioEncoderConfigurationOptions,	
	eGetStreamUri,	
	eSetSynchronizationPoint,
	eGetSnapshotUri,

	eGetNodes,
	eGetNode,
	eGetPresets,
	eSetPreset,
	eRemovePreset,
	eGotoPreset,
	eGotoHomePosition,
	eSetHomePosition,
	eGetStatus,
	eContinuousMove,
	eRelativeMove,
	eAbsoluteMove,
	ePTZStop,
	eGetConfigurations,
	eGetConfiguration,
	eSetConfiguration,	
	eGetConfigurationOptions,	

	eGetEventProperties,
	eRenew,
	eUnsubscribe,
	eSubscribe,
	
	eGetImagingSettings,
	eSetImagingSettings,
	eGetOptions,

	eGetReplayUri,

	eGetRecordingSummary,
	eGetRecordingInformation,
	eGetMediaAttributes,
	eFindRecordings,
	eGetRecordingSearchResults,
	eFindEvents,
	eGetEventSearchResults,
	eGetSearchState,
	eEndSearch,
	
	eActionMax
}eOnvifAction;

typedef struct onvif_action_s
{
	eOnvifAction	type;
	char			action_url[256];
} OVFACTS;

/*************************************************************************
 *
 * onvif request structure
 *
**************************************************************************/ 
typedef struct
{
	E_CAP_CATEGORY	Category; 	// optional, List of categories to retrieve capability information on
} GetCapabilities_REQ;

typedef struct
{
    BOOL IncludeCapability; 	// required, Indicates if the service capabilities (untyped) should be included in the response.
} GetServices_REQ;

typedef struct
{
	int	dummy;
} GetDeviceInformation_REQ;

typedef struct
{
	int	dummy;
} GetNetworkInterfaces_REQ;

typedef struct
{
    ONVIF_NET_INF net_inf;
} SetNetworkInterfaces_REQ;

typedef struct
{
	int	dummy;
} GetNTP_REQ;

typedef struct 
{
	NTP_INFO	ntp;
} SetNTP_REQ;

typedef struct
{
	int	dummy;
} GetHostname_REQ;

typedef struct
{
	char	Name[128];	// required, The hostname to set
} SetHostname_REQ;

typedef struct
{
    BOOL    FromDHCP;	//  required, True if the hostname shall be obtained via DHCP
} SetHostnameFromDHCP_REQ;

typedef struct
{
	int	dummy;
} GetDNS_REQ;

typedef struct 
{
	DNS_INFO	dns;
} SetDNS_REQ;

typedef struct
{
	DDNS_INFO	ddns;
} GetDynamicDNS_REQ;

typedef struct
{
    DDNS_INFO	ddns;
} SetDynamicDNS_REQ;

typedef struct
{
	int	dummy;
} GetNetworkProtocols_REQ;

typedef struct
{
	NETPROTOCOL	protocols;
} SetNetworkProtocols_REQ;

typedef struct
{
	int	dummy;
} GetDiscoveryMode_REQ;

typedef struct
{
	BOOL	DiscoveryMode;	// required, Indicator of discovery mode: Discoverable, NonDiscoverable
} SetDiscoveryMode_REQ;

typedef struct
{
	int	dummy;
} GetNetworkDefaultGateway_REQ;

typedef struct
{
	char 	IPv4Address[MAX_GATEWAY][32];	// optional, Sets IPv4 gateway address used as default setting
} SetNetworkDefaultGateway_REQ;

typedef struct
{
	int	dummy;
} GetSystemDateAndTime_REQ;

typedef struct
{
	int    type;   /* 0 : Manual, 1 : NTP*/
    BOOL   DaylightSavings;
    char   TZ[32];

    /*UTC Time*/
    DATETIME    datetime;
} SetSystemDateAndTime_REQ;

typedef struct
{
	int	dummy;
} SystemReboot_REQ;

typedef struct
{
	int		type;		// 0 -- soft reset; 1 -- hard reset
} SetSystemFactoryDefault_REQ;

typedef struct
{
    int     LogType;    // required, Specifies the type of system log to get
    					// 0 - System, 1 - Access
} GetSystemLog_REQ;

typedef struct
{
	int	dummy;
} GetScopes_REQ;

typedef struct
{
	int	dummy;
} SetScopes_REQ;

typedef struct
{
	int	dummy;
} AddScopes_REQ;

typedef struct
{
	int	dummy;
} RemoveScopes_REQ;


typedef struct
{
	int	dummy;
} GetVideoSources_REQ;

typedef struct
{
	int	dummy;
} GetAudioSources_REQ;

typedef struct
{
    char    Name[ONVIF_NAME_LEN];			// required, friendly name of the profile to be created
    char    Token[ONVIF_TOKEN_LEN];			// optional, Optional token, specifying the unique identifier of the new profile
} CreateProfile_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, this command requests a specific profile
} GetProfile_REQ;

typedef struct
{
	int	dummy;
} GetProfiles_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Reference to the profile where the configuration should be added
    char    ConfigurationToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the VideoEncoderConfiguration to add
} AddVideoEncoderConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Reference to the profile where the configuration should be added
    char    ConfigurationToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the VideoEncoderConfiguration to add
} AddVideoSourceConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Reference to the profile where the configuration should be added
    char    ConfigurationToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the VideoEncoderConfiguration to add
} AddAudioEncoderConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Reference to the profile where the configuration should be added
    char    ConfigurationToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the VideoEncoderConfiguration to add
} AddAudioSourceConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Reference to the profile where the configuration should be added
    char    ConfigurationToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the VideoEncoderConfiguration to add
} AddPTZConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the media profile from which the VideoEncoderConfiguration shall be removed
} RemoveVideoEncoderConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the media profile from which the VideoSourceConfiguration shall be removed
} RemoveVideoSourceConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the media profile from which the AudioEncoderConfiguration shall be removed
} RemoveAudioEncoderConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the media profile from which the AudioSourceConfiguration shall be removed
} RemoveAudioSourceConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, Contains a reference to the media profile from which the PTZConfiguration shall be removed
} RemovePTZConfiguration_REQ;

typedef struct
{
    char    ProfileToken[ONVIF_TOKEN_LEN];	// required, This element contains a  reference to the profile that should be deleted
} DeleteProfile_REQ;

typedef struct
{
	int	dummy;
} GetVideoSourceConfigurations_REQ;

typedef struct
{
	int	dummy;
} GetVideoEncoderConfigurations_REQ;

typedef struct
{
	int	dummy;
} GetAudioSourceConfigurations_REQ;

typedef struct
{
	int	dummy;
} GetAudioEncoderConfigurations_REQ;

typedef struct
{
    char    ConfigurationToken[ONVIF_TOKEN_LEN];	// required, Token of the requested video source configuration
} GetVideoSourceConfiguration_REQ;

typedef struct
{
    char    ConfigurationToken[ONVIF_TOKEN_LEN];	// required, Token of the requested video encoder configuration
} GetVideoEncoderConfiguration_REQ;

typedef struct
{
    char    ConfigurationToken[ONVIF_TOKEN_LEN];	// required, Token of the requested audio source configuration
} GetAudioSourceConfiguration_REQ;

typedef struct
{
    char    ConfigurationToken[ONVIF_TOKEN_LEN];	// required, Token of the requested audio encoder configuration
} GetAudioEncoderConfiguration_REQ;

typedef struct
{
	VIDEO_SRC_CFG	video_src_cfg;

	BOOL	persistence;
} SetVideoSourceConfiguration_REQ;

typedef struct
{
	VIDEO_ENCODER	video_enc;
	
	BOOL	persistence;
} SetVideoEncoderConfiguration_REQ;

typedef struct
{
	AUDIO_SRC_CFG	audio_src_cfg;

	BOOL	persistence;
} SetAudioSourceConfiguration_REQ;

typedef struct
{
	AUDIO_ENCODER	audio_enc;

	BOOL	persistence;
} SetAudioEncoderConfiguration_REQ;

typedef struct
{
    char    profile_token[ONVIF_TOKEN_LEN];
    char    config_token[ONVIF_TOKEN_LEN];
} GetVideoSourceConfigurationOptions_REQ;

typedef struct
{
    char    profile_token[ONVIF_TOKEN_LEN];
    char    config_token[ONVIF_TOKEN_LEN];
} GetVideoEncoderConfigurationOptions_REQ;

typedef struct
{
    char    profile_token[ONVIF_TOKEN_LEN];
    char    config_token[ONVIF_TOKEN_LEN];
} GetAudioSourceConfigurationOptions_REQ;

typedef struct
{
    char    profile_token[ONVIF_TOKEN_LEN];
    char    config_token[ONVIF_TOKEN_LEN];
} GetAudioEncoderConfigurationOptions_REQ;

typedef struct
{
	char 	profile_token[ONVIF_TOKEN_LEN];
	int  	stream_type;	/* 0: RTP-Unicast, 1: RTP-Multicast */
	int  	protocol;		/* 0: UDP 1: TCP 2: RTSP 3: HTTP */
} GetStreamUri_REQ;

typedef struct
{
    char    profile_token[ONVIF_TOKEN_LEN];
} SetSynchronizationPoint_REQ;

typedef struct
{
    char    profile_token[ONVIF_TOKEN_LEN];
} GetSnapshotUri_REQ;


typedef struct
{
	int	dummy;
} GetNodes_REQ;

typedef struct
{
	char	node_token[ONVIF_TOKEN_LEN];
} GetNode_REQ;

typedef struct
{
	char 	profile_token[ONVIF_TOKEN_LEN];
} GetPresets_REQ;

typedef struct
{
    char	profile_token[ONVIF_TOKEN_LEN];
    char	preset_token[ONVIF_TOKEN_LEN];
    char    name[ONVIF_NAME_LEN];
} SetPreset_REQ;

typedef struct
{
	char	profile_token[ONVIF_TOKEN_LEN];
    char	preset_token[ONVIF_TOKEN_LEN];
} RemovePreset_REQ;

typedef struct
{
	char	profile_token[ONVIF_TOKEN_LEN];
	char	preset_token[ONVIF_TOKEN_LEN];

	float   pantilt_speed_x;
    float   pantilt_speed_y;
    float   zoom_speed;
} GotoPreset_REQ;

typedef struct
{
    char	profile_token[ONVIF_TOKEN_LEN];

    float   pantilt_speed_x;
    float   pantilt_speed_y;
    float   zoom_speed;
} GotoHomePosition_REQ;

typedef struct
{
    char	profile_token[ONVIF_TOKEN_LEN];
} SetHomePosition_REQ;

typedef struct
{
	char	profile_token[ONVIF_TOKEN_LEN];
} GetStatus_REQ;

typedef struct
{
	char	profile_token[ONVIF_TOKEN_LEN];
	
	float	pantilt_velocity_x;
	float	pantilt_velocity_y;
	float   zoom_velocity;
	
	int		timeout;	
} ContinuousMove_REQ;

typedef struct
{
	char	profile_token[ONVIF_TOKEN_LEN];

	float	pantilt_translation_x;
	float	pantilt_translation_y;
	float   zoom_translation;
	
	float	pantilt_speed_x;
	float	pantilt_speed_y;
	float   zoom_speed;
} RelativeMove_REQ;

typedef struct
{
	char	profile_token[ONVIF_TOKEN_LEN];

	float	pantilt_position_x;
	float	pantilt_position_y;
	float   zoom_position;
	
	float	pantilt_speed_x;
	float	pantilt_speed_y;
	float   zoom_speed;
} AbsoluteMove_REQ;

typedef struct
{
	char	profile_token[ONVIF_TOKEN_LEN];

	BOOL	stop_pantile;
	BOOL	stop_zoom;
} PTZStop_REQ;

typedef struct
{
	int	dummy;
} GetConfigurations_REQ;

typedef struct
{
	char	config_token[ONVIF_TOKEN_LEN];
} GetConfiguration_REQ;

typedef struct
{
	int	dummy;
} SetConfiguration_REQ;

typedef struct
{
	char	config_token[ONVIF_TOKEN_LEN];
} GetConfigurationOptions_REQ;

typedef struct
{
	int	dummy;
} GetEventProperties_REQ;

typedef struct
{
	int		term_time;
} Renew_REQ;

typedef struct
{
	int	dummy;
} Unsubscribe_REQ;

typedef struct
{
	char	reference_addr[256];
	int		init_term_time;         // InitialTerminationTime, unit is second
} Subscribe_REQ;

typedef struct
{
	char	source_token[ONVIF_TOKEN_LEN];
} GetImagingSettings_REQ;

typedef struct
{
	IMAGE_CFG	img_cfg;
	
	char	source_token[ONVIF_TOKEN_LEN];	
	BOOL	persistence;
} SetImagingSettings_REQ;

typedef struct
{
	char	source_token[ONVIF_TOKEN_LEN];
} GetOptions_REQ;

typedef struct
{
    int  	StreamType;        	// required. Defines if a multicast or unicast stream is requested
                            	// 0 - RTP-Unicast; 1 - RTP-Multicast
    int 	TransportProtocol; 	// required. Defines the network protocol for streaming, either UDP=RTP/UDP, RTSP=RTP/RTSP/TCP or HTTP=RTP/RTSP/HTTP/TCP
                            	// 0 - UDP; 1 - TCP; 2 - RTSP; 3 - HTTP
    char 	RecordingToken[ONVIF_TOKEN_LEN]; // required. The identifier of the recording to be streamed
} GetReplayUri_REQ;

typedef struct
{
	int	dummy;
} GetRecordingSummary_REQ;

typedef struct
{
	char 	RecordingToken[ONVIF_TOKEN_LEN];		// Required
} GetRecordingInformation_REQ;

typedef struct
{
	char 	RecordingTokens[10][ONVIF_TOKEN_LEN]; 	// optional
	time_t	Time;									// required
} GetMediaAttributes_REQ;

typedef struct
{
	char	IncludedSources[10][ONVIF_TOKEN_LEN];	// optional,  A list of sources that are included in the scope. If this list is included, only data from one of these sources shall be searched
	char	IncludedRecordings[10][ONVIF_TOKEN_LEN];// optional,  A list of recordings that are included in the scope. If this list is included, only data from one of these recordings shall be searched
	char	RecordingInformationFilter[256];		// optional,  An xpath expression used to specify what recordings to search. Only those recordings with an RecordingInformation structure that matches the filter shall be searched
	int		MaxMatches;				// optional, The search will be completed after this many matches. If not specified, the search will continue until reaching the endpoint or until the session expires
	int		KeepAliveTime;			// required, The time the search session will be kept alive after responding to this and subsequent requests. A device shall support at least values up to ten seconds
} FindRecordings_REQ;

typedef struct
{	
	char	SearchToken[ONVIF_TOKEN_LEN];	// required, The search session to get results from
	int		MinResults;		// optional, The minimum number of results to return in one response
	int		MaxResults;		// optional, The maximum number of results to return in one response
	int		WaitTime;		// optional, The maximum time before responding to the request, even if the MinResults parameter is not fulfilled
} GetRecordingSearchResults_REQ;

typedef struct
{
	time_t	StartPoint;			// required, The point of time where the search will start
	time_t	EndPoint;			// optional, The point of time where the search will stop. This can be a time before the StartPoint, in which case the search is performed backwards in time
	char	IncludedSources[10][ONVIF_TOKEN_LEN];	// optional,  A list of sources that are included in the scope. If this list is included, only data from one of these sources shall be searched
	char	IncludedRecordings[10][ONVIF_TOKEN_LEN];// optional,  A list of recordings that are included in the scope. If this list is included, only data from one of these recordings shall be searched
	char	RecordingInformationFilter[256];		// optional,  An xpath expression used to specify what recordings to search. Only those recordings with an RecordingInformation structure that matches the filter shall be searched
	BOOL	IncludeStartState;	// required, Setting IncludeStartState to true means that the server should return virtual events representing the start state for any recording included in the scope. Start state events are limited to the topics defined in the SearchFilter that have the IsProperty flag set to true
	int		MaxMatches;			// optional, The search will be completed after this many matches. If not specified, the search will continue until reaching the endpoint or until the session expires
	int		KeepAliveTime;		// required, The time the search session will be kept alive after responding to this and subsequent requests. A device shall support at least values up to ten seconds
} FindEvents_REQ;

typedef struct
{
	char	SearchToken[ONVIF_TOKEN_LEN];	// required, The search session to get results from
	int		MinResults;		// optional, The minimum number of results to return in one response
	int		MaxResults;		// optional, The maximum number of results to return in one response
	int		WaitTime;		// optional, The maximum time before responding to the request, even if the MinResults parameter is not fulfilled
} GetEventSearchResults_REQ;

typedef struct
{
	char	SearchToken[ONVIF_TOKEN_LEN];	// required, The search session to get the state from
} GetSearchState_REQ;

typedef struct
{
	char	SearchToken[ONVIF_TOKEN_LEN];	// required, The search session to end
} EndSearch_REQ;




/*************************************************************************
 *
 * onvif response structure
 *
**************************************************************************/
typedef struct
{
	ONVIF_CAP 	capablity;
} GetCapabilities_RES;

typedef struct
{
    ONVIF_CAP 	capablity;
} GetServices_RES;

typedef struct
{
	DEVICE_INFO	dev_info;
} GetDeviceInformation_RES;


typedef struct
{
	ONVIF_NET_INF * p_net_inf;
} GetNetworkInterfaces_RES;

typedef struct
{
	BOOL	need_reboot;
} SetNetworkInterfaces_RES;

typedef struct
{
	NTP_INFO	ntp;
} GetNTP_RES;

typedef struct 
{	
	int	dummy;
} SetNTP_RES;

typedef struct
{
	HOSTNAME	hostname;
} GetHostname_RES;

typedef struct
{
	int	dummy;
} SetHostname_RES;

typedef struct
{
	BOOL	need_reboot;
} SetHostnameFromDHCP_RES;

typedef struct
{
	DNS_INFO	dns;
} GetDNS_RES;

typedef struct 
{
	int	dummy;
} SetDNS_RES;

typedef struct
{
	DDNS_INFO	ddns;
} GetDynamicDNS_RES;

typedef struct
{
	int	dummy;
} SetDynamicDNS_RES;

typedef struct
{
	NETPROTOCOL	protocols;
} GetNetworkProtocols_RES;

typedef struct
{
	int	dummy;
} SetNetworkProtocols_RES;

typedef struct
{
	BOOL 	discoverable;
} GetDiscoveryMode_RES;

typedef struct
{
	int	dummy;
} SetDiscoveryMode_RES;

typedef struct
{
	char	gateway[MAX_GATEWAY][32];
} GetNetworkDefaultGateway_RES;

typedef struct
{
	int	dummy;
} SetNetworkDefaultGateway_RES;

typedef struct
{
    int     DaylightSavings : 1;
    int     utc_valid   : 1;    /* utc_datetime valid flag */
    int     local_valid : 1;    /* local_datetime valid flag */
    int     reserved    : 29;
    
    int     type;   /* 0 : Manual, 1 : NTP*/
    
    char    TZ[32];

    /*UTC Time*/
    DATETIME    utc_datetime;
    DATETIME    local_datetime;
} GetSystemDateAndTime_RES;

typedef struct
{
	int	dummy;
} SetSystemDateAndTime_RES;

typedef struct
{
	int	dummy;
} SystemReboot_RES;

typedef struct
{
	int	dummy;
} SetSystemFactoryDefault_RES;

typedef struct
{
	int	dummy;
} GetSystemLog_RES;

typedef struct
{
	int	dummy;
} GetScopes_RES;

typedef struct
{
	int	dummy;
} SetScopes_RES;

typedef struct
{
	int	dummy;
} AddScopes_RES;

typedef struct
{
	int	dummy;
} RemoveScopes_RES;


typedef struct
{
	VIDEO_SRC * p_v_src;
} GetVideoSources_RES;

typedef struct
{
    AUDIO_SRC * p_a_src;
} GetAudioSources_RES;

typedef struct
{
	ONVIF_PROFILE profile;
} CreateProfile_RES;

typedef struct
{
	ONVIF_PROFILE profile;
} GetProfile_RES;

typedef struct
{
	ONVIF_PROFILE * p_profile;
} GetProfiles_RES;

typedef struct
{
	int	dummy;
} AddVideoEncoderConfiguration_RES;

typedef struct
{
	int	dummy;
} AddVideoSourceConfiguration_RES;

typedef struct
{
	int	dummy;
} AddAudioEncoderConfiguration_RES;

typedef struct
{
	int	dummy;
} AddAudioSourceConfiguration_RES;

typedef struct
{
	int	dummy;
} AddPTZConfiguration_RES;

typedef struct
{
	int	dummy;
} RemoveVideoEncoderConfiguration_RES;

typedef struct
{
	int	dummy;
} RemoveVideoSourceConfiguration_RES;

typedef struct
{
	int	dummy;
} RemoveAudioEncoderConfiguration_RES;

typedef struct
{
	int	dummy;
} RemoveAudioSourceConfiguration_RES;

typedef struct
{
	int	dummy;
} RemovePTZConfiguration_RES;

typedef struct
{
	int	dummy;
} DeleteProfile_RES;

typedef struct
{
	VIDEO_SRC_CFG * p_v_src_cfg;
} GetVideoSourceConfigurations_RES;

typedef struct
{
	VIDEO_ENCODER * p_v_enc;
} GetVideoEncoderConfigurations_RES;

typedef struct
{
	AUDIO_SRC_CFG * p_a_src_cfg;
} GetAudioSourceConfigurations_RES;

typedef struct
{
	AUDIO_ENCODER * p_a_enc;
} GetAudioEncoderConfigurations_RES;

typedef struct
{
	VIDEO_SRC_CFG	video_src_cfg;
} GetVideoSourceConfiguration_RES;

typedef struct
{
	VIDEO_ENCODER video_enc;
} GetVideoEncoderConfiguration_RES;

typedef struct
{
	AUDIO_SRC_CFG audio_src_cfg;
} GetAudioSourceConfiguration_RES;

typedef struct
{
	AUDIO_ENCODER audio_enc;
} GetAudioEncoderConfiguration_RES;

typedef struct
{
	int	dummy;
} SetVideoSourceConfiguration_RES;

typedef struct
{
	int	dummy;
} SetVideoEncoderConfiguration_RES;

typedef struct
{
	int	dummy;
} SetAudioSourceConfiguration_RES;

typedef struct
{
	int	dummy;
} SetAudioEncoderConfiguration_RES;

typedef struct
{
	int	dummy;
} GetVideoSourceConfigurationOptions_RES;

typedef struct
{
	VIDEO_ENC_CFG video_enc_cfg;
} GetVideoEncoderConfigurationOptions_RES;

typedef struct
{
	int	dummy;
} GetAudioSourceConfigurationOptions_RES;

typedef struct
{
	int	dummy;
} GetAudioEncoderConfigurationOptions_RES;

typedef struct
{
	char	Uri[ONVIF_URI_LEN];

	int 	Timeout;
	
	BOOL	InvalidAfterConnect;
	BOOL	InvalidAfterReboot;	
} GetStreamUri_RES;

typedef struct
{
	int	dummy;
} SetSynchronizationPoint_RES;

typedef struct
{
	char	Uri[ONVIF_URI_LEN];

	int 	Timeout;
	
	BOOL	InvalidAfterConnect;
	BOOL	InvalidAfterReboot;
} GetSnapshotUri_RES;


typedef struct
{
	PTZ_NODE * p_ptz_node;
} GetNodes_RES;

typedef struct
{
	PTZ_NODE ptz_node;
} GetNode_RES;

typedef struct
{
	PTZ_PRESET 	presets[MAX_PTZ_PRESETS];
} GetPresets_RES;

typedef struct
{
	char	preset_token[ONVIF_TOKEN_LEN];
} SetPreset_RES;

typedef struct
{
	int	dummy;
} RemovePreset_RES;

typedef struct
{
	int	dummy;
} GotoPreset_RES;

typedef struct
{
	int	dummy;
} GotoHomePosition_RES;

typedef struct
{
	int	dummy;
} SetHomePosition_RES;

typedef struct
{
	PTZ_STATUS	status;
} GetStatus_RES;

typedef struct
{
	int	dummy;
} ContinuousMove_RES;

typedef struct
{
	int	dummy;
} RelativeMove_RES;

typedef struct
{
	int	dummy;
} AbsoluteMove_RES;

typedef struct
{
	int	dummy;
} PTZStop_RES;

typedef struct
{
    PTZ_CFG * p_ptz_cfg;
} GetConfigurations_RES;

typedef struct
{
    PTZ_CFG ptz_cfg;
} GetConfiguration_RES;

typedef struct
{
	int	dummy;
} SetConfiguration_RES;

typedef struct
{
	PTZ_CFG_OPT ptz_cfg_opt;
} GetConfigurationOptions_RES;

typedef struct
{
	int	dummy;
} GetEventProperties_RES;

typedef struct
{
	int	dummy;
} Renew_RES;

typedef struct
{
	int	dummy;
} Unsubscribe_RES;

typedef struct
{
    char    producter_addr[256];
} Subscribe_RES;

typedef struct
{
    ONVIF_NOTIFY * notify;
} Notify_REQ;

typedef struct
{
    IMAGE_CFG img_cfg;
} GetImagingSettings_RES;

typedef struct
{
	int	dummy;
} SetImagingSettings_RES;

typedef struct
{
	IMAGE_OPTIONS img_opt;
} GetOptions_RES;

typedef struct
{
    char 	Uri[ONVIF_URI_LEN];				// required. The URI to which the client should connect in order to stream the recording
} GetReplayUri_RES;

typedef struct
{
	time_t	DataFrom;						// Required. The earliest point in time where there is recorded data on the device.
	time_t	DataUntil;						// Required. The most recent point in time where there is recorded data on the device
	int		NumberRecordings;				// Required. The device contains this many recordings
} GetRecordingSummary_RES;

typedef struct
{
	RECORDING	RecordingInformation;
} GetRecordingInformation_RES;

typedef struct
{
	MEDIA_ATTR MediaAttributes;
} GetMediaAttributes_RES;

typedef struct
{
	char 	SearchToken[ONVIF_TOKEN_LEN];	// Required, A unique reference to the search session created by this request
} FindRecordings_RES;

typedef struct
{
	RECORDING_RESULT	ResultList;
} GetRecordingSearchResults_RES;

typedef struct
{
	char 	SearchToken[ONVIF_TOKEN_LEN];	// Required, A unique reference to the search session created by this request
} FindEvents_RES;

typedef struct
{
	int	dummy;
} GetEventSearchResults_RES;

typedef struct
{
	SEARCHSTATUS	State;	
} GetSearchState_RES;

typedef struct
{
	time_t 		Endpoint;					// Required, The point of time the search had reached when it was ended. It is equal to the EndPoint specified in Find-operation if the search was completed
} EndSearch_RES;


#ifdef __cplusplus
extern "C" {
#endif

OVFACTS 	  * onvif_find_action_by_type(eOnvifAction type);
ONVIF_NET_INF * onvif_add_network_interface(ONVIF_NET_INF ** p_net_inf);
void			onvif_free_network_interface(ONVIF_NET_INF ** p_net_inf);
ONVIF_NET_INF * onvif_find_network_interface(ONVIF_DEVICE * p_dev, const char * token);
VIDEO_SRC     * onvif_add_video_source(VIDEO_SRC ** p_v_src);
void 		    onvif_free_video_source(VIDEO_SRC ** p_v_src);
VIDEO_SRC     * onvif_find_video_source(ONVIF_DEVICE * p_dev, const char * token);
AUDIO_SRC     * onvif_add_audio_source(AUDIO_SRC ** p_a_src);
void 		    onvif_free_audio_source(AUDIO_SRC ** p_a_src);
AUDIO_SRC     * onvif_find_audio_source(ONVIF_DEVICE * p_dev, const char * token);
VIDEO_SRC_CFG * onvif_add_video_source_cfg(VIDEO_SRC_CFG ** p_v_src_cfg);
void 		    onvif_free_video_source_cfg(VIDEO_SRC_CFG ** p_v_src_cfg);
VIDEO_SRC_CFG * onvif_find_video_source_cfg(ONVIF_DEVICE * p_dev, const char * token);
AUDIO_SRC_CFG * onvif_add_audio_source_cfg(AUDIO_SRC_CFG ** p_a_src_cfg);
void 		    onvif_free_audio_source_cfg(AUDIO_SRC_CFG ** p_a_src_cfg);
AUDIO_SRC_CFG * onvif_find_audio_source_cfg(ONVIF_DEVICE * p_dev, const char * token);
VIDEO_ENCODER * onvif_add_video_encoder(VIDEO_ENCODER ** p_v_enc);
void 		    onvif_free_video_encoder(VIDEO_ENCODER ** p_v_enc);
VIDEO_ENCODER * onvif_find_video_encoder(ONVIF_DEVICE * p_dev, const char * token);
AUDIO_ENCODER * onvif_add_audio_encoder(AUDIO_ENCODER ** p_a_enc);
void 		    onvif_free_audio_encoder(AUDIO_ENCODER ** p_a_enc);
AUDIO_ENCODER * onvif_find_audio_encoder(ONVIF_DEVICE * p_dev, const char * token);
ONVIF_PROFILE * onvif_add_profile(ONVIF_PROFILE ** p_profile);
void            onvif_free_profile(ONVIF_PROFILE ** p_profile);
ONVIF_PROFILE * onvif_find_profile(ONVIF_DEVICE * p_dev, const char * token);
PTZ_NODE      * onvif_add_ptz_node(PTZ_NODE ** p_ptz_node);
void 			onvif_free_ptz_node(PTZ_NODE ** p_ptz_node);
PTZ_NODE      * onvif_find_ptz_node(ONVIF_DEVICE * p_dev, const char * token);
PTZ_CFG       * onvif_add_ptz_cfg(PTZ_CFG ** p_ptz_cfg);
void 			onvif_free_ptz_cfg(PTZ_CFG ** p_ptz_cfg);
PTZ_CFG       * onvif_find_ptz_cfg(ONVIF_DEVICE * p_dev, const char * token);

VIDEO_SRC     * onvif_get_cur_video_src(ONVIF_DEVICE * p_dev);

ONVIF_NOTIFY  * onvif_add_notify(ONVIF_NOTIFY ** p_notify);
void            onvif_free_notify(ONVIF_NOTIFY ** p_notify);
SIMPLE_ITEM   * onvif_add_simple_item(SIMPLE_ITEM ** p_item);
void            onvif_free_simple_item(SIMPLE_ITEM ** p_item);

int             onvif_get_notify_nums(ONVIF_NOTIFY * p_notify);
void            onvif_device_add_notify(ONVIF_DEVICE * p_dev, ONVIF_NOTIFY * p_notify);
int             onvif_device_free_notify(ONVIF_DEVICE * p_dev, int nums);
const char    * onvif_format_simple_item(SIMPLE_ITEM * p_item);

TRACK_LIST    * onvif_add_recording_track(TRACK_LIST ** p_track);
void 			onvif_free_recording_track(TRACK_LIST ** p_track);

TRACK_ATTR_LIST* onvif_add_track_attr(TRACK_ATTR_LIST ** p_track);
void 			onvif_free_track_attr(TRACK_ATTR_LIST ** p_track);

RECORDING_LIST* onvif_add_recording(RECORDING_LIST ** p_recording);
void 			onvif_free_recording(RECORDING_LIST ** p_recording);

#ifdef __cplusplus
}
#endif

#endif


