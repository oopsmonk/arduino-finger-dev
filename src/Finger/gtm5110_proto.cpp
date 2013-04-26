/*
  GTM5110 Protocol
  Author: OopsMonk
  Email: oopsmonk@gmail.com
*/

#include "gtm5110_proto.h"
#include "gtm5110.h"

SoftwareSerial *gtmSerial = NULL;

#define _DEBUG_POTO_DEBUG_ 0
#define _DEBUG_POTO_INFO_ 0

void debugCMD(SB_OEM_PKT pkt)
{
  #if _DEBUG_POTO_DEBUG_
	Serial.print(pkt.Head1, HEX);
	Serial.print(",");
	Serial.print(pkt.Head2, HEX);
	Serial.print(",");
	//Serial.print(highByte(pkt.wDevId), HEX);
	//Serial.print(lowByte(pkt.wDevId), HEX);
	Serial.print(lowByte(pkt.wDevId), HEX);
	Serial.print(",");
	Serial.print(highByte(pkt.wDevId), HEX);
	Serial.print(",");
	Serial.print((BYTE)(pkt.nParam & 0xFF), HEX);
	Serial.print(",");
	Serial.print((BYTE)((pkt.nParam & 0xFF00)<< 8), HEX);
	Serial.print(",");
	Serial.print((BYTE)((pkt.nParam & 0xFF0000)<< 16), HEX);
	Serial.print(",");
	Serial.print((BYTE)((pkt.nParam & 0xFF000000)<< 24), HEX);
	Serial.print(",");
	Serial.print(lowByte(pkt.wCmd), HEX);
	Serial.print(",");
	Serial.print(highByte(pkt.wCmd), HEX);
	Serial.print(",");
	Serial.print(lowByte(pkt.wChkSum), HEX);
	Serial.print(",");
	Serial.println(highByte(pkt.wChkSum), HEX);
#endif
}

int recvData(BYTE *buf, int length)
{
	int count=0;
	int revCount = gtmSerial->available();
	//Serial.print("recvCMD revCount = ");
	//Serial.println(revCount);
#if _DEBUG_POTO_DEBUG_
	Serial.print("recvData = ");
	for(count = 0 ; count < length; count++){
		*buf = gtmSerial->read();
		Serial.print(*buf, HEX);
		Serial.print(" , ");
		buf++;
	}
	Serial.print("	 end count = ");
	Serial.println(count);
#else
	for(count = 0 ; count < length; count++){
		*buf = gtmSerial->read();
		buf++;
	}
#endif
	revCount = gtmSerial->available();
	//Serial.print("recvData revCount = ");
	//Serial.println(revCount);
	
	return count;
}

int recvCMD(SB_OEM_PKT *pkt)
{
	int revCount = gtmSerial->available();
	int dataCount = revCount;
	
#if _DEBUG_POTO_DEBUG_
	Serial.print("recvCMD available Count = ");
	Serial.println(revCount);
#endif

	char tmphi,tmplow, tmp1hi, tmp1low;
	pkt->Head1 = gtmSerial->read();
	pkt->Head2 = gtmSerial->read();
	tmplow = gtmSerial->read();
	tmphi = gtmSerial->read();
	pkt->wDevId = (tmphi << 8) | tmplow;

	tmplow = gtmSerial->read(); //LWORD
	tmphi = gtmSerial->read();
	tmp1low = gtmSerial->read(); //HWORD
	tmp1hi = gtmSerial->read();
	pkt->nParam = (tmp1hi << 24) | (tmp1low << 16) | (tmphi << 8) | tmplow;
	
	tmplow = gtmSerial->read();
	tmphi = gtmSerial->read();
	pkt->wCmd = (tmphi << 8) | tmplow;

	tmplow = gtmSerial->read();
	tmphi = gtmSerial->read();
	pkt->wChkSum = (tmphi << 8) | tmplow;

	revCount = gtmSerial->available();
	
	return dataCount - revCount;
}

void GetSerial(SoftwareSerial *s)
{
	gtmSerial = s;
}

WORD CalcChkSumOfCmdAckPkt( SB_OEM_PKT* pPkt )
{
	WORD wChkSum = 0;
	BYTE* pBuf = (BYTE*)pPkt;
	int i;
	//Serial.print("CalcChkSumOfCmdAckPkt = ");
	//Serial.println(sizeof(SB_OEM_PKT)-SB_OEM_CHK_SUM_SIZE);
	for(i=0;i<(sizeof(SB_OEM_PKT)-SB_OEM_CHK_SUM_SIZE);i++){
		//Serial.print("data = ");
		//Serial.print(pBuf[i], HEX);
		//Serial.print("  sum = ");
		//Serial.print(wChkSum, HEX);
		wChkSum += pBuf[i];
	}
	//Serial.println("   end");
	//Serial.print("CalcChkSumOfCmdAckPkt = ");
	//Serial.println(wChkSum, HEX);
	return wChkSum;
}

WORD CalcChkSumOfDataPkt( BYTE* pDataPkt, int nSize )
{
	int i;
	WORD wChkSum = 0;
	BYTE* pBuf = (BYTE*)pDataPkt;
	
	for(i=0;i<nSize;i++)
		wChkSum += pBuf[i];
	return wChkSum;
}

int CheckCmdAckPkt( WORD wDevID, SB_OEM_PKT* pPkt )
{
	if( ( pPkt->Head1 != STX1 ) || 
		( pPkt->Head2 != STX2 ) )
	{
		return PKT_HDR_ERR;
	}
	
	if( pPkt->wDevId != wDevID ) 
		return PKT_DEV_ID_ERR;
	
	if( pPkt->wChkSum != CalcChkSumOfCmdAckPkt( pPkt ) ) 
		return PKT_CHK_SUM_ERR;

	return 0;
}

int SendCmdOrAck( WORD wDevID, WORD wCmdOrAck, DWORD nParam)
{
	SB_OEM_PKT pkt;
	int nSentBytes;

	pkt.Head1 = (BYTE)STX1;
	pkt.Head2 = (BYTE)STX2;
	pkt.wDevId = wDevID;
	pkt.wCmd = wCmdOrAck;
	pkt.nParam = nParam;
	pkt.wChkSum = CalcChkSumOfCmdAckPkt( &pkt );

#if _DEBUG_POTO_INFO_
	Serial.print("SendCmdOrAck start command : ");
	Serial.println(wCmdOrAck, HEX);
#endif
	nSentBytes = gtmSerial->write( (BYTE*)&pkt,SB_OEM_PKT_SIZE);
	
	if( nSentBytes != SB_OEM_PKT_SIZE ){
             Serial.println("Error....nSentBytes != SB_OEM_PKT_SIZE");
		return PKT_COMM_ERR;
        }

	return 0;

}

int ReceiveCmdOrAck( WORD wDevID, WORD* pwCmdOrAck, DWORD* pnParam )
{
	SB_OEM_PKT pkt;
	int nReceivedBytes;
        //Serial.println("ReceiveCmdOrAck start!! ");
	if( ( pwCmdOrAck == NULL ) || 
		( pnParam == NULL ) )
	{
                Serial.println("Error.....pwCmdOrAck == NULL");
		return PKT_PARAM_ERR;
	}

	int revCount = gtmSerial->available();
 	nReceivedBytes = recvCMD(&pkt);

	debugCMD(pkt);		

	if( nReceivedBytes != SB_OEM_PKT_SIZE ){
                Serial.println("Error.....nReceivedBytes != SB_OEM_PKT_SIZE");
		return PKT_COMM_ERR;
        }

	if( ( pkt.Head1 != STX1 ) || 
		( pkt.Head2 != STX2 ) )
	{
                Serial.println("Error....PKT_HDR_ERR");
		return PKT_HDR_ERR;
	}

	if( pkt.wDevId != wDevID ) {
                Serial.println("Error....PKT_DEV_ID_ERR");
		return PKT_DEV_ID_ERR;
        }

	if( pkt.wChkSum != CalcChkSumOfCmdAckPkt( &pkt ) ) {
                Serial.println("Error....PKT_CHK_SUM_ERR");
		return PKT_CHK_SUM_ERR;
        }
	
	*pwCmdOrAck = pkt.wCmd;
	*pnParam = pkt.nParam;
#if _DEBUG_POTO_INFO_
	Serial.print("Receive data Cmd = ");
	Serial.print(pkt.wCmd, HEX);
	Serial.print(" , nParam = ");
	Serial.println(pkt.nParam, HEX);
#endif
	return 0;

}

int SendData( WORD wDevID, BYTE* pBuf, int nSize )
{
	WORD wChkSum = 0;
	BYTE Buf[4], *pCommBuf;
	int nSentBytes;
	
	if( pBuf == NULL )
		return PKT_PARAM_ERR;

	Buf[0] = (BYTE)STX3;
	Buf[1] = (BYTE)STX4;
	*((WORD*)(&Buf[SB_OEM_HEADER_SIZE])) = wDevID;
	
	wChkSum = CalcChkSumOfDataPkt( Buf, SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE  );
	wChkSum += CalcChkSumOfDataPkt( pBuf, nSize );
	
	pCommBuf = new BYTE[nSize+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE+SB_OEM_CHK_SUM_SIZE];
	memcpy(pCommBuf, Buf, SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE);
	memcpy(pCommBuf+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE, pBuf, nSize);
	*(WORD*)(pCommBuf+nSize+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE) = wChkSum;

	nSentBytes = gtmSerial->write( pCommBuf, nSize+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE+SB_OEM_CHK_SUM_SIZE );
            
	if( nSentBytes != nSize+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE+SB_OEM_CHK_SUM_SIZE )
	{
		if(pCommBuf)
			delete pCommBuf;
		return PKT_COMM_ERR;
	}

	if(pCommBuf)
		delete pCommBuf;

	return 0;
}

int ReceiveData( WORD wDevID, BYTE* pBuf, int nSize )
{
	WORD wReceivedChkSum, wChkSum;
	BYTE Buf[4],*pCommBuf;
	int nReceivedBytes;
	
	if( pBuf == NULL )
		return PKT_PARAM_ERR;


	pCommBuf = new BYTE[nSize+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE+SB_OEM_CHK_SUM_SIZE];

	nReceivedBytes = recvData( pCommBuf, nSize+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE+SB_OEM_CHK_SUM_SIZE );
		
	if( nReceivedBytes != nSize+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE+SB_OEM_CHK_SUM_SIZE )
	{
		if(pCommBuf)
			delete pCommBuf;
		return PKT_COMM_ERR;
	}
	memcpy(Buf, pCommBuf, SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE);
	memcpy(pBuf, pCommBuf+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE, nSize);
	wReceivedChkSum = *(WORD*)(pCommBuf+nSize+SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE);
	if(pCommBuf)
		delete pCommBuf;

	if( ( Buf[0] != STX3 ) || 
		( Buf[1] != STX4 ) )
	{
		return PKT_HDR_ERR;
	}
	
	if( *((WORD*)(&Buf[SB_OEM_HEADER_SIZE])) != wDevID ) 
		return PKT_DEV_ID_ERR;
	
	wChkSum = CalcChkSumOfDataPkt( Buf, SB_OEM_HEADER_SIZE+SB_OEM_DEV_ID_SIZE  );
	wChkSum += CalcChkSumOfDataPkt( pBuf, nSize );
	
	if( wChkSum != wReceivedChkSum ) 
		return PKT_CHK_SUM_ERR;

	return 0;

}

