#include "LCD4SPI.h"



/**
 * @brief IIC延时
 * @param  无
 * @return 无
 */
static void LCD4SPI_Delay_us(uint32_t us)
{
    // 72MHz主频下，单条空指令约1/72μs，循环体包含减1和判断，约消耗8个指令周期
    // 计算循环次数：us * (72MHz / 1000000) / 8 ≈ us * 9
    uint32_t i = us * 9; 
    while (i--)
    {
       __NOP() ; // 空指令，减少编译器优化对延时的影响
    }
}


/**
 * @brief          Configuration GPIO
 * @param      	   mode: SDA_Out or SDA_In
 * @return         null
 * @note           This function configures whether the SDA GPIO is input or output
 */
void LCDSDA_GPIO_Configuration(uint8_t mode)
{
	GPIO_InitTypeDef  GPIO_InitStructure_LCD4SPI;

	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	if(mode == LCDSDA_OUT)
	{
		GPIO_InitStructure_LCD4SPI.Pin = LCDSDA_GPIO_Pin_X;		
		GPIO_InitStructure_LCD4SPI.Mode = GPIO_MODE_OUTPUT_PP; 
		GPIO_InitStructure_LCD4SPI.Pull = GPIO_PULLUP;		 			
		GPIO_InitStructure_LCD4SPI.Speed = GPIO_SPEED_FREQ_HIGH;		 		
		HAL_GPIO_Init(LCDSDA_GPIOX, &GPIO_InitStructure_LCD4SPI);	
	}
	if(mode == LCDSDA_IN)
	{
		GPIO_InitStructure_LCD4SPI.Pin  = LCDSDA_GPIO_Pin_X;
		GPIO_InitStructure_LCD4SPI.Mode = GPIO_MODE_INPUT;
  		GPIO_InitStructure_LCD4SPI.Pull = GPIO_NOPULL;
		GPIO_InitStructure_LCD4SPI.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(LCDSDA_GPIOX, &GPIO_InitStructure_LCD4SPI);				
	}
}


/**
 * @brief          SCL_GPIO_SetLevel
 * @param      	   level: SCL_High or SCL_Low
 * @return         null
 * @note           null
 */
void LCDSCL_GPIO_SetLevel(uint8_t level)
{
    level ? HAL_GPIO_WritePin(LCDSCL_GPIOX, LCDSCL_GPIO_Pin_X, GPIO_PIN_SET) : HAL_GPIO_WritePin(LCDSCL_GPIOX, LCDSCL_GPIO_Pin_X, GPIO_PIN_RESET);
}

/**
 * @brief          CSB_GPIO_SetLevel
 * @param      	   level: CS_High or CS_Low
 * @return         null
 * @note           null
 */
void LCDCS_GPIO_SetLevel(uint8_t level)
{
	level ? HAL_GPIO_WritePin(LCDCS_GPIOX, LCDCS_GPIO_Pin_X, GPIO_PIN_SET) : HAL_GPIO_WritePin(LCDCS_GPIOX, LCDCS_GPIO_Pin_X, GPIO_PIN_RESET);
}


/**
 * @brief          DC_GPIO_SetLevel
 * @param      	   level: DC_High or DC_Low
 * @return         null
 * @note           null
 */
void LCDRS_GPIO_SetLevel(uint8_t level)
{
	level ? HAL_GPIO_WritePin(LCDRS_GPIOX, LCDRS_GPIO_Pin_X, GPIO_PIN_SET) : HAL_GPIO_WritePin(LCDRS_GPIOX, LCDRS_GPIO_Pin_X, GPIO_PIN_RESET);
}

/**
 * @brief          SDA_GPIO_SetLevel
 * @param      	   level: SDA_High or SDA_Low
 * @return         null
 * @note           null
 */
void LCDSDA_GPIO_SetLevel(uint8_t level)
{
	level ? HAL_GPIO_WritePin(LCDSDA_GPIOX, LCDSDA_GPIO_Pin_X, GPIO_PIN_SET) : HAL_GPIO_WritePin(LCDSDA_GPIOX, LCDSDA_GPIO_Pin_X, GPIO_PIN_RESET);
}


/**
 * @brief          SPI4_Write_Byte
 * @param      	   value: This is command
 * @return         null
 * @note           null
 */
void LCD4SPI_Write_Byte(uint8_t value)                                    
{
  	uint8_t i;	
	LCD4SPI_Delay_us(1);
	for(i=0; i<8; i++)   
	{
		LCDSCL_GPIO_SetLevel(LCDSCL_Low);
		LCD4SPI_Delay_us(1);
		
		if(value & 0x80)		
			LCDSDA_GPIO_SetLevel(LCDSDA_High);
		else
			LCDSDA_GPIO_SetLevel(LCDSDA_Low);	
		
		value = (value << 1); 
		LCD4SPI_Delay_us(1);
		
		LCDSCL_GPIO_SetLevel(LCDSCL_High); 
		LCD4SPI_Delay_us(1);
	}

	LCDSCL_GPIO_SetLevel(LCDSCL_Low);
}


/**
 * @brief          SPI4_WriteCMD
 * @param      	   command
 * @return         null
 * @note           null
 */
void LCD4SPI_WriteCMD(uint8_t command)
{
	LCDSDA_GPIO_Configuration(LCDSDA_OUT);			//Config SDA output
	LCD4SPI_Delay_us(1);
	
	LCDCS_GPIO_SetLevel(LCDCS_Low);                   
	LCDRS_GPIO_SetLevel(LCDRS_Low);					// command write
	LCD4SPI_Delay_us(1);
	
	LCD4SPI_Write_Byte(command);
	LCD4SPI_Delay_us(1);
	
	LCDCS_GPIO_SetLevel(LCDCS_High);
}

/**
 * @brief          SPI4_WriteDATA
 * @param      	   data
 * @return         null
 * @note           null
 */
void LCD4SPI_WriteDATA(uint8_t data)
{
	LCDSDA_GPIO_Configuration(LCDSDA_OUT);
  	LCD4SPI_Delay_us(1);
	
  	LCDCS_GPIO_SetLevel(LCDCS_Low);                   
	LCDRS_GPIO_SetLevel(LCDRS_High);
	LCD4SPI_Delay_us(1) ;
	
	LCD4SPI_Write_Byte(data);
	LCD4SPI_Delay_us(1);
	
	LCDCS_GPIO_SetLevel(LCDCS_High);
}

/**
 * @brief          SPI4_ReadDATA
 * @param      	   null
 * @return         Read_Data
 * @note           null
 */
unsigned char LCD4SPI_ReadDATA(void)
{
	uint8_t scnt,Read_Data;
	Read_Data=0;
	
	LCDSDA_GPIO_Configuration(LCDSDA_IN);     
	LCDCS_GPIO_SetLevel(LCDCS_Low);                   
	LCDRS_GPIO_SetLevel(LCDRS_High);
	LCD4SPI_Delay_us(5);
	
	for(scnt=0;scnt<8;scnt++)
	{
		LCDSCL_GPIO_SetLevel(LCDSCL_Low); 
		LCD4SPI_Delay_us(5);
		
		if((HAL_GPIO_ReadPin(LCDSDA_GPIOX,LCDSDA_GPIO_Pin_X)) == 1)
		Read_Data=(Read_Data<<1)|0x01;
		else
		Read_Data=Read_Data<<1;	
		
		LCDSCL_GPIO_SetLevel(LCDSCL_High); 	  
		LCDSCL_GPIO_SetLevel(LCDSCL_Low);  
	}
	return Read_Data;
}	


