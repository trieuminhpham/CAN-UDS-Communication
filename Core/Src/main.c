/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lcd_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* =========================================================================
   CẤU HÌNH CHẾ ĐỘ HOẠT ĐỘNG:
   - 1 (HOME MODE): Ở nhà tự test -> CAN2 tự phát giả lập 0x0A2 (20ms).
   - 0 (EXAM / EXT MODE): Đi thi / Nối board ngoài -> CAN2 NGỪNG phát 0x0A2.
     Board ngoài sẽ phát 0x0A2. CAN2 vẫn chạy bình thường làm ECU cho bài UDS.
   ========================================================================= */
#define CONFIG_SIMULATE_CAN2_AT_HOME    1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint8_t uart3_receive;

extern uint8_t g_Security_Unlocked;
extern uint32_t g_Security_Unlock_Timestamp;

extern uint16_t g_Pending_New_CAN_ID;
extern uint8_t  g_Pending_ID_Ready;

uint16_t g_Current_CAN_ID = 0x012; // ID thuc te phat cua Bai 1
uint16_t g_TemperatureSensorRawValue_u16[1];

CAN_TxHeaderTypeDef CAN1_pHeader;
CAN_RxHeaderTypeDef CAN1_pHeaderRx;
CAN_FilterTypeDef CAN1_sFilterConfig;
CAN_TxHeaderTypeDef CAN2_pHeader;
CAN_RxHeaderTypeDef CAN2_pHeaderRx;
CAN_FilterTypeDef CAN2_sFilterConfig;
uint32_t CAN1_pTxMailbox;
uint32_t CAN2_pTxMailbox;

volatile uint16_t NumBytesReq = 0;
volatile uint8_t  g_Uds_Cmd_Ready = 0;
volatile uint32_t g_Last_Uart_Rx_Tick = 0;
uint8_t  REQ_BUFFER  [4096];
uint8_t  REQ_1BYTE_DATA;


uint8_t CAN1_DATA_TX[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t CAN1_DATA_RX[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t CAN2_DATA_TX[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t CAN2_DATA_RX[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

uint16_t Num_Consecutive_Tester;
uint8_t  Flg_Consecutive = 0;

unsigned int TimeStamp;
// maximum characters send out via UART is 30
char bufsend[30]="XXX: D1 D2 D3 D4 D5 D6 D7 D8  ";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN1_Init(void);
static void MX_CAN2_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
void MX_CAN1_Setup();
void MX_CAN2_Setup();
void USART3_SendString(uint8_t *ch);
void PrintCANLog(uint16_t CANID, uint8_t * CAN_Frame);
void delay(uint16_t delay);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Process_UDS_Payload(uint8_t *payload, uint16_t payload_len)
{
    if (payload_len == 0) return;

    // A. Single Frame (1 den 7 byte - vi du: 22 01 23, 27 01, 2E 01 23 34 56)
    if (payload_len <= 7)
    {
        uint8_t tester_tx_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
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
    // B. Multi-frame (8 byte tro len - vi du Send Key: 27 02 K0 K1 K2 K3 K4 K5 hoac payload dai)
    else
    {
        // 1. Gui First Frame (FF)
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

        HAL_Delay(10); // Cho ECU xu ly va tra loi Flow Control (neu can)

        // 2. Vong lap gui tat ca Consecutive Frames (CF) dong theo SN va so byte con lai
        uint16_t bytes_sent = 6;
        uint8_t sn = 1; // Sequence Number bat dau tu 1
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
            sn = (sn + 1) & 0x0F;

            retry = 50000;
            while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0 && retry--) {}
            HAL_CAN_AddTxMessage(&hcan1, &CAN1_pHeader, cf_data, &CAN1_pTxMailbox);
            if (bytes_sent < payload_len) {
                HAL_Delay(5);
            }
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  MX_CAN1_Setup();
  MX_CAN2_Setup();
  HAL_UART_Receive_IT(&huart3, &REQ_1BYTE_DATA, 1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)g_TemperatureSensorRawValue_u16, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  CAN1_pHeader.StdId = 0x012;
  PrintCANLog(CAN1_pHeader.StdId, &CAN1_DATA_TX[0]);
  
  // 1. Khoi tao LCD ST7789 va nen den
  //Switch_To_LCD();
  //LCD_BKL_H();
  LCD_Init();
  LCD_Init_Dashboard(CONFIG_SIMULATE_CAN2_AT_HOME);

  // 3. Chuyen PB6 sang CAN2_TX de CAN2 phat tin 0x0A2 va phan hoi UDS
  //Switch_To_CAN2();

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    //============================================
    // UU TIEN 1: XU LY UDS DIAGNOSTIC TU PC NGAY LAP TUC
    //============================================
    // TH1: Da nhan du tron ven khung Bosch (0F FF F0 ... F0 00 0F) -> Xu ly tuc thi (0ms delay)
    if (g_Uds_Cmd_Ready == 1)
    {
      if (NumBytesReq >= 6) {
          Process_UDS_Payload(&REQ_BUFFER[3], NumBytesReq - 6);
      }
      NumBytesReq = 0;
      g_Uds_Cmd_Ready = 0;
    }
    // TH2: Timeout sau 15ms im lang neu la lenh tho khong khung
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
          // Goi tin SOF bi dut quang hoac khong co EOF -> Bo qua de bao ve
      }
      else
      {
          // Lenh tho khong khung
          Process_UDS_Payload(&REQ_BUFFER[0], NumBytesReq);
      }
      NumBytesReq = 0;
      g_Uds_Cmd_Ready = 0;
    }

    // ============================================
    // CAP NHAT LCD MOI 500ms (Dashboard giam sat)
    // ============================================
    static uint32_t last_lcd_write = 0;
    if ((HAL_GetTick() - last_lcd_write) >= 500) {
        last_lcd_write = HAL_GetTick();

        // Doc nhiet do ADC thuc te (DID 0x0124)
        uint32_t v_sense = (g_TemperatureSensorRawValue_u16[0] * 3300) / 4095;
        uint8_t real_temp = (uint8_t)(((v_sense - 760) * 10) / 25 + 25);
        uint32_t uptime_sec = HAL_GetTick() / 1000;

        LCD_Update_Dashboard(uptime_sec,
                             CAN1_DATA_RX,
                             (uint16_t)g_Current_CAN_ID,
                             CAN1_DATA_TX,
                             CAN1_DATA_TX[6],
                             real_temp,
                             g_Security_Unlocked,
                             (uint16_t)g_Pending_New_CAN_ID,
                             g_Pending_ID_Ready);
    }

    /* Tu dong khoa va tat LED sau 20s (Service 27)*/
    if (g_Security_Unlocked == 1) {
      if ((HAL_GetTick() - g_Security_Unlock_Timestamp) >= 5000) {
        g_Security_Unlocked = 0; // Tu dong khoa lai
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // Tat led 0
      }
    }

    //============================================
    // BAI 1: CAN COMMUNICATION
    //============================================

	  static uint32_t last_tick_can1 = 0;

    if ((TimeStamp - last_tick_can1) >= 50)
    {
      last_tick_can1 += 50; // Reset lai moc thoi gian

      // 1. Tinh toan du lieu theo yeu cau de bai
      CAN1_DATA_TX[0] = CAN1_DATA_RX[0];
      CAN1_DATA_TX[1] = CAN1_DATA_RX[1];
      CAN1_DATA_TX[2] = CAN1_DATA_RX[0] + CAN1_DATA_RX[1];
      CAN1_DATA_TX[3] = 0x00;
      CAN1_DATA_TX[4] = 0x00;
      CAN1_DATA_TX[5] = 0x00;
      // Tinh  CRC-8 cho 6 byte dau tien tu byte 0 den byte 5 roi nhet vao byte 6
      CAN1_DATA_TX[6] = Caculate_CRC8_SAE_J1850(CAN1_DATA_TX, 6);
      CAN1_DATA_TX[7] = 0x00;

      // 2. Gan nhan (TX Header) - Tu dong chon Standard 11-bit hoac Extended 29-bit
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

      // 3. Ra lenh truyen goi tin vao mang can
      uint32_t retry = 50000;
      while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0 && retry--) {}
      HAL_CAN_AddTxMessage(&hcan1, &CAN1_pHeader, CAN1_DATA_TX, &CAN1_pTxMailbox);

      // 4. In ra man hinh Terminal may tinh qua UART de quan sat (chi 1 lan moi 1000ms)
     static uint32_t last_print_tick = 0;
     
       last_print_tick = TimeStamp;
       PrintCANLog(g_Current_CAN_ID, CAN1_DATA_TX);
     
    }

#if (CONFIG_SIMULATE_CAN2_AT_HOME == 1)
    static uint32_t last_tick_can2 = 0;
    static uint8_t can2_msg_counter = 0;
    if (TimeStamp - last_tick_can2 >= 20)
    {
      last_tick_can2 = TimeStamp;

      // Fake du lieu gia vao 2 byte dau cua can 2
      CAN2_DATA_TX[0] = 0x22;
      CAN2_DATA_TX[1] = 0x33;
      CAN2_DATA_TX[2] = 0x00;
      CAN2_DATA_TX[3] = 0x00;
      CAN2_DATA_TX[4] = 0x00;
      CAN2_DATA_TX[5] = 0x00;
      CAN2_DATA_TX[6] = 0x00;
      // Xu ly counter cho byte 7 (0x0 - 0xF)
      CAN2_DATA_TX[7] = can2_msg_counter;
      can2_msg_counter++;
      if (can2_msg_counter > 15)
    	can2_msg_counter = 0;


      // CAN2 gui di voi ID = 0xA2
      CAN2_pHeader.StdId = 0xA2;
      CAN2_pHeader.IDE = CAN_ID_STD;
      CAN2_pHeader.RTR = CAN_RTR_DATA;
      CAN2_pHeader.DLC = 8;

      // Gui du lieu vao mang can
      uint32_t retry2 = 50000;
      while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) == 0 && retry2--) {}
      HAL_CAN_AddTxMessage(&hcan2, &CAN2_pHeader, CAN2_DATA_TX, &CAN2_pTxMailbox);

    }
#endif


  /* USER CODE END 3 */
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_10TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief CAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 6;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_10TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PC13 PC4 PC5 PC6
                           PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /*Configure GPIO pin: PB0 (LED0) and PB1 (LED1)*/
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Tat LED mac dinh khi khoi dong
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_RESET);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void MX_CAN1_Setup()
{
  // 1. cau hinh filter cho can 1
  CAN1_sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
  CAN1_sFilterConfig.FilterBank = 0;
  CAN1_sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;

  // Cau hinh Filter 16-bit che do list (Cho phep 4 ID di qua)
  CAN1_sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
  CAN1_sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;

  // Mo cua filter cho ID cua bai 1 (0x0A2)
  CAN1_sFilterConfig.FilterIdHigh = 0x0A2 << 5;

  // Mo cua filter cho ID cua bai 2 (0x7A2 - Node 1 cho phan hoi tu ECU)
  CAN1_sFilterConfig.FilterIdLow = 0x7A2 << 5;
  
  // Cac slot con lai khong dung
  CAN1_sFilterConfig.FilterMaskIdHigh = 0x0000;
  CAN1_sFilterConfig.FilterMaskIdLow = 0x0000;

  CAN1_sFilterConfig.SlaveStartFilterBank = 14;

  // Ap dung filter
  if (HAL_CAN_ConfigFilter(&hcan1, &CAN1_sFilterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  // 2. Khoi dong phan cung can 1
  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }

  // 3. Cho phep kich hoat ngat khi co tin nahn den
  if(HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
    Error_Handler();
  }

}

void MX_CAN2_Setup()
{
  CAN2_sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
  CAN2_sFilterConfig.FilterBank = 14;
  CAN2_sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;

  CAN2_sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
  CAN2_sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;

  // Mo cua cho ID cua bai 1 (0x012)
  CAN2_sFilterConfig.FilterIdHigh = 0x012 << 5;

  // Mo cua cho ID cua bai 2 (0x712 - Node 2 nhan lenh tu tester)
  CAN2_sFilterConfig.FilterIdLow = 0x712 << 5;

  // Cac slot con lai khong dung
  CAN2_sFilterConfig.FilterMaskIdHigh = 0x0000;
  CAN2_sFilterConfig.FilterMaskIdLow = 0x0000;


  CAN2_sFilterConfig.SlaveStartFilterBank = 14;

  HAL_CAN_ConfigFilter(&hcan2, &CAN2_sFilterConfig);
  HAL_CAN_Start(&hcan2);
  HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)g_TemperatureSensorRawValue_u16, 1);
}

void USART3_SendString(uint8_t *ch)
{
   while(*ch!=0)
   {
      HAL_UART_Transmit(&huart3, ch, 1,HAL_MAX_DELAY);
      ch++;
   }
}

void PrintCANLog(uint16_t CANID, uint8_t * CAN_Frame)
{
    char printBuf[100]; // Tạo một mảng đủ rộng để chứa toàn bộ câu

    // Gộp tất cả TimeStamp, ID và 8 byte Data vào chung 1 câu duy nhất, có sẵn dấu xuống dòng \r\n
    if (CANID > 0x7FF) {
        sprintf(printBuf, "[%u ms] ID: %04X | Data: %02X %02X %02X %02X %02X %02X %02X %02X \r\n",
                TimeStamp, CANID,
                CAN_Frame[0], CAN_Frame[1], CAN_Frame[2], CAN_Frame[3],
                CAN_Frame[4], CAN_Frame[5], CAN_Frame[6], CAN_Frame[7]);
    } else {
        sprintf(printBuf, "[%u ms] ID: %03X | Data: %02X %02X %02X %02X %02X %02X %02X %02X \r\n",
                TimeStamp, CANID,
                CAN_Frame[0], CAN_Frame[1], CAN_Frame[2], CAN_Frame[3],
                CAN_Frame[4], CAN_Frame[5], CAN_Frame[6], CAN_Frame[7]);
    }

    // Bắn 1 phát duy nhất lên máy tính
    USART3_SendString((uint8_t*)printBuf);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        g_Last_Uart_Rx_Tick = HAL_GetTick();

        if (!g_Uds_Cmd_Ready && NumBytesReq < sizeof(REQ_BUFFER))
        {
            REQ_BUFFER[NumBytesReq] = REQ_1BYTE_DATA;
            NumBytesReq++;

            // Kiem tra da nhan du tron ven khung dong goi Bosch chua:
            // SOF (0F FF F0) + Payload (>= 1) + EOF (F0 00 0F) -> NumBytesReq >= 7
            if (NumBytesReq >= 7)
            {
                if (REQ_BUFFER[0] == 0x0F && REQ_BUFFER[1] == 0xFF && REQ_BUFFER[2] == 0xF0 &&
                    REQ_BUFFER[NumBytesReq - 3] == 0xF0 &&
                    REQ_BUFFER[NumBytesReq - 2] == 0x00 &&
                    REQ_BUFFER[NumBytesReq - 1] == 0x0F)
                {
                    g_Uds_Cmd_Ready = 1; // Khung hoan chinh tu DiagChecker!
                }
            }
        }
    }
}

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

void delay(uint16_t delay)
{
	HAL_Delay(delay);
}

uint8_t Caculate_CRC8_SAE_J1850(uint8_t *data, uint8_t length)
{
  uint8_t crc = 0xFF;
  for (uint8_t i = 0; i < length; i++)
  {
    crc ^=data[i];
    for (uint8_t j = 0; j < 8; j++)
    {
      if ((crc & 0x80) != 0) {
        crc = (uint8_t)((crc << 1) ^ 0x1D);
      } else {
        crc <<= 1;
      }
    }
  }
  return crc ^ 0xFF;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
