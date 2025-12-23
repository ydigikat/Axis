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
#ifndef __PINS_H__
#define __PINS_H__

#include "stm32f4xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_pwr.h"


/* LEDs */
#define LED_GREEN_ON() (LL_GPIO_SetOutputPin(GPIOD,LL_GPIO_PIN_12))
#define LED_ORANGE_ON() (LL_GPIO_SetOutputPin(GPIOD,LL_GPIO_PIN_13))
#define LED_RED_ON() (LL_GPIO_SetOutputPin(GPIOD,LL_GPIO_PIN_14))
#define LED_BLUE_ON() (LL_GPIO_SetOutputPin(GPIOD,LL_GPIO_PIN_15))

#define LED_GREEN_OFF() (LL_GPIO_ResetOutputPin(GPIOD,LL_GPIO_PIN_12))
#define LED_ORANGE_OFF() (LL_GPIO_ResetOutputPin(GPIOD,LL_GPIO_PIN_13))
#define LED_RED_OFF() (LL_GPIO_ResetOutputPin(GPIOD,LL_GPIO_PIN_14))
#define LED_BLUE_OFF() (LL_GPIO_ResetOutputPin(GPIOD,LL_GPIO_PIN_15))

#define USR_LED_ON() LED_GREEN_ON()
#define USR_LED_OFF() LED_GREEN_OFF()

/* USER Button */
#define READ_USR_BTN() (LL_GPIO_ReadInputPort(GPIOA) & LL_GPIO_PIN_0)


/* Clock dividers  */
#define PLL_M (LL_RCC_PLLM_DIV_4)
#define PLL_N (100)
#define PLL_R (LL_RCC_PLLP_DIV_2)
#define FREQ (100000000)

#endif /* __BOARD_H__ */