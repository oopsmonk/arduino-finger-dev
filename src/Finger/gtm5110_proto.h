/*
  GTM5110 Protocol
  Author: OopsMonk
  Email: oopsmonk@gmail.com
*/

#ifndef GTM5110_PROTO_H
#define GTM5110_PROTO_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <SoftwareSerial.h>
#include "string.h"

#define BYTE byte  //unsigned char
#define WORD word  //unsigned int
#define DWORD unsigned long
#define BOOL boolean

// Header Of Cmd and Ack Packets
#define STX1  0x55	//Header1 
#define STX2  0xAA	//Header2

// Header Of Data Packet
#define STX3  0x5A	//Header1 
#define STX4  0xA5	//Header2

// Structure Of Cmd and Ack Packets 
typedef struct {		
	BYTE 	Head1;		
	BYTE 	Head2;		
	WORD	wDevId;
	DWORD	nParam;
	WORD	wCmd;// or nAck
	WORD 	wChkSum;
} SB_OEM_PKT;			

#define SB_OEM_PKT_SIZE      12
#define SB_OEM_HEADER_SIZE   2
#define SB_OEM_DEV_ID_SIZE   2
#define SB_OEM_CHK_SUM_SIZE  2

#define PKT_ERR_START	-500
#define PKT_COMM_ERR	PKT_ERR_START+1
#define PKT_HDR_ERR	PKT_ERR_START+2
#define PKT_DEV_ID_ERR	PKT_ERR_START+3
#define PKT_CHK_SUM_ERR	PKT_ERR_START+4
#define PKT_PARAM_ERR	PKT_ERR_START+5

//extern SoftwareSerial gswSerial;
void GetSerial(SoftwareSerial *s);
int CheckCmdAckPkt( WORD wDevID, SB_OEM_PKT* pPkt );
int SendCmdOrAck( WORD wDevID, WORD wCmdOrAck, DWORD nParam);
int ReceiveCmdOrAck( WORD wDevID, WORD* pwCmdOrAck, DWORD* pnParam);
int SendData( WORD wDevID, BYTE* pBuf, int nSize );
int ReceiveData( WORD wDevID, BYTE* pBuf, int nSize );

#endif
