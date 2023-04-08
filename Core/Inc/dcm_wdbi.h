/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#ifndef _DCM_WDBI_H
#define _DCM_WDBI_H

#include "dcm.h"

typedef struct
{
	uint16_t Did;
	uint8_t  FuncIndx;
	uint8_t  DinMinlength;
} Dcm_Wdbi_DID_table;

typedef struct
{
	Dcm_Wdbi_DID_table const *WdbiDidTable;
	uint16_t numDid;
	uint8_t InvalidLength;
	uint8_t DidNotSupport;
	uint8_t GeneralReject;
} Dcm_Wdbi_Conf;

extern void dcm_wdbi(Dcm_Msg_Info* MsgInfor);

#endif
