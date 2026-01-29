#include "ESL4SPI.h"



/**
 * @brief IIC延时
 * @param  无
 * @return 无
 */
static void SPI4_Delay_us(uint32_t us)
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
void SDA_GPIO_Configuration(uint8_t mode)
{
	GPIO_InitTypeDef  GPIO_InitStructure_ESL4SPI;

	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	if(mode == SDA_OUT)
	{
		GPIO_InitStructure_ESL4SPI.Pin = SDA_GPIO_Pin_X;		
		GPIO_InitStructure_ESL4SPI.Mode = GPIO_MODE_OUTPUT_PP; 
		GPIO_InitStructure_ESL4SPI.Pull = GPIO_PULLUP;		 			
		GPIO_InitStructure_ESL4SPI.Speed = GPIO_SPEED_FREQ_HIGH;		 		
		HAL_GPIO_Init(SDA_GPIOX, &GPIO_InitStructure_ESL4SPI);	
	}
	if(mode == SDA_IN)
	{
		GPIO_InitStructure_ESL4SPI.Pin  = SDA_GPIO_Pin_X;
		GPIO_InitStructure_ESL4SPI.Mode = GPIO_MODE_INPUT;
  		GPIO_InitStructure_ESL4SPI.Pull = GPIO_NOPULL;
		GPIO_InitStructure_ESL4SPI.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(SDA_GPIOX, &GPIO_InitStructure_ESL4SPI);				
	}
}


/**
 * @brief          SCL_GPIO_SetLevel
 * @param      	   level: SCL_High or SCL_Low
 * @return         null
 * @note           null
 */
void SCL_GPIO_SetLevel(uint8_t level)
{
    level ? HAL_GPIO_WritePin(SCL_GPIOX, SCL_GPIO_Pin_X, GPIO_PIN_SET) : HAL_GPIO_WritePin(SCL_GPIOX, SCL_GPIO_Pin_X, GPIO_PIN_RESET);
}

/**
 * @brief          CSB_GPIO_SetLevel
 * @param      	   level: CS_High or CS_Low
 * @return         null
 * @note           null
 */
void CS_GPIO_SetLevel(uint8_t level)
{
	level ? HAL_GPIO_WritePin(CSB_GPIOX, CSB_GPIO_Pin_X, GPIO_PIN_SET) : HAL_GPIO_WritePin(CSB_GPIOX, CSB_GPIO_Pin_X, GPIO_PIN_RESET);
}


/**
 * @brief          DC_GPIO_SetLevel
 * @param      	   level: DC_High or DC_Low
 * @return         null
 * @note           null
 */
void DC_GPIO_SetLevel(uint8_t level)
{
	level ? HAL_GPIO_WritePin(DC_GPIOX, DC_GPIO_Pin_X, GPIO_PIN_SET) : HAL_GPIO_WritePin(DC_GPIOX, DC_GPIO_Pin_X, GPIO_PIN_RESET);
}

/**
 * @brief          SDA_GPIO_SetLevel
 * @param      	   level: SDA_High or SDA_Low
 * @return         null
 * @note           null
 */
void SDA_GPIO_SetLevel(uint8_t level)
{
	level ? HAL_GPIO_WritePin(SDA_GPIOX, SDA_GPIO_Pin_X, GPIO_PIN_SET) : HAL_GPIO_WritePin(SDA_GPIOX, SDA_GPIO_Pin_X, GPIO_PIN_RESET);
}


/**
 * @brief          SPI4_Write_Byte
 * @param      	   value: This is command
 * @return         null
 * @note           null
 */
void SPI4_Write_Byte(uint8_t value)                                    
{
  	uint8_t i;	
	SPI4_Delay_us(1);
	for(i=0; i<8; i++)   
	{
		SCL_GPIO_SetLevel(SCL_Low);
		SPI4_Delay_us(1);
		
		if(value & 0x80)		
			SDA_GPIO_SetLevel(SDA_High);
		else
			SDA_GPIO_SetLevel(SDA_Low);	
		
		value = (value << 1); 
		SPI4_Delay_us(1);
		
		SCL_GPIO_SetLevel(SCL_High); 
		SPI4_Delay_us(1);
	}

	SCL_GPIO_SetLevel(SCL_Low);
}


/**
 * @brief          SPI4_WriteCMD
 * @param      	   command
 * @return         null
 * @note           null
 */
void ESL4SPI_WriteCMD(uint8_t command)
{
	SDA_GPIO_Configuration(SDA_OUT);			//Config SDA output
	SPI4_Delay_us(1);
	
	CS_GPIO_SetLevel(CSB_Low);                   
	DC_GPIO_SetLevel(DC_Low);					// command write
	SPI4_Delay_us(1) ;
	
	SPI4_Write_Byte(command);
	SPI4_Delay_us(1) ;
	
	CS_GPIO_SetLevel(CSB_High);
}

/**
 * @brief          SPI4_WriteDATA
 * @param      	   data
 * @return         null
 * @note           null
 */
void ESL4SPI_WriteDATA(uint8_t data)
{
	SDA_GPIO_Configuration(SDA_OUT);
  	SPI4_Delay_us(1);
	
  	CS_GPIO_SetLevel(CSB_Low);                   
	DC_GPIO_SetLevel(DC_High);
	SPI4_Delay_us(1) ;
	
	SPI4_Write_Byte(data);
	SPI4_Delay_us(1);
	
	CS_GPIO_SetLevel(CSB_High);
}

/**
 * @brief          SPI4_ReadDATA
 * @param      	   null
 * @return         Read_Data
 * @note           null
 */
unsigned char ESL4SPI_ReadDATA(void)
{
	uint8_t scnt,Read_Data;
	Read_Data=0;
	
	SDA_GPIO_Configuration(SDA_IN);     
	CS_GPIO_SetLevel(CSB_Low);                   
	DC_GPIO_SetLevel(DC_High);
	SPI4_Delay_us(5);
	
	for(scnt=0;scnt<8;scnt++)
	{
		SCL_GPIO_SetLevel(SCL_Low); 
		SPI4_Delay_us(5);
		
		if((HAL_GPIO_ReadPin(SDA_GPIOX,SDA_GPIO_Pin_X)) == 1)
		Read_Data=(Read_Data<<1)|0x01;
		else
		Read_Data=Read_Data<<1;	
		
		SCL_GPIO_SetLevel(SCL_High); 	  
		SCL_GPIO_SetLevel(SCL_Low);  
	}
	return Read_Data;
}	


