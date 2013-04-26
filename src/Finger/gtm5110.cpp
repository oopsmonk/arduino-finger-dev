/*
  GTM5110 API
  Author: OopsMonk
  Email: oopsmonk@gmail.com
*/

#include "gtm5110.h"
#define _DEBUG_GTM_ 0
#define _DEBUG_GTM_INFO_ 1

//////////////////////////////////////////////////////////////////////////
BYTE	gbyTemplate[FP_TEMPLATE_SIZE];

WORD gwDevID = 1;
WORD gwLastAck = 0;
DWORD  gwLastAckParam = 0;

unsigned long  gnPassedTime = 0;

devinfo gDevInfo;

//////////////////////////////////////////////////////////////////////////

void gtm_get_serial(SoftwareSerial *s)
{
	GetSerial(s);
}
#define _STime_ 300
#define _LTime_ 800
int CommandRun(WORD wCmd, DWORD nCmdParam, int timeout)
{
	
	if( SendCmdOrAck( gwDevID, wCmd, nCmdParam) < 0 )
		return GTM_ERR;
	//gnPassedTime = millis();
	delay(timeout);
	if( ReceiveCmdOrAck( gwDevID, &gwLastAck, &gwLastAckParam) < 0 )
		return GTM_ERR;
#if _DEBUG_GTM_	
	Serial.print("CommandRun data Cmd = ");
	Serial.println(gwLastAck);
	Serial.print("CommandRun data nParam = ");
	Serial.println(gwLastAckParam);
#endif
	return 0;
}

// initailization
int gtm_open( void )
{
	if ( CommandRun( CMD_OPEN, 1,  _STime_) < 0 )
		return GTM_ERR;
	if( ReceiveData( gwDevID, (BYTE*)&gDevInfo, sizeof(devinfo)) < 0 )
		return GTM_ERR;
	return 0;
}
// termination
int gtm_close( void )
{
	if(!CommandRun( CMD_CLOSE, 0, _STime_))
		return GTM_ERR;
	else
		return GTM_NONE;
}

//chage UART baudrate
int gtm_change_baudrate( DWORD rate )
{
	return CommandRun( CMD_CHANGE_BAUDRATE, rate ,_STime_);
}
// COMS Led On/Off
int gtm_cmos_led( BOOL bOn )
{
	return CommandRun( CMD_CMOS_LED, bOn ? 1 : 0,  100);
}
// get enrolled fingerprint count in gtm5110
int gtm_enroll_count( void )
{
	if(CommandRun( CMD_ENROLL_COUNT, 0,  _STime_ ) != GTM_NONE)
		return GTM_ERR;
	else
		return gwLastAckParam;
}

//Check whether the specified ID is already enrolled
int gtm_check_enrolled( int nPos )
{
	return CommandRun( CMD_CHECK_ENROLLED, nPos,  _STime_ );
}

// start a erollment
int gtm_enroll_start( int nPos )
{
	return CommandRun( CMD_ENROLL_START, nPos,  _STime_);
}

int gtm_enroll_nth( int nPos, int nTurn )
{
	if( CommandRun( CMD_ENROLL_START+nTurn, 0,  _LTime_) < 0 )
		return GTM_ERR;

	if( nPos == -1 && nTurn == 3)
	{
		if(gwLastAck == ACK_OK)
		{
			if( ReceiveData( gwDevID, &gbyTemplate[0], FP_TEMPLATE_SIZE ) < 0 )
				return GTM_ERR;
		}
	}

	return 0;
}

//check if a finger is placed on the sensor
//this function must trun on LED.
int gtm_is_press_finger( void )
{
	int ret = GTM_ERR, rdata;
	ret = CommandRun( CMD_IS_PRESS_FINGER, 0,  _STime_);
	rdata = gwLastAckParam;	
	if(ret != GTM_NONE)
		return GTM_ERR;
	else
		return rdata; 
}

//Delete the fingerprint with the specified ID
int gtm_delete( int nPos )
{
	return CommandRun( CMD_DELETE, nPos,  _STime_);
}

//Delete all fingerprints from the database in gtm5110
int gtm_delete_all( void )
{
	return CommandRun( CMD_DELETE_ALL, 0,  _STime_);
}

//1:1 Verification of the capture fingerprint image with the specified ID
int gtm_verify( int nPos )
{
	return CommandRun( CMD_VERIFY, nPos, _STime_);
}

// 1:N Identification of the capture fingerprint image with the database
//return the id in database, else GTM_ERR.
int gtm_identify( void )
{
	int ret = GTM_ERR, rdata;
	ret = CommandRun( CMD_IDENTIFY, 0, _LTime_);
	rdata = gwLastAckParam;
	
	
	if(ret != GTM_NONE)
		return GTM_ERR;
	else
		return rdata;
}

// Capture a fingerprint image(256x256) from the sensor 
//this function must trun on LED.
int gtm_capture( BOOL bBest )
{
	int ret = GTM_ERR;
	ret = CommandRun( CMD_CAPTURE, bBest, _STime_);
	if(ret != GTM_NONE)
		return GTM_ERR;
	else 
		return GTM_NONE;
}
int IsPress()
{
	int ret = ERROR_OOPS;
	//check if finger pressed
	gtm_cmos_led(true);
	if(gtm_is_press_finger() != NACK_FINGER_IS_NOT_PRESSED)
		ret = 0;
	gtm_cmos_led(false);
	return ret;
}

int OnIdentify(unsigned long timeout)
{
	unsigned long preTime = millis();
	//get fingerprint count
	int cnt = gtm_enroll_count();
	if(cnt == 0){
		//no fp in database
		return ERROR_DB_EMPTY;
	}else if(cnt < 0){
		//get count failed.
		return ERROR_OOPS;
	}
#if _DEBUG_GTM_
	Serial.print("enroll count = ");
	Serial.println(cnt);
#endif
	//check if finger pressed
	gtm_cmos_led(true);
	while(gtm_is_press_finger() != NACK_FINGER_IS_NOT_PRESSED){
		//check if timeout
		unsigned long curTime = millis();
		if((curTime - preTime) > timeout){
			//timeout occurred.
			Serial.print("OnIdentify timeout : ");
			Serial.println(curTime - preTime);			
			gtm_cmos_led(false);
			return ERROR_TIMEOUT;
		}
		
		//get image from sencer
		int cap = gtm_capture(false);
		
		gtm_cmos_led(false);
		int id = gtm_identify();
		
		if (id == NACK_BAD_FINGER){
			Serial.print("OnIdentify NACK_BAD_FINGER");
			continue;
		}
		
		if(id == NACK_IDENTIFY_FAILED){
			Serial.println("FP not in database");
			return ERROR_FP_NOT_IN_DB;
		}else{
#if _DEBUG_GTM_INFO_
			Serial.println("##########");
			Serial.print("Your ID is : ");
			Serial.println(id);
			Serial.println("##########");	
#endif
			return id;
		}
		
	}
	Serial.println("#####Finger not press#######");
	gtm_cmos_led(false);
	return ERROR_NOT_PRESS;
}

int OnCapture(BOOL bBest, unsigned long timeout)
{
	unsigned long preTime, currTime;
	preTime = millis();
	
	do{
		if(gtm_capture(bBest) < 0)
		{
			Serial.println("run capture error");
			return ERROR_OOPS;
		}
		else if(gwLastAck == ACK_OK)
		{
			return 0;
		}
		currTime = millis();
		delay(500);
		
	}while((currTime - preTime) < timeout);
#if _DEBUG_GTM_INFO_
	Serial.println("Capture time out!!!");
#endif
	return ERROR_TIMEOUT;	
}
// sID = -1, use the last id in database, if sID is enrolled return error.
int OnEnroll(int sID)
{
	//check sID status.
	if((sID < 0) | (sID > (FP_MAX_USERS - 1))){
		Serial.println("Sepecified ID is invalid!");
		return ERROR_OOPS;
	}		

	if(gtm_enroll_start(sID) < 0)
	{
		//command error
		Serial.print("command run error");
		goto Error_enroll;
	}
	if(gwLastAck == NACK_INFO)
	{
		//DisplayErr(gwLastAckParam, nID);
		Serial.print("enroll ack error : ");
		Serial.println(gwLastAckParam, HEX);
		goto Error_enroll;
	}

	
	gtm_cmos_led(true);
	for(int i=1; i<4; i++)
	{
#if _DEBUG_GTM_INFO_
		Serial.println("Srart enroll ............");
#endif
		if(OnCapture(true , 5000) < 0){
			goto Error_enroll;
		}
		
#if _DEBUG_GTM_INFO_		
		Serial.println("Processing fingerprint...");
#endif

		if(gtm_enroll_nth(sID, i) < 0)
		{
			Serial.println("run enroll nth error");
			goto Error_enroll;
		}
		if(gwLastAck == NACK_INFO)
		{
			//DisplayErr(gwLastAckParam, 0);
			Serial.print("Error code : ");
			Serial.println(gwLastAckParam, HEX);
			if(gwLastAckParam == 0)
				goto Enroll_IDIsUsed;
			
			goto Error_enroll;
		}
#if _DEBUG_GTM_INFO_
		Serial.println("Take off finger, please...");
#endif
		
		if( i<3 )
		while(1){
			if(  gtm_is_press_finger() < 0 ){
				//m_strResult = _T("Communication error!");
				Serial.println("press_finger error");
				goto Error_enroll;
			}
			if( gwLastAck == NACK_INFO ){
				Serial.print("Error code : ");
				Serial.println(gwLastAckParam, HEX);
				goto Error_enroll;
			}
			if( gwLastAckParam != 0 )
				break;
			delay(1000);
		}

	}
	
	gtm_cmos_led(false);
#if _DEBUG_GTM_INFO_
	Serial.print("Enroll OK....ID : ");
	Serial.println(sID);
#endif
	return 0;

Enroll_IDIsUsed:
	gtm_cmos_led(false);
	return ENROLL_EXIST;	

	
Error_enroll:
	gtm_cmos_led(false);
	return ERROR_ENROLL;
	
}

//remove specified Enrollment.
int OnRemoveEnroll(int fpID)
{
	if(gtm_delete(fpID) < 0)
	{
 		Serial.println("Communication error!");
		return ERROR_OOPS;
	}
	if(gwLastAck == NACK_INFO)
	{
		//DisplayErr(gwLastAckParam, nID);
		Serial.print("delete enroll error code : ");
		Serial.println(gwLastAckParam, HEX);
		return ERROR_DEL_ENROLL;
	}
	//delete enroll OK.
#if _DEBUG_GTM_INFO_
	Serial.print("Revmoe enroll OK....ID: ");
	Serial.println(fpID);
#endif
	return 0;
}
//remove all Enrollment.
int OnRemoveAll()
{
	if(gtm_delete_all() < 0)
	{
 		Serial.println("Communication error!");
		return ERROR_OOPS;
	}
	if(gwLastAck == NACK_INFO)
	{
		//DisplayErr(gwLastAckParam, nID);
		Serial.print("delete all enroll error code : ");
		Serial.println(gwLastAckParam, HEX);
		return ERROR_DEL_ENROLL;
	}
	//delete enroll OK.
#if _DEBUG_GTM_INFO_
	Serial.println("Revmoe All enroll OK....");
#endif
	return 0;
}


