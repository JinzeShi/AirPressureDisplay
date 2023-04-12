#include "MS5611.h"

static uint8_t rxBuffer[16] = {};

void MS5611_Init(void)
{
    MS5611_CSB_L
    MS5611_Transmit(MS5611_COMMAND_RESET);
    HAL_Delay(3);
    MS5611_CSB_H
}

void MS5611_ReadProm(uint16_t *Prom)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        MS5611_CSB_L
        MS5611_Transmit(MS5611_COMMAND_PROMREAD + 2*i);
        MS5611_Receive(rxBuffer + 2*i, 2);
        MS5611_CSB_H
    }

    for (uint8_t i = 0; i < 8; ++i)
    {
        *(Prom + i) = ((uint16_t)rxBuffer[2*i] << 8) | rxBuffer[2*i+1];
    }
}

void MS5611_GetResult(const uint16_t *Prom, double *pTemp, double *pPress, double *pAsl) {
    uint32_t D1 = 0, D2 = 0;
    int32_t Temp = 0, Press = 0;
    uint32_t count = 0;

    for (uint32_t i = 0; i < MS5611_SAMPLE_NUMBER; ++i) {
        MS5611_ReadADC(&D1, MS5611_COMMAND_D1CONVERT_OSC4096);
        MS5611_ReadADC(&D2, MS5611_COMMAND_D2CONVERT_OSC4096);
        if (D1 > 16777216 || D2 > 16777216) {
            continue;
        }
        MS5611_Calculate(D1, D2, Prom, &Temp, &Press);
        if (Temp >= -4000 && Temp <= 8500 && Press >= 1000 && Press <= 120000) {
            MS5611_SAMPLE_TEMP[count] = Temp;
            MS5611_SAMPLE_PRESS[count] = Press;
            ++count;
        }
        HAL_Delay(MS5611_SAMPLE_INTERVAL);
    }
    if (count == 0) {
        return;
    }

    if (BoxPlot(pTemp, MS5611_SAMPLE_TEMP, count, MS5611_SAMPLE_TEMP_IQR_THRESHOLD) != -1)
    {
        *pTemp /= 100.0f;
    }
    if (BoxPlot(pPress, MS5611_SAMPLE_PRESS, count, MS5611_SAMPLE_PRESS_IQR_THRESHOLD) != -1)
    {
        *pPress /= 1000.0f;
    }

//    *pAsl = 44330.0f * (1.0f - pow(*pPress / 101.3250f, 1.0f / 5.255f));
    *pAsl = ((pow((double)*pPress / 101.3250f, 1.0f / 5.255f) - 1) * (*pTemp + 273.15)) / 0.0065;
}

void MS5611_ReadADC(uint32_t *pD, uint8_t convertCommand)
{
    MS5611_CSB_L
    MS5611_Transmit(convertCommand);
    MS5611_CSB_H
    HAL_Delay(10);

    MS5611_CSB_L
    MS5611_Transmit(MS5611_COMMAND_ADCREAD);
    MS5611_Receive(rxBuffer, 3);
    MS5611_CSB_H

    *pD = ((uint32_t)rxBuffer[0] << 16) | ((uint32_t)rxBuffer[1] << 8) | (uint32_t)rxBuffer[2];
}

void MS5611_Calculate(uint32_t D1, uint32_t D2, const uint16_t *Prom, int32_t *pTemp, int32_t *pPress)
{
    int32_t dT = (int32_t)((int64_t)D2 - ((uint64_t)Prom[5] << 8));
    int64_t OFF = ((int64_t)Prom[2] << 16) + (((int64_t)dT * Prom[4]) >> 7);
    int64_t SENS = ((int64_t)Prom[1] << 15) + (((int64_t)dT * Prom[3]) >> 8);;

    *pTemp = 2000 + (int32_t)(((int64_t)dT * Prom[6]) >> 23);

    if (*pTemp < 2000)
    {
        int32_t T2 = (dT * dT) >> 31;
        OFF -= (5 * (*pTemp - 2000) * (*pTemp - 2000)) >> 1;
        SENS -= (5 * (*pTemp - 2000) * (*pTemp - 2000)) >> 2;

        if (*pTemp < -1500)
        {
            OFF -= 7 * (*pTemp + 1500) * (*pTemp + 1500);
            SENS -= (11 * (*pTemp + 1500) * (*pTemp + 1500)) >> 1;
        }

        *pTemp -= T2;
    }

    *pPress = (int32_t)(((((int64_t)D1 * SENS) >> 21) - OFF) >> 15);
}

void MS5611_Transmit(uint8_t txData)
{
    HAL_SPI_Transmit(&MS5611_SPI, &txData, 1, 0xFFFF);
}

void MS5611_Receive(uint8_t *pRxData, uint32_t size)
{
    HAL_SPI_Receive(&MS5611_SPI, pRxData, size, 0xFFFF);
}

int BoxPlot(double *pResult, int32_t *pData, uint32_t size, double threshold)
{
    double Q1, Q2, IQR;
    double result = 0;
    uint32_t start = 0, end = size - 1;
    uint32_t q = (size + 1) / 4, r = (size + 1) % 4;
    qsort(pData, size, sizeof(pData[0]), QsortCmp);

    if (r == 0)
    {
        Q1 = pData[q - 1];
        Q2 = pData[size - q - 1];
    }
    else
    {
        Q1 = (double)(r * pData[q] + (4 - r) * pData[q - 1]) / 4;
        Q2 = (double)(r * pData[size - q - 1] + (4 - r) * pData[size - q]) / 4;
    }

    IQR = Q2 - Q1;
    if (IQR > threshold)
    {
        return -1;
    }
    for(uint32_t i = 0; i < q; ++i)
    {
        if (pData[i] >= Q1 - 1.5 * IQR)
        {
            break;
        }
        ++start;
    }
    for(uint32_t i = 0; i < q; ++i)
    {
        if (pData[size - 1 - i] <= Q2 + 1.5 * IQR)
        {
            break;
        }
        --end;
    }

    for(uint32_t i = start; i <= end; ++i)
    {
        result += pData[i];
    }
    *pResult =  result / (end - start + 1);

    return 0;
}

int QsortCmp(const void *a, const void *b){
    return *(int *)a - *(int *)b;
}