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

  /*Seca will be enabled within 5sec via LED0*/
  if(Seca_Timer>0)
  {
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	  Seca_Timer--;
  }
  else
  {
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  }

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
  if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &CAN1_pHeaderRx, CAN1_DATA_RX) == HAL_OK)
  {

		if(CAN1_DATA_RX[0] <= 0x07)/*single frame response*/
		{
		  /*detect response for SID27*/
		  PrintCANLog(CAN1_pHeaderRx.StdId, &CAN1_DATA_RX[0]);
		  Dcm_Seca_Gen_Keys();

		}
		else if((CAN1_DATA_RX[0] & 0xF0) == DCM_FS_FRAME)/*first frame response*/
		{
		  /*send flow control from CAN1*/
		}
		else if((CAN1_DATA_RX[0] & 0xF0) == DCM_FC_FRAME)/*flowcontrol frame response*/
		{
			Flg_Consecutive = 0x01;
			PrintCANLog(CAN1_pHeaderRx.StdId, &CAN1_DATA_RX[0]);
		}
		else if((CAN1_DATA_RX[0] & 0xF0) == DCM_CC_FRAME)/*consecutive frame response*/
		{

		}
		else{}

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
  * @brief This function handles CAN2 RX0 interrupts.
  */
void CAN2_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN CAN2_RX0_IRQn 0 */
	uint8_t NumByteSend;
  /* USER CODE END CAN2_RX0_IRQn 0 */
  HAL_CAN_IRQHandler(&hcan2);
  /* USER CODE BEGIN CAN2_RX0_IRQn 1 */
	HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO0, &CAN2_pHeaderRx, CAN2_DATA_RX);

	CAN_TP2DCM(CAN2_pHeaderRx.StdId, CAN2_DATA_RX);
	CAN_DCM2TP();

	if(Dcm_Msg_Info_s.respType != DCM_NORESP)
	{
		memset(&CAN2_DATA_TX,0x00,8);
		if((Dcm_Msg_Info_s.dataBuff[0] & 0xF0) == 0x00)
			NumByteSend = Dcm_Msg_Info_s.dataBuff[0] + 1;
		else
			NumByteSend = 8;

		for(LoopIndx = 0; LoopIndx < NumByteSend; LoopIndx++)
		{
			CAN2_DATA_TX[LoopIndx] = Dcm_Msg_Info_s.dataBuff[LoopIndx];
		}
		if((CAN2_DATA_TX[0] & 0xF0) == 0x20)
		{
			Flg_Consecutive = 0x01;
		}
		else
		{
			HAL_CAN_AddTxMessage(&hcan2, &CAN2_pHeader, CAN2_DATA_TX, &CAN2_pTxMailbox);
		}
	}

  /* USER CODE END CAN2_RX0_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void Dcm_Seca_Gen_Keys()
{
	if((CAN1_DATA_RX[1] == 0x67) && (CAN1_DATA_RX[2] == 0x01))
	{
	  CAN1_SEED[0] = CAN1_DATA_RX[3];
	  CAN1_SEED[1] = CAN1_DATA_RX[4];
	  CAN1_SEED[2] = CAN1_DATA_RX[5];
	  CAN1_SEED[3] = CAN1_DATA_RX[6];

	  /*key0 = seed0 XOR seed1*/
	  /*key1 = seed1  +  seed2*/
	  /*key2 = seed2 XOR seed3*/
	  /*key3 = seed3  +  seed1*/
	  CAN1_KEY[0] = (uint8_t)(CAN1_SEED[0] ^ CAN1_SEED[1]);
	  CAN1_KEY[1] = (uint8_t)(CAN1_SEED[1] + CAN1_SEED[2]);
	  CAN1_KEY[2] = (uint8_t)(CAN1_SEED[2] ^ CAN1_SEED[3]);
	  CAN1_KEY[3] = (uint8_t)(CAN1_SEED[3] + CAN1_SEED[0]);
#if SECA_FLOWCONTROL == 1
	  CAN1_KEY[4] = (uint8_t)(CAN1_SEED[0] | CAN1_SEED[1]);
	  CAN1_KEY[5] = (uint8_t)(CAN1_SEED[1] + CAN1_SEED[2]);
	  CAN1_KEY[6] = (uint8_t)(CAN1_SEED[2] | CAN1_SEED[3]);
	  CAN1_KEY[7] = (uint8_t)(CAN1_SEED[3] + CAN1_SEED[0]);
	  CAN1_KEY[8] = (uint8_t)(CAN1_SEED[0] & CAN1_SEED[1]);
	  CAN1_KEY[9] = (uint8_t)(CAN1_SEED[1] + CAN1_SEED[2]);
	  CAN1_KEY[10] = (uint8_t)(CAN1_SEED[2] & CAN1_SEED[3]);
	  CAN1_KEY[11] = (uint8_t)(CAN1_SEED[3] + CAN1_SEED[0]);
	  CAN1_KEY[12] = (uint8_t)(CAN1_SEED[0] - CAN1_SEED[1]);
	  CAN1_KEY[13] = (uint8_t)(CAN1_SEED[1] + CAN1_SEED[2]);
	  CAN1_KEY[14] = (uint8_t)(CAN1_SEED[2] - CAN1_SEED[3]);
	  CAN1_KEY[15] = (uint8_t)(CAN1_SEED[3] + CAN1_SEED[0]);
#endif
	  if(!BtnA)
		  CAN1_KEY[3] = 0xFF;
#if SECA_FLOWCONTROL == 1
	  REQ_BUFFER[0]  = 0x67  ;
	  REQ_BUFFER[1]  = 0x02  ;
	  REQ_BUFFER[2]  = CAN1_KEY[0]   ;
	  REQ_BUFFER[3]  = CAN1_KEY[1]   ;
	  REQ_BUFFER[4]  = CAN1_KEY[2]   ;
	  REQ_BUFFER[5]  = CAN1_KEY[3]   ;
	  REQ_BUFFER[6]  = CAN1_KEY[4]   ;
	  REQ_BUFFER[7]  = CAN1_KEY[5]   ;
	  REQ_BUFFER[8]  = CAN1_KEY[6]   ;
	  REQ_BUFFER[9]  = CAN1_KEY[7]   ;
	  REQ_BUFFER[10] = CAN1_KEY[8]   ;
	  REQ_BUFFER[11] = CAN1_KEY[9]   ;
	  REQ_BUFFER[12] = CAN1_KEY[10]  ;
	  REQ_BUFFER[13] = CAN1_KEY[11]  ;
	  REQ_BUFFER[14] = CAN1_KEY[12]  ;
	  REQ_BUFFER[15] = CAN1_KEY[13]  ;
	  REQ_BUFFER[16] = CAN1_KEY[14]  ;
	  REQ_BUFFER[17] = CAN1_KEY[15]  ;
	  NumBytesReq = 18;
#else
	  memset(&CAN1_DATA_TX,0x00,8);
	  CAN1_DATA_TX[0] = 0x06;
	  CAN1_DATA_TX[1] = 0x27;CAN1_DATA_TX[2] = 0x02;
	  CAN1_DATA_TX[3] = CAN1_KEY[0];CAN1_DATA_TX[4] = CAN1_KEY[1];
	  CAN1_DATA_TX[5] = CAN1_KEY[2];CAN1_DATA_TX[6] = CAN1_KEY[3];
	  PrintCANLog(CAN1_pHeader.StdId, &CAN1_DATA_TX[0]);
	  HAL_CAN_AddTxMessage(&hcan1, &CAN1_pHeader, CAN1_DATA_TX, &CAN1_pTxMailbox);
#endif

	}
	else{}
}
/* USER CODE END 1 */
