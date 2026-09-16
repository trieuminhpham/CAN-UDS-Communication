/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#ifndef _DCM_H
#define _DCM_H

#include "main.h"
#include "stm32f4xx_it.h"

// 1. Khai bao con tro ham cho UDS
typedef void (*DCM_Service_Func)(uint8_t* pPayload, uint16_t length);

// 2. Struct dinh tuyen Service ID (SID)
typedef struct {
    uint8_t SID;
    DCM_Service_Func execute;
} DCM_Router_t;

// Ham chinh de xu ly goi tin UDS tu tang TP
void DCM_Process_Request(uint8_t* pPayload, uint16_t length);

// Buffer luu VIN number de test Multi-Frame dai 20 bytes (17 bytes data)
extern uint8_t g_VIN_Buffer[17];

#endif
