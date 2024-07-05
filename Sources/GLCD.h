
#ifndef GRAPHIC_LCD_KS0108_H_
#define GRAPHIC_LCD_KS0108_H_

#include "_config.h"

#ifdef ENABLE_GLCD

#define KS0108_SCREEN_WIDTH     128
#define KS0108_SCREEN_HEIGHT    64

#define DISPLAY_SET_Y           0x40
#define DISPLAY_SET_X           0xB8
#define DISPLAY_START_LINE      0xC0
#define DISPLAY_ON_CMD          0x3E

#define NETWORK_STS_WIDTH       0X0A
#define BATTERY_STS_WIDTH       0X0D
#define ON                      1
#define OFF                     0

#define BATTERY_MIN_RANGE       10800
#define BATTERY_MAX_RANGE       12000
#define BATTERY_BASE_PERCENTAGE 20
#define DISPLAY_STATUS_BUSY     0x80

typedef struct
{
  char config_mode;
  char config_option;
  char config_option_r1;
  char config_option_r2;
}sys_config_t;

void GLCD_InitalizePorts(void);
void GLCD_EnableController(unsigned char controller);
void GLCD_DisableController(unsigned char controller);
void GLCD_WriteCommand(unsigned char commandToWrite, unsigned char controller);
void GLCD_WriteData(unsigned char dataToWrite);
void GLCD_Initalize(void);
void GLCD_GoTo(unsigned char x, unsigned char y);
void GLCD_ClearScreen(void);
void GLCD_WriteChar(char charToWrite);
void GLCD_WriteString(char *stringToWrite);
void GLCD_Clear_Line(unsigned char line_num);
void GLCD_PrintBatterySts();
void GLCD_PrintRightIcon();
void GLCD_PrintNetworkCnt();
void GLCD_PrintNetworkDscnt();
void Data_Screen_lcd();
void AlarmDisplay();
void updateGlcd();
void Display_BootUPscreen(void);
void print_dash();
void Display_CFGScreen(void);
void GLCD_PrintButtomBoarder();
void GLCD_PrintTopBoarder();
void AlarmDisplayTest();
#endif //ENABLE_GLCD

#endif /* GRAPHIC_LCD_KS0108_H_ */
