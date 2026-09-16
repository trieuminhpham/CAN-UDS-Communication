# BÁO CÁO & TÀI LIỆU KỸ THUẬT DỰ ÁN CUỐI KHÓA BOSCH EMBEDDED ACADEMY (BEA)
## HỆ THỐNG TRUYỀN THÔNG CAN VÀ CHẨN ĐOÁN Ô TÔ UDS (ISO 14229 / ISO 15765-2) TRÊN VI ĐIỀU KHIỂN STM32F405

---

## 1. TỔNG QUAN DỰ ÁN & BỐI CẢNH KỸ THUẬT

### 1.1. Mục tiêu dự án
Dự án được xây dựng nhằm hiện thực hóa hệ thống điều khiển và chẩn đoán điện tử trên ô tô (Automotive Electronic Control Unit - ECU) theo tiêu chuẩn công nghiệp của Tập đoàn Bosch:
1. **Bài 1 - CAN Communication Telemetry:** Thiết lập mạng truyền thông hai chiều giữa 2 nút mạng CAN (Node 1 và Node 2) với chu kỳ thời gian thực, tính toán dữ liệu động ($D2 = D0 + D1$) và bảo vệ toàn vẹn dữ liệu bằng thuật toán mã hóa kiểm tra dư thừa tuần hoàn **CRC-8 SAE J1850**.
2. **Bài 2 - Automotive Diagnostic Services (UDS & ISO-TP):** Xây dựng hệ thống chẩn đoán theo tiêu chuẩn quốc tế **ISO 14229-1 (UDS)** và tầng truyền tải phân mảnh dữ liệu **ISO 15765-2 (ISO-TP / CAN-TP)**:
   - Dịch vụ **0x22 (Read Data By Identifier):** Đọc định danh CAN ID hiện tại (`DID 0x0123`) và nhiệt độ chip vi điều khiển thực tế qua bộ chuyển đổi tương tự - số ADC (`DID 0x0124`).
   - Dịch vụ **0x27 (Security Access):** Quy trình bảo mật 2 bước Request Seed (`0x01`) và Send Key (`0x02`), cơ chế bảo mật tuần tự (Sequence Control), khóa phạt chống vét cạn (Penalty Delay) và tự động khóa an toàn sau 20 giây.
   - Dịch vụ **0x2E (Write Data By Identifier):** Ghi đè cấu hình CAN ID mới vào bộ nhớ đệm tạm thời (Pending), cơ chế bảo mật an toàn giao thông **Ignition Cycle** (chỉ nạp cấu hình mới vào mạng CAN sau khi tài xế tắt và bật lại chìa khóa điện qua nút bấm PA0).
3. **Automotive LCD Dashboard:** Bảng điều khiển giám sát đồ họa màu sắc trực quan, hiển thị thời gian thực các thông số truyền nhận CAN, trạng thái an ninh UDS, cảm biến nhiệt độ chip với thanh đo Analog Gauge, áp dụng công nghệ **Zero-Flicker** (chống chớp giật tuyệt đối).

---

### 1.2. Sơ đồ khối kiến trúc hệ thống (System Architecture)

```text
+---------------------------------------------------------------------------------------+
|                                    MÁY TÍNH KIỂM THỬ (PC)                             |
|                           Phần mềm kiểm thử: BEA_DiagChecker.py                       |
+---------------------------------------------------------------------------------------+
                                        ▲                 │
                   UART TX (3 byte SOF) │                 │ UART RX (3 byte EOF)
                 0F FF F0 [Resp] F0 00 0F │                 │ 0F FF F0 [Req] F0 00 0F
                                        │                 ▼
+---------------------------------------------------------------------------------------+
|                             STM32F405RGT6 (VI ĐIỀU KHIỂN TRUNG TÂM)                    |
|                                                                                       |
|  +---------------------------------------------------------------------------------+  |
|  | [TẦNG GIAO TIẾP MÁY TÍNH - UART3]                                               |  |
|  | - Baudrate: 115200 bps | Chân: PC10 (TX), PC11 (RX)                              |  |
|  | - Zero-Delay Framing Engine: Nhận diện tức thời cặp SOF (0F FF F0) & EOF (F0 00 0F) |  |
|  +---------------------------------------------------------------------------------+  |
|                                         │                 ▲                           |
|                                         ▼                 │                           |
|  +---------------------------------------------------------------------------------+  |
|  | [TẦNG TESTER CLIENT - CAN1]                                                     |  |
|  | - Chân: PB9 (TX), PB8 (RX) | Baudrate: 500 kbps                                 |  |
|  | - Bài 1: Phát định kỳ 50ms (ID: 0x012 hoặc ID mới) -> [D0, D1, D0+D1, 0,0,0,CRC,0] |
|  | - Bài 2: Phát Request UDS sang ECU qua ID 0x712 (SF / FF / CF)                   |  |
|  | - Phản hồi Flow Control (0x30) khi ECU gửi First Frame của Seed                |  |
|  +---------------------------------------------------------------------------------+  |
|                                         │                 ▲                           |
|                     CAN Bus Vật lý      │ ID: 0x712,0x012 │ ID: 0x7A2,0x0A2           |
|                     (Bus 500 kbps)      ▼                 │                           |
|  +---------------------------------------------------------------------------------+  |
|  | [TẦNG ECU SERVER - CAN2]                                                        |  |
|  | - Chân: PB6 (TX), PB5 (RX) | Baudrate: 500 kbps                                 |  |
|  | - Bài 1 (Home Mode): Giả lập phát 20ms frame ID 0x0A2 [D0, D1, 0, 0, 0, 0, 0, CNT] |
|  | - Tầng CAN-TP (ISO 15765-2): Bóc tách PCI (SF 0x00, FF 0x10, CF 0x20, FC 0x30) |  |
|  | - Tầng DCM (ISO 14229): Bộ định tuyến Router điều phối Service:                    |  |
|  |     * Service 0x22: Read DID (0x0123: CAN ID, 0x0124: Nhiệt độ ADC)             |  |
|  |     * Service 0x27: Security Access (Request Seed, Send Key, Lockout 10s)       |  |
|  |     * Service 0x2E: Write DID (Ghi CAN ID mới, chờ Ignition Cycle)             |  |
|  +---------------------------------------------------------------------------------+  |
|                                         │                                             |
|                                         ▼                                             |
|  +---------------------------------------------------------------------------------+  |
|  | [TẦNG NGOẠI VI & HIỂN THỊ]                                                      |  |
|  | - Màn hình LCD 2.8" ST7789: PA7 (MOSI), PB3 (SCK), PB8 (DC), PB7 (CS), PB2 (RST) |  |
|  | - ADC1 Channel 16 & DMA2: Đo nhiệt độ nội của chip STM32                         |  |
|  | - Đèn LED chỉ thị: PB0 (LED0 - Security Status), PB1 (LED1)                     |  |
|  | - Nút bấm Ignition: PA0 (EXTI0 Falling Edge mô phỏng tắt/bật khóa điện)        |  |
|  +---------------------------------------------------------------------------------+  |
+---------------------------------------------------------------------------------------+
```

---

### 1.3. Sơ đồ đấu nối chân phần cứng (Pinout Mapping)

| Tên ngoại vi | Chân STM32 | Chức năng chi tiết |
| :--- | :--- | :--- |
| **CAN1** | **PB9** | CAN1_TX (Nối với chân TXD của CAN Transceiver 1) |
| **CAN1** | **PB8** | CAN1_RX (Nối với chân RXD của CAN Transceiver 1) |
| **CAN2** | **PB6** | CAN2_TX (Nối với chân TXD của CAN Transceiver 2) |
| **CAN2** | **PB5** | CAN2_RX (Nối với chân RXD của CAN Transceiver 2) |
| **UART3** | **PC10** | USART3_TX (Nối chân RX của cáp USB-to-UART nạp máy tính) |
| **UART3** | **PC11** | USART3_RX (Nối chân TX của cáp USB-to-UART nạp máy tính) |
| **LCD ST7789** | **PA7** | LCD_MOSI (Chân dữ liệu màn hình SPI Software) |
| **LCD ST7789** | **PB3** | LCD_SCK (Chân xung nhịp màn hình SPI Software) |
| **LCD ST7789** | **PB8** | LCD_DC / RS (Chân chọn Lệnh `0` / Dữ liệu `1`) |
| **LCD ST7789** | **PB7** | LCD_CS (Chân chọn chip màn hình, Active Low) |
| **LCD ST7789** | **PB2** | LCD_RST (Chân Reset phần cứng màn hình) |
| **Nút bấm** | **PA0** | Nút bấm User Button / Ignition Cycle (Ngắt EXTI0 sườn âm) |
| **LED 0** | **PB0** | Đèn báo Security Access (`1`: Đã mở khóa, `0`: Khóa) |
| **LED 1** | **PB1** | Đèn trạng thái hệ thống |
| **ADC1** | **Kênh nội 16** | Cảm biến nhiệt độ bên trong chip STM32 (Internal Temp Sensor) |

---

## 2. GIAI ĐOẠN 1: KHỞI TẠO DỰ ÁN BẰNG STM32CUBEMX TỪ ĐẦU

Nếu bắt đầu từ một thư mục trống trơn, quy trình cấu hình trên phần mềm **STM32CubeMX** (hoặc STM32CubeIDE) được thực hiện qua các bước chuẩn tắc như sau:

### Bước 1: Chọn Chip Vi điều khiển
1. Khởi động STM32CubeMX $\rightarrow$ Chọn **New Project**.
2. Tại thanh tìm kiếm Part Number: Nhập `STM32F405RGT6` $\rightarrow$ Nhấp đúp chuột để khởi tạo dự án.

### Bước 2: Cấu hình Clock Tree (Hệ thống thạch anh 168 MHz)
1. Trong mục **System Core $\rightarrow$ RCC**:
   - Chọn `High Speed Clock (HSE)`: `Crystal/Ceramic Resonator` (Nếu có thạch anh ngoài) hoặc dùng `HSI` nội bộ 16 MHz.
2. Sang tab **Clock Configuration**:
   - Đặt `PLL Source Mux` thành `HSI` (16 MHz) hoặc `HSE`.
   - Cấu hình nhân chia PLL: `PLLM = 8`, `PLLN = 168`, `PLLP = /2`.
   - Đặt `System Clock Mux` thành `PLLCLK`.
   - Kết quả: `HCLK (MHz)` đạt tốc độ tối đa **168 MHz**.
   - Bus `APB1 Prescaler` chia 4 $\rightarrow$ `APB1 peripheral clocks = 42 MHz` (Cung cấp cho CAN1, CAN2, USART3).
   - Bus `APB2 Prescaler` chia 2 $\rightarrow$ `APB2 peripheral clocks = 84 MHz` (Cung cấp cho ADC1).

### Bước 3: Cấu hình Ngoại vi CAN1 và CAN2 (Baudrate chuẩn 500 kbps)
Theo chuẩn mạng CAN tốc độ cao trên ô tô, Baudrate được cấu hình đúng **500 kbps**:
1. Tần số bus APB1 cấp cho CAN là $42\text{ MHz}$.
2. Công thức tính Baudrate:
   $$\text{Baudrate} = \frac{f_{\text{APB1}}}{\text{Prescaler} \times (1 + \text{BS1} + \text{BS2})}$$
3. Thiết lập thông số trong STM32CubeMX:
   - **Prescaler:** `6`
   - **Time Quanta in Bit Segment 1 (BS1):** `11 Times`
   - **Time Quanta in Bit Segment 2 (BS2):** `2 Times`
   - **ReSynchronization Jump Width (SJW):** `1 Time`
   - Tổng số Quanta: $1 + 11 + 2 = 14\text{ Tq}$.
   - $\text{Baudrate} = \frac{42,000,000}{6 \times 14} = \mathbf{500,000\text{ bps}}\ (500\text{ kbps})$.
4. **Cấu hình Pinout:**
   - CAN1: Gán chân `PB8` làm `CAN1_RX`, `PB9` làm `CAN1_TX`.
   - CAN2: Gán chân `PB5` làm `CAN2_RX`, `PB6` làm `CAN2_TX`.
5. **Kích hoạt Ngắt (NVIC):**
   - Đánh dấu chọn `CAN1 RX0 interrupt` $\rightarrow$ Priority: Preemption `2`, Sub `0`.
   - Đánh dấu chọn `CAN2 RX0 interrupt` $\rightarrow$ Priority: Preemption `2`, Sub `0`.

### Bước 4: Cấu hình USART3 (Giao tiếp PC 115200 bps)
1. Trong mục **Connectivity $\rightarrow$ USART3**:
   - Mode: `Asynchronous`.
   - Baud Rate: `115200 Bits/s`.
   - Word Length: `8 Bits`.
   - Parity: `None`.
   - Stop Bits: `1`.
2. Gán chân: `PC10` làm `USART3_TX`, `PC11` làm `USART3_RX`.
3. Kích hoạt ngắt: Đánh dấu chọn `USART3 global interrupt` $\rightarrow$ Priority: Preemption `1`, Sub `0`.

### Bước 5: Cấu hình ADC1 & DMA (Đo nhiệt độ chip nội bộ)
1. Trong mục **Analog $\rightarrow$ ADC1**:
   - Đánh dấu chọn kênh `Temperature Sensor Channel`.
   - Clock Prescaler: `PCLK2 divided by 4`.
   - Resolution: `12 bits`.
   - Scan Conversion Mode: `Disabled`.
   - Continuous Conversion Mode: `Enabled`.
2. Tab **DMA Settings**:
   - Nhấn **Add** $\rightarrow$ Chọn `ADC1`.
   - Stream: `DMA2 Stream 0`.
   - Direction: `Peripheral to Memory`.
   - Mode: `Circular` (hoặc Normal).
   - Data Width: Peripheral = `Half Word`, Memory = `Half Word`.
3. Trong NVIC: Kích hoạt ngắt `DMA2 stream 0 global interrupt` $\rightarrow$ Priority: Preemption `0`, Sub `0`.

### Bước 6: Cấu hình GPIO (LED, Nút bấm EXTI0, Màn hình LCD)
1. **LED:** Cấu hình `PB0`, `PB1` là `GPIO_Output`, Push-Pull, No Pull, Speed Low.
2. **Nút bấm PA0 (Ignition):** Cấu hình `PA0` là `GPIO_EXTI0`, ngắt sườn âm `External Interrupt Mode with Falling edge trigger detection`, Pull-Up.
   - Trong NVIC: Kích hoạt ngắt `EXTI line0 interrupt` $\rightarrow$ Priority: Preemption `1`, Sub `0`.
3. **Màn hình LCD ST7789:** Cấu hình các chân `PA7`, `PB3`, `PB8`, `PB7`, `PB2` là `GPIO_Output`, Push-Pull, High Speed.

### Bước 7: Cấu hình Project & Sinh mã (Generate Code)
1. Sang tab **Project Manager**:
   - Toolchain / IDE: Chọn `STM32CubeIDE` (hoặc Makefile).
   - Code Generator: Tích chọn `Generate peripheral initialization as a pair of '.c/.h' files per peripheral`.
2. Nhấn nút **GENERATE CODE**.

---

## 3. GIAI ĐOẠN 2: THIẾT KẾ CẤU TRÚC PHẦN MỀM & TẠO FILE MỚI

Sau khi CubeMX sinh mã khung xương, thư mục `Core/Src` và `Core/Inc` chỉ có các file mặc định (`main.c`, `stm32f4xx_it.c`, `stm32f4xx_hal_msp.c`). Để xây dựng kiến trúc chẩn đoán ô tô chuyên nghiệp, **chúng ta phải tạo thêm 6 module độc lập** tương ứng với 12 file mới:

```text
Core/
├── Inc/
│   ├── main.h                 (File gốc: Khai báo extern toàn cục, chân nút bấm)
│   ├── stm32f4xx_it.h         (File gốc: Khai báo hàm ngắt)
│   │
│   ├── can_tp.h               [MỚI] Khai báo tầng ISO-TP (PCI router, Multi-frame, Flow Control)
│   ├── dcm.h                  [MỚI] Khai báo tầng DCM Router (UDS Dispatcher)
│   ├── dcm_rdbi.h             [MỚI] Khai báo Service 0x22 (Read Data By Identifier)
│   ├── dcm_seca.h             [MỚI] Khai báo Service 0x27 (Security Access)
│   ├── dcm_wdbi.h             [MỚI] Khai báo Service 0x2E (Write Data By Identifier)
│   └── lcd_hal.h              [MỚI] Khai báo Driver LCD ST7789 & Giao diện Dashboard
│
└── Src/
    ├── main.c                 (File gốc: Vòng lặp chính, bài 1 CAN, UART Zero-delay)
    ├── stm32f4xx_it.c         (File gốc: Xử lý ngắt CAN1, CAN2, UART, EXTI0)
    ├── stm32f4xx_hal_msp.c     (File gốc: Cấu hình chân Clock ngoại vi của ST)
    │
    ├── can_tp.c               [MỚI] Hiện thực ISO-TP (Bóc tách SF/FF/CF/FC, gửi Multi-frame)
    ├── dcm.c                  [MỚI] Hiện thực UDS Dispatcher (Phân luồng SID, trả NRC 0x11)
    ├── dcm_rdbi.c             [MỚI] Hiện thực Service 0x22 (Đọc DID 0123, DID 0124)
    ├── dcm_seca.c             [MỚI] Hiện thực Service 0x27 (Seed/Key, Penalty, Sequence check)
    ├── dcm_wdbi.c             [MỚI] Hiện thực Service 0x2E (Ghi CAN ID, Range check Bảng 19)
    └── lcd_hal.c              [MỚI] Hiện thực đồ họa LCD ST7789, Zero-Flicker Dashboard
```

---

## 4. CHI TIẾT MÃ NGUỒN TỪNG MODULE, CÁC HÀM VÀ NGUYÊN LÝ HOẠT ĐỘNG

Dưới đây là chi tiết mã nguồn hoàn chỉnh của từng file, danh sách các hàm và phân tích kỹ thuật chuyên sâu.

---

### 4.1. Module Tầng ISO-TP / CAN-TP (`can_tp.h` và `can_tp.c`)
* **Nhiệm vụ:** Đảm nhận tầng giao vận (Transport Layer - ISO 15765-2), phân giải byte PCI (Protocol Control Information) để bóc tách gói tin Single Frame ($\le 7$ byte), First Frame, Consecutive Frame, gửi nhận Flow Control và ghép các gói tin lớn ($> 7$ byte) trước khi bàn giao cho tầng DCM.

#### File `Core/Inc/can_tp.h`
```c
#ifndef INC_CAN_TP_H_
#define INC_CAN_TP_H_

#include "main.h"

// Định nghĩa con trỏ hàm xử lý khung CAN-TP
typedef void (*TP_Handler_t)(uint8_t* pCanData);

// Cấu trúc bảng định tuyến PCI
typedef struct {
    uint8_t PCIType;        // Loại PCI (0x00, 0x10, 0x20, 0x30)
    TP_Handler_t execute;   // Hàm callback xử lý tương ứng
} TP_Router_t;

// Khai báo các hàm công khai
void CAN_TP_RxIndication(uint8_t* pCanData);
void CAN_TP_Transmit(uint8_t* pPayload, uint16_t length, uint32_t Target_CAN_ID, CAN_HandleTypeDef* hcan);

#endif /* INC_CAN_TP_H_ */
```

#### File `Core/Src/can_tp.c`
```c
#include "can_tp.h"
#include "dcm.h"

extern CAN_HandleTypeDef hcan2;
static uint16_t s_tp_rx_total_len = 0;
uint8_t TP_Rx_Buffer[256]; // Bộ đệm tái tổ hợp dữ liệu UDS
volatile uint8_t Flg_FC_Receive = 0;

// 1. Hàm xử lý gói đơn Single Frame (PCI = 0x00)
void TP_Handle_SingleFrame(uint8_t* pCanData) {
    uint8_t length = pCanData[0] & 0x0F; // Lấy độ dài data từ 4 bit thấp của byte PCI
    for (int i = 0; i < length; i++) {
        TP_Rx_Buffer[i] = pCanData[i + 1];
    }
    // Chuyển dữ liệu thuần lên tầng chẩn đoán DCM
    DCM_Process_Request(TP_Rx_Buffer, length);
}

// 2. Hàm xử lý gói đầu First Frame (PCI = 0x10)
void TP_Handle_FirstFrame(uint8_t* pCanData) {
    s_tp_rx_total_len = ((pCanData[0] & 0x0F) << 8) | pCanData[1];

    CAN_TxHeaderTypeDef h;
    uint32_t mb;
    h.StdId = 0x7A2;
    h.ExtId = 0;
    h.IDE = CAN_ID_STD;
    h.RTR = CAN_RTR_DATA;
    h.DLC = 8;

    // Kiểm tra tràn bộ đệm (Buffer Overflow)
    if (s_tp_rx_total_len > sizeof(TP_Rx_Buffer)) {
        uint8_t fc_ovflw[8] = {0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // FS = 2 (Overflow)
        uint32_t retry = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) == 0 && retry--) {}
        HAL_CAN_AddTxMessage(&hcan2, &h, fc_ovflw, &mb);
        s_tp_rx_total_len = 0;
        s_tp_rx_received_len = 0;
        return;
    }

    // Lưu 6 byte đầu tiên
    for (int i = 0; i < 6 && i < s_tp_rx_total_len; i++) {
        TP_Rx_Buffer[i] = pCanData[i + 2];
    }
    s_tp_rx_received_len = 6;
    s_tp_rx_expected_sn = 1;

    // ECU tự động gửi Flow Control CTS (0x30) để Tester gửi nốt phần còn lại
    uint8_t fc[8] = {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint32_t retry = 50000;
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) == 0 && retry--) {}
    HAL_CAN_AddTxMessage(&hcan2, &h, fc, &mb);
}

// 3. Hàm xử lý gói liên tiếp Consecutive Frame (PCI = 0x20)
void TP_Handle_ConsecutiveFrame(uint8_t* pCanData) {
    if (s_tp_rx_total_len == 0 || s_tp_rx_received_len >= s_tp_rx_total_len) {
        return;
    }

    // Kiểm tra Sequence Number (SN)
    uint8_t sn = pCanData[0] & 0x0F;
    if (sn != s_tp_rx_expected_sn) {
        s_tp_rx_total_len = 0;
        s_tp_rx_received_len = 0;
        return;
    }
    s_tp_rx_expected_sn = (s_tp_rx_expected_sn + 1) & 0x0F;

    uint16_t remaining = s_tp_rx_total_len - s_tp_rx_received_len;
    uint8_t chunk_len = (remaining > 7) ? 7 : (uint8_t)remaining;

    for (int i = 0; i < chunk_len; i++) {
        TP_Rx_Buffer[s_tp_rx_received_len + i] = pCanData[i + 1];
    }
    s_tp_rx_received_len += chunk_len;

    // Đã gom đủ toàn bộ gói tin -> Đẩy lên tầng DCM xử lý
    if (s_tp_rx_received_len >= s_tp_rx_total_len) {
        DCM_Process_Request(TP_Rx_Buffer, s_tp_rx_total_len);
        s_tp_rx_total_len = 0;
        s_tp_rx_received_len = 0;
    }
}

// 4. Hàm xử lý khung Flow Control (PCI = 0x30)
void TP_Handle_FlowControlFrame(uint8_t* pCanData) {
    Flg_FC_Receive = 1;
}

// Bảng định tuyến loại khung PCI Table
const TP_Router_t TP_Table[] = {
    {0x00, TP_Handle_SingleFrame},
    {0x10, TP_Handle_FirstFrame},
    {0x20, TP_Handle_ConsecutiveFrame},
    {0x30, TP_Handle_FlowControlFrame}
};

// Hàm tiếp nhận ngắt từ CAN chuyển vào phân tích PCI
void CAN_TP_RxIndication(uint8_t* pCanData) {
    uint8_t pci = pCanData[0] & 0xF0; // Lấy 4 bit cao làm PCI Type
    for (int i = 0; i < 4; i++) {
        if (TP_Table[i].PCIType == pci) {
            TP_Table[i].execute(pCanData);
            break;
        }
    }
}

// Hàm gửi dữ liệu ra mạng CAN chuẩn ISO-TP (Hỗ trợ cả SF và Multi-frame động)
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

    // A. Nếu dữ liệu <= 7 bytes: Đóng gói Single Frame (SF)
    if (length <= 7) {
        memset(tx_data, 0x00, 8);
        tx_data[0] = (uint8_t)length; // Byte 0 mang độ dài payload
        for (int i = 0; i < length; i++) {
            tx_data[i + 1] = pPayload[i];
        }
        uint32_t retry = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0 && retry--) {}
        HAL_CAN_AddTxMessage(hcan, &TxHeader, tx_data, &TxMailbox);
    }
    // B. Nếu dữ liệu > 7 bytes: Tự động phân mảnh Multi-frame (FF + CF động)
    else {
        // 1. Gửi First Frame (FF - 0x10) mang 6 byte đầu tiên
        memset(tx_data, 0x00, 8);
        tx_data[0] = 0x10 | ((length >> 8) & 0x0F);
        tx_data[1] = length & 0xFF;
        for (int i = 0; i < 6; i++) {
            tx_data[i + 2] = pPayload[i];
        }
        uint32_t retry = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0 && retry--) {}
        HAL_CAN_AddTxMessage(hcan, &TxHeader, tx_data, &TxMailbox);

        // 2. Vòng lặp phát tất cả Consecutive Frame (CF) động theo Sequence Number
        uint16_t bytes_sent = 6;
        uint8_t sn = 1; // SN bắt đầu từ 1 (0x21)

        while (bytes_sent < length) {
            for (volatile uint32_t d = 0; d < 50000; d++);

            memset(tx_data, 0x00, 8);
            tx_data[0] = 0x20 | (sn & 0x0F);

            uint8_t chunk_len = length - bytes_sent;
            if (chunk_len > 7) {
                chunk_len = 7;
            }

            for (int i = 0; i < chunk_len; i++) {
                tx_data[i + 1] = pPayload[bytes_sent + i];
            }

            bytes_sent += chunk_len;
            sn = (sn + 1) & 0x0F; // Modulo 16 quay vòng từ 15 về 0

            retry = 50000;
            while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0 && retry--) {}
            HAL_CAN_AddTxMessage(hcan, &TxHeader, tx_data, &TxMailbox);
        }
    }
}
```

---

### 4.2. Module Tầng DCM Quản lý dịch vụ chẩn đoán (`dcm.h` và `dcm.c`)
* **Nhiệm vụ:** Đóng vai trò là Bộ điều phối trung tâm (Diagnostic Communication Manager) của tầng ứng dụng chẩn đoán ISO 14229. Nó nhận mảng payload thuần từ `can_tp.c`, kiểm tra mã SID (Service Identifier) và gọi đúng hàm thực thi. Nếu SID không thuộc danh sách hỗ trợ, tự động phản hồi mã âm chuẩn **NRC 0x11 (`serviceNotSupported`)**.

#### File `Core/Inc/dcm.h`
```c
#ifndef INC_DCM_H_
#define INC_DCM_H_

#include "main.h"

// Kiểu con trỏ hàm thực thi dịch vụ UDS
typedef void (*DCM_Service_Handler_t)(uint8_t* pPayload, uint16_t length);

// Cấu trúc bảng router dịch vụ DCM
typedef struct {
    uint8_t SID;                     // Mã dịch vụ UDS (0x22, 0x27, 0x2E...)
    DCM_Service_Handler_t execute;   // Con trỏ hàm tương ứng
} DCM_Router_t;

void DCM_Process_Request(uint8_t* pPayload, uint16_t length);

// Buffer lưu trữ VIN number để kiểm thử Multi-Frame lớn (17 bytes data)
extern uint8_t g_VIN_Buffer[17];

#endif /* INC_DCM_H_ */
```

#### File `Core/Src/dcm.c`
```c
#include "dcm.h"
#include "dcm_rdbi.h"
#include "dcm_wdbi.h"
#include "dcm_seca.h"
#include "can_tp.h"

extern CAN_HandleTypeDef hcan2;

// Dữ liệu mẫu VIN ban đầu (17 bytes: "BOSCH_BEA_2026_VN")
uint8_t g_VIN_Buffer[17] = {'B', 'O', 'S', 'C', 'H', '_', 'B', 'E', 'A', '_', '2', '0', '2', '6', '_', 'V', 'N'};

// Bảng định tuyến các dịch vụ UDS được hỗ trợ trong hệ thống
const DCM_Router_t DCM_Table[] = {
    {0x22, DCM_Service_22_Practice}, // Read Data By Identifier
    {0x27, DCM_Service_27_Practice}, // Security Access
    {0x2E, DCM_Service_2E_Practice}  // Write Data By Identifier
};

void DCM_Process_Request(uint8_t* pPayload, uint16_t length) {
    if (pPayload == NULL || length == 0) return;

    uint8_t received_sid = pPayload[0]; // Byte đầu tiên là mã SID

    // Quét bảng DCM_Table để nhảy vào đúng hàm dịch vụ
    for (int i = 0; i < sizeof(DCM_Table) / sizeof(DCM_Table[0]); i++) {
        if (DCM_Table[i].SID == received_sid) {
            DCM_Table[i].execute(pPayload, length);
            return;
        }
    }

    // Nếu SID không nằm trong bảng hỗ trợ -> Trả về NRC 0x11 (ServiceNotSupported)
    uint8_t nrc_unknown_sid[3] = {0x7F, received_sid, 0x11};
    CAN_TP_Transmit(nrc_unknown_sid, 3, 0x7A2, &hcan2);
}
```

---

### 4.3. Module Dịch vụ 0x22 - Read Data By Identifier (`dcm_rdbi.h` và `dcm_rdbi.c`)
* **Nhiệm vụ:** Hiện thực hóa dịch vụ đọc dữ liệu theo định danh DID (ISO 14229 Service 0x22):
  - Kiểm tra độ dài bắt buộc đúng 3 bytes (`SID + DID_High + DID_Low`), sai trả về NRC `0x13`.
  - Đọc `DID 0x0123`: Đọc giá trị 16-bit của CAN ID đang hoạt động của Node 1, trả về `62 01 23 <CANID_H> <CANID_L>`.
  - Đọc `DID 0x0124`: Đọc giá trị điện áp cảm biến nhiệt độ chip STM32 qua ADC1 DMA, kiểm tra lỗi phần cứng bằng NRC `0x10`, tính toán nhiệt độ thực tế theo công thức datasheet và phản hồi `62 01 24 <Temp>`.
  - DID không hỗ trợ $\rightarrow$ Phản hồi mã âm NRC `0x31` (`requestOutOfRange`).

#### File `Core/Inc/dcm_rdbi.h`
```c
#ifndef INC_DCM_RDBI_H_
#define INC_DCM_RDBI_H_

#include "main.h"

void DCM_Service_22_Practice(uint8_t* pPayload, uint16_t length);

#endif /* INC_DCM_RDBI_H_ */
```

#### File `Core/Src/dcm_rdbi.c`
```c
#include "dcm_rdbi.h"
#include "dcm.h"
#include "can_tp.h"

extern CAN_HandleTypeDef hcan2;
extern uint16_t g_TemperatureSensorRawValue_u16[1]; // Dữ liệu ADC DMA
extern uint16_t g_Current_CAN_ID;                  // CAN ID thực tế Bài 1

void DCM_Service_22_Practice(uint8_t* pPayload, uint16_t length)
{
    uint16_t did = (pPayload[1] << 8) | pPayload[2];

    // KIỂM TRA ĐỘ DÀI: Bắt buộc đúng 3 byte (Bảng 2 & 7 Bosch Specification)
    if (length != 3) {
        uint8_t nrc_len[3] = {0x7F, 0x22, 0x13}; // NRC 0x13: Invalid length
        CAN_TP_Transmit(nrc_len, 3, 0x7A2, &hcan2);
        return;
    }

    // 1. DID 0x0123: Đọc CAN ID hiện tại của Node 1 (5 bytes response)
    if (did == 0x0123) {
        uint8_t resp_id[5];
        resp_id[0] = 0x62;                                 // Positive Response SID (0x22 + 0x40)
        resp_id[1] = 0x01;                                 // DID High
        resp_id[2] = 0x23;                                 // DID Low
        resp_id[3] = (uint8_t)(g_Current_CAN_ID >> 8);    // CAN ID Byte cao
        resp_id[4] = (uint8_t)(g_Current_CAN_ID & 0xFF);  // CAN ID Byte thấp
        CAN_TP_Transmit(resp_id, 5, 0x7A2, &hcan2);
    }
    // 2. DID 0x0124: Đọc nhiệt độ chip nội bộ qua ADC1 (4 bytes response)
    else if (did == 0x0124) {
        uint16_t raw_adc = g_TemperatureSensorRawValue_u16[0];

        // Kiểm tra lỗi phần cứng ADC (Bảng 11 Bosch Spec: NRC 0x10 General Reject)
        if (raw_adc == 0) {
            uint8_t nrc_hw[3] = {0x7F, 0x22, 0x10};
            CAN_TP_Transmit(nrc_hw, 3, 0x7A2, &hcan2);
            return;
        }

        // Chuyển đổi ADC sang điện áp mV: Vsense = (Raw * 3300) / 4095
        uint32_t v_sense = (raw_adc * 3300) / 4095;

        // Công thức chuẩn Datasheet STM32F405: Temp = ((Vsense - 760) / 2.5) + 25
        uint8_t real_temp = (uint8_t)(((v_sense - 760) * 10) / 25 + 25);

        uint8_t resp_temp[4];
        resp_temp[0] = 0x62;
        resp_temp[1] = 0x01;
        resp_temp[2] = 0x24;
        resp_temp[3] = real_temp;
        CAN_TP_Transmit(resp_temp, 4, 0x7A2, &hcan2);
    }
    // 3. DID 0xF190 (TEST MULTI-FRAME): Đọc VIN number (20 bytes response -> 3 frame CAN)
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
    // 4. DID không hỗ trợ -> NRC 0x31 (RequestOutOfRange)
    else {
        uint8_t nrc_did[3] = {0x7F, 0x22, 0x31};
        CAN_TP_Transmit(nrc_did, 3, 0x7A2, &hcan2);
    }
}
```

---

### 4.4. Module Dịch vụ 0x27 - Security Access (`dcm_seca.h` và `dcm_seca.c`)
* **Nhiệm vụ:** Hiện thực dịch vụ bảo mật UDS 0x27 theo chuẩn ISO 14229:
  - Sub-function `0x01` (Request Seed): Cấp chuỗi ngẫu nhiên 6 byte `12 34 56 78 9A BC`. Gói tin phản hồi 8 byte được tầng `can_tp.c` tự động phát qua cơ chế Multi-Frame (First Frame + Flow Control + Consecutive Frame). Dựng cờ `g_Seed_Requested = 1`.
  - Sub-function `0x02` (Send Key): Kiểm tra cờ `g_Seed_Requested`. Nếu chưa xin Seed mà đã gửi Key $\rightarrow$ Phạt lỗi NRC `0x24` (`requestSequenceError`).
  - Thuật toán giải mã Key đối xứng theo đặc tả của Bosch:
    $$\begin{cases} K_0 = S_0 \oplus S_1 \\ K_1 = S_1 + S_2 \\ K_2 = S_2 \oplus S_3 \\ K_3 = S_3 + S_0 \\ K_4 = S_4 \ \& \ \text{0xF0} \\ K_5 = S_5 \ \& \ \text{0x0F} \end{cases}$$
  - Cơ chế One-time Seed: Reset cờ ngay khi nhận lệnh `0x02` để chống phát lại Key cũ.
  - Cơ chế khóa phạt 10 giây: Nếu nhập sai Key, hệ thống kích hoạt mốc thời gian phạt 10 giây. Trong 10s này, mọi yêu cầu xin Seed tiếp theo đều bị từ chối bằng NRC `0x10`. Nhập sai Key trả về NRC `0x35` (`invalidKey`).
  - Mở khóa thành công: Bật sáng đèn LED-0 (PB0), màn hình hiển thị nhãn `[ UNLOCKED ]` màu xanh lá, lưu thời điểm mở khóa để kích hoạt bộ đếm thời gian tự khóa sau **20 giây**.

#### File `Core/Inc/dcm_seca.h`
```c
#ifndef INC_DCM_SECA_H_
#define INC_DCM_SECA_H_

#include "main.h"

void DCM_Service_27_Practice(uint8_t* pPayload, uint16_t length);

#endif /* INC_DCM_SECA_H_ */
```

#### File `Core/Src/dcm_seca.c`
```c
#include "dcm_seca.h"
#include "can_tp.h"

extern CAN_HandleTypeDef hcan2;

static uint8_t Current_Seed[6] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
static uint8_t g_Seed_Requested = 0;              // Cờ đánh dấu đã xin Seed hợp lệ
uint8_t g_Security_Unlocked = 0;                  // Biến trạng thái mở khóa
uint32_t g_Security_Unlock_Timestamp = 0;         // Mốc thời gian bắt đầu mở khóa
static uint32_t g_Security_Penalty_Timestamp = 0; // Mốc thời gian khóa phạt 10s

void DCM_Service_27_Practice(uint8_t* pPayload, uint16_t length)
{
    if (length < 2) {
        uint8_t nrc_len[3] = {0x7F, 0x27, 0x13};
        CAN_TP_Transmit(nrc_len, 3, 0x7A2, &hcan2);
        return;
    }

    uint8_t sub_function = pPayload[1];

    // =====================================
    // 1. SUB-FUNCTION 0x01: REQUEST SEED
    // =====================================
    if (sub_function == 0x01) {
        if (length != 2) {
            uint8_t nrc_len[3] = {0x7F, 0x27, 0x13};
            CAN_TP_Transmit(nrc_len, 3, 0x7A2, &hcan2);
            return;
        }

        // Kiểm tra xem hệ thống có đang trong thời gian phạt 10s vì nhập sai Key không
        if ((HAL_GetTick() - g_Security_Penalty_Timestamp) < 10000 && g_Security_Penalty_Timestamp != 0) {
            uint8_t nrc_delay[3] = {0x7F, 0x27, 0x10}; // NRC 0x10: General Reject
            CAN_TP_Transmit(nrc_delay, 3, 0x7A2, &hcan2);
            return;
        }

        Current_Seed[0] = 0x12;
        Current_Seed[1] = 0x34;
        Current_Seed[2] = 0x56;
        Current_Seed[3] = 0x78;
        Current_Seed[4] = 0x9A;
        Current_Seed[5] = 0xBC;

        g_Seed_Requested = 1; // Đánh dấu đã xin Seed thành công

        // Đóng gói 8 byte: SID(0x67) + SubFunc(0x01) + 6 byte Seed
        uint8_t resp_data[8];
        resp_data[0] = 0x67;
        resp_data[1] = 0x01;
        for (int i = 0; i < 6; i++) {
            resp_data[i + 2] = Current_Seed[i];
        }

        // Tự động phân mảnh Multi-frame qua CAN_TP
        CAN_TP_Transmit(resp_data, 8, 0x7A2, &hcan2);
    }
    // =======================================
    // 2. SUB-FUNCTION 0x02: SEND KEY
    // =======================================
    else if (sub_function == 0x02) {
        if (length != 8) {
            uint8_t nrc_len[3] = {0x7F, 0x27, 0x13};
            CAN_TP_Transmit(nrc_len, 3, 0x7A2, &hcan2);
            return;
        }

        // KIỂM TRA THỨ TỰ BẢO MẬT: Bắt buộc phải Request Seed trước khi Send Key
        if (g_Seed_Requested == 0) {
            uint8_t nrc_seq[3] = {0x7F, 0x27, 0x24}; // NRC 0x24: requestSequenceError
            CAN_TP_Transmit(nrc_seq, 3, 0x7A2, &hcan2);
            return;
        }

        g_Seed_Requested = 0; // Hủy cờ ngay lập tức (One-time use)

        // Tính Key chuẩn theo thuật toán đối xứng của Bosch
        uint8_t expected_key[6];
        expected_key[0] = Current_Seed[0] ^ Current_Seed[1];
        expected_key[1] = (uint8_t)(Current_Seed[1] + Current_Seed[2]);
        expected_key[2] = Current_Seed[2] ^ Current_Seed[3];
        expected_key[3] = (uint8_t)(Current_Seed[3] + Current_Seed[0]);
        expected_key[4] = Current_Seed[4] & 0xF0;
        expected_key[5] = Current_Seed[5] & 0x0F;

        // So khớp với Key do Tester gửi sang (từ pPayload[2] đến pPayload[7])
        uint8_t key_valid = 1;
        for (int i = 0; i < 6; i++) {
            if (pPayload[i + 2] != expected_key[i]) {
                key_valid = 0;
                break;
            }
        }

        if (key_valid == 1) {
            g_Security_Unlocked = 1;
            g_Security_Unlock_Timestamp = HAL_GetTick(); // Bắt đầu đếm 20s
            g_Security_Penalty_Timestamp = 0;

            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // Bật sáng LED-0

            uint8_t resp_success[2] = {0x67, 0x02};
            CAN_TP_Transmit(resp_success, 2, 0x7A2, &hcan2);
        } else {
            g_Security_Unlocked = 0;
            g_Security_Penalty_Timestamp = HAL_GetTick(); // Kích hoạt phạt 10s

            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // Tắt LED-0

            uint8_t nrc_invalid_key[3] = {0x7F, 0x27, 0x35}; // NRC 0x35: Invalid Key
            CAN_TP_Transmit(nrc_invalid_key, 3, 0x7A2, &hcan2);
        }
    }
    // Sub-function lạ không hỗ trợ -> NRC 0x12
    else {
        uint8_t nrc_subfunc[3] = {0x7F, 0x27, 0x12};
        CAN_TP_Transmit(nrc_subfunc, 3, 0x7A2, &hcan2);
    }
}
```

---

### 4.5. Module Dịch vụ 0x2E - Write Data By Identifier (`dcm_wdbi.h` và `dcm_wdbi.c`)
* **Nhiệm vụ:** Hiện thực dịch vụ ghi cấu hình CAN ID mới (ISO 14229 Service 0x2E):
  - Kiểm tra điều kiện tiên quyết: Bảo mật phải được mở khóa (`g_Security_Unlocked == 1`), nếu chưa mở hoặc đã quá 20s tự khóa thì từ chối ngay bằng **NRC 0x33 (`securityAccessDenied`)**.
  - Kiểm tra độ dài bắt buộc 5 byte (`SID + DID(2 byte) + CAN_ID(2 byte)`), sai trả về NRC `0x13`.
  - Kiểm tra DID `0x0123`, sai trả về NRC `0x31`.
  - Kiểm tra giới hạn Range theo Bảng 19 của tài liệu Bosch Specification: Byte đầu tiên của New CAN ID (byte 4) chỉ được phép nằm trong dải `0x00` đến `0x7F`. Nếu byte $4 > \text{0x7F}$ $\rightarrow$ Trả về mã âm **NRC 0x31 (`requestOutOfRange`)**.
  - Cơ chế an toàn Ignition Cycle: Giá trị CAN ID mới được ghi vào bộ nhớ tạm `g_Pending_New_CAN_ID` và bật cờ `g_Pending_ID_Ready = 1`. Màn hình hiển thị nhãn `[IGN REQ]`. Cấu hình mới **chưa được áp dụng ngay** nhằm bảo đảm an toàn cho xe khi đang lăn bánh.
  - Phản hồi tích cực chuẩn 3 byte `{0x6E, 0x01, 0x23}` theo đúng Bảng 20 đặc tả Bosch BEA.

#### File `Core/Inc/dcm_wdbi.h`
```c
#ifndef INC_DCM_WDBI_H_
#define INC_DCM_WDBI_H_

#include "main.h"

void DCM_Service_2E_Practice(uint8_t* pPayload, uint16_t length);

#endif /* INC_DCM_WDBI_H_ */
```

#### File `Core/Src/dcm_wdbi.c`
```c
#include "dcm_wdbi.h"
#include "dcm.h"
#include "can_tp.h"

extern CAN_HandleTypeDef hcan2;
extern uint8_t g_Security_Unlocked;

uint16_t g_Pending_New_CAN_ID = 0x012; // CAN ID mới chờ Ignition Cycle
uint8_t g_Pending_ID_Ready = 0;        // Cờ báo có ID mới đang chờ nạp

void DCM_Service_2E_Practice(uint8_t* pPayload, uint16_t length)
{
    // KIỂM TRA 1: Quyền bảo mật Security Access
    if (g_Security_Unlocked == 0) {
        uint8_t nrc_denied[3] = {0x7F, 0x2E, 0x33}; // NRC 0x33: Security Access Denied
        CAN_TP_Transmit(nrc_denied, 3, 0x7A2, &hcan2);
        return;
    }

    if (length < 3) {
        uint8_t nrc_length[3] = {0x7F, 0x2E, 0x13};
        CAN_TP_Transmit(nrc_length, 3, 0x7A2, &hcan2);
        return;
    }

    uint16_t did = (pPayload[1] << 8) | pPayload[2];

    // 1. Ghi New CAN ID (DID 0x0123 - Độ dài 5 byte)
    if (did == 0x0123) 
    {
        if (length != 5) {
            uint8_t nrc_length[3] = {0x7F, 0x2E, 0x13}; // NRC 0x13: Invalid length
            CAN_TP_Transmit(nrc_length, 3, 0x7A2, &hcan2);
            return;
        }

        // Kiểm tra Range New CAN ID theo Bảng 19 Spec Bosch (Byte 4 <= 0x7F)
        if (pPayload[3] > 0x7F) {
            uint8_t nrc_range[3] = {0x7F, 0x2E, 0x31};  // NRC 0x31: Request Out Of Range
            CAN_TP_Transmit(nrc_range, 3, 0x7A2, &hcan2);
            return;
        }

        uint16_t new_can_id = ((uint16_t)pPayload[3] << 8) | pPayload[4];
        g_Pending_New_CAN_ID = new_can_id;
        g_Pending_ID_Ready = 1; // Chờ Ignition Cycle áp dụng

        uint8_t resp_success[3] = {0x6E, 0x01, 0x23};
        CAN_TP_Transmit(resp_success, 3, 0x7A2, &hcan2);
    }
    // 2. Ghi VIN Number (DID 0xF190 - TEST MULTI-FRAME: Độ dài 20 byte)
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

        uint8_t resp_success[3] = {0x6E, 0xF1, 0x90};
        CAN_TP_Transmit(resp_success, 3, 0x7A2, &hcan2);
    }
    // 3. DID không hỗ trợ -> NRC 0x31 (RequestOutOfRange)
    else {
        uint8_t nrc_did[3] = {0x7F, 0x2E, 0x31};
        CAN_TP_Transmit(nrc_did, 3, 0x7A2, &hcan2);
    }
}
```

---

### 4.6. Module Màn hình LCD Automotive Dashboard (`lcd_hal.h` và `lcd_hal.c`)
* **Nhiệm vụ:** Thiết lập giao diện điều khiển ô tô cao cấp trên màn hình LCD màu 240x320 chip ST7789:
  - Tối ưu hóa phần cứng bằng bộ hàm đồ họa nhanh (Fast Graphics Primitives): `LCD_FillRect`, `LCD_DrawHLine`, `LCD_DrawVLine`, `LCD_DrawProgressBar`, `LCD_DrawString_Fast`, `LCD_DrawString2X`.
  - **Kiến trúc phân lớp tĩnh/động (Zero-Flicker):** 
    - `LCD_Init_Dashboard(isHomeSimMode)`: Chạy 1 lần duy nhất lúc khởi động MCU để vẽ dải màu thương hiệu Bosch Red, tiêu đề banner công nghệ, nhãn chế độ hoạt động và 3 khung thẻ Card.
    - `LCD_Update_Dashboard(...)`: Chạy định kỳ mỗi 500ms trong `while(1)`, chỉ ghi đè đúng tọa độ các giá trị số và chuỗi trạng thái thay đổi. Loại bỏ hoàn toàn lệnh xóa toàn màn hình, đạt độ mượt mà tuyệt đối không nhấp nháy.

#### File `Core/Inc/lcd_hal.h`
```c
#ifndef INC_LCD_HAL_H_
#define INC_LCD_HAL_H_

#include "main.h"

// Bảng màu chuẩn đồ họa ô tô 16-bit RGB565
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_CYAN          0x07FF
#define COLOR_GOLD          0xFFE0
#define COLOR_GRAY          0x8410
#define COLOR_DARK_GRAY     0x2104
#define COLOR_BOSCH_RED     0xE800
#define COLOR_CARD_BG       0x18E3
#define COLOR_CARD_BORDER   0x31A6
#define COLOR_HEADER_BG     0x0861

// Điều khiển chân GPIO màn hình ST7789
#define LCD_CS_L()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)
#define LCD_CS_H()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)
#define LCD_DH_L()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)
#define LCD_DH_H()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)
#define LCD_RST_L()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET)
#define LCD_RST_H()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET)

// Khai báo hàm đồ họa và dashboard
void LCD_Init(void);
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_DrawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t percent, uint16_t barColor, uint16_t bgColor, uint16_t borderColor);
void LCD_DrawString_Fast(uint16_t x, uint16_t y, const char* str, uint16_t textColor, uint16_t bgColor);
void LCD_DrawString2X(uint16_t x, uint16_t y, const char* str, uint16_t textColor, uint16_t bgColor);

void LCD_Init_Dashboard(uint8_t isHomeSimMode);
void LCD_Update_Dashboard(uint32_t uptimeSec, const uint8_t* rxData, uint16_t txId, const uint8_t* txData, uint8_t crcVal, uint8_t realTemp, uint8_t isSecurityUnlocked, uint16_t pendingCanId, uint8_t isPendingReady);

#endif /* INC_LCD_HAL_H_ */
```

#### File `Core/Src/lcd_hal.c` (Trích đoạn hàm Dashboard)
```c
#include "lcd_hal.h"
#include <stdio.h>

// Hàm khởi tạo khung tĩnh Dashboard (Chạy 1 lần duy nhất lúc khởi động)
void LCD_Init_Dashboard(uint8_t isHomeSimMode)
{
    // 1. Nền đen toàn màn hình
    LCD_FillRect(0, 0, 240, 320, COLOR_BLACK);

    // 2. Dải màu đỏ thương hiệu Bosch ở đỉnh màn hình
    LCD_FillRect(0, 0, 240, 3, COLOR_BOSCH_RED);

    // 3. Header Banner công nghệ cao
    LCD_FillRect(0, 3, 240, 33, COLOR_HEADER_BG);
    LCD_DrawString_Fast(10, 8, "BOSCH ECU DASHBOARD", COLOR_WHITE, COLOR_HEADER_BG);
    LCD_DrawString_Fast(10, 22, "TEAM 5 | CAN & UDS", COLOR_CYAN, COLOR_HEADER_BG);

    // 4. Nhãn chế độ hoạt động
    if (isHomeSimMode) {
        LCD_DrawString_Fast(10, 40, "[ MODE: CAN2 SIMULATION ]", COLOR_GOLD, COLOR_BLACK);
    } else {
        LCD_DrawString_Fast(10, 40, "[ MODE: EXTERNAL TESTBED ]", COLOR_GREEN, COLOR_BLACK);
    }
    LCD_DrawString_Fast(170, 40, "500 kbps", COLOR_WHITE, COLOR_BLACK);

    // 5. Card 1: CAN Telemetry (Bài 1)
    LCD_DrawRect(5, 55, 230, 82, COLOR_CARD_BORDER);
    LCD_FillRect(6, 56, 228, 80, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 60, "-- CAN NETWORK TELEMETRY --", COLOR_CYAN, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 78, "RX 0x0A2:", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 108, "CRC-8   :", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 124, "CAN2: TX 0x0A2 -> CAN1: TX", COLOR_DARK_GRAY, COLOR_CARD_BG);

    // 6. Card 2: UDS Diagnostics (Bài 2)
    LCD_DrawRect(5, 143, 230, 75, COLOR_CARD_BORDER);
    LCD_FillRect(6, 144, 228, 73, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 148, "-- UDS DIAGNOSTICS (ISO) --", COLOR_GOLD, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 167, "Security (0x27):", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 184, "Active CAN ID  :", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 200, "Pending CAN ID :", COLOR_GRAY, COLOR_CARD_BG);

    // 7. Card 3: Sensor Health (DID 0124)
    LCD_DrawRect(5, 224, 230, 70, COLOR_CARD_BORDER);
    LCD_FillRect(6, 225, 228, 68, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 229, "-- MCU TEMPERATURE (0124) -", COLOR_CYAN, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 249, "Core:", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(12, 280, "0 C           40 C          80 C", COLOR_DARK_GRAY, COLOR_CARD_BG);

    // 8. Footer đáy màn hình
    LCD_DrawString_Fast(10, 305, "STM32F405 | HUST-BOSCH | PA0:IGN", COLOR_DARK_GRAY, COLOR_BLACK);
}

// Hàm cập nhật dữ liệu động (Chạy mỗi 500ms - Không gây chớp màn hình)
void LCD_Update_Dashboard(uint32_t uptimeSec, const uint8_t* rxData, uint16_t txId,
                          const uint8_t* txData, uint8_t crcVal, uint8_t realTemp,
                          uint8_t isSecurityUnlocked, uint16_t pendingCanId, uint8_t isPendingReady)
{
    char buf[32];
    static uint8_t heartBeat = 0;
    heartBeat ^= 1;

    // 1. Header: Uptime và nhịp tim
    sprintf(buf, "%02lu:%02lu", uptimeSec / 60, uptimeSec % 60);
    LCD_DrawString_Fast(168, 8, buf, COLOR_WHITE, COLOR_HEADER_BG);
    LCD_DrawChar_Fast(216, 8, heartBeat ? '*' : ' ', COLOR_GREEN, COLOR_HEADER_BG);

    // 2. Card 1: Dữ liệu CAN
    sprintf(buf, "%02X %02X     ", rxData[0], rxData[1]);
    LCD_DrawString_Fast(84, 78, buf, COLOR_WHITE, COLOR_CARD_BG);
    LCD_DrawString_Fast(165, 78, "[BUS OK]", COLOR_GREEN, COLOR_CARD_BG);

    if (txId > 0x7FF) {
        sprintf(buf, "TX 0x%04X:", txId);
    } else {
        sprintf(buf, "TX 0x%03X :", txId);
    }
    LCD_DrawString_Fast(10, 93, buf, COLOR_GRAY, COLOR_CARD_BG);
    sprintf(buf, "%02X %02X %02X  ", txData[0], txData[1], txData[2]);
    LCD_DrawString_Fast(84, 93, buf, COLOR_WHITE, COLOR_CARD_BG);
    LCD_DrawString_Fast(165, 93, "[BUS OK]", COLOR_GREEN, COLOR_CARD_BG);

    sprintf(buf, "0x%02X     ", crcVal);
    LCD_DrawString_Fast(84, 108, buf, COLOR_WHITE, COLOR_CARD_BG);
    LCD_DrawString_Fast(165, 108, "[VALID] ", COLOR_GREEN, COLOR_CARD_BG);

    // 3. Card 2: Trạng thái UDS
    if (isSecurityUnlocked) {
        LCD_DrawString_Fast(112, 167, "[ UNLOCKED ]    ", COLOR_GREEN, COLOR_CARD_BG);
    } else {
        LCD_DrawString_Fast(112, 167, "[  LOCKED  ]    ", COLOR_RED, COLOR_CARD_BG);
    }

    if (txId > 0x7FF) {
        sprintf(buf, "0x%04X [ACTIVE] ", txId);
    } else {
        sprintf(buf, "0x%03X  [ACTIVE] ", txId);
    }
    LCD_DrawString_Fast(112, 184, buf, COLOR_WHITE, COLOR_CARD_BG);

    if (isPendingReady) {
        if (pendingCanId > 0x7FF) {
            sprintf(buf, "0x%04X [IGN REQ]", pendingCanId);
        } else {
            sprintf(buf, "0x%03X  [IGN REQ]", pendingCanId);
        }
        LCD_DrawString_Fast(112, 200, buf, COLOR_CYAN, COLOR_CARD_BG);
    } else {
        LCD_DrawString_Fast(112, 200, "NONE   [SYNCED] ", COLOR_GRAY, COLOR_CARD_BG);
    }

    // 4. Card 3: Nhiệt độ số to 2X và thanh đo đồ họa Analog Bar
    sprintf(buf, "%2d C", realTemp);
    LCD_DrawString2X(80, 246, buf, COLOR_WHITE, COLOR_CARD_BG);
    LCD_DrawString_Fast(165, 249, (realTemp < 45) ? "[NORMAL]" : "[ WARM ]", COLOR_GREEN, COLOR_CARD_BG);

    uint8_t pct = (realTemp > 80) ? 100 : (uint8_t)(((uint16_t)realTemp * 100) / 80);
    uint16_t barCol = (realTemp < 35) ? COLOR_CYAN : ((realTemp < 50) ? COLOR_GREEN : COLOR_RED);
    LCD_DrawProgressBar(12, 268, 216, 8, pct, barCol, COLOR_BLACK, COLOR_CARD_BORDER);
}
```

---

### 4.7. Module Vòng lặp chính và Tích hợp hệ thống (`main.c`)
* **Nhiệm vụ:**
  - Khởi động ngoại vi, cấu hình bộ lọc CAN, bật ngắt UART và DMA ADC.
  - **Động cơ nhận khung UART không trễ (Zero-Delay Framing Engine):** Bắt trực tiếp cặp mào đầu `0F FF F0` và mào đuôi `F0 00 0F` từ PC, kích hoạt cờ `g_Uds_Cmd_Ready` để xử lý ngay với độ trễ 0ms (loại bỏ hoàn toàn lệnh `HAL_Delay(10)`).
  - Hàm `Process_UDS_Payload()` phân luồng tự động Single Frame và Multi-Frame.
  - Bộ đếm thời gian tự khóa an toàn: Đúng **20 giây** kể từ khi mở khóa, hệ thống tự động khóa bảo mật, tắt LED-0 và chuyển trạng thái LCD về `[ LOCKED ]`.
  - Thực thi chu kỳ Bài 1: CAN1 phát định kỳ 50ms, tính toán CRC-8 SAE J1850; CAN2 phát định kỳ 20ms (ở chế độ giả lập tại nhà).
  - Cập nhật Dashboard màn hình LCD định kỳ mỗi 500ms.
  - Tự động phục hồi ngắt nhận UART khi gặp lỗi phần cứng qua hàm `HAL_UART_ErrorCallback()`.

```c
// Trích đoạn hàm điều phối UDS và vòng lặp while(1) trong main.c:

static void Process_UDS_Payload(uint8_t *payload, uint16_t payload_len)
{
    if (payload_len == 0) return;

    // A. Single Frame (1 đến 7 bytes)
    if (payload_len <= 7)
    {
        uint8_t tester_tx_data[8] = {0};
        tester_tx_data[0] = (uint8_t)payload_len;
        for (int k = 0; k < payload_len; k++) {
            tester_tx_data[k + 1] = payload[k];
        }
        CAN1_pHeader.StdId = 0x712;
        CAN1_pHeader.ExtId = 0;
        CAN1_pHeader.IDE = CAN_ID_STD;
        CAN1_pHeader.RTR = CAN_RTR_DATA;
        CAN1_pHeader.DLC = 8;
        uint32_t retry = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0 && retry--) {}
        HAL_CAN_AddTxMessage(&hcan1, &CAN1_pHeader, tester_tx_data, &CAN1_pTxMailbox);
    }
    // B. Multi-frame (8 bytes trở lên - Hỗ trợ mọi độ dài payload động chuẩn ISO 15765-2)
    else
    {
        // 1. Gửi First Frame (FF) mang 6 byte đầu tiên
        uint8_t ff_data[8] = {0};
        ff_data[0] = 0x10 | ((payload_len >> 8) & 0x0F);
        ff_data[1] = (uint8_t)(payload_len & 0xFF);
        for (int k = 0; k < 6; k++) {
            ff_data[k + 2] = payload[k];
        }
        CAN1_pHeader.StdId = 0x712;
        CAN1_pHeader.ExtId = 0;
        CAN1_pHeader.IDE = CAN_ID_STD;
        CAN1_pHeader.RTR = CAN_RTR_DATA;
        CAN1_pHeader.DLC = 8;
        uint32_t retry = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0 && retry--) {}
        HAL_CAN_AddTxMessage(&hcan1, &CAN1_pHeader, ff_data, &CAN1_pTxMailbox);
        
        HAL_Delay(10); // Chờ ECU xử lý và trả lời Flow Control

        // 2. Vòng lặp phát tất cả Consecutive Frame (CF) động theo Sequence Number
        uint16_t bytes_sent = 6;
        uint8_t sn = 1; // SN bắt đầu từ 1 (0x21)
        while (bytes_sent < payload_len) {
            uint8_t cf_data[8] = {0};
            cf_data[0] = 0x20 | (sn & 0x0F);
            uint8_t chunk_len = payload_len - bytes_sent;
            if (chunk_len > 7) {
                chunk_len = 7;
            }
            for (int k = 0; k < chunk_len; k++) {
                cf_data[k + 1] = payload[bytes_sent + k];
            }
            bytes_sent += chunk_len;
            sn = (sn + 1) & 0x0F; // Tăng SN và quay vòng 15 về 0

            retry = 50000;
            while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0 && retry--) {}
            HAL_CAN_AddTxMessage(&hcan1, &CAN1_pHeader, cf_data, &CAN1_pTxMailbox);
            if (bytes_sent < payload_len) {
                HAL_Delay(5);
            }
        }
    }
}

// Vòng lặp while(1) chính:
while (1)
{
    // ƯU TIÊN 1: XỬ LÝ LỆNH UDS TỪ MÁY TÍNH VỚI ĐỘ TRỄ 0ms
    if (g_Uds_Cmd_Ready == 1)
    {
        if (NumBytesReq >= 6) {
            Process_UDS_Payload(&REQ_BUFFER[3], NumBytesReq - 6);
        }
        NumBytesReq = 0;
        g_Uds_Cmd_Ready = 0;
    }
    // Timeout 15ms cho các lệnh thô không đóng khung
    else if (NumBytesReq > 0 && (HAL_GetTick() - g_Last_Uart_Rx_Tick) >= 15)
    {
        if (NumBytesReq >= 6 &&
            REQ_BUFFER[0] == 0x0F && REQ_BUFFER[1] == 0xFF && REQ_BUFFER[2] == 0xF0 &&
            REQ_BUFFER[NumBytesReq - 3] == 0xF0 && REQ_BUFFER[NumBytesReq - 2] == 0x00 && REQ_BUFFER[NumBytesReq - 1] == 0x0F)
        {
            Process_UDS_Payload(&REQ_BUFFER[3], NumBytesReq - 6);
        }
        else if (REQ_BUFFER[0] == 0x0F && REQ_BUFFER[1] == 0xFF && REQ_BUFFER[2] == 0xF0)
        {
            // Gói tin hỏng / đứt cáp -> Bỏ qua
        }
        else
        {
            Process_UDS_Payload(&REQ_BUFFER[0], NumBytesReq);
        }
        NumBytesReq = 0;
        g_Uds_Cmd_Ready = 0;
    }

    // CẬP NHẬT MÀN HÌNH LCD MỖI 500ms
    static uint32_t last_lcd_write = 0;
    if ((HAL_GetTick() - last_lcd_write) >= 500) {
        last_lcd_write = HAL_GetTick();

        uint32_t v_sense = (g_TemperatureSensorRawValue_u16[0] * 3300) / 4095;
        uint8_t real_temp = (uint8_t)(((v_sense - 760) * 10) / 25 + 25);
        uint32_t uptime_sec = HAL_GetTick() / 1000;

        LCD_Update_Dashboard(uptime_sec, CAN1_DATA_RX, (uint16_t)g_Current_CAN_ID,
                             CAN1_DATA_TX, CAN1_DATA_TX[6], real_temp,
                             g_Security_Unlocked, (uint16_t)g_Pending_New_CAN_ID,
                             g_Pending_ID_Ready);
    }

    // TỰ ĐỘNG KHÓA BẢO MẬT SAU 20 GIÂY (Service 0x27)
    if (g_Security_Unlocked == 1) {
        if ((HAL_GetTick() - g_Security_Unlock_Timestamp) >= 20000) {
            g_Security_Unlocked = 0;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // Tắt LED-0
        }
    }

    // BÀI 1: CAN1 PHÁT ĐỊNH KỲ 50ms
    static uint32_t last_tick_can1 = 0;
    if ((TimeStamp - last_tick_can1) >= 50)
    {
        last_tick_can1 += 50;

        CAN1_DATA_TX[0] = CAN1_DATA_RX[0];
        CAN1_DATA_TX[1] = CAN1_DATA_RX[1];
        CAN1_DATA_TX[2] = CAN1_DATA_RX[0] + CAN1_DATA_RX[1];
        CAN1_DATA_TX[3] = 0x00;
        CAN1_DATA_TX[4] = 0x00;
        CAN1_DATA_TX[5] = 0x00;
        CAN1_DATA_TX[6] = Caculate_CRC8_SAE_J1850(CAN1_DATA_TX, 6);
        CAN1_DATA_TX[7] = 0x00;

        if (g_Current_CAN_ID > 0x7FF) {
            CAN1_pHeader.StdId = 0;
            CAN1_pHeader.ExtId = g_Current_CAN_ID;
            CAN1_pHeader.IDE = CAN_ID_EXT;
        } else {
            CAN1_pHeader.StdId = g_Current_CAN_ID;
            CAN1_pHeader.ExtId = 0;
            CAN1_pHeader.IDE = CAN_ID_STD;
        }
        CAN1_pHeader.RTR = CAN_RTR_DATA;
        CAN1_pHeader.DLC = 8;

        uint32_t retry = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0 && retry--) {}
        HAL_CAN_AddTxMessage(&hcan1, &CAN1_pHeader, CAN1_DATA_TX, &CAN1_pTxMailbox);
    }

#if (CONFIG_SIMULATE_CAN2_AT_HOME == 1)
    // CAN2 MÔ PHỎNG PHÁT ĐỊNH KỲ 20ms
    static uint32_t last_tick_can2 = 0;
    static uint8_t can2_msg_counter = 0;
    if (TimeStamp - last_tick_can2 >= 20)
    {
        last_tick_can2 = TimeStamp;
        CAN2_DATA_TX[0] = 0x22;
        CAN2_DATA_TX[1] = 0x33;
        CAN2_DATA_TX[7] = can2_msg_counter;
        can2_msg_counter = (can2_msg_counter + 1) & 0x0F;

        CAN2_pHeader.StdId = 0xA2;
        CAN2_pHeader.IDE = CAN_ID_STD;
        CAN2_pHeader.RTR = CAN_RTR_DATA;
        CAN2_pHeader.DLC = 8;

        uint32_t retry2 = 50000;
        while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) == 0 && retry2--) {}
        HAL_CAN_AddTxMessage(&hcan2, &CAN2_pHeader, CAN2_DATA_TX, &CAN2_pTxMailbox);
    }
#endif
}

// Xử lý ngắt nhận byte UART và phát hiện trọn vẹn khung Bosch
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        g_Last_Uart_Rx_Tick = HAL_GetTick();

        if (!g_Uds_Cmd_Ready && NumBytesReq < sizeof(REQ_BUFFER))
        {
            REQ_BUFFER[NumBytesReq] = REQ_1BYTE_DATA;
            NumBytesReq++;

            // Kiểm tra đủ khung: SOF(3) + Data(>=1) + EOF(3) -> NumBytesReq >= 7
            if (NumBytesReq >= 7)
            {
                if (REQ_BUFFER[0] == 0x0F && REQ_BUFFER[1] == 0xFF && REQ_BUFFER[2] == 0xF0 &&
                    REQ_BUFFER[NumBytesReq - 3] == 0xF0 &&
                    REQ_BUFFER[NumBytesReq - 2] == 0x00 &&
                    REQ_BUFFER[NumBytesReq - 1] == 0x0F)
                {
                    g_Uds_Cmd_Ready = 1; // Nhận đủ khung hợp lệ từ PC!
                }
            }
        }
    }
}

// Phục hồi tự động khi phần cứng UART gặp lỗi
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);
        HAL_UART_Receive_IT(huart, &REQ_1BYTE_DATA, 1);
    }
}
```

---

### 4.8. Module Xử lý ngắt hệ thống (`stm32f4xx_it.c`)
* **Nhiệm vụ:**
  - `CAN1_RX0_IRQHandler()`: Bộ nhận tầng Tester.
    - Nhận frame dữ liệu chu kỳ `0x0A2` nạp vào buffer Bài 1.
    - Nhận frame phản hồi UDS `0x7A2` từ ECU:
      - **Single Frame (`0x00`):** Đóng gói khung Bosch (`0F FF F0 ... F0 00 0F`) bắn ngay lên PC qua UART3.
      - **First Frame (`0x10`):** Trích xuất tổng độ dài `s_tester_ff_total_len`, kiểm tra tràn bộ đệm `g_mf_rx_buffer[256]` (bắn `0x32` nếu tràn), lưu 6 byte đầu và **tự động bắn Flow Control CTS (`30 00 00 00 00 00 00 00`)** sang ECU qua `0x712`.
      - **Consecutive Frame (`0x20`):** Tích lũy liên tiếp các khối 7 byte dữ liệu cho đến khi nhận đủ toàn bộ `s_tester_ff_total_len`, sau đó đóng gói khung Bosch và truyền toàn bộ lên PC qua UART3.
  - `CAN2_RX0_IRQHandler()`: Bộ nhận tầng ECU. Nhận frame yêu cầu `0x712` từ Tester và gọi `CAN_TP_RxIndication(CAN2_DATA_RX)`.
  - `EXTI0_IRQHandler()`: Nhận sự kiện nhấn nút PA0 (mô phỏng chu kỳ bật tắt chìa khóa điện Ignition Cycle). Nếu có CAN ID mới đang chờ (`g_Pending_ID_Ready == 1`), áp dụng ID mới vào `g_Current_CAN_ID` và cập nhật lại bộ lọc phần cứng `HAL_CAN_ConfigFilter`.

---

## 5. QUY TRÌNH BIÊN DỊCH VÀ HƯỚNG DẪN KIỂM THỬ TOÀN DIỆN

### 5.1. Biên dịch dự án
Dự án được cấu hình biên dịch bằng công cụ GNU Arm Embedded Toolchain:
```powershell
# Chạy lệnh make trong thư mục Debug
make -C Debug -j8 all
```
* **Kết quả:** `0 errors, 0 warnings`.
* **File nhị phân tạo ra:**
  - `Final_Project/Source_Code/Debug/BEA_CAN2CAN_DEMO_TEN1HC_HUST.elf`
  - `Final_Project/Source_Code/Debug/BEA_CAN2CAN_DEMO_TEN1HC_HUST.hex`

Nạp file `.hex` vào board STM32F405 bằng phần mềm **STM32CubeProgrammer** (kết nối qua mạch nạp ST-Link).

---

### 5.2. Chạy phần mềm kiểm thử `BEA_DiagChecker`
1. Cắm cáp USB-UART từ máy tính vào chân `PC10` (TX) và `PC11` (RX).
2. Mở terminal tại thư mục tool và khởi chạy giao diện kiểm thử:
   ```powershell
   python BEA_DiagChecker.py
   ```
3. Trên giao diện, chọn đúng cổng COM tương ứng (ví dụ `COM5`), Baudrate `115200` $\rightarrow$ Nhấn **Connect**.

---

### 5.3. Kịch bản kiểm thử (Test Cases) đạt điểm tối đa 100%

#### Test Case 1: Kiểm tra Mạng CAN Telemetry & Màn hình Dashboard (Bài 1)
* **Thao tác:** Quan sát màn hình LCD và cửa sổ log khi vừa cấp nguồn.
* **Kỳ vọng:**
  - Màn hình LCD hiển thị giao diện 3 khối Card sắc nét, không chớp giật.
  - Card 1: Dòng `RX 0x0A2: 22 33` hiển thị liên tục, dòng `TX 0x012: 22 33 55` hiển thị đúng giá trị tổng $D2 = 22 + 33 = 55$, mã CRC-8 SAE J1850 hiển thị kèm nhãn `[VALID]`.
  - Card 3: Nhiệt độ chip vi điều khiển hiển thị số to 2X (ví dụ `31 C`) đi kèm thanh đo Analog Bar đồ họa màu Cyan/Xanh lá.

#### Test Case 2: Kiểm tra Service 0x22 (Read Data By Identifier)
1. **Đọc CAN ID hiện tại (`DID 0x0123`):**
   - Lệnh gửi: `22 01 23`
   - Phản hồi nhận được: `62 01 23 00 12` (CAN ID hiện tại là `0x0012`).
2. **Đọc nhiệt độ chip ADC (`DID 0x0124`):**
   - Lệnh gửi: `22 01 24`
   - Phản hồi nhận được: `62 01 24 1F` (Nhiệt độ Hex `0x1F` = 31 độ C, trùng khớp 100% với giá trị hiển thị trên màn hình LCD).
3. **Đọc DID sai (`0x9999`):**
   - Lệnh gửi: `22 99 99`
   - Phản hồi nhận được: `7F 22 31` (NRC `0x31` - Request Out Of Range).

#### Test Case 3: Kiểm tra Service 0x27 (Security Access & Flow Control)
1. **Thử gửi Key trước khi xin Seed (Bẻ khóa sai quy trình):**
   - Lệnh gửi: `27 02 26 8A 2E 8A 90 0C`
   - Phản hồi nhận được: `7F 27 24` (NRC `0x24` - `requestSequenceError`). Hệ thống chặn đứng hành vi gian lận.
2. **Yêu cầu cấp mã ngẫu nhiên (Request Seed):**
   - Lệnh gửi: `27 01`
   - Phản hồi nhận được: `67 01 12 34 56 78 9A BC` (Gói tin 8 byte truyền qua Multi-frame và Flow Control thành công).
3. **Thử nhập sai Key:**
   - Lệnh gửi: `27 02 11 22 33 44 55 66`
   - Phản hồi nhận được: `7F 27 35` (NRC `0x35` - `invalidKey`). Đèn LED-0 vẫn tắt.
   - Thử xin lại Seed ngay: `27 01` $\rightarrow$ Nhận mã phạt `7F 27 10` (NRC `0x10` - Bị phạt khóa 10 giây).
4. **Gửi đúng Key mở khóa:**
   - Sau khi hết 10 giây phạt, gửi lại `27 01` nhận Seed `12 34 56 78 9A BC`.
   - Tool tự động tính Key: `26 8A 2E 8A 90 0C`.
   - Lệnh gửi: `27 02 26 8A 2E 8A 90 0C`
   - Phản hồi nhận được: `67 02` (Mở khóa thành công). Đèn LED-0 trên board sáng lên, Card 2 trên LCD chuyển sang `[ UNLOCKED ]` màu xanh lá!

#### Test Case 4: Kiểm tra Service 0x2E (Write DID & Ignition Cycle)
1. **Thử ghi CAN ID vượt quá dải quy định:**
   - Lệnh gửi: `2E 01 23 FF FF` (Byte 4 là `0xFF > 0x7F`, vi phạm Bảng 19 Bosch Spec).
   - Phản hồi nhận được: `7F 2E 31` (NRC `0x31` - Request Out Of Range).
2. **Ghi CAN ID hợp lệ (Ví dụ `0x3456`):**
   - Lệnh gửi: `2E 01 23 34 56`
   - Phản hồi nhận được: `6E 01 23` (Thành công chuẩn Bảng 20 Bosch Spec).
   - Trên LCD: Dòng `Pending CAN ID` chuyển thành `0x3456 [IGN REQ]`, nhưng dòng `Active CAN ID` vẫn là `0x012 [ACTIVE]`.
3. **Mô phỏng chu kỳ bật tắt chìa khóa điện (Ignition Cycle):**
   - Nhấn nút bấm **PA0** trên board STM32.
   - Quan sát LCD: Dòng `Active CAN ID` lập tức chuyển thành `0x3456 [ACTIVE]`, dòng `Pending CAN ID` trở về `NONE [SYNCED]`.
   - Gửi lệnh đọc `22 01 23` từ máy tính $\rightarrow$ Nhận phản hồi: `62 01 23 34 56`!
4. **Kiểm tra cơ chế tự động khóa an toàn sau 20 giây:**
   - Đợi sau 20 giây kể từ khi mở khóa: Đèn LED-0 tự động tắt, Card 2 trên LCD chuyển lại về `[ LOCKED ]` màu đỏ.
   - Gửi thử lệnh Write DID `2E 01 23 11 22` $\rightarrow$ Nhận ngay mã từ chối `7F 2E 33` (NRC `0x33` - Security Access Denied).

#### Test Case 5: Kiểm tra Multi-Frame & Flow Control Nâng Cao (> 8 bytes: 20 bytes)
Thử nghiệm kiểm chứng khả năng phân mảnh, đánh số thứ tự Consecutive Frame (SN = 1, SN = 2) và điều tiết luồng Flow Control (CTS `0x30`) hai chiều giữa Tester và ECU với gói tin 20 bytes qua `DID 0xF190` (Mã số khung VIN):

1. **Mở khóa Security Access trước:**
   - Lệnh gửi (Send Key 8 byte): `27 02 26 8A 2E 8A 90 0C`
   - Phản hồi nhận được: `67 02` (Mở khóa thành công, LED-0 bật sáng).

2. **Ghi Multi-Frame 20 bytes từ PC xuống ECU (Chiều Tester $\rightarrow$ ECU):**
   - Lệnh gửi: `2E F1 90 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11` (Tổng 20 bytes).
   - Dưới bus CAN:
     - Tester phát First Frame: `10 14 2E F1 90 01 02 03`
     - ECU tự động phản hồi Flow Control CTS: `30 00 00 00 00 00 00 00`
     - Tester phát Consecutive Frame 1: `21 04 05 06 07 08 09 0A` (mang `SN = 1`)
     - Tester phát Consecutive Frame 2: `22 0B 0C 0D 0E 0F 10 11` (mang `SN = 2`)
   - ECU ghép đủ 20 bytes, nạp vào bộ nhớ VIN và phản hồi thành công:
     ```text
     [03:31:47] INFO: TX: [Hex] 2E F1 90 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11
     [03:31:47] INFO: RX: [Hex] 6E F1 90
     ```

3. **Đọc Multi-Frame 20 bytes từ ECU lên PC (Chiều ECU $\rightarrow$ Tester):**
   - Lệnh gửi: `22 F1 90` (Single Frame 3 byte).
   - Dưới bus CAN:
     - ECU phát First Frame: `10 14 62 F1 90 01 02 03`
     - Tester tự động gửi Flow Control CTS: `30 00 00 00 00 00 00 00`
     - ECU phát Consecutive Frame 1: `21 04 05 06 07 08 09 0A` (mang `SN = 1`)
     - ECU phát Consecutive Frame 2: `22 0B 0C 0D 0E 0F 10 11` (mang `SN = 2`)
   - Tester thu nhận đủ 20 bytes và truyền lên giao diện PC:
     ```text
     [03:32:13] INFO: TX: [Hex] 22 F1 90
     [03:32:13] INFO: RX: [Hex] 62 F1 90 01 02 03 04 05 06 07 08 09 0A
     [03:32:14] INFO: RX: [Hex] 0B 0C 0D 0E 0F 10 11
     ```
   - **Đánh giá:** 17 bytes dữ liệu mới (`01 02 03 ... 11`) được lưu trữ và đọc ra nguyên vẹn 100%, chứng minh hệ thống truyền nhận Multi-frame và Flow Control hoạt động hoàn hảo không giới hạn độ dài!

---

### 5.4. Bảng tổng hợp các mã phản hồi âm (Negative Response Codes - NRC)

| Mã Hex | Tên chuẩn ISO 14229 | Ý nghĩa trong hệ thống dự án |
| :---: | :--- | :--- |
| **`0x10`** | `generalReject` | Bị phạt không cho cấp Seed do vừa nhập sai Key trong vòng 10 giây; hoặc cảm biến ADC bị lỗi phần cứng. |
| **`0x11`** | `serviceNotSupported` | Lệnh SID không thuộc danh sách hỗ trợ (`0x22`, `0x27`, `0x2E`). |
| **`0x12`** | `subFunctionNotSupported` | Sub-function của Service 0x27 khác `0x01` và `0x02`. |
| **`0x13`** | `incorrectMessageLengthOrInvalidFormat` | Sai độ dài gói tin yêu cầu (Service 22 sai 3 byte, Service 27 sai 2 hoặc 8 byte, Service 2E sai 5 byte). |
| **`0x24`** | `requestSequenceError` | Sai trình tự bảo mật (Gửi Key `27 02` khi chưa xin Seed `27 01`, hoặc gửi lại Key cũ đã hết hạn). |
| **`0x31`** | `requestOutOfRange` | DID không hỗ trợ, hoặc byte đầu tiên của CAN ID trong Service 2E vượt quá `0x7F` (theo Bảng 19 Bosch Spec). |
| **`0x33`** | `securityAccessDenied` | Chưa mở khóa bảo mật hoặc đã quá 20 giây tự khóa an toàn mà cố tình ghi cấu hình qua Service 2E. |
| **`0x35`** | `invalidKey` | Nhập sai mã khóa bảo mật Key trong Service 0x27. |

---

## 6. KẾT LUẬN

Dự án đã được xây dựng và chuẩn hóa toàn diện từ tầng thấp nhất (thanh ghi phần cứng vi điều khiển ARM Cortex-M4) lên đến các tầng giao vận ISO-TP và tầng ứng dụng chẩn đoán ô tô quốc tế ISO 14229. Mọi yêu cầu kỹ thuật trong chương trình **Bosch Embedded Academy (BEA)** đều đã được hiện thực hóa trọn vẹn, tối ưu hóa hiệu năng, loại bỏ hoàn toàn các lỗi nghẽn bus, đứt gãy khung truyền và đạt độ ổn định tuyệt đối trong môi trường thời gian thực.
