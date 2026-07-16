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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
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
extern void bootloader_jump_to_user_app(void);
extern void bootloader_uart_read_data(void);

extern void bootloader_handle_getver_cmd(uint8_t* bl_rx_buffer);
extern void bootloader_handle_gethelp_cmd(uint8_t* bl_rx_buffer);
extern void bootloader_handle_getcid_cmd(uint8_t* bl_rx_buffer);
extern void bootloader_handle_getrdp_cmd(uint8_t* bl_rx_buffer);
extern void bootloader_handle_go_cmd(uint8_t* bl_rx_buffer);
extern void bootloader_handle_flash_erase_cmd(uint8_t* bl_rx_buffer);
extern void bootloader_handle_mem_write_cmd(uint8_t* bl_rx_buffer);
extern void bootloader_handle_en_rw_protect(uint8_t* bl_rx_buffer);
extern void bootloader_handle_mem_read(uint8_t* bl_rx_buffer);
extern void bootloader_handle_read_sector_protection_status(uint8_t* bl_rx_buffer);
extern void bootloader_handle_read_otp(uint8_t* bl_rx_buffer);
extern void bootloader_handle_dis_rw_protect(uint8_t* bl_rx_buffer);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define BL_ACK					0xA5
#define BL_NACK					0x7E
#define BL_VERSION				0x10

#define VERIFY_CRC_SUCCESS		1
#define VERIFY_CRC_FAIL			0

//this cmd is used to read bootloadeer version from the mcu
#define BL_GET_VER				0x51

//used to know all the commands supported by the bootloader
#define BL_GET_HELP				0x52

//used to read the mcu chip identification number
#define BL_GET_CID				0x53

//this command is used to read the flash read protection level
#define BL_GET_RDP_STATUS		0x54

//this command is used to jump bootloader to user specified address
#define BL_GO_TO_ADDR			0x55

//command used to mass erase or sector erase of the mcu flash
#define BL_FLASH_ERASE			0x56

//This command is used to write data in to different memories of the MCU
#define BL_MEM_WRITE			0x57

//This command is used to enable read/write protect on different sectors of the user flash .
#define BL_EN_R_W_PROTECT		0x58

//This command is used to read data from different memories of the microcontroller.
#define BL_MEM_READ				0x59

//This command is used to read all the sector protection status.
#define BL_READ_SECTOR_STATUS	0x5A

//This command is used to read the OTP contents.
#define BL_OTP_READ				0x5B

//This command is used to disable read/write protection on different sectors of the user flash .
//This command takes the protection status to default state.
#define BL_DIS_R_W_PROTECT		0x5C

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
