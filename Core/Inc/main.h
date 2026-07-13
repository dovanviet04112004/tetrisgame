/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define VSYNC_FREQ_Pin GPIO_PIN_2
#define VSYNC_FREQ_GPIO_Port GPIOE
#define RENDER_TIME_Pin GPIO_PIN_3
#define RENDER_TIME_GPIO_Port GPIOE
#define FRAME_RATE_Pin GPIO_PIN_4
#define FRAME_RATE_GPIO_Port GPIOE
#define MCU_ACTIVE_Pin GPIO_PIN_5
#define MCU_ACTIVE_GPIO_Port GPIOE
#define SPI5_NCS_Pin GPIO_PIN_1
#define SPI5_NCS_GPIO_Port GPIOC
#define BTN_PAUSE_Pin GPIO_PIN_0
#define BTN_PAUSE_GPIO_Port GPIOA
#define BTN_LEFT_Pin GPIO_PIN_2
#define BTN_LEFT_GPIO_Port GPIOG
#define BTN_RIGHT_Pin GPIO_PIN_3
#define BTN_RIGHT_GPIO_Port GPIOG
#define BTN_ROT_CW_Pin GPIO_PIN_11
#define BTN_ROT_CW_GPIO_Port GPIOC
#define BTN_ROT_CCW2_Pin GPIO_PIN_12
#define BTN_ROT_CCW2_GPIO_Port GPIOC
#define BTN_SOFT_Pin GPIO_PIN_2
#define BTN_SOFT_GPIO_Port GPIOD
#define BTN_ROT_CCW_Pin GPIO_PIN_4
#define BTN_ROT_CCW_GPIO_Port GPIOD
#define BTN_HOLD_Pin GPIO_PIN_5
#define BTN_HOLD_GPIO_Port GPIOD
#define BTN_HARD_Pin GPIO_PIN_7
#define BTN_HARD_GPIO_Port GPIOD
#define JOY_SW_Pin GPIO_PIN_9
#define JOY_SW_GPIO_Port GPIOG
#define LED_GREEN_Pin GPIO_PIN_13
#define LED_GREEN_GPIO_Port GPIOG
#define LED_RED_Pin GPIO_PIN_14
#define LED_RED_GPIO_Port GPIOG
#define BTN_HOLD2_Pin GPIO_PIN_7
#define BTN_HOLD2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
