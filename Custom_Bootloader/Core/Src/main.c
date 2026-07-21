/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BL_DEBUG_MSG_EN

#define D_UART    &huart3
#define C_UART    &huart2

#define FLASH_SECTOR2_BASE_ADDR    0x08008000U
#define BL_RX_LEN                  200
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ADDR_VALID		1
#define ADDR_INVALID	0

#define INVALID_SECTOR	0
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint8_t bl_rx_buffer[BL_RX_LEN];

uint8_t supported_commands[]={
		BL_GET_VER,
		BL_GET_HELP,
		BL_GET_CID,
		BL_GET_RDP_STATUS,
		BL_GO_TO_ADDR,
		BL_FLASH_ERASE,
		BL_MEM_WRITE,
		BL_EN_R_W_PROTECT,
		BL_MEM_READ,
		BL_READ_SECTOR_STATUS,
		BL_OTP_READ,
		BL_DIS_R_W_PROTECT
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
static void printmsg(char *format,...);
void bootloader_jump_to_user_app(void);
void bootloader_uart_read_data(void);

void bootloader_handle_getver_cmd(uint8_t* bl_rx_buffer);
void bootloader_handle_gethelp_cmd(uint8_t* bl_rx_buffer);
void bootloader_handle_getcid_cmd(uint8_t* bl_rx_buffer);
void bootloader_handle_getrdp_cmd(uint8_t* bl_rx_buffer);
void bootloader_handle_go_cmd(uint8_t* bl_rx_buffer);
void bootloader_handle_flash_erase_cmd(uint8_t* bl_rx_buffer);
void bootloader_handle_mem_write_cmd(uint8_t* bl_rx_buffer);
void bootloader_handle_en_rw_protect(uint8_t* bl_rx_buffer);
void bootloader_handle_mem_read(uint8_t* bl_rx_buffer);
void bootloader_handle_read_sector_protection_status(uint8_t* bl_rx_buffer);
void bootloader_handle_read_otp(uint8_t* bl_rx_buffer);
void bootloader_handle_dis_rw_protect(uint8_t* bl_rx_buffer);

void bootloader_send_nack(void);
void bootloader_send_ack(uint8_t command_code, uint8_t follow_len);
uint8_t bootloader_verify_crc(uint8_t* pData, uint32_t len, uint32_t crc_host);
uint8_t get_bootloader_version(void);
void bootloader_uart_write_data(uint8_t* pBuffer, uint32_t len);
uint32_t get_mcu_chip_id(void);
uint8_t get_flash_rdp_level(void);
uint8_t verify_addr(uint32_t go_address);
uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sector);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_CRC_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)==GPIO_PIN_RESET)
  {
	  printmsg("\nBL_DEBUG_MSG: Button is pressed .. going to BL mode\n\r");

	  //continue in bootloader mode
	  bootloader_uart_read_data();
  }
  else
  {
	  printmsg("BL_DEBUG_MSG: Button is NOT pressed .. executing user function\n\r");
	  bootloader_jump_to_user_app();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */


    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  GPIO_InitStruct.Pin = GPIO_PIN_10; //USART3_TX
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = 7;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_5; //USART3_RX
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = 7;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void printmsg(char *format,...)
{
#ifdef BL_DEBUG_MSG_EN

	char str[80];

	/* Extract the argumet list using VA apis*/
	va_list args;
	va_start(args, format);
	vsprintf(str, format, args);
	HAL_UART_Transmit(D_UART, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
	va_end(args);

#endif
}

void bootloader_uart_read_data(void)
{
	uint8_t rcv_len=0;

	while(1)
	{
		memset(bl_rx_buffer,0,200);
		//here we will read and decode the commands coming from the host
		//first read will consist of only 1 byte from the host, which is going to be the length field
		HAL_UART_Receive(C_UART, bl_rx_buffer, 1, HAL_MAX_DELAY);
		rcv_len=bl_rx_buffer[0];

		//receiving the actual command
		HAL_UART_Receive(C_UART, bl_rx_buffer, rcv_len, HAL_MAX_DELAY);

		switch ( bl_rx_buffer[0] )
		{
		case BL_GET_VER:
		  bootloader_handle_getver_cmd(bl_rx_buffer);
		  break;
		case BL_GET_HELP:
		  bootloader_handle_gethelp_cmd(bl_rx_buffer);
		  break;
		case BL_GET_CID:
		  bootloader_handle_getcid_cmd(bl_rx_buffer);
		  break;
		case BL_GET_RDP_STATUS:
		  bootloader_handle_getrdp_cmd(bl_rx_buffer);
		  break;
		case BL_GO_TO_ADDR:
		  bootloader_handle_go_cmd(bl_rx_buffer);
		  break;
		case BL_FLASH_ERASE:
		  bootloader_handle_flash_erase_cmd(bl_rx_buffer);
		  break;
		case BL_MEM_WRITE:
		  bootloader_handle_mem_write_cmd(bl_rx_buffer);
		  break;
		case BL_EN_R_W_PROTECT:
		  bootloader_handle_en_rw_protect(bl_rx_buffer);
		  break;
		case BL_MEM_READ:
		  bootloader_handle_mem_read(bl_rx_buffer);
		  break;
		case BL_READ_SECTOR_STATUS:
		  bootloader_handle_read_sector_protection_status(bl_rx_buffer);
		  break;
		case BL_OTP_READ:
		  bootloader_handle_read_otp(bl_rx_buffer);
		  break;
		case BL_DIS_R_W_PROTECT:
		  bootloader_handle_dis_rw_protect(bl_rx_buffer);
		  break;
		default:
		  printmsg("BL_DEBUG_MSG: Invalid command code received from host \n");
		  break;
		}
	}
}

void bootloader_jump_to_user_app(void)
{
	//a function pointer to hold the addr of the reset handler of the user app
	void(*app_reset_handler)(void);
	printmsg("BL_DEBUG_MSG: bootloader_jump_to_user_app\n");

	//1. Config the MSP by adding the value from the base addr of the sector 2
	uint32_t msp_value=*(volatile uint32_t*)FLASH_SECTOR2_BASE_ADDR;
	printmsg("BL_DEBUG_MSG: MSP value: %#x\n",msp_value);

	//This function comes from CMSIS
	__set_MSP(msp_value);

	//2. Now the addr of the reset handler of the user application
	// is fetched from the location FLASH_SECTOR2_BASE_ADDR+4
	uint32_t resethandler_addr=*(volatile uint32_t*)(FLASH_SECTOR2_BASE_ADDR+4);

	app_reset_handler=(void*)resethandler_addr;
	printmsg("BL_DEBUG_MSG: App reset handler address: %#x\n",app_reset_handler);

	//3. jump to reset handler of the user application
	app_reset_handler();
}

void bootloader_handle_getver_cmd(uint8_t* bl_rx_buffer)
{
	uint8_t bl_version;

	//1. Verify the checksum
	printmsg("BL_DEBUG_MSG: bootloader_handle_getver_cmd\n");

	//total length of command packet
	//here bl_rx_buffer[0] contains the value of bytes to follow and
	//1 is added to include the rcv_len byte i.e. 1st byte (bl_rx_buffer[0])
	uint32_t command_packet_len=bl_rx_buffer[0]+1;

	//extract the crc32 sent by the host
	//4 is being subtracted here because total len is of 6 bytes and first 2 bytes
	//are len tto follow and command code the rest of the 4 are crc
	uint32_t host_crc=*((uint32_t*)(bl_rx_buffer+command_packet_len-4));

	//this funct returns 0 then crc is good if non zero val returned then crc is bad
	if(!bootloader_verify_crc(&bl_rx_buffer[0],command_packet_len-4,host_crc))
	{
		printmsg("BL_DEBUG_MSG: Checksum successful!!\n");
		bootloader_send_ack(bl_rx_buffer[0],1);
		bl_version=get_bootloader_version();
		printmsg("BL_DEBUG_MSG: BL VERSION %#x\n",bl_version);
		bootloader_uart_write_data(&bl_version,1);
	}
	else
	{
		printmsg("BL_DEBUG_MSG: checksum fail!!\n");
		//checksum is wrong sending nack
		bootloader_send_nack();
	}
}

//bootloader sneds out ALL supported functions
void bootloader_handle_gethelp_cmd(uint8_t* bl_rx_buffer)
{
	//1. Verify the checksum
	printmsg("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd\n");

	//total length of command packet
	//here bl_rx_buffer[0] contains the value of bytes to follow and
	//1 is added to include the rcv_len byte i.e. 1st byte (bl_rx_buffer[0])
	uint32_t command_packet_len=bl_rx_buffer[0]+1;

	//extract the crc32 sent by the host
	//4 is being subtracted here because total len is of 6 bytes and first 2 bytes
	//are len tto follow and command code the rest of the 4 are crc
	uint32_t host_crc=*((uint32_t*)(bl_rx_buffer+command_packet_len-4));

	//this funct returns 0 then crc is good if non zero val returned then crc is bad
	if(!bootloader_verify_crc(&bl_rx_buffer[0],command_packet_len-4,host_crc))
	{
		printmsg("BL_DEBUG_MSG: Checksum successful!!\n");
		bootloader_send_ack(bl_rx_buffer[0],sizeof(supported_commands));
		bootloader_uart_write_data(supported_commands,sizeof(supported_commands));
	}
	else
	{
		printmsg("BL_DEBUG_MSG: checksum fail!!\n");
		//checksum is wrong sending nack
		bootloader_send_nack();
	}
}

void bootloader_handle_getcid_cmd(uint8_t* bl_rx_buffer)
{
	uint32_t chip_id;

	//1. Verify the checksum
	printmsg("BL_DEBUG_MSG: bootloader_handle_getcid_cmd\n");

	//total length of command packet
	//here bl_rx_buffer[0] contains the value of bytes to follow and
	//1 is added to include the rcv_len byte i.e. 1st byte (bl_rx_buffer[0])
	uint32_t command_packet_len=bl_rx_buffer[0]+1;

	//extract the crc32 sent by the host
	//4 is being subtracted here because total len is of 6 bytes and first 2 bytes
	//are len tto follow and command code the rest of the 4 are crc
	uint32_t host_crc=*((uint32_t*)(bl_rx_buffer+command_packet_len-4));

	//this funct returns 0 then crc is good if non zero val returned then crc is bad
	if(!bootloader_verify_crc(&bl_rx_buffer[0],command_packet_len-4,host_crc))
	{
		printmsg("BL_DEBUG_MSG: Checksum successful!!\n");
		bootloader_send_ack(bl_rx_buffer[0],2);
		chip_id=get_mcu_chip_id();
		uint16_t dev_id = (uint16_t)(chip_id & 0xFFF);
		printmsg("BL_DEBUG_MSG: Chip ID is %#x\n",(chip_id&0xFFF));
		uint16_t chip_rev=(chip_id>>16)&0xFFFF;
		printmsg("BL_DEBUG_MSG: Chip REV is %#x\n",chip_rev);
		bootloader_uart_write_data((uint8_t*)&dev_id,2);
	}
	else
	{
		printmsg("BL_DEBUG_MSG: checksum fail!!\n");
		//checksum is wrong sending nack
		bootloader_send_nack();
	}
}

void bootloader_handle_getrdp_cmd(uint8_t* bl_rx_buffer)
{
	uint8_t rdp_level;

	//1. Verify the checksum
	printmsg("BL_DEBUG_MSG: bootloader_handle_getrdp_cmd\n");

	//total length of command packet
	//here bl_rx_buffer[0] contains the value of bytes to follow and
	//1 is added to include the rcv_len byte i.e. 1st byte (bl_rx_buffer[0])
	uint32_t command_packet_len=bl_rx_buffer[0]+1;

	//extract the crc32 sent by the host
	//4 is being subtracted here because total len is of 6 bytes and first 2 bytes
	//are len tto follow and command code the rest of the 4 are crc
	uint32_t host_crc=*((uint32_t*)(bl_rx_buffer+command_packet_len-4));

	//this funct returns 0 then crc is good if non zero val returned then crc is bad
	if(!bootloader_verify_crc(&bl_rx_buffer[0],command_packet_len-4,host_crc))
	{
		printmsg("BL_DEBUG_MSG: Checksum successful!!\n");
		bootloader_send_ack(bl_rx_buffer[0],1);
		rdp_level=get_flash_rdp_level();
		printmsg("BL_DEBUG_MSG: RDP level is %#x\n",rdp_level);
		bootloader_uart_write_data(&rdp_level,1);
	}
	else
	{
		printmsg("BL_DEBUG_MSG: checksum fail!!\n");
		//checksum is wrong sending nack
		bootloader_send_nack();
	}
}

void bootloader_handle_go_cmd(uint8_t* bl_rx_buffer)
{
	uint32_t go_addr;
	uint8_t addr_valid=ADDR_VALID;
	uint8_t addr_invalid=ADDR_INVALID;

	//1. Verify the checksum
	printmsg("BL_DEBUG_MSG: bootloader_handle_go_cmd\n");

	//total length of command packet
	//here bl_rx_buffer[0] contains the value of bytes to follow and
	//1 is added to include the rcv_len byte i.e. 1st byte (bl_rx_buffer[0])
	uint32_t command_packet_len=bl_rx_buffer[0]+1;

	//extract the crc32 sent by the host
	//4 is being subtracted here because total len is of 6 bytes and first 2 bytes
	//are len tto follow and command code the rest of the 4 are crc
	uint32_t host_crc=*((uint32_t*)(bl_rx_buffer+command_packet_len-4));

	//this funct returns 0 then crc is good if non zero val returned then crc is bad
	if(!bootloader_verify_crc(&bl_rx_buffer[0],command_packet_len-4,host_crc))
	{
		printmsg("BL_DEBUG_MSG: Checksum successful!!\n");
		bootloader_send_ack(bl_rx_buffer[0],1);

		//extract the go address
		go_addr=*((uint32_t*)&bl_rx_buffer[1]);
		printmsg("BL_DEBUG_MSG: GO address is %#x\n",go_addr);
		if(verify_addr(go_addr)==ADDR_VALID)
		{
			//tell host addr is fine
			bootloader_uart_write_data(&addr_valid,1);

			/*
		    uint32_t msp_value = *(volatile uint32_t*)go_addr;
		    __set_MSP(msp_value);

		    uint32_t resethandler_addr = *(volatile uint32_t*)(go_addr + 4);
		    void(*lets_jump)(void) = (void*)resethandler_addr;
		    */

		    go_addr+=1; //make T bit =1

		    void (*lets_jump)(void) = (void *)go_addr;

			printmsg("BL_DEBUG_MSG: Jumping to given address..\n");

			//when code jumps to this addr there must be valid executable code there
			//to see any result so the user must ensure that there is valid code present here
			lets_jump();
		}
		else
		{
			printmsg("BL_DEBUG_MSG: INVALID address\n");
			bootloader_uart_write_data(&addr_invalid,1);
		}
	}
	else
	{
		printmsg("BL_DEBUG_MSG: checksum fail!!\n");
		//checksum is wrong sending nack
		bootloader_send_nack();
	}
}

void bootloader_handle_flash_erase_cmd(uint8_t* bl_rx_buffer)
{
	uint8_t erase_status=0x00;
	printmsg("BL_DEBUG_MSG: bootloader_handle_flash_erase_cmd\n");

	//total length of command packet
	//here bl_rx_buffer[0] contains the value of bytes to follow and
	//1 is added to include the rcv_len byte i.e. 1st byte (bl_rx_buffer[0])
	uint32_t command_packet_len=bl_rx_buffer[0]+1;

	//extract the crc32 sent by the host
	//4 is being subtracted here because total len is of 6 bytes and first 2 bytes
	//are len tto follow and command code the rest of the 4 are crc
	uint32_t host_crc=*((uint32_t*)(bl_rx_buffer+command_packet_len-4));

	//this funct returns 0 then crc is good if non zero val returned then crc is bad
	if(!bootloader_verify_crc(&bl_rx_buffer[0],command_packet_len-4,host_crc))
	{
		printmsg("BL_DEBUG_MSG: Checksum successful!!\n");
		bootloader_send_ack(bl_rx_buffer[0],1);
		printmsg("BL_DEBUG_MSG: initial_sector: %d no_ofsectors: %d\n",bl_rx_buffer[1],bl_rx_buffer[2]);

		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
		erase_status=execute_flash_erase(bl_rx_buffer[1],bl_rx_buffer[2]);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);

		printmsg("BL_DEBUG_MSG: Flash erase status: %#x\n",erase_status);

		bootloader_uart_write_data(&erase_status, 1);
	}
	else
	{
		printmsg("BL_DEBUG_MSG: checksum fail!!\n");
		//checksum is wrong sending nack
		bootloader_send_nack();
	}

}

void bootloader_handle_mem_write_cmd(uint8_t* bl_rx_buffer)
{

}

void bootloader_handle_en_rw_protect(uint8_t* bl_rx_buffer)
{

}

void bootloader_handle_mem_read(uint8_t* bl_rx_buffer)
{

}

void bootloader_handle_read_sector_protection_status(uint8_t* bl_rx_buffer)
{

}

void bootloader_handle_read_otp(uint8_t* bl_rx_buffer)
{

}

void bootloader_handle_dis_rw_protect(uint8_t* bl_rx_buffer)
{

}

/* Helper functions */
uint8_t bootloader_verify_crc(uint8_t* pData, uint32_t len, uint32_t crc_host)
{
	uint32_t uwCRCValue=0xff;

	for(uint32_t i=0; i<len;i++)
	{
		uint32_t i_data=pData[i];
		uwCRCValue=HAL_CRC_Accumulate(&hcrc, &i_data, 1);
	}

	if(uwCRCValue==crc_host)
	{
		return VERIFY_CRC_SUCCESS;
	}
	return VERIFY_CRC_FAIL;
}

void bootloader_send_ack(uint8_t command_code, uint8_t follow_len)
{
	//here we send 2 byte
	//1st byte is ack and 2nd byte is len value
	uint8_t ack_buf[2];
	ack_buf[0]=BL_ACK;
	ack_buf[1]=follow_len;
	HAL_UART_Transmit(C_UART, ack_buf, 2, HAL_MAX_DELAY);
}

void bootloader_send_nack(void)
{
	uint8_t nack=BL_NACK;
	HAL_UART_Transmit(C_UART, &nack, 1, HAL_MAX_DELAY);
}

uint8_t get_bootloader_version(void)
{
	return (uint8_t)BL_VERSION;
}

void bootloader_uart_write_data(uint8_t* pBuffer, uint32_t len)
{
	HAL_UART_Transmit(C_UART, pBuffer, len, HAL_MAX_DELAY);
}

uint32_t get_mcu_chip_id(void)
{
	/*The STM32F446xx MCUs integrate an MCU ID code. This ID identifies the ST MCU part-
	number and the die revision. It is part of the DBG_MCU component and is mapped on the
	external PPB bus (see Section 33.16 on page 1292 ref manual).*/
	uint32_t cid;
	cid=(uint32_t)(DBGMCU->IDCODE);
	return cid;
}

uint8_t get_flash_rdp_level(void)
{
	uint8_t rdp_status=0;
	volatile uint32_t* pOB_addr=(uint32_t*)0x1FFFC000;
	rdp_status=(uint8_t)(*pOB_addr>>8);

	return rdp_status;
}

uint8_t verify_addr(uint32_t go_address)
{
	//system mem ===> valid address
	//sram1 mem ===> valid address
	//sram2 mem ===> valid address
	//sram mem ===> valid address
	//peripheral mem ===> valid address but do NOT allow
	//external mem ===> valid address

	if(go_address >= SRAM1_BASE && go_address <= (SRAM1_BASE+112*1024))
	{
		return ADDR_VALID;
	}
	else if(go_address >= SRAM2_BASE && go_address <= (SRAM1_BASE+16*1024))
	{
		return ADDR_VALID;
	}
	else if(go_address >= FLASH_BASE && go_address <= FLASH_END)
	{
		return ADDR_VALID;
	}
	else if(go_address >= BKPSRAM_BASE && go_address <= (BKPSRAM_BASE+4*1024))
	{
		return ADDR_VALID;
	}
	else
		return ADDR_INVALID;
}

uint8_t execute_flash_erase(uint8_t sector_number, uint8_t number_of_sector)
{
	//total 8 sectors on nucleo f446re (0 to 7)
	//if sector_number=0xff, then mass erase is initiated
	FLASH_EraseInitTypeDef flashErase_handle;
	uint32_t sectorError;
	HAL_StatusTypeDef status;

	if(number_of_sector>8)
		return INVALID_SECTOR;

	if((sector_number == 0xff) || (sector_number<=7))
	{
		if(sector_number == (uint8_t) 0xff)
		{
			flashErase_handle.TypeErase=FLASH_TYPEERASE_MASSERASE;
		}else{
			uint8_t remaining_sector=8-sector_number;
			if(number_of_sector>remaining_sector)
			{
				number_of_sector=remaining_sector;
			}
			flashErase_handle.TypeErase=FLASH_TYPEERASE_SECTORS;
			flashErase_handle.Sector=sector_number;
			flashErase_handle.NbSectors=number_of_sector;
		}
		flashErase_handle.Banks=FLASH_BANK_1;

		HAL_FLASH_Unlock();
		flashErase_handle.VoltageRange=FLASH_VOLTAGE_RANGE_3;
		status=(uint8_t)HAL_FLASHEx_Erase(&flashErase_handle, &sectorError);
		HAL_FLASH_Lock();

		return status;
	}else{
		return INVALID_SECTOR;
	}
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
