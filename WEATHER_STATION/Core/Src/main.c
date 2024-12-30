/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stdio.h"
#include "ILI9341_STM32_Driver.h"
#include "fonts.h"
#include "Image_Lib.h"
#include "touch.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
//#include "W25Qxx.h"
//#include <calibrate.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_PIN_9

#define MAX_LEN 6
#define MAX_TOKENS 1024

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
struct DataSend{
	float lat;
	float lon;
};

struct DataReceive{
	int numberDay[7];
	int tempMax[7];
	int tempMin[7];
	int humidi[7];
};
struct DataReceive data_receive;

struct DataSend HoChiMinh = {10.8231,106.6297};
struct DataSend Hue = {16.4637,107.5909};
struct DataSend HaNoi = {21.0278,105.8342 };

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI2_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
RTC_DateTypeDef date_of_week;
//uint8_t current_day;
char *day_char[] = {"Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat"};

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t RHI, RHD, TCI, TCD, sum;// 8 bit high humidity, low humidity, high temperature, low temprature, checksum(parity)
uint32_t pMilis, cMillis;
float fCelsius = 0;
float fFahrenheit = 0;
float RH = 0;
uint8_t TFI = 0;
uint8_t TFD = 0;
char strCopy[30];
uint8_t rx_data;
char rx_buffer[1024];
char rx_letter[1024];
char rx_Day[1024];
int i = 0;
float temperatures[8];     // Lưu nhiệt độ 8 ngày
int valuesIndex = 0;
// Mảng chứa các chuỗi đã tách
char *tokens[8];
uint8_t values[MAX_TOKENS];        // Mảng lưu giá trị float từ các token
int state = 0; // 0: màn hình chính, 1: Hue, 2: Ho Chi Minh, 3: Nhiet do va do am
int isTitleDisplayed = 0; // Cờ để kiểm tra nếu Title đã hiển thị

void microDelay(uint16_t t)
{
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while(__HAL_TIM_GET_COUNTER(&htim1) < t);
}

uint8_t DHT11_Start()
{
	uint8_t Response = 0;
	GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
	GPIO_InitStructPrivate.Pin = DHT11_PIN;
	GPIO_InitStructPrivate.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructPrivate.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate);
	HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 0); // yeu cau data tu sensor
	HAL_Delay(20);// cho 20ms
	HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, 1); // micro gui tin hieu yeu cau
	microDelay(30); // delay 30us
	GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructPrivate.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructPrivate); // set the pin as input
	microDelay(40);
	if(!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)))
	{
		microDelay(80);
		if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
			Response = 1;
	}
	pMilis = HAL_GetTick();
	cMillis = HAL_GetTick();
	while((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && pMilis + 2 > cMillis)
	{
		cMillis = HAL_GetTick();
	}
	return Response;
}

uint8_t DHT11_Read()
{
	uint8_t a, b = 0;
	for(a = 0;a < 8 ;a++)
	{
		pMilis = HAL_GetTick();
		cMillis = HAL_GetTick();
		while(!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && (pMilis + 2 > cMillis))
		{
			//wait pin high
			cMillis = HAL_GetTick();
		}
		microDelay(40);
		if(!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
			b &= ~(1 << (7 - a));
		else
			b |= (1 << (7 - a));
		pMilis = HAL_GetTick();
		cMillis = HAL_GetTick();
		while((HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) && pMilis + 2 > cMillis)
		{
			//wait pin high
			cMillis = HAL_GetTick();
		}
	}
	return b;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	HAL_UART_Receive_IT(&huart1,(uint8_t *)&data_receive, sizeof(data_receive));
}

void Title(){ // display main screen
	ILI9341_FillRectangle(19, 258, 200, 32, ILI9341_COLOR565(254, 197, 230));
	ILI9341_FillRectangle(19, 203, 200, 32, ILI9341_COLOR565(254, 197, 230));
	ILI9341_FillRectangle(19, 148, 200, 32, ILI9341_COLOR565(254, 197, 230));
	ILI9341_FillRectangle(19, 93, 200, 32, ILI9341_COLOR565(254, 197, 230));
	ILI9341_WriteString(112, 10, "Du bao", Font_11x18, ILI9341_COLOR565(102, 255, 255), ILI9341_COLOR565(255, 204, 0));
	ILI9341_WriteString(48, 38, "thoi tiet", Font_16x26, ILI9341_COLOR565(102, 255, 255), ILI9341_COLOR565(255, 204, 0));
	ILI9341_WriteString(60, 100, "Ho Chi Minh", Font_11x18, ILI9341_COLOR565(58, 12, 163), ILI9341_COLOR565(254, 197, 230));
	ILI9341_WriteString(82, 155, "Ha Noi", Font_11x18, ILI9341_COLOR565(58, 12, 163), ILI9341_COLOR565(254, 197, 230));
	ILI9341_WriteString(96, 210, "Hue", Font_11x18, ILI9341_COLOR565(58, 12, 163), ILI9341_COLOR565(254, 197, 230));
	ILI9341_WriteString(25, 265, "Nhiet do va do am", Font_11x18, ILI9341_COLOR565(58, 12, 163), ILI9341_COLOR565(254, 197, 230));
	isTitleDisplayed = 1;
	//}
}

void DisplayWeather(){
	uint16_t x, y;
	if(ILI9341_TouchGetCoordinates(&x, &y) && state == 0)
	{
		//sprintf(rx_buffer, "%u %u ", x, y);
		//ILI9341_WriteString(0, 0, rx_buffer, Font_11x18, WHITE, BLACK);
		if(x >= 188 && x <= 196 && y >= 9 && y <= 318)
		{
			HAL_UART_Transmit(&huart1, (uint8_t *)&Hue, sizeof(Hue),1000);
			ILI9341_FillScreen(BLACK);
			ILI9341_WriteString(103, 1, "Hue", Font_11x18, ILI9341_COLOR565(255, 51, 0), BLACK);
			ILI9341_DrawImage(206, 1, 24, 24, button_back_Image);
			while(state != 1){
				for(int t = 0;t < 7;t++){
					int currentDay = (data_receive.numberDay[t] + i) % 7;
						if(t == 0){
							sprintf(rx_Day, "%s",day_char[currentDay]);
							ILI9341_WriteString(5, 30, rx_Day, Font_11x18, WHITE, BLACK);

							sprintf(rx_buffer, "%d",data_receive.tempMax[t]);
							ILI9341_WriteString(50, 30, rx_buffer, Font_11x18, WHITE, BLACK);
							sprintf(rx_letter, "%s","o");
							ILI9341_WriteString(73, 30, rx_letter, Font_7x10, WHITE, BLACK);

							sprintf(rx_buffer, "/%d",data_receive.tempMin[t]);
							ILI9341_WriteString(80, 30, rx_buffer, Font_11x18, WHITE, BLACK);
							sprintf(rx_letter, "%s","o");
							ILI9341_WriteString(113, 30, rx_letter, Font_7x10, WHITE, BLACK);

							sprintf(rx_buffer, "%d%%",data_receive.humidi[t]);
							ILI9341_WriteString(50, 55, rx_buffer, Font_11x18, WHITE, BLACK);
							if(data_receive.tempMax[t] >= 25){
								ILI9341_DrawImage(134, 30, 32, 32, Image_Sun);
							}
							else{
								ILI9341_DrawImage(134, 30, 32, 32, Image_Rain);
							}
						}
						else{
							sprintf(rx_Day, "%s",day_char[currentDay]);
							ILI9341_WriteString(5,35*t + 70 , rx_Day, Font_7x10, WHITE, BLACK);


							sprintf(rx_buffer, "%d",data_receive.tempMax[t]);
							ILI9341_WriteString(60, 35*t + 70, rx_buffer, Font_7x10, WHITE, BLACK);
							sprintf(rx_letter, "%s","o");
							ILI9341_WriteString(75, 35*t + 66, rx_letter, Font_7x10, WHITE, BLACK);

							sprintf(rx_buffer, "%d",data_receive.tempMin[t]);
							ILI9341_WriteString(90, 35*t + 70, rx_buffer, Font_7x10, WHITE, BLACK);
							sprintf(rx_letter, "%s","o");
							ILI9341_WriteString(105, 35*t + 66, rx_letter, Font_7x10, WHITE, BLACK);

							sprintf(rx_buffer, "%d%%",data_receive.humidi[t]);
							ILI9341_WriteString(164, 35*t + 70, rx_buffer, Font_7x10, WHITE, BLACK);
							if(data_receive.tempMax[t] >= 25){
								ILI9341_DrawImage(40, 35*t + 65, 16, 16, Image_Sun_16);
							}
							else{
								ILI9341_DrawImage(40, 35*t + 65, 16, 16, Image_rain_16);
							}
							ILI9341_DrawImage(140, 35*t + 65, 16, 16, Image_humidity_16);
						}

				}
				if (ILI9341_TouchGetCoordinates(&x, &y)) {
					if(x >= 4 && x <= 20 && y >= 284 && y <= 320){
						state = 1;
					}
				}
			}

		}
		else if((x >= 77 && x <= 99) && (y >= 9 && y <= 318))
		{
			HAL_UART_Transmit(&huart1, (uint8_t *)&HoChiMinh, sizeof(HoChiMinh),1000);
			ILI9341_FillScreen(BLACK);
			ILI9341_WriteString(50, 1, "Ho Chi Minh", Font_11x18, ILI9341_COLOR565(255, 51, 0), BLACK);
			ILI9341_DrawImage(206, 1, 24, 24, button_back_Image);
			while(state != 2){
				for(int t = 0;t < 7;t++){
					int currentDay = (data_receive.numberDay[t] + i) % 7;
					if(t == 0){
						sprintf(rx_Day, "%s",day_char[currentDay]);
						ILI9341_WriteString(5, 30, rx_Day, Font_11x18, WHITE, BLACK);

						sprintf(rx_buffer, "%d",data_receive.tempMax[t]);
						ILI9341_WriteString(50, 30, rx_buffer, Font_11x18, WHITE, BLACK);
						sprintf(rx_letter, "%s","o");
						ILI9341_WriteString(73, 30, rx_letter, Font_7x10, WHITE, BLACK);

						sprintf(rx_buffer, "/%d",data_receive.tempMin[t]);
						ILI9341_WriteString(80, 30, rx_buffer, Font_11x18, WHITE, BLACK);
						sprintf(rx_letter, "%s","o");
						ILI9341_WriteString(113, 30, rx_letter, Font_7x10, WHITE, BLACK);

						sprintf(rx_buffer, "%d%%",data_receive.humidi[t]);
						ILI9341_WriteString(50, 50, rx_buffer, Font_11x18, WHITE, BLACK);
						if(data_receive.tempMax[t] >= 25){
							ILI9341_DrawImage(134, 30, 32, 32, Image_Sun);
						}
						else{
							ILI9341_DrawImage(134, 30, 32, 32, Image_Rain);
						}
					}
					else{
						sprintf(rx_Day, "%s",day_char[currentDay]);
						ILI9341_WriteString(5,35*t + 70 , rx_Day, Font_7x10, WHITE, BLACK);

						sprintf(rx_buffer, "%d",data_receive.tempMax[t]);
						ILI9341_WriteString(60, 35*t + 70, rx_buffer, Font_7x10, WHITE, BLACK);
						sprintf(rx_letter, "%s","o");
						ILI9341_WriteString(75, 35*t + 66, rx_letter, Font_7x10, WHITE, BLACK);

						sprintf(rx_buffer, "%d",data_receive.tempMin[t]);
						ILI9341_WriteString(90, 35*t + 70, rx_buffer, Font_7x10, WHITE, BLACK);
						sprintf(rx_letter, "%s","o");
						ILI9341_WriteString(105, 35*t + 66, rx_letter, Font_7x10, WHITE, BLACK);

						sprintf(rx_buffer, "%d%%",data_receive.humidi[t]);
						ILI9341_WriteString(164, 35*t + 70, rx_buffer, Font_7x10, WHITE, BLACK);
						if(data_receive.tempMax[t] >= 25){
							ILI9341_DrawImage(40, 35*t + 65, 16, 16, Image_Sun_16);
						}
						else{
							ILI9341_DrawImage(40, 35*t + 65, 16, 16, Image_rain_16);
						}
						ILI9341_DrawImage(140, 35*t + 65, 16, 16, Image_humidity_16);
					}
				}
				//HAL_Delay(200);
				if (ILI9341_TouchGetCoordinates(&x, &y)) {
					if(x >= 4 && x <= 20 && y >= 284 && y <= 320){
						state = 2;
						return;
					}
				}
			}
		}

		else if((x >= 116 && x <= 185) && (y >= 9 && y <= 318))
		{
			HAL_UART_Transmit(&huart1, (uint8_t *)&HaNoi, sizeof(HaNoi),1000);
			ILI9341_FillScreen(BLACK);
			ILI9341_WriteString(87, 1, "Ha Noi", Font_11x18, ILI9341_COLOR565(255, 51, 0), BLACK);
			ILI9341_DrawImage(206, 1, 24, 24, button_back_Image);
			while(state != 2){
				for(int t = 0;t < 7;t++){
					int currentDay = (data_receive.numberDay[t] + i) % 7;
					if(t == 0){
						sprintf(rx_Day, "%s",day_char[currentDay]);
						ILI9341_WriteString(5, 30, rx_Day, Font_11x18, WHITE, BLACK);
						sprintf(rx_buffer, "%d",data_receive.tempMax[t]);
						ILI9341_WriteString(50, 30, rx_buffer, Font_11x18, WHITE, BLACK);
						sprintf(rx_letter, "%s","o");
						ILI9341_WriteString(73, 30, rx_letter, Font_7x10, WHITE, BLACK);

						sprintf(rx_buffer, "/%d",data_receive.tempMin[t]);
						ILI9341_WriteString(80, 30, rx_buffer, Font_11x18, WHITE, BLACK);
						sprintf(rx_letter, "%s","o");
						ILI9341_WriteString(113, 30, rx_letter, Font_7x10, WHITE, BLACK);

						sprintf(rx_buffer, "%d%%",data_receive.humidi[t]);
						ILI9341_WriteString(50, 50, rx_buffer, Font_11x18, WHITE, BLACK);
						if(data_receive.tempMax[t] >= 25){
							ILI9341_DrawImage(134, 30, 32, 32, Image_Sun);
							}
						else{
							ILI9341_DrawImage(134, 30, 32, 32, Image_Rain);
						}
					}
					else{
						sprintf(rx_Day, "%s",day_char[currentDay]);
						ILI9341_WriteString(5,35*t + 70 , rx_Day, Font_7x10, WHITE, BLACK);
						sprintf(rx_buffer, "%d",data_receive.tempMax[t]);
						ILI9341_WriteString(60, 35*t + 70, rx_buffer, Font_7x10, WHITE, BLACK);
						sprintf(rx_letter, "%s","o");
						ILI9341_WriteString(75, 35*t + 66, rx_letter, Font_7x10, WHITE, BLACK);

						sprintf(rx_buffer, "%d",data_receive.tempMin[t]);
						ILI9341_WriteString(90, 35*t + 70, rx_buffer, Font_7x10, WHITE, BLACK);
						sprintf(rx_letter, "%s","o");
						ILI9341_WriteString(105, 35*t + 66, rx_letter, Font_7x10, WHITE, BLACK);

						sprintf(rx_buffer, "%d%%",data_receive.humidi[t]);
						ILI9341_WriteString(164, 35*t + 70, rx_buffer, Font_7x10, WHITE, BLACK);
						if(data_receive.tempMax[t] >= 25){
							ILI9341_DrawImage(40, 35*t + 65, 16, 16, Image_Sun_16);
						}
						else{
							ILI9341_DrawImage(40, 35*t + 65, 16, 16, Image_rain_16);
						}
						ILI9341_DrawImage(140, 35*t + 65, 16, 16, Image_humidity_16);
						}
					}
					//HAL_Delay(200);
					if (ILI9341_TouchGetCoordinates(&x, &y)) {
						if(x >= 4 && x <= 20 && y >= 284 && y <= 320){
							state = 2;
							return;
						}
					}
				}
			}

		else if(x >= 202 && x <= 231 && y >= 9 && y <= 318){
			ILI9341_FillScreen(BLACK);
			ILI9341_DrawImage(206, 1, 24, 24, button_back_Image);
			ILI9341_WriteString(32, 93, "Temperature", Font_16x26, WHITE, BLACK);
			ILI9341_DrawImage(46, 129, 32, 32, Image_Temp);
			ILI9341_WriteString(56, 175, "Humidity", Font_16x26, WHITE, BLACK);
			ILI9341_DrawImage(46, 211, 32, 32, Image_Humidity);
			while(state != 3){
				if(DHT11_Start()){
					RHI = DHT11_Read();
					RHD = DHT11_Read();
					TCI = DHT11_Read();
					TCD = DHT11_Read();
					sum = DHT11_Read();
					// Kiểm tra tổng(checksum)
					if( RHI + RHD + TCI + TCD == sum)
					{
						// Tính toán nhiệt độ và độ ẩm
						fCelsius = (float)TCI + (float)(TCD/10.0);
						fFahrenheit = fCelsius * 9/5 + 32;
						RH = (float)RHI + (float)(RHD/10.0);
						// Can use tCelsius, tFahrenheit and RH for any purposes
						TFI = fFahrenheit;  // Fahrenheit integral
						TFD = fFahrenheit * 10 - TFI * 10; // Fahrenheit decimal
						// hiển thị nhiệt độ
						sprintf(strCopy, "%d.%d", TCI, TCD);
						ILI9341_WriteString(88, 131, strCopy, Font_16x26, WHITE, BLACK);
						sprintf(rx_letter, "%s","o");
						ILI9341_WriteString(169, 128, rx_letter, Font_11x18, WHITE, BLACK);
						// hiển thị độ ẩm
						sprintf(strCopy, "%d.%d %%", RHI, RHD);
						ILI9341_WriteString(88, 220, strCopy, Font_16x26, WHITE, BLACK);
					}
				}
				if (ILI9341_TouchGetCoordinates(&x, &y)) {
					if(x >= 4 && x <= 20 && y >= 284 && y <= 320){
						state = 3;
						return;
					}
				}
			}
		}
	}
}

void ReturnBack(){
	uint16_t x, y;
	if(ILI9341_TouchGetCoordinates(&x, &y)){
		if((x >= 4 && x <= 20 && y >= 284 && y <= 320) && (state == 1 || state == 2 || state == 3)){
			ILI9341_FillScreen(ILI9341_COLOR565(255, 204, 0));
			state = 0;
			isTitleDisplayed = 0;
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
	//uint8_t index;
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
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_SPI2_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim1);
  ILI9341_Init();
  ILI9341_FillScreen(ILI9341_COLOR565(255, 204, 0));
  //HAL_UART_Transmit(&huart1, (uint8_t *)&HoChiMinh, sizeof(HoChiMinh),1000);
  //HAL_UART_Transmit(&huart1, (uint8_t *)&Hue, sizeof(Hue),1000);
  //HAL_UART_Transmit(&huart1, (uint8_t *)&HaNoi, sizeof(Hue),1000);
  HAL_UART_Receive_IT(&huart1,(uint8_t*)&data_receive, sizeof(data_receive));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(state == 0 && isTitleDisplayed == 0){
	  Title();
	  }
	  DisplayWeather();
	  ReturnBack();
	  HAL_Delay(1000);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 15;
  sTime.Minutes = 28;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_NOVEMBER;
  DateToUpdate.Date = 18;
  DateToUpdate.Year = 24;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB10 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
