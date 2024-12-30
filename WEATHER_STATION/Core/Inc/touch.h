#ifndef TOUCH_H
#define TOUCH_H

#include <stdbool.h>
#include "stm32f1xx_hal.h"

/*** Redefine if necessary ***/

// Warning! Use SPI bus with < 1.3 Mbit speed, better ~650 Kbit to be save.
extern SPI_HandleTypeDef hspi2;

#define TOUCH_IRQ_PIN GPIO_PIN_3
#define TOUCH_IRQ_PORT GPIOA
#define TOUCH_CS_PIN GPIO_PIN_4
#define TOUCH_CS_PORT  GPIOA

// change depending on screen orientation
#define ILI9341_TOUCH_SCALE_X 240
#define ILI9341_TOUCH_SCALE_Y 320

// to calibrate uncomment UART_Printf line in ili9341_touch.c
#define ILI9341_TOUCH_MIN_RAW_X 1500
#define ILI9341_TOUCH_MAX_RAW_X 31000
#define ILI9341_TOUCH_MIN_RAW_Y 3276
#define ILI9341_TOUCH_MAX_RAW_Y 30110

// call before initializing any SPI devices
void ILI9341_TouchUnselect();

bool ILI9341_TouchPressed();
bool ILI9341_TouchGetCoordinates(uint16_t* x, uint16_t* y);

#endif

