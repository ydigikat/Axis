/*
   MIT License
   Copyright (c) 2025 Jason Wilden

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/

#include "board.h"


bool board_init(void)
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);  
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  if(LL_GPIO_Init(GPIOC,&(LL_GPIO_InitTypeDef){
    .Mode = LL_GPIO_MODE_OUTPUT,
    .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
    .Pull = LL_GPIO_PULL_NO,
    .Pin = LL_GPIO_PIN_13}) != SUCCESS)
  {
    return false;
  }

  /* USR BUTTON */
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_UP);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
  
  return true;
}


