/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#ifndef _DCM_H
#define _DCM_H

#include "main.h"
#include "stm32f4xx_it.h"

#define SECA_FLOWCONTROL 1

#define POS_RES                        0x00 /*Positive response*/
#define NRC13_INVALID_LEN              0x13 /*requested length was not correct*/
#define NRC31_DID_NOTSUPPORT           0x31 /*requested did was not supported*/
#define NRC33_SECURITY_ACCESS_DENIED   0x33 /*security access denied*/
#define NRC22_CONDITON_NOTCORRECT      0x22 /*condition is not correct*/
#define NRC11_SERVICE_NOTSUPPORT       0x11 /*requested service was not supported*/
#define NRC10_GENERAL_REJECT           0x10 /*general reject*/
#define NRC24_SEQUENCE_ERROR           0x24 /*Security access sequence error*/
#define NRC35_INVALID_KEY              0x35 /*Security access invalid keys*/
/*you can define more NRC code here to practice on other service*/

#define DCM_SG_FRAME 0x00 /*Single frame*/
#define DCM_FS_FRAME 0x10 /*First frame*/
#define DCM_FC_FRAME 0x30 /*Flow control frame*/
#define DCM_CC_FRAME 0x20 /*Consecutive frame*/

extern uint32_t ProtocolI_RX;
extern uint32_t ProtocolI_TX;
extern uint8_t DCM_NEW_REQ;
extern uint8_t Seca_Key_Internal[16];
extern uint16_t Seca_Timer; /*Seca will be enabled within 5000ms*/
extern uint8_t dummy_eeprom_buffer [1024];

extern uint16_t Consecutive_Curr;
extern uint16_t Consecutive_Next;

enum
{
	DCM_NORESP,
	DCM_POS,
	DCM_NEG,
	DCM_WAIT,
};

enum
{
	READ_CANID_TESTER,
	READ_CANID_ECU,
	WRITE_CANID_TESTER,
	WRITE_CANID_ECU,
	READ_22and2E_DATA,
	WRITE_22and2E_DATA,
	GEN_SEED_LV1,
	COMPARE_KEY_LV1
};

typedef struct
{
	uint8_t   Sid;
	uint8_t   dataBuff[4096];
	uint16_t  dataLen;
	uint16_t  numByetReq;
	uint16_t  numByteRes;
	uint8_t   respType;
	uint8_t   transType;
	uint32_t   protoID;
}Dcm_Msg_Info;
extern Dcm_Msg_Info Dcm_Msg_Info_s;

typedef void (*dcm_service_fp)(Dcm_Msg_Info* Dcm_Msg_Info);

typedef const struct
{
	dcm_service_fp service_fp;
	uint8_t        sid;
}dcm_service_table;



typedef uint16_t dcm_func_table ();



extern void CAN_TP2DCM(uint32_t CANID, uint8_t * TransBuffer);
extern void CAN_DCM2TP();
extern uint16_t Read_CanID_Tester();
extern uint16_t Read_CanID_ECU();
extern uint16_t Write_CanID_Tester();
extern uint16_t Write_CanID_ECU();
extern uint16_t Read_22_2E_Data();
extern uint16_t Write_22_2E_Data();
extern uint16_t Gen_Ramdom_Seed();
extern uint16_t Compare_Key();

extern uint16_t (*const dcm_funcs_fp[])();

#endif
