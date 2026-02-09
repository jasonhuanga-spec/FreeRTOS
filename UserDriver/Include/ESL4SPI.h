#ifndef ESL4SPI_H
#define ESL4SPI_H


#include "main.h"
#include "gpio.h"


//Configuration GPIO
#define SCL_GPIO_Pin_X  	        GPIO_PIN_11
#define	SCL_GPIOX					GPIOC

#define	DC_GPIO_Pin_X			    GPIO_PIN_2
#define	DC_GPIOX					GPIOD

#define	CSB_GPIO_Pin_X			    GPIO_PIN_12
#define	CSB_GPIOX					GPIOC

#define	SDA_GPIO_Pin_X		        GPIO_PIN_10
#define	SDA_GPIOX					GPIOC


//Funcation params
#define SDA_OUT 					1
#define SDA_IN  					0

#define	SCL_High					1
#define	SCL_Low						0

#define	CSB_High					1
#define	CSB_Low						0

#define	DC_High						1
#define	DC_Low						0

#define	SDA_High					1
#define	SDA_Low						0




void SDA_GPIO_Configuration(uint8_t mode);
void SCL_GPIO_SetLevel(uint8_t level);
void CS_GPIO_SetLevel(uint8_t level);
void DC_GPIO_SetLevel(uint8_t level);
void SDA_GPIO_SetLevel(uint8_t level);
void SPI4_Write_Byte(uint8_t value);
void ESL4SPI_WriteCMD(uint8_t command);
void ESL4SPI_WriteDATA(uint8_t data);
unsigned char ESL4SPI_ReadDATA(void);


#endif

