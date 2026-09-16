#ifndef LCD_HAL_H
#define LCD_HAL_H

#include "main.h"

// Chan dieu khien LCD ST7789
// PB8 = DC (RS), PB7 = CS, PB2 = RST, PB3 = SCK, PA7 = MOSI
#define LCD_DH_H()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)
#define LCD_DH_L()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)

#define LCD_CS_H()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)
#define LCD_CS_L()    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)

#define LCD_RST_H()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET)
#define LCD_RST_L()   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET)

// Bang mau RGB565 chuan Automotive
#define WHITE             0xFFFF
#define BLACK             0x0000
#define RED               0xF800
#define GREEN             0x07E0
#define BLUE              0x001F
#define YELLOW            0xFFE0

#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_GOLD        0xFDE0
#define COLOR_ORANGE      0xFD20
#define COLOR_GRAY        0x7BEF
#define COLOR_DARKGRAY    0x39E7
#define COLOR_BOSCH_RED   0xE000
#define COLOR_HEADER_BG   0x0862
#define COLOR_CARD_BG     0x0842
#define COLOR_CARD_BORDER 0x2187
#define COLOR_FOOTER_BG   0x0862

// Ham dieu khien phan cung LCD
void LCD_GPIO_Init(void);
void LCD_SPI_Send(uint8_t data);
void LCD_WriteCmd(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_WriteData16(uint16_t data);
void LCD_Init(void);
void LCD_ClearScreen(uint16_t color);
void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void LCD_SetCursor(uint16_t x, uint16_t y);

// Ham ve co ban (Legacy & Fast)
void LCD_DrawChar(uint16_t x, uint16_t y, uint8_t ch, uint8_t size, uint16_t color);
void LCD_DrawString(uint16_t x, uint16_t y, const char* str, uint8_t size, uint16_t color);
void LCD_DrawChar_Fast(uint16_t x, uint16_t y, uint8_t ch, uint16_t textColor, uint16_t bgColor);
void LCD_DrawString_Fast(uint16_t x, uint16_t y, const char* str, uint16_t textColor, uint16_t bgColor);
void LCD_DrawChar2X(uint16_t x, uint16_t y, uint8_t ch, uint16_t textColor, uint16_t bgColor);
void LCD_DrawString2X(uint16_t x, uint16_t y, const char* str, uint16_t textColor, uint16_t bgColor);

// Ham do hoa hinh hoc
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color);
void LCD_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color);
void LCD_DrawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t percent, uint16_t barColor, uint16_t bgColor, uint16_t borderColor);
void LCD_DrawBadge(uint16_t x, uint16_t y, const char* text, uint16_t textColor, uint16_t badgeColor);

// Ham Dashboard ECU chuyen nghiep
void LCD_Init_Dashboard(uint8_t isHomeSimMode);
void LCD_Update_Dashboard(uint32_t uptimeSec,
                          const uint8_t* rxData,
                          uint16_t txId,
                          const uint8_t* txData,
                          uint8_t crcVal,
                          uint8_t realTemp,
                          uint8_t isSecurityUnlocked,
                          uint16_t pendingCanId,
                          uint8_t isPendingReady);

#endif // LCD_HAL_H
