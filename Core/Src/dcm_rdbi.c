
/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm_rdbi.h"
#include "dcm.h"
#include "can_tp.h"

extern CAN_HandleTypeDef hcan2; // ECU gui phan hoi qua CAN2

// 1. Bien nhiet do ADC
extern uint16_t g_TemperatureSensorRawValue_u16[1];

// 2. Bien CAN ID hien tai tu main.c
extern uint16_t g_Current_CAN_ID;

void DCM_Service_22_Practice(uint8_t* pPayload, uint16_t length)
{
   // pPayload[0] la lenh SID (0x22)
   // 2 byte tiep theo la DID (Ma so du lieu muon doc)
   uint16_t did = (pPayload[1] << 8) | pPayload[2];

   // ==========================================================
   // KIEM TRA: Do dai goi tin yeu cau
   // Bang 2 & 7: SID (1 byte) + DID (2 byte) = 3 bytes
   // ==========================================================
   if (length != 3) {
       // NRC 0x13: Invalid length/response format
       uint8_t nrc_len[3] = {0x7F, 0x22, 0x13};
       CAN_TP_Transmit(nrc_len, 3, 0x7A2, &hcan2);
       return;
   }

   // ==========================================================
   // PRACTICE 1: Doc CAN ID hien tai cua Tester (DID = 0x0123)
   // Response format: 62 01 23 <CANID_H> <CANID_L>
   // ==========================================================
   if (did == 0x0123) 
   {
       uint8_t resp_id[5];
       resp_id[0] = 0x62;
       resp_id[1] = 0x01;
       resp_id[2] = 0x23;
       resp_id[3] = (uint8_t)(g_Current_CAN_ID >> 8);          // Byte cao
       resp_id[4] = (uint8_t)(g_Current_CAN_ID & 0xFF);        // Byte thap

       CAN_TP_Transmit(resp_id, 5, 0x7A2, &hcan2);
   }

   // ==========================================================
   // PRACTICE 2: Doc gia tri nhiet do tu ADC (DID = 0x0124)
   // Response format: 62 01 24 <Temperature>
   // ==========================================================
   else if (did == 0x0124) {

       // 1. Lay gia tri ADC tho tu DMA
       uint16_t raw_adc = g_TemperatureSensorRawValue_u16[0];

       // Kiem tra loi phan cung ADC (Bang 11: General Reject 0x10)
       if (raw_adc == 0) {
           uint8_t nrc_hw[3] = {0x7F, 0x22, 0x10};
           CAN_TP_Transmit(nrc_hw, 3, 0x7A2, &hcan2);
           return;
       }

       // 2. Doi ADC sang dien ap (mV)
       uint32_t v_sense = (raw_adc * 3300) / 4095;

       // 3. Tinh nhiet do theo chuan datasheet STM32F405
       // Công thức gốc: Temp = ((Vsense - V25) / Avg_Slope) + 25
       uint8_t real_temp = (uint8_t)(((v_sense - 760) * 10) / 25 + 25);

       // 4. Dong goi phan hoi
       uint8_t resp_temp[4];
       resp_temp[0] = 0x62;          // (0x22 + 0x40)
       resp_temp[1] = 0x01;
       resp_temp[2] = 0x24;
       resp_temp[3] = real_temp;     // Nhiet do thuc te

       CAN_TP_Transmit(resp_temp, 4, 0x7A2, &hcan2);
   }

   // ==========================================================
   // PRACTICE 3 (TEST MULTI-FRAME): Đọc VIN number (DID = 0xF190)
   // Phản hồi 20 bytes: 62 F1 90 + 17 bytes VIN (Tự động bẻ 3 frame CAN: FF, CF1, CF2)
   // ==========================================================
   else if (did == 0xF190) {
       uint8_t resp_vin[20];
       resp_vin[0] = 0x62;
       resp_vin[1] = 0xF1;
       resp_vin[2] = 0x90;
       for (int i = 0; i < 17; i++) {
           resp_vin[3 + i] = g_VIN_Buffer[i];
       }
       CAN_TP_Transmit(resp_vin, 20, 0x7A2, &hcan2);
   }

   // ==========================================================
   // Negative Response: DID khong ho tro (NRC 0x31)
   // ==========================================================
   else {
       uint8_t nrc_did[3] = {0x7F, 0x22, 0x31};
       CAN_TP_Transmit(nrc_did, 3, 0x7A2, &hcan2);
   }
}
