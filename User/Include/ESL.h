#ifndef ESL_H
#define ESL_H


#include "main.h"
#include "gpio.h"
#include "ESL4SPI.h"


//ESL IC用到的reset和busy信号，进行配置GPIO
#define	RESET_GPIO_Pin_X	        GPIO_PIN_3
#define	RESET_GPIOX				    GPIOB

#define	BUSY_GPIO_Pin_X		        GPIO_PIN_4
#define	BUSY_GPIOX				    GPIOB



void ESL_RESET(void);
void Check_Busy(void);
void ESLtest(void);
void E4_Read_ID(void);

#endif          