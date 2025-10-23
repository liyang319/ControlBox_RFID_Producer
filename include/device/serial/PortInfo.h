#pragma once

//串口结构
typedef struct
{
	char	prompt;				//prompt after reciving data
	int 	baudrate;			//baudrate
	char	databit;			//data bits, 5, 6, 7, 8
	char 	debug;				//debug mode, 0: none, 1: debug
	char 	echo;				//echo mode, 0: none, 1: echo
	char	fctl;				//flow control, 0: none, 1: hardware, 2: software
	char	parity;				//parity 0: none, 1: odd, 2: even
	char	stopbit;			//stop bits, 1, 2
	const int reserved;	        //reserved, must be zero
} portinfo_t;

typedef portinfo_t *pportinfo_t;

#define DEFAULT_BAUDRATE    	9600
#define DEFAULT_STOPBIT     	1
#define DEFAULT_DATABIT     	8
#define DEFAULT_PARITY      	0

#define RELAY_INPUT_REGISTER_ADDR	1000	//继电器输入寄存器地址
