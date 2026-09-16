/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm_seca.h"
#include "can_tp.h"

extern CAN_HandleTypeDef hcan2;

// Luu tru Seed hien tai (6 byte) va trang thai mo khoa, cac moc thoi gian
static uint8_t Current_Seed[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
static uint8_t g_Seed_Requested = 0; // Co danh dau Tester da goi Request Seed (0x01)
uint8_t g_Security_Unlocked = 0;
uint32_t g_Security_Unlock_Timestamp = 0;
static uint32_t g_Security_Penalty_Timestamp = 0;

void DCM_Service_27_Practice(uint8_t* pPayload, uint16_t length)
{
    // Kiem tra do dai toi thieu de co byte Sub-function
    if (length < 2) {
        // NRC 0x13: Invalid length/response format
        uint8_t nrc_len[3] = {0x7F, 0x27, 0x13};
        CAN_TP_Transmit(nrc_len, 3, 0x7A2, &hcan2);
        return;
    }

    uint8_t sub_function = pPayload[1];

    // =====================================
    // 1. SUB-FUNCTION 0x01: REQUEST SEED
    // =====================================
    if (sub_function == 0x01) {

        // Bang 12: Do dai goi tin Request Seed phai dung 2 bytes (SID 0x27 + SubFunc 0x01)
        if (length != 2) {
            uint8_t nrc_len[3] = {0x7F, 0x27, 0x13};
            CAN_TP_Transmit(nrc_len, 3, 0x7A2, &hcan2);
            return;
        }

        // Kiem tra xe co dinh phat 10s vi nhap sai key khong
        if ((HAL_GetTick() - g_Security_Penalty_Timestamp) < 10000 && g_Security_Penalty_Timestamp != 0) {
            // Gui Negative Response: 0x7F 0x27 0x10 (General Reject / Delay not expired)
            uint8_t nrc_delay[3] = {0x7F, 0x27, 0x10};
            CAN_TP_Transmit(nrc_delay, 3, 0x7A2, &hcan2);
            return; // Dung, khong cap Seed
        }
        // Sinh Seed (Co the co dinh hoac random)
        Current_Seed[0] = 0x12;
        Current_Seed[1] = 0x34;
        Current_Seed[2] = 0x56;
        Current_Seed[3] = 0x78;
        Current_Seed[4] = 0x9A;
        Current_Seed[5] = 0xBC;

        // Danh dau Tester da xin Seed thanh cong
        g_Seed_Requested = 1;

        // Dong goi 8 byte: SID_Resp(0x67) + SubFunc(0x01) + 6 byte Seed
        uint8_t resp_data[8];
        resp_data[0] = 0x67;
        resp_data[1] = 0x01;
        for (int i = 0; i < 6; i++) {
            resp_data[i + 2] = Current_Seed[i];
        }

        // Gui qua CAN_TP (Ham nay tu dong gui First Frame va Consecutive Frame)
        CAN_TP_Transmit(resp_data, 8, 0x7A2, &hcan2);
    }

    // =======================================
    // 2. SUB-FUNCTION 0x02: SEND KEY
    // =======================================
    else if (sub_function == 0x02) {

        // Bang 14: Do dai goi tin Send Key phai dung 8 bytes (SID 0x27 + SubFunc 0x02 + 6 bytes Key)
        if (length != 8) {
            uint8_t nrc_len[3] = {0x7F, 0x27, 0x13};
            CAN_TP_Transmit(nrc_len, 3, 0x7A2, &hcan2);
            return;
        }

        // KIEM TRA THU TU: Bat buoc phai Request Seed (0x01) truoc khi Send Key (0x02)
        if (g_Seed_Requested == 0) {
            // NRC 0x24: requestSequenceError
            uint8_t nrc_seq[3] = {0x7F, 0x27, 0x24};
            CAN_TP_Transmit(nrc_seq, 3, 0x7A2, &hcan2);
            return;
        }

        // Seed chi duoc su dung 1 lan duy nhat (One-time use)
        g_Seed_Requested = 0;

        // Tinh key theo thuat toan da duoc cung cap
        uint8_t expected_key[6];
        expected_key[0] = Current_Seed[0] ^ Current_Seed[1];
        expected_key[1] = (uint8_t)(Current_Seed[1] + Current_Seed[2]);
        expected_key[2] = Current_Seed[2] ^ Current_Seed[3];
        expected_key[3] = (uint8_t)(Current_Seed[3] + Current_Seed[0]);
        expected_key[4] = Current_Seed[4] & 0xF0;
        expected_key[5] = Current_Seed[5] & 0x0F;

        // So khop voi 6 byte do tester gui sang (nam tu pPayload[2] -> pPayload[7])
        uint8_t key_valid = 1;
        for (int i = 0; i < 6; i ++) {
            if (pPayload[i + 2] != expected_key[i]) {
                key_valid = 0;
                break;
            }
        }

        // --- TH1: Neu dung key ---
        if (key_valid == 1) {
            g_Security_Unlocked = 1;
            g_Security_Unlock_Timestamp = HAL_GetTick(); // Luu lai thoi diem mo khoa
            g_Security_Penalty_Timestamp = 0;

            // Bat sang LED-0 (Chan PB0)
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

            // Gui Positive Response: 0x67 0x02 (Single Frame)
            uint8_t resp_success[2] = {0x67, 0x02};
            CAN_TP_Transmit(resp_success, 2, 0x7A2, &hcan2);
        }
        // --- TH2: Neu sai key ---
        else {
            g_Security_Unlocked = 0;
            g_Security_Penalty_Timestamp = HAL_GetTick(); // Kich hoat moc phat 10s

            // Tat LED-0
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

            // Gui Negative Response (NRC 0x35: Invalid Key)
            uint8_t nrc_invalid_key[3] = {0x7F, 0x27, 0x35};
            CAN_TP_Transmit(nrc_invalid_key, 3, 0x7A2, &hcan2);
        }
    }

    // =======================================
    // 3. SUB-FUNCTION KHONG DUOC HO TRO (NRC 0x12)
    // =======================================
    else {
        // NRC 0x12: SubFunctionNotSupported
        uint8_t nrc_subfunc[3] = {0x7F, 0x27, 0x12};
        CAN_TP_Transmit(nrc_subfunc, 3, 0x7A2, &hcan2);
    }
}

