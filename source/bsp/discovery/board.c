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


void enable_peripheral_clocks()
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);  
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
}

bool gpio_init()
{
  /* LEDs */
  LL_GPIO_InitTypeDef led =
      {
          .Mode = LL_GPIO_MODE_OUTPUT,
          .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
          .Pull = LL_GPIO_PULL_DOWN,
          .Pin = LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15};

  if (LL_GPIO_Init(GPIOD, &led) != SUCCESS)
  {
    return false;
  }

  /* USR button */
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);

  /* Probe toggles */
  LL_GPIO_InitTypeDef probe =
      {
          .Mode = LL_GPIO_MODE_OUTPUT,
          .Speed = LL_GPIO_SPEED_FREQ_HIGH,
          .Pull = LL_GPIO_PULL_DOWN,
          .Pin = LL_GPIO_PIN_9 | LL_GPIO_PIN_8 | LL_GPIO_PIN_7 | LL_GPIO_PIN_6 | LL_GPIO_PIN_5 | LL_GPIO_PIN_4 |
                 LL_GPIO_PIN_3 | LL_GPIO_PIN_2};

  if (LL_GPIO_Init(GPIOB, &probe) != SUCCESS)
  {
    return false;
  }

  return true;
}

bool board_init_cb()
{  
  enable_peripheral_clocks();
  return gpio_init();
}

