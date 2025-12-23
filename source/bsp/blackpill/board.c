/*
  ------------------------------------------------------------------------------   
   Frugi
  ------------------------------------------------------------------------------
   MIT License
   Copyright (c) 2025 Jason Wilden

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
  ------------------------------------------------------------------------------
*/

#include "board.h"

/**
 * enable_peripheral_clocks
 * \brief Enable the specific clocks needed by this board
 * \note core_board_init() will call this during initialisation of peripherals.
 */
void enable_peripheral_clocks()
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);  
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
}


/**
 * Initialize GPIO pins
 */
bool gpio_init()
{
  /* LED */
  LL_GPIO_InitTypeDef led =
  {
    .Mode = LL_GPIO_MODE_OUTPUT,
    .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
    .Pull = LL_GPIO_PULL_NO,
    .Pin = LL_GPIO_PIN_13,
  };

  if(LL_GPIO_Init(GPIOC,&led) != SUCCESS)
  {
    return false;
  }

  /* USR BUTTON */
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);

  /* Probe toggle pins */
  LL_GPIO_InitTypeDef probe =
      {
          .Mode = LL_GPIO_MODE_OUTPUT,
          .Speed = LL_GPIO_SPEED_FREQ_HIGH,
          .Pull = LL_GPIO_PULL_DOWN,
          .Pin = LL_GPIO_PIN_9 | LL_GPIO_PIN_8 | LL_GPIO_PIN_7 | LL_GPIO_PIN_6 | LL_GPIO_PIN_5 | LL_GPIO_PIN_4 |
                 LL_GPIO_PIN_3 | LL_GPIO_PIN_2};

  if(LL_GPIO_Init(GPIOB, &probe) != SUCCESS)
  {
    return false;
  }

  return true;
}


/**
 * \brief Initialise the board
 */
bool board_init_cb(void)
{
  enable_peripheral_clocks();
  return gpio_init();
}


