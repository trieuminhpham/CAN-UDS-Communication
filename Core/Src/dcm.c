/*********************************************************/
/*********BOSCH BEA PROGRAM SKELETON DEMO CODE************/
/*********************************************************/

#include "dcm.h"
/*for further services please add service header here*/
#include "dcm_rdbi.h"
#include "dcm_wdbi.h"
#include "dcm_seca.h"
#include "can_tp.h"

extern CAN_HandleTypeDef hcan2;

// Dữ liệu mẫu VIN ban đầu (17 bytes: "BOSCH_BEA_2026_VN")
uint8_t g_VIN_Buffer[17] = {'B', 'O', 'S', 'C', 'H', '_', 'B', 'E', 'A', '_', '2', '0', '2', '6', '_', 'V', 'N'};

// Bang dinh tuyen tang UDS
const DCM_Router_t DCM_Table[] = {
    {0x22, DCM_Service_22_Practice},
    {0x27, DCM_Service_27_Practice},
    {0x2E, DCM_Service_2E_Practice}
};

// Ham nay duoc tang can_tp.c goi khi da boc tach xong request
void DCM_Process_Request(uint8_t* pPayload, uint16_t length) {
    if (pPayload == NULL || length == 0) return;

    uint8_t received_sid = pPayload[0]; // Byte dau tien la ma SID

    // Quet bang de nhay dung vao ham service
    for (int i = 0; i < sizeof(DCM_Table) / sizeof(DCM_Table[0]); i++) {
        if (DCM_Table[i].SID == received_sid) {
            DCM_Table[i].execute(pPayload, length);
            return;
        }
    }

    // Neu SID khong duoc ho tro trong he thong -> Tra ve NRC 0x11 (ServiceNotSupported)
    uint8_t nrc_unknown_sid[3] = {0x7F, received_sid, 0x11};
    CAN_TP_Transmit(nrc_unknown_sid, 3, 0x7A2, &hcan2);
}

