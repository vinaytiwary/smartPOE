/*
 * Display.h
 *
 *  Created on: Apr 24, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES_DISPLAY_H_
#define SOURCES_DISPLAY_H_

#include "_config.h"

typedef enum 
{
  home,
  config,
  router1,
  router2
}screen_state_t;

typedef struct 
{
  char config_mode;
  char config_option;
  char config_option_r1;
  char config_option_r2;
}sys_config_t;


unsigned char getChecksum_(unsigned char* buff, char len);

void PrepareLcdBuffer(unsigned char*, char,char);

void data_screen();

void get_random_data();

void vinay(void);

void clear_lcd(void);

void Data_Screen_lcd(void);

void Display_CFGScreen(void);

void Display_BootUpScreen(void);

void AlarmDisplay(void);

#endif /* SOURCES_DISPLAY_H_ */
