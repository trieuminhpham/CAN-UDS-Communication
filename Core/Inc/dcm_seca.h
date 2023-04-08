/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#ifndef _DCM_SECA_H
#define _DCM_SECA_H

#include "dcm.h"

typedef struct
{
	uint16_t Subfunc;
	uint8_t  FuncIndx;
} Dcm_Seca_Subfunc_table;

typedef struct
{
	Dcm_Seca_Subfunc_table const *SecaSubFuncTable;
	uint16_t numSub;
	uint8_t InvalidLength;
	uint8_t SubFuncNotSupport;
	uint8_t SequenceError;
	uint8_t InvalidKeys;
	uint8_t GeneralReject;
} Dcm_Seca_Conf;

extern void dcm_seca(Dcm_Msg_Info* MsgInfor);

#endif
