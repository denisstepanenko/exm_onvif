


/*
	S.M.A.R.T 技术相关的函数和定义
*/

#ifndef _SMART_H
#define _SMART_H


#include "errno.h"

// Maximum allowed number of SMART Attributes
#define NUMBER_ATA_SMART_ATTRIBUTES     30

#define STRANGE_BUFFER_LENGTH (4+512*0xf8)

typedef unsigned long long	uint64_t;

#define ATTR_PACKED __attribute__((packed))

#pragma pack(1) 
struct ata_identify_device {
  unsigned short words000_009[10];
  unsigned char  serial_no[20];
  unsigned short words020_022[3];
  unsigned char  fw_rev[8];
  unsigned char  model[40];
  unsigned short words047_079[33];
  unsigned short major_rev_num;
  unsigned short minor_rev_num;
  unsigned short command_set_1;
  unsigned short command_set_2;
  unsigned short command_set_extension;
  unsigned short cfs_enable_1;
  unsigned short word086;
  unsigned short csf_default;
  unsigned short words088_255[168];
} ATTR_PACKED;
#pragma pack()

typedef unsigned char task_ioreg_t;

typedef struct hd_drive_task_hdr {
  task_ioreg_t data;
  task_ioreg_t feature;
  task_ioreg_t sector_count;
  task_ioreg_t sector_number;
  task_ioreg_t low_cylinder;
  task_ioreg_t high_cylinder;
  task_ioreg_t device_head;
  task_ioreg_t command;
} task_struct_t;

typedef enum {
  // returns no data, just succeeds or fails
  ENABLE,
  DISABLE,
  AUTOSAVE,
  IMMEDIATE_OFFLINE,
  AUTO_OFFLINE,
  STATUS,       // just says if SMART is working or not
  STATUS_CHECK, // says if disk's SMART status is healthy, or failing
  // return 512 bytes of data:
  READ_VALUES,
  READ_THRESHOLDS,
  READ_LOG,
  IDENTIFY,
  PIDENTIFY,
  // returns 1 byte of data
  CHECK_POWER_MODE,
  // writes 512 bytes of data:
  WRITE_LOG
} smart_command_set;

struct ata_smart_errorlog_command_struct {
  unsigned char devicecontrolreg;
  unsigned char featuresreg;
  unsigned char sector_count;
  unsigned char sector_number;
  unsigned char cylinder_low;
  unsigned char cylinder_high;
  unsigned char drive_head;
  unsigned char commandreg;
  unsigned int timestamp;
};

#pragma pack(1) 
struct ata_smart_errorlog_error_struct {
  unsigned char reserved;
  unsigned char error_register;
  unsigned char sector_count;
  unsigned char sector_number;
  unsigned char cylinder_low;
  unsigned char cylinder_high;
  unsigned char drive_head;
  unsigned char status;
  unsigned char extended_error[19];
  unsigned char state;
  unsigned short timestamp;
}ATTR_PACKED;
#pragma pack()

#pragma pack(1)
struct ata_smart_errorlog_struct {
  struct ata_smart_errorlog_command_struct commands[5];
  struct ata_smart_errorlog_error_struct error_struct;
} ATTR_PACKED;
#pragma pack()

#pragma pack(1)
struct ata_smart_errorlog {
  unsigned char revnumber;
  unsigned char error_log_pointer;
  struct ata_smart_errorlog_struct errorlog_struct[5];
  unsigned short int ata_error_count;
  unsigned char reserved[57];
  unsigned char checksum;
} ATTR_PACKED;
#pragma pack()

/* ata_smart_attribute is the vendor specific in SFF-8035 spec */ 
#pragma pack(1)
struct ata_smart_attribute {
  unsigned char id;
  // meaning of flag bits: see MACROS just below
  // WARNING: MISALIGNED!
  unsigned short flags; 
  unsigned char current;
  unsigned char worst;
  unsigned char raw[6];
  unsigned char reserv;
} ATTR_PACKED;
#pragma pack()


#pragma pack(1)
struct ata_smart_values {
  unsigned short int revnumber;
  struct ata_smart_attribute vendor_attributes [NUMBER_ATA_SMART_ATTRIBUTES];
  unsigned char offline_data_collection_status;
  unsigned char self_test_exec_status;  //IBM # segments for offline collection
  unsigned short int total_time_to_complete_off_line; // IBM different
  unsigned char vendor_specific_366; // Maxtor & IBM curent segment pointer
  unsigned char offline_data_collection_capability;
  unsigned short int smart_capability;
  unsigned char errorlog_capability;
  unsigned char vendor_specific_371;  // Maxtor, IBM: self-test failure checkpoint see below!
  unsigned char short_test_completion_time;
  unsigned char extend_test_completion_time;
  unsigned char conveyance_test_completion_time;
  unsigned char reserved_375_385[11];
  unsigned char vendor_specific_386_510[125]; // Maxtor bytes 508-509 Attribute/Threshold Revision #
  unsigned char chksum;
} ATTR_PACKED;
#pragma pack()

#pragma pack(1)
struct ata_smart_selftestlog_struct {
  unsigned char selftestnumber; // Sector number register
  unsigned char selfteststatus;
  unsigned short int timestamp;
  unsigned char selftestfailurecheckpoint;
  unsigned int lbafirstfailure;
  unsigned char vendorspecific[15];
} ATTR_PACKED;
#pragma pack()

#pragma pack(1)
struct ata_smart_selftestlog {
  unsigned short int revnumber;
  struct ata_smart_selftestlog_struct selftest_struct[21];
  unsigned char vendorspecific[2];
  unsigned char mostrecenttest;
  unsigned char reserved[2];
  unsigned char chksum;
} ATTR_PACKED;
#pragma pack()

// ATA Specification Command Register Values (Commands)
#define ATA_IDENTIFY_DEVICE             0xec                                              
#define ATA_IDENTIFY_PACKET_DEVICE      0xa1
#define ATA_SMART_CMD                   0xb0
#define ATA_CHECK_POWER_MODE            0xe5

// ATA Specification Feature Register Values (SMART Subcommands).
// Note that some are obsolete as of ATA-7.
#define ATA_SMART_READ_VALUES           0xd0
#define ATA_SMART_READ_THRESHOLDS       0xd1
#define ATA_SMART_AUTOSAVE              0xd2
#define ATA_SMART_SAVE                  0xd3
#define ATA_SMART_IMMEDIATE_OFFLINE     0xd4
#define ATA_SMART_READ_LOG_SECTOR       0xd5
#define ATA_SMART_WRITE_LOG_SECTOR      0xd6
#define ATA_SMART_WRITE_THRESHOLDS      0xd7
#define ATA_SMART_ENABLE                0xd8
#define ATA_SMART_DISABLE               0xd9
#define ATA_SMART_STATUS                0xda
// SFF 8035i Revision 2 Specification Feature Register Value (SMART
// Subcommand)
#define ATA_SMART_AUTO_OFFLINE          0xdb

// Sector Number values for ATA_SMART_IMMEDIATE_OFFLINE Subcommand
#define OFFLINE_FULL_SCAN               0
#define SHORT_SELF_TEST                 1
#define EXTEND_SELF_TEST                2
#define CONVEYANCE_SELF_TEST            3
#define SELECTIVE_SELF_TEST             4
#define ABORT_SELF_TEST                 127
#define SHORT_CAPTIVE_SELF_TEST         129
#define EXTEND_CAPTIVE_SELF_TEST        130
#define CONVEYANCE_CAPTIVE_SELF_TEST    131
#define SELECTIVE_CAPTIVE_SELF_TEST     132
#define CAPTIVE_MASK                    (0x01<<7)



typedef union ide_reg_valid_s {
  unsigned all			: 16;
  struct {
    unsigned data		: 1;
    unsigned error_feature	: 1;
    unsigned sector		: 1;
    unsigned nsector		: 1;
    unsigned lcyl		: 1;
    unsigned hcyl		: 1;
    unsigned select		: 1;
    unsigned status_command	: 1;
    unsigned data_hob		: 1;
    unsigned error_feature_hob	: 1;
    unsigned sector_hob		: 1;
    unsigned nsector_hob	: 1;
    unsigned lcyl_hob		: 1;
    unsigned hcyl_hob		: 1;
    unsigned select_hob		: 1;
    unsigned control_hob	: 1;
  } b;
} ide_reg_valid_t;

typedef struct ide_task_request_s {
  task_ioreg_t	   io_ports[8];
  task_ioreg_t	   hob_ports[8];
  ide_reg_valid_t  out_flags;
  ide_reg_valid_t  in_flags;
  int		   data_phase;
  int		   req_cmd;
  unsigned long	   out_size;
  unsigned long	   in_size;
} ide_task_request_t;

#define TASKFILE_NO_DATA	  0x0000
#define TASKFILE_IN		  0x0001
#define TASKFILE_OUT		  0x0004
#define HDIO_DRIVE_TASK_HDR_SIZE  8*sizeof(task_ioreg_t)
#define IDE_DRIVE_TASK_NO_DATA	       0
#define IDE_DRIVE_TASK_IN	       2
#define IDE_DRIVE_TASK_OUT	       3
#define HDIO_DRIVE_CMD            0x031f
#define HDIO_DRIVE_TASK           0x031e
#define HDIO_DRIVE_TASKFILE       0x031d
#define HDIO_GET_IDENTITY         0x030d


//以下为硬盘属性的编号,目前只支持这些,以后需要可以再加
#define	REALLOCATED_SECTOR_CT	5
#define	POWER_ON_HOURS			9
#define	TEMPERATURE_CELSIUS		194
#define	CURRENT_PENDING_SECTOR	197

//以下为硬盘参数值的定义
#define	RAWVALUE				1	//当前的实际值
#define	WORSTVALUE				2	//历史上的最差值


//将设备的信息读到drive结构中.遇错返回负值
int ataReadHDIdentity (int devicefd, struct ata_identify_device *drive);

// 返回ata硬盘的错误条数。负值表示错误
int ataReadErrorLog (int devicefd);

// Convenience function for formatting strings from ata_identify_device
void formatdriveidstring(char *out, const char *in, int n);

//  读取SMART 属性,找到指定序号的属性的指定值并返回。失败返回负值
//	devicefd: 打开设备节点的描述符
//  attribute_no: 属性的编号，在smart.h中有定义
//  valuetype:	  值的类型（如:当前,最差..，在smarg.h中有定义）	
long long ataReadSmartValues(int devicefd, int attribute_no, int valuetype);


//返回厂商编号,便于处理一些随厂商不同而不同的情况
//失败返回负值
int get_hd_manufactor(int devicefd);


//返回最近一次指定类型的自检的结果。0表示通过，1表示失败，2表示读不到,3表示进行中
//testtype:	0-短测试，1-长测试
//percent_done:如果在进行中,完成的百分比

int ataGetSelfTestLog (int devicefd, int testtype ,int *percent_done );

//运行smart测试，testtype: 0-短，1-长
//返回值,0表示成功,其他表示失败
int ataSmartTest(int devicefd, int testtype);
#endif
