/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#ifndef _DCM_RDBI_H
#define _DCM_RDBI_H

#include "dcm.h"

typedef struct
{
	uint16_t Did;
	uint8_t  FuncIndx;
} Dcm_Rdbi_DID_table;

typedef struct
{
	Dcm_Rdbi_DID_table const *RdbiDidTable;
	uint16_t numDid;
	uint8_t InvalidLength;
	uint8_t DidNotSupport;
	uint8_t GeneralReject;
} Dcm_Rdbi_Conf;

extern void dcm_rdbi(Dcm_Msg_Info* MsgInfor);

#endif
