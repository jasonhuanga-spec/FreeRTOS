/*

硬件电路配置文件

*/

#ifndef HWCI_H
#define HWCI_H

#include "main.h"
#include "gpio.h"

#define VDDSwitchGPIOX                  GPIOA
#define VDDSwitchPINX                   GPIO_PIN_0

#define LVSwitchGPIOX                   GPIOC
#define LVSwitchPINX                    GPIO_PIN_13

#define SelectInductorIN1GPIOX          GPIOC
#define SelectInductorIN1PINX           GPIO_PIN_0
#define SelectInductorIN2GPIOX          GPIOC
#define SelectInductorIN2PINX           GPIO_PIN_1

#define SelectResistanceIN1GPIOX        GPIOB
#define SelectResistanceIN1PINX         GPIO_PIN_9
#define SelectResistanceIN2GPIOX        GPIOB
#define SelectResistanceIN2PINX         GPIO_PIN_8

#define SelectESLSPIIN1GPIOX            GPIOC
#define SelectESLSPIIN1PINX             GPIO_PIN_6
#define SelectESLSPIIN2GPIOX            GPIOB
#define SelectESLSPIIN2PINX             GPIO_PIN_15

#define VDDOrAAVSPL2IN1GPIOX            GPIOA
#define VDDOrAAVSPL2IN1PINX             GPIO_PIN_15
#define VDDOrAAVSPL2IN2GPIOX            GPIOA
#define VDDOrAAVSPL2IN2PINX             GPIO_PIN_14

#define VDDLOrHizIN1GPIOX               GPIOB
#define VDDLOrHizIN1PINX                GPIO_PIN_10
#define VDDLOrHizIN2GPIOX               GPIOB
#define VDDLOrHizIN2PINX                GPIO_PIN_11

#define TSCLOrVSPL2IN1GPIOX             GPIOB
#define TSCLOrVSPL2IN1PINX              GPIO_PIN_13
#define TSCLOrVSPL2IN2GPIOX             GPIOB
#define TSCLOrVSPL2IN2PINX              GPIO_PIN_14

#define VSNL2rVMTPIN1GPIOX              GPIOB
#define VSNL2rVMTPIN1PINX               GPIO_PIN_12
#define VSNL2rVMTPIN2GPIOX              GPIOA
#define VSNL2rVMTPIN2PINX               GPIO_PIN_13

#define VDDON       1
#define VDDOFF      0

#define LVON        1
#define LVOFF       0

#define P47uH       1
#define P10uH       0

#define P2p2ohm     1
#define P0p47ohm    2
#define P0ohm       0

#define ESL4SPI     1
#define ESL3SPI     0

#define VDD         1
#define AAVSPL2     0

#define VDDL        1
#define Hiz         0

#define VAVSPL2     1
#define TSCL        0

#define VMTP        1
#define VSNL2       0


void VDDSwitch(uint8_t VDDStates);
void LVSwitch(uint8_t LVSwitch);
void SelectInductor(uint8_t InductorParameter);
void SelectResistance(uint8_t ResistanceParameter);
void SelectESLSPI(uint8_t SPIXLine);
void VDDOrAAVSPL2(uint8_t SelectU4HW);
void VDDLOrHiz(uint8_t SelectU2HW);
void TSCLOrVSPL2(uint8_t SelectU3HW);
void VSNL2rVMTP(uint8_t SelectU8HW);

#endif