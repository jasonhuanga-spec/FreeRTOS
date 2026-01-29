#ifndef LCD4SPI_H
#define LCD4SPI_H


#include "main.h"
#include "gpio.h"


//Configuration GPIO
#define LCDSCL_GPIO_Pin_X  	            GPIO_PIN_7
#define	LCDSCL_GPIOX					GPIOC

#define	LCDRS_GPIO_Pin_X			    GPIO_PIN_9
#define	LCDRS_GPIOX					    GPIOC

#define	LCDCS_GPIO_Pin_X			    GPIO_PIN_8
#define	LCDCS_GPIOX					    GPIOA

#define	LCDSDA_GPIO_Pin_X		        GPIO_PIN_8
#define	LCDSDA_GPIOX					GPIOC


//Funcation params
#define LCDSDA_OUT 					1
#define LCDSDA_IN  					0

#define	LCDSCL_High					1
#define	LCDSCL_Low					0

#define	LCDCS_High					1
#define	LCDCS_Low					0

#define	LCDRS_High					1
#define	LCDRS_Low					0

#define	LCDSDA_High					1
#define	LCDSDA_Low					0




void LCDSDA_GPIO_Configuration(uint8_t mode);
void LCDSCL_GPIO_SetLevel(uint8_t level);
void LCDCS_GPIO_SetLevel(uint8_t level);
void LCDRS_GPIO_SetLevel(uint8_t level);
void LCDSDA_GPIO_SetLevel(uint8_t level);
void LCD4SPI_Write_Byte(uint8_t value);
void LCD4SPI_WriteCMD(uint8_t command);
void LCD4SPI_WriteDATA(uint8_t data);
unsigned char LCD4SPI_ReadDATA(void);


#endif

