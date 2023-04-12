#include "ili9325_GUI.h"

void ili9325_GUI_Draw(double Temp, double Press, double Asl)
{
    if (ILI9325_GUI_FLAG_REFRESH)
    {
        ili9325_Clear(ILI9325_GUI_COLOR_BACK_TABLE[ILI9325_GUI_COLOR_BACK_INDEX]);
        ili9325_GUI_DrawFramework();
        memset(ILI9325_GUI_DATA_TEMP, ILI9325_GUI_CHAR_BLANK, sizeof(ILI9325_GUI_DATA_TEMP));
        memset(ILI9325_GUI_DATA_PRESS, ILI9325_GUI_CHAR_BLANK, sizeof(ILI9325_GUI_DATA_PRESS));
        memset(ILI9325_GUI_DATA_ASL, ILI9325_GUI_CHAR_BLANK, sizeof(ILI9325_GUI_DATA_ASL));
        ILI9325_GUI_FLAG_REFRESH = 0;
    }

    ili9325_GUI_DrawData((int32_t)(Temp * 100), 1, 2, ILI9325_GUI_DATA_TEMP, ILI9325_GUI_Y_TEMP);
    ili9325_GUI_DrawData((int32_t)(Press * 1000), 0, 3, ILI9325_GUI_DATA_PRESS, ILI9325_GUI_Y_PRESS);

    uint8_t asl_decimal;
    if(Asl < 1000.0f && Asl > -100.0f)
    {
        asl_decimal = 3;
    }
    else if(Asl < 10000.0f && Asl > -1000.0f)
    {
        asl_decimal = 2;
    }
    else if(Asl < 100000.0f && Asl > -10000.0f)
    {
        asl_decimal = 1;
    }
    else if(Asl < 10000000.0f && Asl > -1000000.0f)
    {
        asl_decimal = 0;
    }
    else
    {
        return;
    }
    ili9325_GUI_DrawData((int32_t)(Asl * pow(10, asl_decimal)), 0, asl_decimal, ILI9325_GUI_DATA_ASL, ILI9325_GUI_Y_ASL);
}

void ili9325_GUI_ChangeColor(uint8_t option)
{
    if (option == ILI9325_GUI_FORE)
    {
        do {
            if (++ILI9325_GUI_COLOR_FORE_INDEX == ILI9325_GUI_COLOR_FORE_SIZE)
            {
                ILI9325_GUI_COLOR_FORE_INDEX = 0;
            }
        } while (ILI9325_GUI_COLOR_FORE_TABLE[ILI9325_GUI_COLOR_FORE_INDEX] ==
                 ILI9325_GUI_COLOR_BACK_TABLE[ILI9325_GUI_COLOR_BACK_INDEX]);
        ILI9325_GUI_FLAG_REFRESH = 1;
    }
    else if (option == ILI9325_GUI_BACK)
    {
        do {
            if (++ILI9325_GUI_COLOR_BACK_INDEX == ILI9325_GUI_COLOR_BACK_SIZE)
            {
                ILI9325_GUI_COLOR_BACK_INDEX = 0;
            }
        } while (ILI9325_GUI_COLOR_BACK_TABLE[ILI9325_GUI_COLOR_BACK_INDEX] ==
                 ILI9325_GUI_COLOR_FORE_TABLE[ILI9325_GUI_COLOR_FORE_INDEX]);
        ILI9325_GUI_FLAG_REFRESH = 1;
    }
}

void ili9325_GUI_Refresh(void)
{
    ILI9325_GUI_FLAG_REFRESH = 1;
}

void ili9325_GUI_DrawFramework(void)
{
    // Temp
    ili9325_GUI_DrawChar(ILI9325_GUI_X_TEMP, ILI9325_GUI_Y_TEMP, BITMAP_TEMP, BITMAP_TEMP_SIZE);
    ili9325_GUI_DrawChar(ILI9325_GUI_X_COLON, ILI9325_GUI_Y_TEMP, BITMAP_COLON, BITMAP_COLON_SIZE);

    ili9325_GUI_DrawChar(ILI9325_GUI_X_UNIT, ILI9325_GUI_Y_TEMP, BITMAP_TEMPUNIT, BITMAP_TEMPUNIT_SIZE);

    // Pressure
    ili9325_GUI_DrawChar(ILI9325_GUI_X_PRESS, ILI9325_GUI_Y_PRESS, BITMAP_PRESS, BITMAP_PRESS_SIZE);
    ili9325_GUI_DrawChar(ILI9325_GUI_X_COLON, ILI9325_GUI_Y_PRESS, BITMAP_COLON, BITMAP_COLON_SIZE);
    ili9325_GUI_DrawChar(ILI9325_GUI_X_UNIT, ILI9325_GUI_Y_PRESS, BITMAP_PRESSUNIT, BITMAP_PRESSUNIT_SIZE);

    // ASL
    ili9325_GUI_DrawChar(ILI9325_GUI_X_ASL, ILI9325_GUI_Y_ASL, BITMAP_ASL, BITMAP_ASL_SIZE);
    ili9325_GUI_DrawChar(ILI9325_GUI_X_COLON, ILI9325_GUI_Y_ASL, BITMAP_COLON, BITMAP_COLON_SIZE);
    ili9325_GUI_DrawChar(ILI9325_GUI_X_UNIT, ILI9325_GUI_Y_ASL, BITMAP_ASLUNIT, BITMAP_ASLUNIT_SIZE);
}

void ili9325_GUI_DrawData(int32_t data, uint8_t offset, uint8_t decimalNum, uint8_t *pRRecord, uint16_t Ypos)
{
    uint8_t ch, minus_flag = 0;
    uint32_t count = 0;
    uint32_t data_len = ILI9325_GUI_X_CHAR_TABLE_SIZE - offset;

    if (data < 0)
    {
        data = -data;
        minus_flag = 1;
    }
    while(data != 0 || count < decimalNum + 2)
    {
        ch = data % 10;
        if (*(pRRecord + count) != ch)
        {
            ili9325_GUI_SetChar(ch);
            ili9325_GUI_DrawChar(ILI9325_GUI_X_CHAR_TABLE[data_len - 1 - count], Ypos, BITMAP_CHAR, BITMAP_CHAR_SIZE);
            *(pRRecord + count) = ch;
        }
        data /= 10;
        ++count;
        // dot
        if (count == decimalNum)
        {
            if (*(pRRecord + count) != ILI9325_GUI_CHAR_DOT)
            {
                ili9325_GUI_SetChar(ILI9325_GUI_CHAR_DOT);
                ili9325_GUI_DrawChar(ILI9325_GUI_X_CHAR_TABLE[data_len - 1 - count], Ypos, BITMAP_CHAR, BITMAP_CHAR_SIZE);
                *(pRRecord + count) = ILI9325_GUI_CHAR_DOT;
            }
            ++count;
        }
    }
    if (minus_flag)
    {
        if (*(pRRecord + count) != ILI9325_GUI_CHAR_MINUS)
        {
            ili9325_GUI_SetChar(BITMAP_CHAR_TABLE_INDEX_MINUS);
            ili9325_GUI_DrawChar(ILI9325_GUI_X_CHAR_TABLE[data_len - 1 - count], Ypos, BITMAP_CHAR, BITMAP_CHAR_SIZE);
            *(pRRecord + count) = ILI9325_GUI_CHAR_MINUS;
        }
        ++count;
    }

    for (;count < data_len; ++count)
    {
        if(*(pRRecord + count) != ILI9325_GUI_CHAR_BLANK)
        {
            ili9325_GUI_SetChar(BITMAP_CHAR_TABLE_INDEX_BLANK);
            ili9325_GUI_DrawChar(ILI9325_GUI_X_CHAR_TABLE[data_len - 1 - count], Ypos, BITMAP_CHAR, BITMAP_CHAR_SIZE);
            *(pRRecord + count) = ILI9325_GUI_CHAR_BLANK;
        }
    }
}

void ili9325_GUI_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint8_t *pData, const uint32_t *pDataSize)
{
    ili9325_DrawChar(Xpos, Ypos, pData, pDataSize,
                     ILI9325_GUI_COLOR_FORE_TABLE[ILI9325_GUI_COLOR_FORE_INDEX],
                     ILI9325_GUI_COLOR_BACK_TABLE[ILI9325_GUI_COLOR_BACK_INDEX]);

}

void ili9325_GUI_SetChar(uint8_t c)
{
    for (uint32_t i = 0; i < BITMAP_CHAR_SIZE[0]; ++i)
    {
        for (uint32_t j = 0; j < BITMAP_CHAR_SIZE[1]; ++j)
        {
            BITMAP_CHAR[i * BITMAP_CHAR_SIZE[1] + j] =
                    BITMAP_CHAR_TABLE[BITMAP_CHAR_SIZE[1] * (BITMAP_CHAR_TABLE_SIZE*i + c) + j];
        }
    }
}