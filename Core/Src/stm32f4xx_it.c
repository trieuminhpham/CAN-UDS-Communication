/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can_tp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t g_mf_rx_buffer[256];
uint8_t CAN1_SEED[4];
uint8_t CAN1_KEY[16];
uint8_t LoopIndx;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void Dcm_Seca_Gen_Keys();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern UART_HandleTypeDef huart3;
extern uint16_t g_Pending_New_CAN_ID;
extern uint8_t g_Pending_ID_Ready;
extern CAN_TxHeaderTypeDef CAN1_pHeader;
extern uint16_t g_Current_CAN_ID;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /*Support to print time stamp in CAN log*/
  TimeStamp ++;

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  // Gia lap Ignition Cycle: Neu co CAN ID moi dang cho thi ap dung
  if (g_Pending_ID_Ready ==1) {
    g_Pending_ID_Ready = 0; // Xoa co, danh dau da ap dung

    // 1. Khi an nut PA0 moi doi ID cua bai 1
    g_Current_CAN_ID = g_Pending_New_CAN_ID;

    // 2. Cap nhat lai bo loc CAN2 de nhan ID moi thay cho 0x012
    CAN2_sFilterConfig.FilterIdHigh = (g_Pending_New_CAN_ID & 0x7FF) << 5;
    // Giu nguyen slot 0x712 cho Diagnostic
    CAN2_sFilterConfig.FilterIdLow = 0x712 << 5;

    HAL_CAN_ConfigFilter(&hcan2, &CAN2_sFilterConfig);
  }

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles CAN1 RX0 interrupts.
  */
void CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN CAN1_RX0_IRQn 0 */

  /* USER CODE END CAN1_RX0_IRQn 0 */
  HAL_CAN_IRQHandler(&hcan1);
  /* USER CODE BEGIN CAN1_RX0_IRQn 1 */
  static uint16_t s_tester_ff_total_len = 0;
  static uint16_t s_tester_ff_recv_len = 0;
  uint8_t rx_raw[8];

  // Doc can bo dem FIFO0 de khong bi sot frame (Zero-loss)
  while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0)
  {
    if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &CAN1_pHeaderRx, rx_raw) != HAL_OK) {
      break;
    }

    // TH1: Nhan frame du lieu 0x0A2 tu Verification Board (hoac CAN2 gia lap)
    if (CAN1_pHeaderRx.StdId == 0x0A2) {
      for (int i = 0; i < 8; i++) {
        CAN1_DATA_RX[i] = rx_raw[i];
      }
    }
    // TH2: Nhan frame phan hoi UDS 0x7A2 tu ECU va chuyen tiep ve PC qua UART3
    else if (CAN1_pHeaderRx.StdId == 0x7A2) {
      uint8_t pci_type = rx_raw[0] & 0xF0;

      // Sub-case 1: Single Frame (0x00) - vi du Service 22, 2E hoac phan hoi 67 02
      if (pci_type == 0x00) {
        uint8_t data_len = rx_raw[0] & 0x0F;
        if (data_len > 0 && data_len <= 7) {
          uint8_t uart_resp[20];
          uart_resp[0] = 0x0F;
          uart_resp[1] = 0xFF;
          uart_resp[2] = 0xF0;
          for (int k = 0; k < data_len; k++) {
              uart_resp[3 + k] = rx_raw[1 + k];
          }
          uart_resp[3 + data_len]     = 0xF0;
          uart_resp[3 + data_len + 1] = 0x00;
          uart_resp[3 + data_len + 2] = 0x0F;

          uint32_t u_retry = 50000;
          while (huart3.gState != HAL_UART_STATE_READY && u_retry--) {}
          HAL_UART_Transmit(&huart3, uart_resp, 3 + data_len + 3, 100);
        }
      }
      // Sub-case 2: First Frame (0x10) - ECU phan hoi Multi-frame (vi du Seed 8 byte hoac dai hon)
      else if (pci_type == 0x10) {
        s_tester_ff_total_len = ((rx_raw[0] & 0x0F) << 8) | rx_raw[1];
        if (s_tester_ff_total_len > sizeof(g_mf_rx_buffer)) {
          // Báo lỗi tràn bộ đệm qua Flow Control Overflow
          uint8_t fc_frame[8] = {0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
          CAN_TxHeaderTypeDef FCHeader;
          uint32_t mailbox;
          FCHeader.StdId = 0x712;
          FCHeader.ExtId = 0;
          FCHeader.IDE = CAN_ID_STD;
          FCHeader.RTR = CAN_RTR_DATA;
          FCHeader.DLC = 8;
          uint32_t retry = 50000;
          while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0 && retry--) {}
          HAL_CAN_AddTxMessage(&hcan1, &FCHeader, fc_frame, &mailbox);
          s_tester_ff_total_len = 0;
          s_tester_ff_recv_len = 0;
        } else {
          for (int k = 0; k < 6 && k < s_tester_ff_total_len; k++) {
            g_mf_rx_buffer[k] = rx_raw[2 + k];
          }
          s_tester_ff_recv_len = 6;

          // Tester tu dong phan hoi Flow Control CTS (0x30) cho ECU
          uint8_t fc_frame[8] = {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
          CAN_TxHeaderTypeDef FCHeader;
          uint32_t mailbox;
          FCHeader.StdId = 0x712;
          FCHeader.ExtId = 0;
          FCHeader.IDE = CAN_ID_STD;
          FCHeader.RTR = CAN_RTR_DATA;
          FCHeader.DLC = 8;

          uint32_t retry = 50000;
          while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0 && retry--) {}
          HAL_CAN_AddTxMessage(&hcan1, &FCHeader, fc_frame, &mailbox);
        }
      }
      // Sub-case 3: Consecutive Frame (0x20) - ECU gui cac byte con lai
      else if (pci_type == 0x20) {
        if (s_tester_ff_total_len > 0 && s_tester_ff_recv_len < s_tester_ff_total_len) {
          uint16_t rem = s_tester_ff_total_len - s_tester_ff_recv_len;
          uint8_t chunk = (rem > 7) ? 7 : (uint8_t)rem;
          for (int k = 0; k < chunk; k++) {
            g_mf_rx_buffer[s_tester_ff_recv_len + k] = rx_raw[1 + k];
          }
          s_tester_ff_recv_len += chunk;

          // Neu da nhan du toan bo cac byte -> Dong goi khung Bosch va ban ve PC qua UART3
          if (s_tester_ff_recv_len >= s_tester_ff_total_len) {
            uint8_t uart_resp[270];
            uart_resp[0] = 0x0F;
            uart_resp[1] = 0xFF;
            uart_resp[2] = 0xF0;
            for (int k = 0; k < s_tester_ff_total_len; k++) {
              uart_resp[3 + k] = g_mf_rx_buffer[k];
            }
            uart_resp[3 + s_tester_ff_total_len]     = 0xF0;
            uart_resp[3 + s_tester_ff_total_len + 1] = 0x00;
            uart_resp[3 + s_tester_ff_total_len + 2] = 0x0F;

            uint32_t u_retry = 50000;
            while (huart3.gState != HAL_UART_STATE_READY && u_retry--) {}
            HAL_UART_Transmit(&huart3, uart_resp, 3 + s_tester_ff_total_len + 3, 100);

            s_tester_ff_total_len = 0;
            s_tester_ff_recv_len = 0;
          }
        }
      }
    }
  }
  /* USER CODE END CAN1_RX0_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */
  HAL_UART_Receive_IT(&huart3, &REQ_1BYTE_DATA, 1);
  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream0 global interrupt.
  */
void DMA2_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */

  /* USER CODE END DMA2_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */

  /* USER CODE END DMA2_Stream0_IRQn 1 */
}

/**
  * @brief This function handles CAN2 RX0 interrupts.
  */
void CAN2_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN CAN2_RX0_IRQn 0 */
	//uint8_t NumByteSend;
  /* USER CODE END CAN2_RX0_IRQn 0 */
  HAL_CAN_IRQHandler(&hcan2);
  /* USER CODE BEGIN CAN2_RX0_IRQn 1 */

  // Doc het tat ca frame trong bo dem FIFO0
  while (HAL_CAN_GetRxFifoFillLevel(&hcan2, CAN_RX_FIFO0) > 0)
  {
    if (HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &CAN2_pHeaderRx, CAN2_DATA_RX) != HAL_OK) {
      break;
    }

    // Kiem tra bai 1: Neu la 0x012 thi xu ly COM Task
    if (CAN2_pHeaderRx.StdId == 0x012) {
      // Code bai 1
    }
    // Kiem tra bai 2: Neu la 0x712 (Lenh tester gui) thi dua vao ISO-TP
    else if (CAN2_pHeaderRx.StdId == 0x712) {
      CAN_TP_RxIndication(CAN2_DATA_RX);
    }
  }

  /* USER CODE END CAN2_RX0_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
