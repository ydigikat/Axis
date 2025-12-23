/*
   MIT License
   Copyright (c) 2025 YDigiKat

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/
#include "trace.h"

/* This will include the header for specific board we're using */
#include "board.h"    

/* Boards should provide a specific override of this function */
__attribute__((weak)) bool board_init(void)
{
  return false;
}

/**
 * clock_init
 * \brief initialises the STM32F4xx clock tree
 * \note we use the maximum 100MHz for sythesisers
 */
__attribute__((weak)) void clock_init()
{
  /* Use the external high-speed xtal */
  LL_RCC_HSE_Enable(); 
  while (!LL_RCC_HSE_IsReady())
    ;

  /* Power settings */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);         
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1); 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);                   

  /* Configure the PLL */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, PLL_M, PLL_N, PLL_R);
  LL_RCC_PLL_Enable(); 
  while (!LL_RCC_PLL_IsReady())
    ;

  /* Clock source is the PLL */  
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL); 
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }; 

  /* Scale the bus clock signals - APB1 runs at half speed */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1); 
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2); 
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);   

  /* Tell the system our speed */
  SystemCoreClock = FREQ;                     

  /* Enable the ART flash cache subsystem */
  LL_FLASH_EnablePrefetch();  
  LL_FLASH_EnableInstCache(); 
}

/**
 * \brief Enable the floating point unit (FPU)
 */
__attribute__((weak)) void fpu_init()
{
  /* Enable */
  SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));  

  /* Subnormals as zeroes */
  __set_FPSCR(__get_FPSCR() | (1 << 24));
  __ISB(); 
  __DSB(); 
}

/**
 * init
 * \brief This initialises the board (hardware)
 * \details There is a specific order to the init sequence:
 * 
 *          1 - Shared basics:   clock tree, flash and FPU
 *          2 - Board specific:  gpio pins, peripheral clocks etc
 *          3 - Shared hardware: peripherals I2S, UART etc
 *          
 *          The board specifics are sandwiched between 1 and 3 because it is often
 *          responsible for defining the pins and peripheral clocks, busses etc needed
 *          by the subsequent shared hardware in 3.  
 * 
 * \note    The init functions are all defined as weak which means they
 *          can be overriden in the board specific file should that be needed.  
 */
bool init(void)
{  
  clock_init();     /* Initialise clocks  */  
  fpu_init();       /* Initialise the FPU */

  if(!board_init()) /* Board specific initialisation */
  {
    return false;
  }
  
  /* Remaining shared initialisation goes here */

  return true; 
}


/**
 * \brief Hard fault exception handler
 * \note If the processor hits a serious error, this gets called.
 *       It logs the error and hangs the system (better than random behavior).
 */
__attribute__((weak))  void HardFault_Handler(void)
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
__attribute__((weak)) void MemManage_Handler(void)
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
__attribute__((weak)) void BusFault_Handler(void)
{
  RTT_LOG("BusFault_Handler\n");
  while (1)
    ; 
}

/**
 * \brief Usage fault exception handler
 * \note Called on various CPU usage errors like trying to execute invalid instructions.
 */
__attribute__((weak)) void UsageFault_Handler(void)
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
__attribute__((weak)) void NMI_Handler(void)
{
  RTT_LOG("NMI_Handler\n");
  while (1)
    ; 
}