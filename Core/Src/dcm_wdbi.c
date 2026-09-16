
/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm_wdbi.h"
#include "dcm.h"
#include "can_tp.h"

extern CAN_HandleTypeDef hcan2;
extern uint8_t g_Security_Unlocked;

// Bien luu CAN ID moi (cho Ignition Cycle moi ap dung)
uint16_t g_Pending_New_CAN_ID = 0x012; // Mac dinh la can ID goc
uint8_t g_Pending_ID_Ready = 0;        // Co bao gia tri moi cho ap dung

void DCM_Service_2E_Practice(uint8_t* pPayload, uint16_t length)
{
    // ======================================
    // KIEM TRA 1: Bao mat da mo khoa chua
    // ======================================
    if (g_Security_Unlocked == 0) {
        // NRC 0x33: Security Access Denied
        uint8_t nrc_denied[3] = {0x7F, 0x2E, 0x33};
        CAN_TP_Transmit(nrc_denied, 3, 0x7A2, &hcan2);
        return;
    }

    if (length < 3) {
        uint8_t nrc_length[3] = {0x7F, 0x2E, 0x13};
        CAN_TP_Transmit(nrc_length, 3, 0x7A2, &hcan2);
        return;
    }

    uint16_t did = (pPayload[1] << 8) | pPayload[2];

    // =============================================
    // PRACTICE 1: Ghi New CAN ID (DID = 0x0123)
    // Bang 19: SID (1) + DID (2) + CAN ID (2) = 5 byte
    // =============================================
    if (did == 0x0123) 
    {
        if (length != 5) {
            uint8_t nrc_length[3] = {0x7F, 0x2E, 0x13};
            CAN_TP_Transmit(nrc_length, 3, 0x7A2, &hcan2);
            return;
        }

        // Kiểm tra Range New CAN ID theo Bảng 19 Spec Bosch (Byte 4 <= 0x7F)
        if (pPayload[3] > 0x7F) {
            uint8_t nrc_range[3] = {0x7F, 0x2E, 0x31};
            CAN_TP_Transmit(nrc_range, 3, 0x7A2, &hcan2);
            return;
        }

        uint16_t new_can_id = ((uint16_t)pPayload[3] << 8) | pPayload[4];
        g_Pending_New_CAN_ID = new_can_id;
        g_Pending_ID_Ready = 1; // Danh dau cho Ignition ap dung

        // Positive Response: 0x6E 0x01 0x23
        uint8_t resp_success[3] = {0x6E, 0x01, 0x23};
        CAN_TP_Transmit(resp_success, 3, 0x7A2, &hcan2);
    }
    // =============================================
    // PRACTICE 2 (TEST MULTI-FRAME): Ghi VIN Number (DID = 0xF190)
    // Tổng chiều dài 20 byte: SID (1) + DID (2) + 17 bytes Data (Nhận qua 3 frame CAN)
    // =============================================
    else if (did == 0xF190)
    {
        if (length != 20) {
            uint8_t nrc_length[3] = {0x7F, 0x2E, 0x13};
            CAN_TP_Transmit(nrc_length, 3, 0x7A2, &hcan2);
            return;
        }

        for (int i = 0; i < 17; i++) {
            g_VIN_Buffer[i] = pPayload[3 + i];
        }

        // Positive Response: 0x6E 0xF1 0x90 (Single Frame 3 byte)
        uint8_t resp_success[3] = {0x6E, 0xF1, 0x90};
        CAN_TP_Transmit(resp_success, 3, 0x7A2, &hcan2);
    }
    // =============================================
    // Negative Response: DID khong ho tro (NRC 0x31)
    // =============================================
    else {
        uint8_t nrc_did[3] = {0x7F, 0x2E, 0x31};
        CAN_TP_Transmit(nrc_did, 3, 0x7A2, &hcan2);
    }
}

