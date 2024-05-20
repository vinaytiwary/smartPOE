/*
 * _common.cpp
 *
 *  Created on: Apr 20, 2024
 *      Author: ADMIN
 */

#include "stdint.h"
#include "stdbool.h"
#include <string.h>
#include "math.h"
#include "time.h"
#include <stdarg.h>
#include <stdlib.h>

#include "gprs.h"
#include "_debug.h"
#include "_common.h"
#include "UartCore.h"

char getChecksum(unsigned char* buff, char len)
{
    char chksum=0;
    int i=0;

    for(i=0 ; i<len ; i++)
    {
        chksum+=buff[i];
    }

    chksum = (~chksum) + 1;

    return chksum;
}

void my_sprintf(char *recv_buff,int no_of_data, ...)
{
    char temp_buff[10], *nextString, *arg_idn, nextChar;
    int i,j,k,value,no_of_digit;
    va_list argList;
    va_start (argList, no_of_data);
    arg_idn = va_arg (argList, char*);
#ifdef DEBUG_SPRINTF
    vUART_SendStr(UART_PC,"\nsp_tx:");
    vUART_SendStr(UART_PC,(const uint8_t *)arg_idn);
#endif
    for(i = 0, j = 0; i < strlen(arg_idn);)
    {
        if(arg_idn[i] == '%')
        {

            i++;
            if(arg_idn[i] == 's')
            {
                nextString = va_arg (argList, char*);

                memcpy(&recv_buff[j],nextString,strlen(nextString));
                j += strlen(nextString);
                //i++;
            }
            else if(arg_idn[i] == 'c')
            {
                nextChar = va_arg (argList, char);
                recv_buff[j] = nextChar;
                j++;

            }
            else if(arg_idn[i] == '%')
            {
                //nextChar = va_arg (argList, char);
                recv_buff[j] = '%';
                j++;

            }
            else
            {
                memset(temp_buff,0,sizeof(temp_buff));
                k = 0;
                //i++;
                while(arg_idn[i] != 'd')
                {
                    temp_buff[k++] = arg_idn[i];
                    i++;
                }
                no_of_digit = atoi(temp_buff);
                value = va_arg (argList, int);
                memset(temp_buff,0,sizeof(temp_buff));
                intToStr(value,temp_buff,no_of_digit);
                memcpy(&recv_buff[j],temp_buff,strlen(temp_buff));
                j += strlen(temp_buff);
            }
        }
        else
        {
            recv_buff[j++] = arg_idn[i];
        }
        i++;
    }
    va_end (argList);
#ifdef DEBUG_SPRINTF
    vUART_SendStr(UART_PC," , ");
    vUART_SendStr(UART_PC,(const uint8_t *)recv_buff);
#endif
}

int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
    str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}
void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}

//PP on 26-04-24: I'm skeptical about this as ther's no protection condition incase the forloop becomes an infinite one.
void delete_SubStr(char* Str, char* SubStr, char end)
{
    char *start = 0;
    uint16_t i = 0;


    start = strstr(Str, SubStr);

//#ifdef DEBUG_RANDOM_
//        vUART_SendStr(UART_PC,"\nstart:");
//        vUART_SendStr(UART_PC,(const uint8_t *)start);
//#endif
    if(start != NULL)
    {
        if (end)
        {
            for(; start[i++] != end;);
            memcpy(start, start+i,strlen(&start[i]));
            //Str[strlen(Str)-i] = '\0';
            memset(&Str[strlen(Str)-i], 0, i);
        }
        else
        {
            memcpy(start, &start[strlen(SubStr)],strlen(&start[strlen(SubStr)]));
            memset(&Str[strlen(Str)-strlen(SubStr)], 0, strlen(SubStr));
        }
    }
}

void generateRandomKey(int length, char* key)
{
    srand((unsigned int)time(NULL));
    int i=0;
    //char* key = (char*)malloc((length + 1) * sizeof(char));
    //char key[length+1];
    memset(key,0,sizeof(key));
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int charsetLength = sizeof(charset) - 1;

    if (key)
    {
        for (i = 0; i < length; ++i)
        {
            key[i] = charset[rand() % charsetLength];
        }
        key[length] = '\0'; // Null-terminate the string
    }
}

unsigned int count_doubleSquareBracket(char *arr,unsigned int length)
{
    unsigned int i, count = 0;
    static int timeout=0;
    //len = strlen(arr);
    for (i=0; i <= length; i++)
    {
        if(arr[i] == ']')
        {
            count++;
        }
    }
    if(count == 1)
    {
        if(timeout++ >= RETRY_COUNT)
        {
            timeout = 0;
        }
    }
    else
    {

    }
    return count;
}

unsigned int count_comma (char *arr, uint16_t max_len)
{
  	unsigned int i, cnt = 0;
  	for (i = 0; i <= strlen (arr); i++)
	{
		if((i >= max_len) /*&& !cnt*/)
	    {
	    	break;
	    }
	    
	    if (arr[i] == ',')
		{
		  cnt++;
		}
    }

  	return cnt;
}

char *my_strstr(const char *haystack, const char *needle, int haystack_len)
{
    size_t  needle_len;

    needle_len = strlen(needle);
    //while (*haystack)
    while(haystack_len--)
    {
        if (*haystack == *needle)
        {
            if (!strncmp(haystack, needle, needle_len))
                    return ((char *)haystack);
        }
        haystack++;
    }
    return (NULL);
}

