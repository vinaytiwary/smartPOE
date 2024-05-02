/*
 * _common.h
 *
 *  Created on: Apr 20, 2024
 *      Author: ADMIN
 */

#ifndef SOURCES__COMMON_H_
#define SOURCES__COMMON_H_

#define HIGH    1
#define LOW     0
#define TRUE    1
#define FALSE   0

#define RETRY_COUNT          (100)

char getChecksum(unsigned char* buff, char len);

void my_sprintf(char *recv_buff, int no_of_data, ...);

int intToStr(int x, char str[], int d);

void reverse(char *str, int len);

void delete_SubStr(char *Str, char *SubStr, char end);

void generateRandomKey(int length, char *key);

unsigned int count_doubleSquareBracket(char *arr, unsigned int length);

unsigned int count_comma(char *arr, uint16_t max_len);

#endif /* SOURCES__COMMON_H_ */
