#include "can_tp.h"
#include "dcm.h"
#include <string.h>

extern CAN_HandleTypeDef hcan2;
static uint16_t s_tp_rx_total_len = 0;
static uint16_t s_tp_rx_received_len = 0;
static uint8_t  s_tp_rx_expected_sn = 1;

// Bo dem de chua data sau khi boc tach
uint8_t TP_Rx_Buffer[256];

volatile uint8_t Flg_FC_Receive = 0; // Co bao hieu khi nhan duoc flow control

// 1. Ham xu ly goi don (Single Frame)
void TP_Handle_SingleFrame(uint8_t* pCanData) {
    uint8_t length = pCanData[0] & 0x0F; // Lay do dai data tu 4 bit thap

    // Copy phan data vao loi buffer
    for (int i = 0; i < length; i++) {
        TP_Rx_Buffer[i] = pCanData[i + 1];
    }

    // Nem phan data loi len tang DCM xu ly
    DCM_Process_Request(TP_Rx_Buffer, length);
}

// 2. Cac ham xu ly multi-frame
void TP_Handle_FirstFrame(uint8_t* pCanData) {
    s_tp_rx_total_len = ((pCanData[0] & 0x0F) << 8) | pCanData[1];

    CAN_TxHeaderTypeDef h;
    uint32_t mb;
    h.StdId = 0x7A2;
    h.ExtId = 0;
    h.IDE = CAN_ID_STD;
    h.RTR = CAN_RTR_DATA;
    h.DLC = 8;

    // Kiem tra thuc trang 1: Tran bo dem (Overflow)
    if (s_tp_rx_total_len > sizeof(TP_Rx_Buffer)) {
        uint8_t fc_ovflw[8] = {0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // FS = 2 (Overflow)
        uint32_t retry = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) == 0 && retry--) {}
        HAL_CAN_AddTxMessage(&hcan2, &h, fc_ovflw, &mb);
        s_tp_rx_total_len = 0;
        s_tp_rx_received_len = 0;
        return;
    }

    // Kiem tra thuc trang 2: Bo dem du cho -> Luu 6 byte dau tien
    for (int i = 0; i < 6 && i < s_tp_rx_total_len; i++) {
        TP_Rx_Buffer[i] = pCanData[i + 2];  
    }
    s_tp_rx_received_len = 6;
    s_tp_rx_expected_sn = 1; // CF dau tien phai co Sequence Number = 1

    // ECU tu dong gui Flow Control CTS (0x30) de Tester gui not phan con lai
    uint8_t fc[8] = {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint32_t retry = 50000;
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) == 0 && retry--) {}
    HAL_CAN_AddTxMessage(&hcan2, &h, fc, &mb);
}

void TP_Handle_ConsecutiveFrame(uint8_t* pCanData) {
    if (s_tp_rx_total_len == 0 || s_tp_rx_received_len >= s_tp_rx_total_len) {
        return;
    }

    // Kiem tra Sequence Number (SN) cua frame CF
    uint8_t sn = pCanData[0] & 0x0F;
    if (sn != s_tp_rx_expected_sn) {
        // Sai Sequence Number -> Huy phien nhan de bao ve tinh toan ven du lieu
        s_tp_rx_total_len = 0;
        s_tp_rx_received_len = 0;
        return;
    }
    s_tp_rx_expected_sn = (s_tp_rx_expected_sn + 1) & 0x0F; // Tang SN va quay vong tu 15 ve 0

    uint16_t remaining = s_tp_rx_total_len - s_tp_rx_received_len;
    uint8_t chunk_len = (remaining > 7) ? 7 : (uint8_t)remaining;

    for (int i = 0; i < chunk_len; i++) {
        TP_Rx_Buffer[s_tp_rx_received_len + i] = pCanData[i + 1];
    }
    s_tp_rx_received_len += chunk_len;

    // Khi da gom du toan bo goi tin -> Chuyen len tang DCM xu ly
    if (s_tp_rx_received_len >= s_tp_rx_total_len) {
        DCM_Process_Request(TP_Rx_Buffer, s_tp_rx_total_len);
        s_tp_rx_total_len = 0;
        s_tp_rx_received_len = 0;
    }
}

void TP_Handle_FlowControlFrame(uint8_t* pCanData) {
    Flg_FC_Receive = 1;
}

// 3. Bang dinh tuyen PCI
const TP_Router_t TP_Table[] = {
    {0x00, TP_Handle_SingleFrame},
    {0x10, TP_Handle_FirstFrame},
    {0x20, TP_Handle_ConsecutiveFrame},
    {0x30, TP_Handle_FlowControlFrame}
};

// 4. Ham nhan data tu ngat CAN
void CAN_TP_RxIndication(uint8_t* pCanData) {
    uint8_t pci = pCanData[0] & 0xF0; // Lay 4 bit cao cua byte 0 lam PCI

    // Quet mang struct de nhay vao dung ham xu ly
    for (int i = 0; i < 4; i++) {
        if (TP_Table[i].PCIType == pci) {
            TP_Table[i].execute(pCanData);
            break;
        }
    }
}

// 5. Ham phat du lieu ISO-TP tong quat (Single & Multi-Frame dong)
void CAN_TP_Transmit(uint8_t* pPayload, uint16_t length, uint32_t Target_CAN_ID, CAN_HandleTypeDef* hcan)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t TxMailbox;
    TxHeader.StdId = Target_CAN_ID;
    TxHeader.ExtId = 0;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.DLC = 8;

    uint8_t tx_data[8];

    // Neu data <= 7 BYTE: Gui Single Frame (vi du: 67 02, NRCs, Doc DID)
    if (length <= 7) {
        memset(tx_data, 0x00, 8);
        tx_data[0] = (uint8_t)length;
        for(int i = 0; i < length; i++) {
            tx_data[i + 1] = pPayload[i];
        }
        uint32_t retry = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0 && retry--) {}
        HAL_CAN_AddTxMessage(hcan, &TxHeader, tx_data, &TxMailbox);
    }
    // Neu data > 7 Byte: Gui Multi-Frame tong quat theo ISO 15765-2
    else {
        // A. Gui First Frame (FF) mang 6 byte dau tien
        memset(tx_data, 0x00, 8);
        tx_data[0] = 0x10 | ((length >> 8) & 0x0F);
        tx_data[1] = length & 0xFF;
        for (int i = 0; i < 6; i++) {
            tx_data[i + 2] = pPayload[i];
        }
        uint32_t retry = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0 && retry--) {}
        HAL_CAN_AddTxMessage(hcan, &TxHeader, tx_data, &TxMailbox);

        // B. Vong lap gui Consecutive Frame (CF) dong theo SN va so byte con lai
        uint16_t bytes_sent = 6;
        uint8_t sn = 1; // Sequence Number bat dau tu 1

        while (bytes_sent < length) {
            // Cho frame truoc do phat xong tren bus
            for (volatile uint32_t d = 0; d < 50000; d++);

            memset(tx_data, 0x00, 8);
            tx_data[0] = 0x20 | (sn & 0x0F); // PCI Consecutive Frame kem SN

            uint8_t chunk_len = length - bytes_sent;
            if (chunk_len > 7) {
                chunk_len = 7;
            }

            for (int i = 0; i < chunk_len; i++) {
                tx_data[i + 1] = pPayload[bytes_sent + i];
            }

            bytes_sent += chunk_len;
            sn = (sn + 1) & 0x0F; // Modulo 16 (0..15)

            retry = 50000;
            while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0 && retry--) {}
            HAL_CAN_AddTxMessage(hcan, &TxHeader, tx_data, &TxMailbox);
        }
    }
}
