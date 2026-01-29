/*

硬件电路配置文件

*/
#include "HWCI.h"


/**
 * @函数名称       控制ESL的VDD开关
 * @说明           VDDStates:传入需要的VDD状态
 */
void VDDSwitch(uint8_t VDDStates)
{
    //打开VDD
    if (VDDStates == VDDON) HAL_GPIO_WritePin(VDDSwitchGPIOX, VDDSwitchPINX, GPIO_PIN_RESET);
    //关闭VDD
    if (VDDStates == VDDOFF) HAL_GPIO_WritePin(VDDSwitchGPIOX, VDDSwitchPINX, GPIO_PIN_SET);
}

/**
 * @函数名称       电平转换芯片的使能
 * @说明           LVSwitch:传入需要的LV状态
 */
void LVSwitch(uint8_t LVSwitch)
{
    //打开LV
    if (LVSwitch == LVON) HAL_GPIO_WritePin(LVSwitchGPIOX, LVSwitchPINX, GPIO_PIN_SET);
    //关闭LV
    if (LVSwitch == LVOFF) HAL_GPIO_WritePin(LVSwitchGPIOX, LVSwitchPINX, GPIO_PIN_RESET);
}

/**
 * @函数名称       选择电感
 * @说明           InductorParameter:电感参数
 */
void SelectInductor(uint8_t InductorParameter)
{
    if (InductorParameter == P47uH)
    {
        HAL_GPIO_WritePin(SelectInductorIN1GPIOX, SelectInductorIN1PINX, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(SelectInductorIN2GPIOX, SelectInductorIN2PINX, GPIO_PIN_SET);
    }
    if (InductorParameter == P10uH)
    {
        HAL_GPIO_WritePin(SelectInductorIN1GPIOX, SelectInductorIN1PINX, GPIO_PIN_SET);
        HAL_GPIO_WritePin(SelectInductorIN2GPIOX, SelectInductorIN2PINX, GPIO_PIN_RESET);
    }
}

/**
 * @函数名称       选择电阻
 * @说明           ResistanceParameter:电阻参数
 */
void SelectResistance(uint8_t ResistanceParameter)
{
    if (ResistanceParameter == P2p2ohm)
    {
        HAL_GPIO_WritePin(SelectResistanceIN1GPIOX, SelectResistanceIN1PINX, GPIO_PIN_SET);
        HAL_GPIO_WritePin(SelectResistanceIN2GPIOX, SelectResistanceIN2PINX, GPIO_PIN_SET);
    }
    if (ResistanceParameter == P0p47ohm)
    {
        HAL_GPIO_WritePin(SelectResistanceIN1GPIOX, SelectResistanceIN1PINX, GPIO_PIN_SET);
        HAL_GPIO_WritePin(SelectResistanceIN2GPIOX, SelectResistanceIN2PINX, GPIO_PIN_RESET);
    }
    if (ResistanceParameter == P0ohm)
    {
        HAL_GPIO_WritePin(SelectResistanceIN1GPIOX, SelectResistanceIN1PINX, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(SelectResistanceIN2GPIOX, SelectResistanceIN2PINX, GPIO_PIN_SET);
    }
}

/**
 * @函数名称       控制U4的模拟开关芯片，选择SPI
 * @说明           ResistanceParameter:电阻参数
 */
void SelectESLSPI(uint8_t SPIXLine)
{
    if (SPIXLine == ESL4SPI)
    {
        HAL_GPIO_WritePin(SelectESLSPIIN1GPIOX, SelectESLSPIIN1PINX, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(SelectESLSPIIN2GPIOX, SelectESLSPIIN2PINX, GPIO_PIN_SET);
    }
    if (SPIXLine == ESL3SPI)
    {
        HAL_GPIO_WritePin(SelectESLSPIIN1GPIOX, SelectESLSPIIN1PINX, GPIO_PIN_SET);
        HAL_GPIO_WritePin(SelectESLSPIIN2GPIOX, SelectESLSPIIN2PINX, GPIO_PIN_RESET);
    }
}

/**
 * @函数名称       控制U7的模拟开关芯片，选择VSPL2还是VDD
 * @说明           SelectU4HW:选择电路
 */
void VDDOrAAVSPL2(uint8_t SelectU4HW)
{
    if (SelectU4HW == VDD)
    {
        HAL_GPIO_WritePin(VDDOrAAVSPL2IN1GPIOX, VDDOrAAVSPL2IN1PINX, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(VDDOrAAVSPL2IN2GPIOX, VDDOrAAVSPL2IN2PINX, GPIO_PIN_SET);
    }   
    if (SelectU4HW == AAVSPL2)
    {
        HAL_GPIO_WritePin(VDDOrAAVSPL2IN1GPIOX, VDDOrAAVSPL2IN1PINX, GPIO_PIN_SET);
        HAL_GPIO_WritePin(VDDOrAAVSPL2IN2GPIOX, VDDOrAAVSPL2IN2PINX, GPIO_PIN_RESET);
    }
}


/**
 * @函数名称       控制U2的模拟开关芯片，选择VDDL还是NULL
 * @说明           SelectU2HW:选择电路
 */
void VDDLOrHiz(uint8_t SelectU2HW)
{
    if (SelectU2HW == VDDL)
    {
        HAL_GPIO_WritePin(VDDLOrHizIN1GPIOX, VDDLOrHizIN1PINX, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(VDDLOrHizIN2GPIOX, VDDLOrHizIN2PINX, GPIO_PIN_SET);
    }   
    if (SelectU2HW == Hiz)
    {
        HAL_GPIO_WritePin(VDDLOrHizIN1GPIOX, VDDLOrHizIN1PINX, GPIO_PIN_SET);
        HAL_GPIO_WritePin(VDDLOrHizIN2GPIOX, VDDLOrHizIN2PINX, GPIO_PIN_RESET);
    }
}

/**
 * @函数名称       控制U3的模拟开关芯片，选择TSCL还是VSPL2
 * @说明           SelectU3HW:选择电路
 */
void TSCLOrVSPL2(uint8_t SelectU3HW)
{
    if (SelectU3HW == VAVSPL2)
    {
        HAL_GPIO_WritePin(TSCLOrVSPL2IN1GPIOX, TSCLOrVSPL2IN1PINX, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(TSCLOrVSPL2IN2GPIOX, TSCLOrVSPL2IN2PINX, GPIO_PIN_SET);
    }   
    if (SelectU3HW == TSCL)
    {
        HAL_GPIO_WritePin(TSCLOrVSPL2IN1GPIOX, TSCLOrVSPL2IN1PINX, GPIO_PIN_SET);
        HAL_GPIO_WritePin(TSCLOrVSPL2IN2GPIOX, TSCLOrVSPL2IN2PINX, GPIO_PIN_RESET);
    }
}

/**
 * @函数名称       控制U8的模拟开关芯片，选择VSNL2还是VMTP
 * @说明           SelectU8HW:选择电路
 */
void VSNL2rVMTP(uint8_t SelectU8HW)
{
    if (SelectU8HW == VMTP)
    {
        HAL_GPIO_WritePin(TSCLOrVSPL2IN1GPIOX, TSCLOrVSPL2IN1PINX, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(TSCLOrVSPL2IN2GPIOX, TSCLOrVSPL2IN2PINX, GPIO_PIN_SET);
    }   
    if (SelectU8HW == VSNL2)
    {
        HAL_GPIO_WritePin(TSCLOrVSPL2IN1GPIOX, TSCLOrVSPL2IN1PINX, GPIO_PIN_SET);
        HAL_GPIO_WritePin(TSCLOrVSPL2IN2GPIOX, TSCLOrVSPL2IN2PINX, GPIO_PIN_RESET);
    }
}
