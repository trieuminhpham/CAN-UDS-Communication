#include "lcd_hal.h"
#include <string.h>
#include <stdio.h>

// Font 6x8 don gian (95 ki tu ASCII tu x020 den 0x7E)
const unsigned char c_chFont1206[95][12] = {
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*" ",0*/
{0x00,0x00,0x00,0x00,0x3F,0x40,0x00,0x00,0x00,0x00,0x00,0x00},/*"!",1*/
{0x00,0x00,0x30,0x00,0x40,0x00,0x30,0x00,0x40,0x00,0x00,0x00},/*""",2*/
{0x09,0x00,0x0B,0xC0,0x3D,0x00,0x0B,0xC0,0x3D,0x00,0x09,0x00},/*"#",3*/
{0x18,0xC0,0x24,0x40,0x7F,0xE0,0x22,0x40,0x31,0x80,0x00,0x00},/*"$",4*/
{0x18,0x00,0x24,0xC0,0x1B,0x00,0x0D,0x80,0x32,0x40,0x01,0x80},/*"%",5*/
{0x03,0x80,0x1C,0x40,0x27,0x40,0x1C,0x80,0x07,0x40,0x00,0x40},/*"&",6*/
{0x10,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"'",7*/
{0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x80,0x20,0x40,0x40,0x20},/*"(",8*/
{0x00,0x00,0x40,0x20,0x20,0x40,0x1F,0x80,0x00,0x00,0x00,0x00},/*")",9*/
{0x09,0x00,0x06,0x00,0x1F,0x80,0x06,0x00,0x09,0x00,0x00,0x00},/*"*",10*/
{0x04,0x00,0x04,0x00,0x3F,0x80,0x04,0x00,0x04,0x00,0x00,0x00},/*"+",11*/
{0x00,0x10,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*",",12*/
{0x04,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x00,0x00},/*"-",13*/
{0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*".",14*/
{0x00,0x20,0x01,0xC0,0x06,0x00,0x38,0x00,0x40,0x00,0x00,0x00},/*"/",15*/
{0x1F,0x80,0x20,0x40,0x20,0x40,0x20,0x40,0x1F,0x80,0x00,0x00},/*"0",16*/
{0x00,0x00,0x10,0x40,0x3F,0xC0,0x00,0x40,0x00,0x00,0x00,0x00},/*"1",17*/
{0x18,0xC0,0x21,0x40,0x22,0x40,0x24,0x40,0x18,0x40,0x00,0x00},/*"2",18*/
{0x10,0x80,0x20,0x40,0x24,0x40,0x24,0x40,0x1B,0x80,0x00,0x00},/*"3",19*/
{0x02,0x00,0x0D,0x00,0x11,0x00,0x3F,0xC0,0x01,0x40,0x00,0x00},/*"4",20*/
{0x3C,0x80,0x24,0x40,0x24,0x40,0x24,0x40,0x23,0x80,0x00,0x00},/*"5",21*/
{0x1F,0x80,0x24,0x40,0x24,0x40,0x34,0x40,0x03,0x80,0x00,0x00},/*"6",22*/
{0x30,0x00,0x20,0x00,0x27,0xC0,0x38,0x00,0x20,0x00,0x00,0x00},/*"7",23*/
{0x1B,0x80,0x24,0x40,0x24,0x40,0x24,0x40,0x1B,0x80,0x00,0x00},/*"8",24*/
{0x1C,0x00,0x22,0xC0,0x22,0x40,0x22,0x40,0x1F,0x80,0x00,0x00},/*"9",25*/
{0x00,0x00,0x00,0x00,0x08,0x40,0x00,0x00,0x00,0x00,0x00,0x00},/*":",26*/
{0x00,0x00,0x00,0x00,0x04,0x60,0x00,0x00,0x00,0x00,0x00,0x00},/*";",27*/
{0x00,0x00,0x04,0x00,0x0A,0x00,0x11,0x00,0x20,0x80,0x40,0x40},/*"<",28*/
{0x09,0x00,0x09,0x00,0x09,0x00,0x09,0x00,0x09,0x00,0x00,0x00},/*"=",29*/
{0x00,0x00,0x40,0x40,0x20,0x80,0x11,0x00,0x0A,0x00,0x04,0x00},/*">",30*/
{0x18,0x00,0x20,0x00,0x23,0x40,0x24,0x00,0x18,0x00,0x00,0x00},/*"?",31*/
{0x1F,0x80,0x20,0x40,0x27,0x40,0x29,0x40,0x1F,0x40,0x00,0x00},/*"@",32*/
{0x00,0x40,0x07,0xC0,0x39,0x00,0x0F,0x00,0x01,0xC0,0x00,0x40},/*"A",33*/
{0x20,0x40,0x3F,0xC0,0x24,0x40,0x24,0x40,0x1B,0x80,0x00,0x00},/*"B",34*/
{0x1F,0x80,0x20,0x40,0x20,0x40,0x20,0x40,0x30,0x80,0x00,0x00},/*"C",35*/
{0x20,0x40,0x3F,0xC0,0x20,0x40,0x20,0x40,0x1F,0x80,0x00,0x00},/*"D",36*/
{0x20,0x40,0x3F,0xC0,0x24,0x40,0x2E,0x40,0x30,0xC0,0x00,0x00},/*"E",37*/
{0x20,0x40,0x3F,0xC0,0x24,0x40,0x2E,0x00,0x30,0x00,0x00,0x00},/*"F",38*/
{0x0F,0x00,0x10,0x80,0x20,0x40,0x22,0x40,0x33,0x80,0x02,0x00},/*"G",39*/
{0x20,0x40,0x3F,0xC0,0x04,0x00,0x04,0x00,0x3F,0xC0,0x20,0x40},/*"H",40*/
{0x20,0x40,0x20,0x40,0x3F,0xC0,0x20,0x40,0x20,0x40,0x00,0x00},/*"I",41*/
{0x00,0x60,0x20,0x20,0x20,0x20,0x3F,0xC0,0x20,0x00,0x20,0x00},/*"J",42*/
{0x20,0x40,0x3F,0xC0,0x24,0x40,0x0B,0x00,0x30,0xC0,0x20,0x40},/*"K",43*/
{0x20,0x40,0x3F,0xC0,0x20,0x40,0x00,0x40,0x00,0x40,0x00,0xC0},/*"L",44*/
{0x3F,0xC0,0x3C,0x00,0x03,0xC0,0x3C,0x00,0x3F,0xC0,0x00,0x00},/*"M",45*/
{0x20,0x40,0x3F,0xC0,0x0C,0x40,0x23,0x00,0x3F,0xC0,0x20,0x00},/*"N",46*/
{0x1F,0x80,0x20,0x40,0x20,0x40,0x20,0x40,0x1F,0x80,0x00,0x00},/*"O",47*/
{0x20,0x40,0x3F,0xC0,0x24,0x40,0x24,0x00,0x18,0x00,0x00,0x00},/*"P",48*/
{0x1F,0x80,0x21,0x40,0x21,0x40,0x20,0xE0,0x1F,0xA0,0x00,0x00},/*"Q",49*/
{0x20,0x40,0x3F,0xC0,0x24,0x40,0x26,0x00,0x19,0xC0,0x00,0x40},/*"R",50*/
{0x18,0xC0,0x24,0x40,0x24,0x40,0x22,0x40,0x31,0x80,0x00,0x00},/*"S",51*/
{0x30,0x00,0x20,0x40,0x3F,0xC0,0x20,0x40,0x30,0x00,0x00,0x00},/*"T",52*/
{0x20,0x00,0x3F,0x80,0x00,0x40,0x00,0x40,0x3F,0x80,0x20,0x00},/*"U",53*/
{0x20,0x00,0x3E,0x00,0x01,0xC0,0x07,0x00,0x38,0x00,0x20,0x00},/*"V",54*/
{0x38,0x00,0x07,0xC0,0x3C,0x00,0x07,0xC0,0x38,0x00,0x00,0x00},/*"W",55*/
{0x20,0x40,0x39,0xC0,0x06,0x00,0x39,0xC0,0x20,0x40,0x00,0x00},/*"X",56*/
{0x20,0x00,0x38,0x40,0x07,0xC0,0x38,0x40,0x20,0x00,0x00,0x00},/*"Y",57*/
{0x30,0x40,0x21,0xC0,0x26,0x40,0x38,0x40,0x20,0xC0,0x00,0x00},/*"Z",58*/
{0x00,0x00,0x00,0x00,0x7F,0xE0,0x40,0x20,0x40,0x20,0x00,0x00},/*"[",59*/
{0x00,0x00,0x70,0x00,0x0C,0x00,0x03,0x80,0x00,0x40,0x00,0x00},/*"\",60*/
{0x00,0x00,0x40,0x20,0x40,0x20,0x7F,0xE0,0x00,0x00,0x00,0x00},/*"]",61*/
{0x00,0x00,0x20,0x00,0x40,0x00,0x20,0x00,0x00,0x00,0x00,0x00},/*"^",62*/
{0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10},/*"_",63*/
{0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*"`",64*/
{0x00,0x00,0x02,0x80,0x05,0x40,0x05,0x40,0x03,0xC0,0x00,0x40},/*"a",65*/
{0x20,0x00,0x3F,0xC0,0x04,0x40,0x04,0x40,0x03,0x80,0x00,0x00},/*"b",66*/
{0x00,0x00,0x03,0x80,0x04,0x40,0x04,0x40,0x06,0x40,0x00,0x00},/*"c",67*/
{0x00,0x00,0x03,0x80,0x04,0x40,0x24,0x40,0x3F,0xC0,0x00,0x40},/*"d",68*/
{0x00,0x00,0x03,0x80,0x05,0x40,0x05,0x40,0x03,0x40,0x00,0x00},/*"e",69*/
{0x00,0x00,0x04,0x40,0x1F,0xC0,0x24,0x40,0x24,0x40,0x20,0x00},/*"f",70*/
{0x00,0x00,0x02,0xE0,0x05,0x50,0x05,0x50,0x06,0x50,0x04,0x20},/*"g",71*/
{0x20,0x40,0x3F,0xC0,0x04,0x40,0x04,0x00,0x03,0xC0,0x00,0x40},/*"h",72*/
{0x00,0x00,0x04,0x40,0x27,0xC0,0x00,0x40,0x00,0x00,0x00,0x00},/*"i",73*/
{0x00,0x10,0x00,0x10,0x04,0x10,0x27,0xE0,0x00,0x00,0x00,0x00},/*"j",74*/
{0x20,0x40,0x3F,0xC0,0x01,0x40,0x07,0x00,0x04,0xC0,0x04,0x40},/*"k",75*/
{0x20,0x40,0x20,0x40,0x3F,0xC0,0x00,0x40,0x00,0x40,0x00,0x00},/*"l",76*/
{0x07,0xC0,0x04,0x00,0x07,0xC0,0x04,0x00,0x03,0xC0,0x00,0x00},/*"m",77*/
{0x04,0x40,0x07,0xC0,0x04,0x40,0x04,0x00,0x03,0xC0,0x00,0x40},/*"n",78*/
{0x00,0x00,0x03,0x80,0x04,0x40,0x04,0x40,0x03,0x80,0x00,0x00},/*"o",79*/
{0x04,0x10,0x07,0xF0,0x04,0x50,0x04,0x40,0x03,0x80,0x00,0x00},/*"p",80*/
{0x00,0x00,0x03,0x80,0x04,0x40,0x04,0x50,0x07,0xF0,0x00,0x10},/*"q",81*/
{0x04,0x40,0x07,0xC0,0x02,0x40,0x04,0x00,0x04,0x00,0x00,0x00},/*"r",82*/
{0x00,0x00,0x06,0x40,0x05,0x40,0x05,0x40,0x04,0xC0,0x00,0x00},/*"s",83*/
{0x00,0x00,0x04,0x00,0x1F,0x80,0x04,0x40,0x00,0x40,0x00,0x00},/*"t",84*/
{0x04,0x00,0x07,0x80,0x00,0x40,0x04,0x40,0x07,0xC0,0x00,0x40},/*"u",85*/
{0x04,0x00,0x07,0x00,0x04,0xC0,0x01,0x80,0x06,0x00,0x04,0x00},/*"v",86*/
{0x06,0x00,0x01,0xC0,0x07,0x00,0x01,0xC0,0x06,0x00,0x00,0x00},/*"w",87*/
{0x04,0x40,0x06,0xC0,0x01,0x00,0x06,0xC0,0x04,0x40,0x00,0x00},/*"x",88*/
{0x04,0x10,0x07,0x10,0x04,0xE0,0x01,0x80,0x06,0x00,0x04,0x00},/*"y",89*/
{0x00,0x00,0x04,0x40,0x05,0xC0,0x06,0x40,0x04,0x40,0x00,0x00},/*"z",90*/
{0x00,0x00,0x00,0x00,0x04,0x00,0x7B,0xE0,0x40,0x20,0x00,0x00},/*"{",91*/
{0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xF0,0x00,0x00,0x00,0x00},/*"|",92*/
{0x00,0x00,0x40,0x20,0x7B,0xE0,0x04,0x00,0x00,0x00,0x00,0x00},/*"}",93*/
{0x40,0x00,0x80,0x00,0x40,0x00,0x20,0x00,0x20,0x00,0x40,0x00},/*"~",94*/
}; // Lay tu LCD_lib.c

// =========================================
// Ham chuyen doi chan PB6 
// =========================================
// void Switch_To_LCD(void)
// {
//     HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
//     GPIO_InitTypeDef g = {0};
//     g.Pin = GPIO_PIN_6;
//     g.Mode = GPIO_MODE_OUTPUT_PP;
//     g.Pull = GPIO_NOPULL;
//     g.Speed = GPIO_SPEED_FREQ_HIGH;
//     HAL_GPIO_Init(GPIOB, &g);
// }

// void Switch_To_CAN2(void)
// {
//     HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
//     GPIO_InitTypeDef g = {0};
//     g.Pin = GPIO_PIN_6;
//     g.Mode = GPIO_MODE_AF_PP;
//     g.Pull = GPIO_NOPULL;
//     g.Speed = GPIO_SPEED_FREQ_HIGH;
//     g.Alternate = GPIO_AF9_CAN2;
//     HAL_GPIO_Init(GPIOB, &g);
// }

// =============================================
// Ham khoi tao GPIO cho LCD (khong bao gom PB6)
// =============================================
void LCD_GPIO_Init(void)
{
    // Bật Clock cho GPIOA và GPIOB
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};

    // PB2(RST), PB7(CS), PB8(DC), PB3(SCK) = Output Push-Pull
    g.Pin = GPIO_PIN_2 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_3;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &g);

    // PA7(MOSI) = Output Push-Pull cho Software SPI
    g.Pin = GPIO_PIN_7;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &g);

    // Đặt trạng thái ban đầu: CS = 1, SCK = 1
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
}

// =============================================
// SPI SOFTWARE (Bit Bang) gui 1 byte
// =============================================
void LCD_SPI_Send(uint8_t data)
{
    for (int i = 7; i >= 0; i--) {
        // SCK LOW
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
        // Dat bit data len MOSI (PA7)
        if (data & (1 << i))
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
        // SCK HIGH -> LCD doc bit
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
    }
}

void LCD_WriteCmd(uint8_t cmd)
{
    LCD_DH_L(); // DC = 0 -> Lenh
    LCD_CS_L(); // CS = 0 -> Chon LCD
    LCD_SPI_Send(cmd);
    LCD_CS_H(); // CS = 1 -> Nghi
}

void LCD_WriteData(uint8_t data)
{
    LCD_DH_H(); // DC = 1 -> Data
    LCD_CS_L(); // CS = 0 -> Chon LCD
    LCD_SPI_Send(data);
    LCD_CS_H(); // CS = 1 -> Nghi
}

void LCD_WriteData16(uint16_t data)
{
    LCD_DH_H(); // DC = 1 (Chế độ dữ liệu)
    LCD_CS_L(); // CS = 0 (Chọn LCD)
    LCD_SPI_Send(data >> 8);   // Byte cao
    LCD_SPI_Send(data & 0xFF); // Byte thấp
    LCD_CS_H(); // CS = 1 (Nghỉ)
}

// =============================================
// Khoi tao LCD HX8347D
// =============================================
// =============================================
// Thiet lap vung toa do ve cho ST7789
// =============================================
void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    LCD_WriteCmd(0x2A); // CASET: Column Address Set
    LCD_WriteData(x0 >> 8);
    LCD_WriteData(x0 & 0xFF);
    LCD_WriteData(x1 >> 8);
    LCD_WriteData(x1 & 0xFF);

    LCD_WriteCmd(0x2B); // RASET: Row Address Set
    LCD_WriteData(y0 >> 8);
    LCD_WriteData(y0 & 0xFF);
    LCD_WriteData(y1 >> 8);
    LCD_WriteData(y1 & 0xFF);

    LCD_WriteCmd(0x2C); // RAMWR: Memory Write
}

void LCD_SetCursor(uint16_t x, uint16_t y) 
{
    LCD_SetAddressWindow(x, y, x, y);
}

// =============================================
// Khoi tao LCD chip ST7789 (Waveshare 2.8inch)
// =============================================
void LCD_Init(void)
{
    LCD_GPIO_Init();

    // 1. Reset phan cung
    LCD_RST_H();
    HAL_Delay(50);
    LCD_RST_L();
    HAL_Delay(100);
    LCD_RST_H();
    HAL_Delay(150);

    // 2. Tap lenh khoi dong ST7789
    LCD_WriteCmd(0x01); // Software Reset
    HAL_Delay(150);

    LCD_WriteCmd(0x11); // Sleep Out
    HAL_Delay(120);

    LCD_WriteCmd(0x36); // Memory Data Access Control (Orientation)
    LCD_WriteData(0x00); // Che do dung (Portrait)

    LCD_WriteCmd(0x3A); // Interface Pixel Format
    LCD_WriteData(0x05); // 16-bit RGB 565

    LCD_WriteCmd(0xB2); // Porch Setting
    LCD_WriteData(0x0C);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x00);
    LCD_WriteData(0x33);
    LCD_WriteData(0x33);

    LCD_WriteCmd(0xB7); // Gate Control
    LCD_WriteData(0x35);

    LCD_WriteCmd(0xBB); // VCOM Setting
    LCD_WriteData(0x19);

    LCD_WriteCmd(0xC0); // LCM Control
    LCD_WriteData(0x2C);

    LCD_WriteCmd(0xC2); // VDV and VRH Enable
    LCD_WriteData(0x01);

    LCD_WriteCmd(0xC3); // VRH Set
    LCD_WriteData(0x12);

    LCD_WriteCmd(0xC4); // VDV Set
    LCD_WriteData(0x20);

    LCD_WriteCmd(0xC6); // Frame Rate Control
    LCD_WriteData(0x0F);

    LCD_WriteCmd(0xD0); // Power Control 1
    LCD_WriteData(0xA4);
    LCD_WriteData(0xA1);

    LCD_WriteCmd(0x21); // Display Inversion ON (cho tam nen IPS)

    LCD_WriteCmd(0x29); // Display ON
    HAL_Delay(50);
}

// =============================================
// Xoa toan bo man hinh voi 1 mau
// =============================================
void LCD_ClearScreen(uint16_t color) 
{
    LCD_SetAddressWindow(0, 0, 239, 319);

    LCD_DH_H();
    LCD_CS_L();
    uint8_t c_hi = color >> 8;
    uint8_t c_lo = color & 0xFF;
    for (uint32_t i = 0; i < 240 * 320; i++) {
        LCD_SPI_Send(c_hi);
        LCD_SPI_Send(c_lo);
    }
    LCD_CS_H();
}

// =============================================
// Cac ham ve hinh hoc co ban
// =============================================
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (w == 0 || h == 0 || x >= 240 || y >= 320) return;
    if (x + w > 240) w = 240 - x;
    if (y + h > 320) h = 320 - y;

    LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    LCD_DH_H();
    LCD_CS_L();
    uint8_t c_hi = color >> 8;
    uint8_t c_lo = color & 0xFF;
    uint32_t total = (uint32_t)w * h;
    for (uint32_t i = 0; i < total; i++) {
        LCD_SPI_Send(c_hi);
        LCD_SPI_Send(c_lo);
    }
    LCD_CS_H();
}

void LCD_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
    LCD_FillRect(x, y, w, 1, color);
}

void LCD_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color)
{
    LCD_FillRect(x, y, 1, h, color);
}

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    LCD_DrawHLine(x, y, w, color);
    LCD_DrawHLine(x, y + h - 1, w, color);
    LCD_DrawVLine(x, y, h, color);
    LCD_DrawVLine(x + w - 1, y, h, color);
}

void LCD_DrawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t percent, uint16_t barColor, uint16_t bgColor, uint16_t borderColor)
{
    if (percent > 100) percent = 100;
    LCD_DrawRect(x, y, w, h, borderColor);
    uint16_t inner_w = (w > 2) ? (w - 2) : 0;
    uint16_t inner_h = (h > 2) ? (h - 2) : 0;
    uint16_t fill_w = (inner_w * percent) / 100;

    if (fill_w > 0) {
        LCD_FillRect(x + 1, y + 1, fill_w, inner_h, barColor);
    }
    if (fill_w < inner_w) {
        LCD_FillRect(x + 1 + fill_w, y + 1, inner_w - fill_w, inner_h, bgColor);
    }
}

void LCD_DrawBadge(uint16_t x, uint16_t y, const char* text, uint16_t textColor, uint16_t badgeColor)
{
    uint8_t len = strlen(text);
    uint16_t badge_w = len * 7 + 7;
    uint16_t badge_h = 14;
    LCD_FillRect(x, y, badge_w, badge_h, badgeColor);
    LCD_DrawString_Fast(x + 4, y + 1, text, textColor, badgeColor);
}

// =============================================
// Cac ham ve chu (Fast & 2X)
// =============================================
void LCD_DrawChar_Fast(uint16_t x, uint16_t y, uint8_t ch, uint16_t textColor, uint16_t bgColor)
{
    if (ch < 0x20 || ch > 0x7E) return;
    if (x + 7 > 240 || y + 12 > 320) return;
    uint8_t idx = ch - 0x20;

    uint16_t cols[6];
    for (uint8_t c = 0; c < 6; c++) {
        cols[c] = (c_chFont1206[idx][c * 2] << 8) | c_chFont1206[idx][c * 2 + 1];
    }

    uint8_t t_hi = textColor >> 8;
    uint8_t t_lo = textColor & 0xFF;
    uint8_t b_hi = bgColor >> 8;
    uint8_t b_lo = bgColor & 0xFF;

    LCD_SetAddressWindow(x, y, x + 6, y + 11);
    LCD_DH_H();
    LCD_CS_L();

    for (uint8_t row = 0; row < 12; row++) {
        uint16_t mask = 0x8000 >> row;
        for (uint8_t col = 0; col < 6; col++) {
            if (cols[col] & mask) {
                LCD_SPI_Send(t_hi);
                LCD_SPI_Send(t_lo);
            } else {
                LCD_SPI_Send(b_hi);
                LCD_SPI_Send(b_lo);
            }
        }
        // Cot dem 1 pixel giua cac chu
        LCD_SPI_Send(b_hi);
        LCD_SPI_Send(b_lo);
    }
    LCD_CS_H();
}

void LCD_DrawString_Fast(uint16_t x, uint16_t y, const char* str, uint16_t textColor, uint16_t bgColor)
{
    while (*str) {
        LCD_DrawChar_Fast(x, y, *str, textColor, bgColor);
        x += 7;
        str++;
    }
}

void LCD_DrawChar2X(uint16_t x, uint16_t y, uint8_t ch, uint16_t textColor, uint16_t bgColor)
{
    if (ch < 0x20 || ch > 0x7E) return;
    if (x + 14 > 240 || y + 24 > 320) return;
    uint8_t idx = ch - 0x20;

    uint16_t cols[6];
    for (uint8_t c = 0; c < 6; c++) {
        cols[c] = (c_chFont1206[idx][c * 2] << 8) | c_chFont1206[idx][c * 2 + 1];
    }

    uint8_t t_hi = textColor >> 8;
    uint8_t t_lo = textColor & 0xFF;
    uint8_t b_hi = bgColor >> 8;
    uint8_t b_lo = bgColor & 0xFF;

    LCD_SetAddressWindow(x, y, x + 13, y + 23);
    LCD_DH_H();
    LCD_CS_L();

    for (uint8_t row = 0; row < 12; row++) {
        uint16_t mask = 0x8000 >> row;
        for (uint8_t r_dup = 0; r_dup < 2; r_dup++) {
            for (uint8_t col = 0; col < 6; col++) {
                uint8_t is_lit = (cols[col] & mask) != 0;
                for (uint8_t c_dup = 0; c_dup < 2; c_dup++) {
                    if (is_lit) {
                        LCD_SPI_Send(t_hi);
                        LCD_SPI_Send(t_lo);
                    } else {
                        LCD_SPI_Send(b_hi);
                        LCD_SPI_Send(b_lo);
                    }
                }
            }
            // Cot dem 2 pixel
            for (uint8_t c_dup = 0; c_dup < 2; c_dup++) {
                LCD_SPI_Send(b_hi);
                LCD_SPI_Send(b_lo);
            }
        }
    }
    LCD_CS_H();
}

void LCD_DrawString2X(uint16_t x, uint16_t y, const char* str, uint16_t textColor, uint16_t bgColor)
{
    while (*str) {
        LCD_DrawChar2X(x, y, *str, textColor, bgColor);
        x += 14;
        str++;
    }
}

// Legacy API de tuong thich code cu
void LCD_DrawChar(uint16_t x, uint16_t y, uint8_t ch, uint8_t size, uint16_t color) {
    if (size == 2) {
        LCD_DrawChar2X(x, y, ch, color, BLACK);
    } else {
        LCD_DrawChar_Fast(x, y, ch, color, BLACK);
    }
}

void LCD_DrawString(uint16_t x, uint16_t y, const char* str, uint8_t size, uint16_t color) {
    if (size == 2) {
        LCD_DrawString2X(x, y, str, color, BLACK);
    } else {
        LCD_DrawString_Fast(x, y, str, color, BLACK);
    }
}

// =============================================
// Khoi tao Dashboard ECU Tinh (Static Layout)
// =============================================
void LCD_Init_Dashboard(uint8_t isHomeSimMode)
{
    // 1. Xoa toan man hinh sang mau den
    LCD_ClearScreen(COLOR_BLACK);

    // 2. Dai mau do thuong hieu Bosch (Bosch Red Strip)
    LCD_FillRect(0, 0, 240, 4, COLOR_BOSCH_RED);

    // 3. Header Banner (Thanh tieu de cong nghe)
    LCD_FillRect(0, 4, 240, 32, COLOR_HEADER_BG);
    LCD_DrawString_Fast(6, 8, "BOSCH ECU DASHBOARD", COLOR_CYAN, COLOR_HEADER_BG);
    LCD_DrawString_Fast(6, 22, "TEAM 5 | CAN & UDS", COLOR_GRAY, COLOR_HEADER_BG);
    LCD_DrawHLine(0, 36, 240, COLOR_CARD_BORDER);

    // 4. Dai che do hoat dong (Operating Mode)
    if (isHomeSimMode) {
        LCD_DrawBadge(6, 40, "MODE: CAN2 SIMULATION", COLOR_BLACK, COLOR_GOLD);
    } else {
        LCD_DrawBadge(6, 40, "MODE: EXTERNAL TESTBED", COLOR_BLACK, COLOR_GREEN);
    }
    LCD_DrawString_Fast(176, 42, "500 kbps", COLOR_GRAY, COLOR_BLACK);

    // 5. Card 1: CAN Network Telemetry (Bai 1)
    LCD_FillRect(6, 57, 228, 84, COLOR_CARD_BG);
    LCD_DrawRect(6, 57, 228, 84, COLOR_CARD_BORDER);
    LCD_DrawString_Fast(10, 61, "[1] CAN TELEMETRY (CAN2 <-> CAN1)", COLOR_CYAN, COLOR_CARD_BG);
    LCD_DrawHLine(10, 74, 220, COLOR_CARD_BORDER);
    LCD_DrawString_Fast(10, 78, "RX 0x0A2:", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 93, "TX 0x012:", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 108, "CRC-8   :", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 124, "CAN2: TX 0x0A2 -> CAN1: TX 0x012", COLOR_CYAN, COLOR_CARD_BG);

    // 6. Card 2: UDS Diagnostic Services (Bai 2)
    LCD_FillRect(6, 146, 228, 76, COLOR_CARD_BG);
    LCD_DrawRect(6, 146, 228, 76, COLOR_CARD_BORDER);
    LCD_DrawString_Fast(10, 150, "[2] UDS ISO-14229", COLOR_GOLD, COLOR_CARD_BG);
    LCD_DrawHLine(10, 163, 220, COLOR_CARD_BORDER);
    LCD_DrawString_Fast(10, 167, "Security 0x27:", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 184, "Active CAN ID:", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 200, "Pending CANID:", COLOR_GRAY, COLOR_CARD_BG);

    // 7. Card 3: Sensors & System Health (DID 0124)
    LCD_FillRect(6, 227, 228, 68, COLOR_CARD_BG);
    LCD_DrawRect(6, 227, 228, 68, COLOR_CARD_BORDER);
    LCD_DrawString_Fast(10, 231, "[3] SENSOR & HEALTH (0124)", COLOR_GREEN, COLOR_CARD_BG);
    LCD_DrawHLine(10, 244, 220, COLOR_CARD_BORDER);
    LCD_DrawString_Fast(10, 249, "MCU Temp:", COLOR_GRAY, COLOR_CARD_BG);
    LCD_DrawString_Fast(10, 280, "0 C           40 C          80 C", COLOR_GRAY, COLOR_CARD_BG);

    // 8. Footer Bar
    LCD_FillRect(0, 302, 240, 18, COLOR_FOOTER_BG);
    LCD_DrawHLine(0, 301, 240, COLOR_CARD_BORDER);
    LCD_DrawString_Fast(8, 305, "STM32F405 | HUST-BOSCH | PA0:IGN", COLOR_WHITE, COLOR_FOOTER_BG);
}

// =============================================
// Cap nhat du lieu dong moi chu ky (Zero-Flicker)
// =============================================
void LCD_Update_Dashboard(uint32_t uptimeSec,
                          const uint8_t* rxData,
                          uint16_t txId,
                          const uint8_t* txData,
                          uint8_t crcVal,
                          uint8_t realTemp,
                          uint8_t isSecurityUnlocked,
                          uint16_t pendingCanId,
                          uint8_t isPendingReady)
{
    char buf[32];
    static uint8_t heartBeat = 0;
    heartBeat ^= 1;

    // --- Header Updates ---
    uint32_t min = uptimeSec / 60;
    uint32_t sec = uptimeSec % 60;
    sprintf(buf, "%02lu:%02lu", min, sec);
    LCD_DrawString_Fast(168, 8, buf, COLOR_WHITE, COLOR_HEADER_BG);
    LCD_DrawChar_Fast(216, 8, heartBeat ? '*' : ' ', COLOR_GREEN, COLOR_HEADER_BG);

    // --- Card 1: CAN Telemetry ---
    // RX Payload
    sprintf(buf, "%02X %02X     ", rxData[0], rxData[1]);
    LCD_DrawString_Fast(84, 78, buf, COLOR_WHITE, COLOR_CARD_BG);
    LCD_DrawString_Fast(165, 78, "[BUS OK]", COLOR_GREEN, COLOR_CARD_BG);

    // TX Label & Payload
    if (txId > 0x7FF) {
        sprintf(buf, "TX 0x%04X:", txId);
    } else {
        sprintf(buf, "TX 0x%03X :", txId);
    }
    LCD_DrawString_Fast(10, 93, buf, COLOR_GRAY, COLOR_CARD_BG);
    sprintf(buf, "%02X %02X %02X  ", txData[0], txData[1], txData[2]);
    LCD_DrawString_Fast(84, 93, buf, COLOR_WHITE, COLOR_CARD_BG);
    LCD_DrawString_Fast(165, 93, "[BUS OK]", COLOR_GREEN, COLOR_CARD_BG);

    // CRC-8 SAE J1850
    sprintf(buf, "0x%02X     ", crcVal);
    LCD_DrawString_Fast(84, 108, buf, COLOR_WHITE, COLOR_CARD_BG);
    LCD_DrawString_Fast(165, 108, "[VALID] ", COLOR_GREEN, COLOR_CARD_BG);

    // --- Card 2: UDS Diagnostics ---
    // Security 0x27
    if (isSecurityUnlocked) {
        LCD_DrawString_Fast(112, 167, "[ UNLOCKED ]    ", COLOR_GREEN, COLOR_CARD_BG);
    } else {
        LCD_DrawString_Fast(112, 167, "[  LOCKED  ]    ", COLOR_RED, COLOR_CARD_BG);
    }

    // Active CAN ID
    if (txId > 0x7FF) {
        sprintf(buf, "0x%04X [ACTIVE] ", txId);
    } else {
        sprintf(buf, "0x%03X  [ACTIVE] ", txId);
    }
    LCD_DrawString_Fast(112, 184, buf, COLOR_WHITE, COLOR_CARD_BG);

    // Pending CAN ID (0x2E)
    if (isPendingReady) {
        if (pendingCanId > 0x7FF) {
            sprintf(buf, "0x%04X [IGN REQ]", pendingCanId);
        } else {
            sprintf(buf, "0x%03X  [IGN REQ]", pendingCanId);
        }
        LCD_DrawString_Fast(112, 200, buf, COLOR_CYAN, COLOR_CARD_BG);
    } else {
        LCD_DrawString_Fast(112, 200, "NONE   [SYNCED] ", COLOR_GRAY, COLOR_CARD_BG);
    }

    // --- Card 3: Sensor & Health ---
    // Temperature 2X
    sprintf(buf, "%2d C", realTemp);
    LCD_DrawString2X(80, 246, buf, COLOR_WHITE, COLOR_CARD_BG);

    if (realTemp < 45) {
        LCD_DrawString_Fast(165, 249, "[NORMAL]", COLOR_GREEN, COLOR_CARD_BG);
    } else {
        LCD_DrawString_Fast(165, 249, "[ WARM ]", COLOR_GOLD, COLOR_CARD_BG);
    }

    // Progress Bar (0..80 C)
    uint8_t pct = (realTemp > 80) ? 100 : (uint8_t)(((uint16_t)realTemp * 100) / 80);
    uint16_t barCol = (realTemp < 35) ? COLOR_CYAN : ((realTemp < 50) ? COLOR_GREEN : COLOR_RED);
    LCD_DrawProgressBar(12, 268, 216, 8, pct, barCol, COLOR_BLACK, COLOR_CARD_BORDER);
}




