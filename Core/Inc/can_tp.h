#ifndef CAN_TP_H
#define CAN_TP_H

#include "main.h"

// 1. Khai bao kieu con tro ham
typedef void (*TP_Action_Func)(uint8_t* pCanData);

// 2. Struct dinh tuyen PCI
typedef struct {
    uint8_t PCIType;
    TP_Action_Func execute;
} TP_Router_t;

// 3. Ham chinh de nhan du lieu tu ngat can
void CAN_TP_RxIndication(uint8_t* pCanData);

void CAN_TP_Transmit(uint8_t* pPayload, uint16_t length, uint32_t Target_CAN_ID, CAN_HandleTypeDef* hcan);


#endif /*CAN_TP_H*/