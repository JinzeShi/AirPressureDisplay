#ifndef __MS5611_H
#define __MS5611_H

#include <stdlib.h>
#include <math.h>
#include "stm32f1xx_hal.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;
#define MS5611_SPI hspi2
#define MS5611_CSB_L HAL_GPIO_WritePin(MS5611_CSB_GPIO_Port, MS5611_CSB_Pin, GPIO_PIN_RESET);
#define MS5611_CSB_H HAL_GPIO_WritePin(MS5611_CSB_GPIO_Port, MS5611_CSB_Pin, GPIO_PIN_SET);

#define MS5611_SAMPLE_NUMBER                20
#define MS5611_SAMPLE_INTERVAL              5       // 5 ms
#define MS5611_SAMPLE_TEMP_IQR_THRESHOLD    200     // 2 degree celsius
#define MS5611_SAMPLE_PRESS_IQR_THRESHOLD   2000    // 2000 kpa
static int32_t MS5611_SAMPLE_TEMP[MS5611_SAMPLE_NUMBER];
static int32_t MS5611_SAMPLE_PRESS[MS5611_SAMPLE_NUMBER];

#define MS5611_COMMAND_RESET                0x1E
#define MS5611_COMMAND_D1CONVERT_OSC256     0x40
#define MS5611_COMMAND_D1CONVERT_OSC512     0x42
#define MS5611_COMMAND_D1CONVERT_OSC1024    0x44
#define MS5611_COMMAND_D1CONVERT_OSC2048    0x46
#define MS5611_COMMAND_D1CONVERT_OSC4096    0x48
#define MS5611_COMMAND_D2CONVERT_OSC256     0x50
#define MS5611_COMMAND_D2CONVERT_OSC512     0x52
#define MS5611_COMMAND_D2CONVERT_OSC1024    0x54
#define MS5611_COMMAND_D2CONVERT_OSC2048    0x56
#define MS5611_COMMAND_D2CONVERT_OSC4096    0x58
#define MS5611_COMMAND_ADCREAD              0x00
#define MS5611_COMMAND_PROMREAD             0xA0

void MS5611_Init(void);
void MS5611_ReadProm(uint16_t *Prom);
void MS5611_GetResult(const uint16_t *Prom, double *pTemp, double *pPress, double *pAsl);

void MS5611_ReadADC(uint32_t *pD, uint8_t convertCommand);
void MS5611_Calculate(uint32_t D1, uint32_t D2, const uint16_t *Prom, int32_t *pTemp, int32_t *pPress);
void MS5611_Transmit(uint8_t txData);
void MS5611_Receive(uint8_t *pRxData, uint32_t size);

int BoxPlot(double *pResult, int32_t *pData, uint32_t size, double threshold);
int QsortCmp(const void *a, const void *b);

#endif /* __MS5611_H */