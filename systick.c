/*
 * systick.c
 *
 *  Created on: 31 de dez. de 2025
 *      Author: vinicius.andrade
 */

#include "fsl_common.h"
#include "systick.h"
#include <stdint.h>

#define SYSTICK_HZ 1000U

static volatile uint32_t ms_ticks = 0;


void SysTick_Handler(void)
{
	ms_ticks++;
}


void systick_init(void)
{
    SystemCoreClockUpdate();
    // todo: checar retorno de SysTick_Config para detectar falha de configuracao.
    SysTick_Config(SystemCoreClock / SYSTICK_HZ);
}

uint32_t systick_get_ms(void)
{
    return (ms_ticks);
}

void delay_ms(uint32_t ms)
{
    uint32_t start = ms_ticks;
    // todo: substituir espera ativa por sleep/evento para reduzir consumo quando possivel.
    while ((ms_ticks - start) < ms)
    {
        __NOP();
    }
}
