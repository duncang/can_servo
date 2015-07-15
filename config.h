/* config.h for can_servo.c
 * running on AT90CAN128
 * Duncan Greer 13 Jan 2012
 */


#ifndef _CONFIG_H
#define _CONFIG_H

#include <avr/io.h>

#include "compiler.h"


#define FOSC 16000UL
#define F_CPU (FOSC*1000)

#define USE_UART      UART_0
#define UART_BAUDRATE 38400


#define CAN_BAUDRATE   100 


#endif
