#include "uart1.h"
#include "stm32f1xx.h"

void UART1_Init(void)
{
    /* Enable clocks for GPIOA, AFIO, USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN;

    /* Configure PA9 as AF Push-Pull, 50MHz (TX) */
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |= (0xB << 4);

    /* Configure PA10 as floating input (RX) */
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |= (0x4 << 8);

    /* Reset USART1 configuration */
    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;

    /* Baud rate 115200 @ 72MHz: BRR = 72,000,000 / 115200 = 625 (0x271) */
    USART1->BRR = 625;

    /* Enable RX, TX, IDLE & RXNE interrupts, and USART */
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_IDLEIE | USART_CR1_RXNEIE | USART_CR1_UE;

    /* Enable USART1 interrupt in NVIC */
    NVIC_SetPriority(USART1_IRQn, 5);
    NVIC_EnableIRQ(USART1_IRQn);
}

void UART1_SendBlocking(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return;
    }

    for (uint16_t i = 0; i < len; ++i) {
        uint32_t guard = 1000000U;
        while ((USART1->SR & USART_SR_TXE) == 0) {
            if (guard-- == 0U) {
                return;
            }
        }
        USART1->DR = data[i];
    }

    uint32_t guard = 1000000U;
    while ((USART1->SR & USART_SR_TC) == 0) {
        if (guard-- == 0U) {
            return;
        }
    }
}
