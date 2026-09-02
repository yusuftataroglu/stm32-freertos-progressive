/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os2.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_MSG_SIZE messageQueue_t
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define lcdRegisterSelect_Pin GPIO_PIN_10
#define lcdRegisterSelect_GPIO_Port GPIOB
#define lcdReadWrite_Pin GPIO_PIN_11
#define lcdReadWrite_GPIO_Port GPIOB
#define lcdData0_Pin GPIO_PIN_12
#define lcdData0_GPIO_Port GPIOB
#define lcdData1_Pin GPIO_PIN_13
#define lcdData1_GPIO_Port GPIOB
#define lcdData2_Pin GPIO_PIN_14
#define lcdData2_GPIO_Port GPIOB
#define lcdData3_Pin GPIO_PIN_15
#define lcdData3_GPIO_Port GPIOB
#define lcdEnable_Pin GPIO_PIN_6
#define lcdEnable_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
    typedef struct
    {
        uint8_t event_id;
        uint8_t data[32];
    } messageQueue_t;
    extern UART_HandleTypeDef huart1;
    extern ADC_HandleTypeDef hadc1;
    extern osMessageQueueId_t lcdQueueHandle;
    extern osMutexId_t lcdMutexHandle;
    extern uint8_t uartData[32];
    extern uint16_t adcData[2];
    extern messageQueue_t msg;
    extern DMA_HandleTypeDef hdma_adc1;
    extern osThreadId_t EmergencyTaskHandle;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
