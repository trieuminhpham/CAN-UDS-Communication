/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm_seca.h"

Dcm_Seca_Subfunc_table const Dcm_Seca_Subfunc_Table1[]=
{
	{
			0x01,
			GEN_SEED_LV1
	},
	{
			0x02,
			COMPARE_KEY_LV1
	}
};

Dcm_Seca_Conf const Dcm_Seca_Conf1=
{
	&Dcm_Seca_Subfunc_Table1[0],
	sizeof(Dcm_Seca_Subfunc_Table1)/sizeof(Dcm_Seca_Subfunc_Table1[0]),
	NRC13_INVALID_LEN,
	NRC31_DID_NOTSUPPORT,
	NRC24_SEQUENCE_ERROR,
	NRC35_INVALID_KEY,
	NRC10_GENERAL_REJECT
};

/*Service state: 0->Seed, 1->Key*/
uint8_t Service_Stae = 0;

void dcm_seca(Dcm_Msg_Info* MsgInfor)
{
	uint8_t  NegRes;
	uint8_t Tem_SubFunc;
	uint8_t  LoopIdx;
	uint8_t  FuncIdx;
	uint16_t Dcm_Func_retval;

	/*Local variable initialization*/
	Tem_SubFunc = 0x0000;
	NegRes = 0x00;
	LoopIdx = 0x00;
	Dcm_Func_retval = 0x0000;


	if(MsgInfor->numByetReq > 0x06)
	{
		/*dirty code for flow control with fixed keys length*/
		NegRes = 0;
	}
	else if((MsgInfor->numByetReq != 0x02)&&(Service_Stae==0))
	{
		/*check if request length is correct*/
		NegRes = Dcm_Seca_Conf1.InvalidLength;
	}
	else if((MsgInfor->numByetReq != 0x06)&&(Service_Stae==1))
	{
		/*check if request length is correct*/
		NegRes = Dcm_Seca_Conf1.InvalidLength;
	}
	else{}

	if (NegRes == 0)
	{
		/*Get DID value from request buffer*/
		Tem_SubFunc = MsgInfor->dataBuff[2];

		/*Search DIDs*/
		for(LoopIdx = 0; LoopIdx < Dcm_Seca_Conf1.numSub ; LoopIdx++)
		{
			if(Dcm_Seca_Conf1.SecaSubFuncTable[LoopIdx].Subfunc == Tem_SubFunc)
			{
				break;
			}
		}
		if(LoopIdx >= Dcm_Seca_Conf1.numSub)
		{
			NegRes = Dcm_Seca_Conf1.SubFuncNotSupport;
		}
		else
		{
			if((Tem_SubFunc % 2 == 0) && (Service_Stae == 0))
			{
				NegRes = Dcm_Seca_Conf1.SequenceError;
			}
			else if((Tem_SubFunc % 2 != 0) && (Service_Stae == 1))
			{
				NegRes = Dcm_Seca_Conf1.SequenceError;
			}
			else
			{
				if (Service_Stae == 0) Service_Stae = 1;
				else                   Service_Stae = 0;
				FuncIdx = Dcm_Seca_Conf1.SecaSubFuncTable[LoopIdx].FuncIndx;
			}
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
			case NRC35_INVALID_KEY:
			{
				/*invalid key*/
				NegRes = NRC35_INVALID_KEY;
				break;
			}
			case NRC33_SECURITY_ACCESS_DENIED:
			{
				/*for further practice*/
				NegRes = NRC33_SECURITY_ACCESS_DENIED;
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
				NegRes = Dcm_Seca_Conf1.GeneralReject;
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

