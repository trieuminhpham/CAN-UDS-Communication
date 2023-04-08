
/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm_wdbi.h"


Dcm_Wdbi_DID_table const Dcm_Wdbi_did_Table1[]=
{
	{
			0x0123,
			WRITE_CANID_TESTER,
			0x07
	},
	{
			0x0321,
			WRITE_CANID_ECU,
			0x07
	},
	{
			0xF001,
			WRITE_22and2E_DATA,
			0x08,
	},
};

Dcm_Wdbi_Conf const Dcm_Wdbi_Conf1=
{
	&Dcm_Wdbi_did_Table1[0],
	sizeof(Dcm_Wdbi_did_Table1)/sizeof(Dcm_Wdbi_did_Table1[0]),
	NRC13_INVALID_LEN,
	NRC31_DID_NOTSUPPORT,
	NRC10_GENERAL_REJECT
};

void dcm_wdbi(Dcm_Msg_Info* MsgInfor)
{
	uint8_t  NegRes;
	uint16_t Tem_DID;
	uint8_t  LoopIdx;
	uint8_t  FuncIdx;
	uint16_t Dcm_Func_retval;

	/*Local variable initialization*/
	Tem_DID = 0x0000;
	NegRes = 0x00;
	LoopIdx = 0x00;
	Dcm_Func_retval = 0x0000;

	if(MsgInfor->numByetReq < 4)
	{
		/*check if minimum request length is correct*/
		NegRes = Dcm_Wdbi_Conf1.InvalidLength;
	}
	else
	{
		/*Get DID value from request buffer*/
		Tem_DID  = (uint16_t)MsgInfor->dataBuff[2];
		Tem_DID  = Tem_DID<<8;
		Tem_DID |=(uint16_t)MsgInfor->dataBuff[3];

		/*Search DIDs*/
		for(LoopIdx = 0; LoopIdx < Dcm_Wdbi_Conf1.numDid ; LoopIdx++)
		{
			if(Dcm_Wdbi_Conf1.WdbiDidTable[LoopIdx].Did == Tem_DID)
			{
				break;
			}
		}
		if(LoopIdx >= Dcm_Wdbi_Conf1.numDid)
		{
			NegRes = Dcm_Wdbi_Conf1.DidNotSupport;
		}
		else
		{
			FuncIdx = Dcm_Wdbi_Conf1.WdbiDidTable[LoopIdx].FuncIndx;
		}

		/*DIDs minimum length check*/
		if(MsgInfor->numByetReq < Dcm_Wdbi_Conf1.WdbiDidTable[LoopIdx].DinMinlength)
		{
			NegRes = Dcm_Wdbi_Conf1.InvalidLength;
		}
	}

	if(NegRes == 0x00)
	{
		/*Execute service DID and send Positive response*/
		Dcm_Func_retval = (*dcm_funcs_fp[FuncIdx])();

		switch (Dcm_Func_retval)
		{
			case POS_RES:
			{
				/*Send Positive response*/
				MsgInfor->respType = DCM_POS;
				break;
			}
			case NRC33_SECURITY_ACCESS_DENIED:
			{
				/*for further practice*/
				NegRes = NRC33_SECURITY_ACCESS_DENIED;
				break;
			}
			case NRC31_DID_NOTSUPPORT:
			{
				/*for further practice*/
				NegRes = NRC31_DID_NOTSUPPORT;
				break;
			}
			case NRC22_CONDITON_NOTCORRECT:
			{
				/*for further practice*/
				NegRes = NRC22_CONDITON_NOTCORRECT;
				break;
			}
			default:
			{
				/*for further practice*/
				NegRes = Dcm_Wdbi_Conf1.GeneralReject;
				break;
			}
		}

	}
	if(NegRes != 0x00)
	{
		/*Send negative response*/
		MsgInfor->dataBuff[3] = NegRes;
		MsgInfor->respType = DCM_NEG;
	}

	DCM_NEW_REQ = 1;
	return;
}

