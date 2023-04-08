/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm.h"
/*for further services please add service header here*/
#include "dcm_rdbi.h"
#include "dcm_wdbi.h"
#include "dcm_seca.h"

uint32_t ProtocolI_RX = 0x000007E0; /*ECU receive*/
uint32_t ProtocolI_TX = 0x000007E8; /*ECU send*/

uint8_t DCM_NEW_REQ = 1;
uint8_t Seca_Key_Internal[16];
uint16_t Seca_Timer;
uint8_t dummy_eeprom_buffer [1024];

uint16_t Consecutive_Curr;
uint16_t Consecutive_Next;
uint8_t  Consecutive_Last = 0;

static dcm_service_table SID_TABLE[] =
{
		{&dcm_rdbi, 0x22},
		{&dcm_wdbi, 0x2E},
		{&dcm_seca, 0x27}
};

Dcm_Msg_Info Dcm_Msg_Info_s;

uint16_t (*const dcm_funcs_fp[])() =
{
	Read_CanID_Tester,
	Read_CanID_ECU,
	Write_CanID_Tester,
	Write_CanID_ECU,
	Read_22_2E_Data,
	Write_22_2E_Data,
	Gen_Ramdom_Seed,
	Compare_Key
};

void CAN_TP2DCM(uint32_t CANID, uint8_t TransBuffer[])
{
	uint8_t LoopIndx;
	uint8_t temSID;
	uint8_t sidFound;

	LoopIndx = 0;
	temSID = 0;
	sidFound = 0;

	if(DCM_NEW_REQ == 1)
	{
		/*confirm new request is ready*/
		DCM_NEW_REQ = 0;
	}

	if(DCM_NEW_REQ == 0)
	{
		/*init Dcm State*/
		Dcm_Msg_Info_s.protoID = CANID ;
		Dcm_Msg_Info_s.respType = DCM_NORESP;

		if(TransBuffer[0] == 0)
		{
			Dcm_Msg_Info_s.respType = DCM_NORESP;
		}
		else if(Dcm_Msg_Info_s.protoID != ProtocolI_RX)
		{
			Dcm_Msg_Info_s.respType = DCM_NORESP;
		}
		else
		{
			switch(TransBuffer[0] & 0xF0)
			{
				case DCM_SG_FRAME:{Dcm_Msg_Info_s.transType = DCM_SG_FRAME;break;}
				case DCM_FS_FRAME:{Dcm_Msg_Info_s.transType = DCM_FS_FRAME;break;}
				case DCM_FC_FRAME:{Dcm_Msg_Info_s.transType = DCM_FC_FRAME;break;}
				case DCM_CC_FRAME:{Dcm_Msg_Info_s.transType = DCM_CC_FRAME;break;}
				default:          {                                        break;}
			};

			if(Dcm_Msg_Info_s.transType == DCM_SG_FRAME)/*Single request frame*/
			{
				Dcm_Msg_Info_s.Sid = TransBuffer[1];
				memset(&Dcm_Msg_Info_s.dataBuff,0x00,4096);
				for(LoopIndx = 0; LoopIndx < TransBuffer[0] + 1; LoopIndx++)
				{
					Dcm_Msg_Info_s.dataBuff[LoopIndx] = TransBuffer[LoopIndx];
				}
				Dcm_Msg_Info_s.numByetReq = TransBuffer[0];
				temSID = Dcm_Msg_Info_s.Sid;
				/*Search loop for supported SIDs*/
				for(LoopIndx = 0; LoopIndx < sizeof(SID_TABLE)/sizeof(SID_TABLE[0]); LoopIndx++)
				{
					if(temSID == SID_TABLE[LoopIndx].sid)
					{sidFound = 1;break;}
				}
				if(sidFound)
				{
					/*execute service function*/
					SID_TABLE[LoopIndx].service_fp(&Dcm_Msg_Info_s);
				}
				else
				{
					/*service is not supported, send NRC$11*/
					Dcm_Msg_Info_s.dataBuff[0] = 0x03;
					Dcm_Msg_Info_s.dataBuff[1] = 0x7F;
					Dcm_Msg_Info_s.dataBuff[2] = Dcm_Msg_Info_s.Sid;
					Dcm_Msg_Info_s.dataBuff[3] = NRC11_SERVICE_NOTSUPPORT;
					Dcm_Msg_Info_s.respType    = DCM_NEG;
				}
			}
			else if (Dcm_Msg_Info_s.transType == DCM_FS_FRAME)/*First frame*/
			{
				Dcm_Msg_Info_s.Sid = TransBuffer[2];
				memset(&Dcm_Msg_Info_s.dataBuff,0x00,4096);

				Dcm_Msg_Info_s.numByetReq = TransBuffer[0] ;
				Dcm_Msg_Info_s.numByetReq <<= 8 ;
				Dcm_Msg_Info_s.numByetReq |= TransBuffer[1] ;
				Dcm_Msg_Info_s.numByetReq &= 0x0FFF;



				for(LoopIndx = 0; LoopIndx < 6; LoopIndx++)
				{
					/*take data from 1st frame*/
					Dcm_Msg_Info_s.dataBuff[LoopIndx + 1] = TransBuffer[LoopIndx + 2];
				}

				Consecutive_Curr = 7; /*next position to fill data in consecutive process*/
				Dcm_Msg_Info_s.respType = DCM_POS;
				DCM_NEW_REQ = 1;

			}
			else if (Dcm_Msg_Info_s.transType == DCM_FC_FRAME)/*FlowControl frame*/
			{

			}
			else if (Dcm_Msg_Info_s.transType == DCM_CC_FRAME)/*Consecutive frame*/
			{
				for (Consecutive_Next = 0; Consecutive_Next < 7; Consecutive_Next ++)
				{
					Dcm_Msg_Info_s.dataBuff[Consecutive_Next + Consecutive_Curr] = TransBuffer[Consecutive_Next + 1];
				}
				Consecutive_Curr += Consecutive_Next;

				if(Consecutive_Curr >= Dcm_Msg_Info_s.numByetReq)/*Last CC frame was sent*/
				{
					temSID = Dcm_Msg_Info_s.Sid;
					/*Search loop for supported SIDs*/
					for(LoopIndx = 0; LoopIndx < sizeof(SID_TABLE)/sizeof(SID_TABLE[0]); LoopIndx++)
					{
						if(temSID == SID_TABLE[LoopIndx].sid)
						{
							sidFound = 1;
							break;
						}
						else if((temSID - 0x40) == SID_TABLE[LoopIndx].sid)
						{
							Dcm_Msg_Info_s.Sid -= 0x40;
							sidFound = 1;
							break;
						}
						else{}
					}
					if(sidFound)
					{
						/*execute service function*/
						SID_TABLE[LoopIndx].service_fp(&Dcm_Msg_Info_s);
					}
					else
					{
						/*service is not supported, send NRC$11*/
						Dcm_Msg_Info_s.dataBuff[0] = 0x03;
						Dcm_Msg_Info_s.dataBuff[1] = 0x7F;
						Dcm_Msg_Info_s.dataBuff[2] = Dcm_Msg_Info_s.Sid;
						Dcm_Msg_Info_s.dataBuff[3] = NRC11_SERVICE_NOTSUPPORT;
						Dcm_Msg_Info_s.respType    = DCM_NEG;
					}
				}
				else
				{
					Dcm_Msg_Info_s.respType = DCM_POS;
					DCM_NEW_REQ = 1;
				}

			}
			else
			{
				Dcm_Msg_Info_s.respType = DCM_NORESP;
				DCM_NEW_REQ = 1;
			}
		}
	}
}
void CAN_DCM2TP()
{
	if(Dcm_Msg_Info_s.transType == DCM_SG_FRAME)/*Single request frame*/
	{
		if(Dcm_Msg_Info_s.respType == DCM_NORESP)
		{
			/*do nothing*/
		}
		else if(Dcm_Msg_Info_s.respType == DCM_NEG)
		{
			Dcm_Msg_Info_s.dataBuff[0] = 0x03;
			Dcm_Msg_Info_s.dataBuff[1] = 0x7F;
			Dcm_Msg_Info_s.dataBuff[2] = Dcm_Msg_Info_s.Sid;
		}
		else
		{

		}
	}
	else if (Dcm_Msg_Info_s.transType == DCM_FS_FRAME)/*First frame*/
	{
		Dcm_Msg_Info_s.dataBuff[0] = 0x30;
	}
	else if (Dcm_Msg_Info_s.transType == DCM_FC_FRAME)/*FlowControl frame*/
	{

	}
	else if (Dcm_Msg_Info_s.transType == DCM_CC_FRAME)/*Consecutive frame*/
	{
		if(Consecutive_Curr < Dcm_Msg_Info_s.numByetReq)
			Dcm_Msg_Info_s.dataBuff[0] = 0x20;

		if(Dcm_Msg_Info_s.respType == DCM_NORESP)
		{
			/*do nothing*/
		}
		else if(Dcm_Msg_Info_s.respType == DCM_NEG)
		{
			Dcm_Msg_Info_s.dataBuff[0] = 0x03;
			Dcm_Msg_Info_s.dataBuff[1] = 0x7F;
			Dcm_Msg_Info_s.dataBuff[2] = Dcm_Msg_Info_s.Sid;
		}
		else
		{

		}
	}
	else{}

}

uint16_t Read_CanID_Tester()
{
	uint16_t ret_val;
	uint32_t ret_CANID;

	ret_val = 0;
	ret_CANID = ProtocolI_RX;

	Dcm_Msg_Info_s.dataBuff[0] = 0x07;
	Dcm_Msg_Info_s.dataBuff[1] = Dcm_Msg_Info_s.Sid + 0x40;
	Dcm_Msg_Info_s.dataBuff[4] = ret_CANID >> 24;
	Dcm_Msg_Info_s.dataBuff[5] = ret_CANID >> 16;
	Dcm_Msg_Info_s.dataBuff[6] = ret_CANID >> 8;
	Dcm_Msg_Info_s.dataBuff[7] = ret_CANID;

	return ret_val;
}

uint16_t Read_CanID_ECU()
{
	uint16_t ret_val;
	uint32_t ret_CANID;

	ret_val = 0;
	ret_CANID = ProtocolI_TX;

	Dcm_Msg_Info_s.dataBuff[0] = 0x07;
	Dcm_Msg_Info_s.dataBuff[1] = Dcm_Msg_Info_s.Sid + 0x40;
	Dcm_Msg_Info_s.dataBuff[4] = ret_CANID >> 24;
	Dcm_Msg_Info_s.dataBuff[5] = ret_CANID >> 16;
	Dcm_Msg_Info_s.dataBuff[6] = ret_CANID >> 8;
	Dcm_Msg_Info_s.dataBuff[7] = ret_CANID;

	return ret_val;
}
uint16_t Write_CanID_Tester()
{
	uint16_t ret_val;
	uint32_t CANID;

	ret_val = 0;

	if(Seca_Timer == 0)
	{
		ret_val =  NRC33_SECURITY_ACCESS_DENIED;
		return ret_val;
	}
	else
	{
		CANID = 0;
		CANID = Dcm_Msg_Info_s.dataBuff[4];
		CANID = (CANID << 24) | Dcm_Msg_Info_s.dataBuff[5];
		CANID = (CANID << 16) | Dcm_Msg_Info_s.dataBuff[6];
		CANID = (CANID << 8 ) | Dcm_Msg_Info_s.dataBuff[7];
		if(((CANID & 0xFFFFFFFF) > 0x7FF) | (CANID == 0))
		{
			ret_val =  NRC31_DID_NOTSUPPORT;
			return ret_val;
		}
		else if(CANID == ProtocolI_TX)
		{
			ret_val =  NRC22_CONDITON_NOTCORRECT;
			return ret_val;
		}
		else
		{
			ProtocolI_RX = CANID;
			Dcm_Msg_Info_s.dataBuff[0] = 0x01;
			Dcm_Msg_Info_s.dataBuff[1] = Dcm_Msg_Info_s.Sid + 0x40;
		}
	}

	return ret_val;
}

uint16_t Write_CanID_ECU()
{
	uint16_t ret_val;
	uint32_t CANID;

	ret_val = 0;

	if(Seca_Timer == 0)
	{
		ret_val =  NRC33_SECURITY_ACCESS_DENIED;
		return ret_val;
	}
	else
	{
		CANID = 0;
		CANID = Dcm_Msg_Info_s.dataBuff[4];
		CANID = (CANID << 24) | Dcm_Msg_Info_s.dataBuff[5];
		CANID = (CANID << 16) | Dcm_Msg_Info_s.dataBuff[6];
		CANID = (CANID << 8 ) | Dcm_Msg_Info_s.dataBuff[7];
		if(((CANID & 0xFFFFFFFF) > 0x7FF) | (CANID == 0))
		{
			ret_val =  NRC31_DID_NOTSUPPORT;
			return ret_val;
		}
		else if(CANID == ProtocolI_RX)
		{
			ret_val =  NRC22_CONDITON_NOTCORRECT;
			return ret_val;
		}
		else
		{
			ProtocolI_TX = CANID;
			Dcm_Msg_Info_s.dataBuff[0] = 0x01;
			Dcm_Msg_Info_s.dataBuff[1] = Dcm_Msg_Info_s.Sid + 0x40;
		}
	}

	return ret_val;
}

uint16_t Read_22_2E_Data()
{
	uint16_t ret_val = 0;
	/*for further implement*/
	return ret_val;
}

uint16_t Write_22_2E_Data()
{
	uint16_t ret_val = 0;
	uint16_t loop;
	if(Seca_Timer == 0)
	{
		ret_val =  NRC33_SECURITY_ACCESS_DENIED;
		return ret_val;
	}
	else
	{
		for(loop = 0; loop < (Dcm_Msg_Info_s.numByetReq - 3); loop ++ )
		{
			dummy_eeprom_buffer[loop] = Dcm_Msg_Info_s.dataBuff[loop + 4];
		}
		Dcm_Msg_Info_s.dataBuff[0] = 0x01;
		Dcm_Msg_Info_s.dataBuff[1] = Dcm_Msg_Info_s.Sid + 0x40;

		ret_val = 0;
	}

	return ret_val;
}

uint16_t Gen_Ramdom_Seed()
{
	uint16_t ret_val = 0;

	Dcm_Msg_Info_s.dataBuff[0] = 0x06;
	Dcm_Msg_Info_s.dataBuff[1] = Dcm_Msg_Info_s.Sid + 0x40;
	Dcm_Msg_Info_s.dataBuff[3] = rand();/*seed0*/
	Dcm_Msg_Info_s.dataBuff[4] = rand();/*seed1*/
	Dcm_Msg_Info_s.dataBuff[5] = rand();/*seed2*/
	Dcm_Msg_Info_s.dataBuff[6] = rand();/*seed3*/
	Seca_Key_Internal[0] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[3] ^ Dcm_Msg_Info_s.dataBuff[4]);
	Seca_Key_Internal[1] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[4] + Dcm_Msg_Info_s.dataBuff[5]);
	Seca_Key_Internal[2] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[5] ^ Dcm_Msg_Info_s.dataBuff[6]);
	Seca_Key_Internal[3] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[6] + Dcm_Msg_Info_s.dataBuff[3]);
#if SECA_FLOWCONTROL == 1
	Seca_Key_Internal[4] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[3] | Dcm_Msg_Info_s.dataBuff[4]);
	Seca_Key_Internal[5] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[4] + Dcm_Msg_Info_s.dataBuff[5]);
	Seca_Key_Internal[6] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[5] | Dcm_Msg_Info_s.dataBuff[6]);
	Seca_Key_Internal[7] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[6] + Dcm_Msg_Info_s.dataBuff[3]);
	Seca_Key_Internal[8] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[3] & Dcm_Msg_Info_s.dataBuff[4]);
	Seca_Key_Internal[9] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[4] + Dcm_Msg_Info_s.dataBuff[5]);
	Seca_Key_Internal[10] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[5] & Dcm_Msg_Info_s.dataBuff[6]);
	Seca_Key_Internal[11] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[6] + Dcm_Msg_Info_s.dataBuff[3]);
	Seca_Key_Internal[12] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[3] - Dcm_Msg_Info_s.dataBuff[4]);
	Seca_Key_Internal[13] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[4] + Dcm_Msg_Info_s.dataBuff[5]);
	Seca_Key_Internal[14] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[5] - Dcm_Msg_Info_s.dataBuff[6]);
	Seca_Key_Internal[15] = (uint8_t)(Dcm_Msg_Info_s.dataBuff[6] + Dcm_Msg_Info_s.dataBuff[3]);
#else

#endif
	return ret_val;
}
uint16_t Compare_Key()
{
	uint16_t ret_val = 0;
	uint8_t Seca_Key_External[16];
	uint8_t loop = 0;

#if SECA_FLOWCONTROL == 1
	Seca_Key_External[0] = Dcm_Msg_Info_s.dataBuff[3];
	Seca_Key_External[1] = Dcm_Msg_Info_s.dataBuff[4];
	Seca_Key_External[2] = Dcm_Msg_Info_s.dataBuff[5];
	Seca_Key_External[3] = Dcm_Msg_Info_s.dataBuff[6];
	Seca_Key_External[4] = Dcm_Msg_Info_s.dataBuff[7];
	Seca_Key_External[5] = Dcm_Msg_Info_s.dataBuff[8];
	Seca_Key_External[6] = Dcm_Msg_Info_s.dataBuff[9];
	Seca_Key_External[7] = Dcm_Msg_Info_s.dataBuff[10];
	Seca_Key_External[8] = Dcm_Msg_Info_s.dataBuff[11];
	Seca_Key_External[9] = Dcm_Msg_Info_s.dataBuff[12];
	Seca_Key_External[10] = Dcm_Msg_Info_s.dataBuff[13];
	Seca_Key_External[11] = Dcm_Msg_Info_s.dataBuff[14];
	Seca_Key_External[12] = Dcm_Msg_Info_s.dataBuff[15];
	Seca_Key_External[13] = Dcm_Msg_Info_s.dataBuff[16];
	Seca_Key_External[14] = Dcm_Msg_Info_s.dataBuff[17];
	Seca_Key_External[15] = Dcm_Msg_Info_s.dataBuff[18];
	for(loop = 0; loop < 16; loop++)
	{
		if(Seca_Key_External[loop] != Seca_Key_Internal[loop])
		{
			break;
		}
	}
	if(loop < 16)
	{
		ret_val = NRC35_INVALID_KEY;
	}
#else
	Seca_Key_External[0] = Dcm_Msg_Info_s.dataBuff[3];
	Seca_Key_External[1] = Dcm_Msg_Info_s.dataBuff[4];
	Seca_Key_External[2] = Dcm_Msg_Info_s.dataBuff[5];
	Seca_Key_External[3] = Dcm_Msg_Info_s.dataBuff[6];

	for(loop = 0; loop < 4; loop++)
	{
		if(Seca_Key_External[loop] != Seca_Key_Internal[loop])
		{
			break;
		}
	}
	if(loop < 4)
	{
		ret_val = NRC35_INVALID_KEY;
	}
#endif
	else
	{
		Seca_Timer = 5000; /*Set Seca timer to 5 seconds*/
		Dcm_Msg_Info_s.dataBuff[0] = 0x02;
		Dcm_Msg_Info_s.dataBuff[1] = Dcm_Msg_Info_s.Sid + 0x40;
		Dcm_Msg_Info_s.dataBuff[2] = 0x02;
	}

	return ret_val;
}
