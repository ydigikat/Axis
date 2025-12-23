/*
   MIT License
   Copyright (c) 2025 YDigiKat

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/
#include "trace.h"

#include "board.h"    /* This will include the header for specific board we're using */

/* The different boards should provide a specific override of this function */
__attribute__((weak)) bool board_init_cb(void)
{
  return false;
}

static void clock_init()
{
  LL_RCC_HSE_Enable(); 
  while (!LL_RCC_HSE_IsReady())
    ;

  /* Power management settings */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);         
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1); 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);                   

  /* Configure the PLL */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, PLL_M, PLL_N, PLL_R);
  LL_RCC_PLL_Enable(); 
  while (!LL_RCC_PLL_IsReady())
    ;

  /* Scale the system clock signal */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1); 
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL); 
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }; 

  /* Scale the bus clock signals - APB1 runs at half speed */
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2); 
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1); 

  /* Tell the system our speed */
  SystemCoreClock = FREQ;                     
}

bool init(void)
{
  /* Initialise system clock tree and FPU*/
  clock_init(); 
 
  /* Enable ART flash cache subsystem */
  LL_FLASH_EnablePrefetch();  
  LL_FLASH_EnableInstCache(); 

  /* Call the board specific initialisation */
  if(!board_init_cb())
  {
    return false;
  }
  
  return true; 
}


/**
 * \brief Hard fault exception handler
 * \note If the processor hits a serious error, this gets called.
 *       It logs the error and hangs the system (better than random behavior).
 */
void HardFault_Handler(void)
{
  RTT_LOG("HardFault_Handler\n");
  while (1)
    ; 
}

/**
 * \brief Memory management fault handler
 * \note Called on memory protection errors.
 *       Generally means trying to access memory it shouldn't.
 */
void MemManage_Handler(void)
{
  RTT_LOG("MemManage_Handler\n");
  while (1)
    ; 
}

/**
 * \brief Bus fault exception handler
 * \note Called on errors during memory transfers.
 *       Typically means trying to access hardware that isn't there.
 */
void BusFault_Handler(void)
{
  RTT_LOG("BusFault_Handler\n");
  while (1)
    ; 
}

/**
 * \brief Usage fault exception handler
 * \note Called on various CPU usage errors like trying to execute invalid instructions.
 */
void UsageFault_Handler(void)
{
  RTT_LOG("UsageFault_Handler\n");
  while (1)
    ; 
}

/**
 * \brief Non-maskable interrupt handler
 * \note This interrupt that can't be disabled.
 *       If this triggers, something bad happened with the hardware.
 */
void NMI_Handler(void)
{
  RTT_LOG("NMI_Handler\n");
  while (1)
    ; 
}