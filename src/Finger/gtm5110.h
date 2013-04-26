/*
  GTM5110 API
  Author: OopsMonk
  Email: oopsmonk@gmail.com
*/
#ifndef GTM5110_H
#define GTM5110_H
#include "gtm5110_proto.h"

enum
{
  CMD_NONE		  = 0x00,
  CMD_OPEN		  = 0x01,
  CMD_CLOSE		  = 0x02,
  CMD_USB_INTERNAL_CHECK  = 0x03,
  CMD_CHANGE_BAUDRATE	  = 0x04,
	
  CMD_CMOS_LED		  = 0x12,

  CMD_ENROLL_COUNT	  = 0x20,
  CMD_CHECK_ENROLLED	  = 0x21,
  CMD_ENROLL_START	  = 0x22,
  CMD_ENROLL1		  = 0x23,
  CMD_ENROLL2		  = 0x24,
  CMD_ENROLL3		  = 0x25,
  CMD_IS_PRESS_FINGER	  = 0x26,

  CMD_DELETE		  = 0x40,
  CMD_DELETE_ALL	  = 0x41,
	
  CMD_VERIFY		  = 0x50,
  CMD_IDENTIFY		  = 0x51,
  CMD_VERIFY_TEMPLATE	  = 0x52,
  CMD_IDENTIFY_TEMPLATE	  = 0x53,
	
  CMD_CAPTURE		  = 0x60,

  CMD_GET_IMAGE		  = 0x62,
  CMD_GET_RAWIMAGE	  = 0x63,
	
  CMD_GET_TEMPLATE	  = 0x70,
  CMD_ADD_TEMPLATE	  = 0x71,
  CMD_GET_DATABASE_START  = 0x72,
  CMD_GET_DATABASE_END    = 0x73,
	
  CMD_FW_UPDATE		  = 0x80,
  CMD_ISO_UPDATE	  = 0x81,
	
  ACK_OK		  = 0x30,
  NACK_INFO		  = 0x31,
};

enum
{
  NACK_NONE				= 0x1000,
  NACK_TIMEOUT,				
  NACK_INVALID_BAUDRATE,		
  NACK_INVALID_POS,			
  NACK_IS_NOT_USED,			
  NACK_IS_ALREADY_USED,		
  NACK_COMM_ERR,				
  NACK_VERIFY_FAILED,			
  NACK_IDENTIFY_FAILED,		
  NACK_DB_IS_FULL,				
  NACK_DB_IS_EMPTY,			
  NACK_TURN_ERR,				
  NACK_BAD_FINGER,
  NACK_ENROLL_FAILED,
  NACK_IS_NOT_SUPPORTED,
  NACK_DEV_ERR,
  NACK_CAPTURE_CANCELED,
  NACK_INVALID_PARAM,
  NACK_FINGER_IS_NOT_PRESSED,
};

enum
{
  GTM_NONE		=0,
  GTM_ERR = -1,				
};

typedef struct _devinfo
{
	DWORD FirmwareVersion;
	DWORD IsoAreaMaxSize;
	BYTE DeviceSerialNumber[16];
} devinfo;

//Error code for Arduino UNO
enum
{
	ERROR_OOPS = -100,
	ERROR_DB_EMPTY =-200,
	ERROR_TIMEOUT = -300,
	ERROR_FP_NOT_IN_DB = -400,
	ERROR_NOT_PRESS = -500,
	ERROR_ENROLL = -600,
	ERROR_DEL_ENROLL = -700,
	
	ENROLL_EXIST = -1000,
};

//////////////////////////////////////////////////////////////////////////
#define FP_MAX_USERS		2000
#define FP_TEMPLATE_SIZE	498
#define EEPROM_SIZE			16

extern BYTE	gbyTemplate[FP_TEMPLATE_SIZE];

extern WORD gwDevID;
extern WORD gwLastAck;
extern DWORD  gwLastAckParam;

extern unsigned long  gnPassedTime;

extern devinfo gDevInfo;

//////////////////////////////////////////////////////////////////////////
void gtm_get_serial(SoftwareSerial *s);
int gtm_open( void );
int gtm_close( void );

int gtm_change_baudrate( DWORD rate );

int gtm_cmos_led( BOOL bOn );

int gtm_enroll_count( void );
int gtm_check_enrolled( int nPos );
int gtm_enroll_start( int nPos );
/*AVW*/
int gtm_enroll_nth( int nPos, int nTurn );
int gtm_is_press_finger( void );

int gtm_delete( int nPos );
int gtm_delete_all( void );

int gtm_verify( int nPos );
int gtm_identify( void );

int gtm_capture( BOOL bBest );

int IsPress();
int OnIdentify(unsigned long timeout);
int OnEnroll(int sID);
int OnRemoveEnroll(int fpID);
int OnRemoveAll();

#endif
