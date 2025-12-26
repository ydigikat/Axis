/*
   MIT License
   Copyright (c) 2025 Jason Wilden

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include "stm32f4xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_pwr.h"


/* LEDs */
#define LED_GREEN_ON() (LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_12))
#define LED_ORANGE_ON() (LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_13))
#define LED_RED_ON() (LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_14))
#define LED_BLUE_ON() (LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_15))

#define LED_GREEN_OFF() (LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_12))
#define LED_ORANGE_OFF() (LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_13))
#define LED_RED_OFF() (LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_14))
#define LED_BLUE_OFF() (LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_15))

#define USR_LED_ON() LED_GREEN_ON()
#define USR_LED_OFF() LED_GREEN_OFF()

/* USER Button */
#define READ_USR_BTN() (LL_GPIO_ReadInputPort(GPIOA) & LL_GPIO_PIN_0)

/* Clock dividers  */
#define PLL_M (LL_RCC_PLLM_DIV_4)
#define PLL_N (100)
#define PLL_R (LL_RCC_PLLP_DIV_2)
#define FREQ (100000000)

/* I2S configuration */
#define I2S (SPI3)                        /* I2S shares the SPI peripheral */
#define I2S_AF (LL_GPIO_AF_6)             /* The alternate function for I2S3 */

#define I2S_WS_PIN (LL_GPIO_PIN_4)        
#define I2S_WS_PORT (GPIOA)
#define I2S_SDO_PIN (LL_GPIO_PIN_12)      
#define I2S_SDO_PORT (GPIOC)
#define I2S_SCK_PIN (LL_GPIO_PIN_10)      
#define I2S_SCK_PORT (GPIOC)
#define I2S_PLL_M (LL_RCC_PLLI2SM_DIV_8)   
#define MCLK_PIN (LL_GPIO_PIN_7)           
#define MCLK_PORT (GPIOC)


/* DMA configuration */
#define DMA (DMA1)                         /* The DMA periperhal, stream and channel are specific to the I2S peripheral */
#define DMA_STREAM (LL_DMA_STREAM_5)      
#define DMA_CHANNEL (LL_DMA_CHANNEL_0)    
#define DMA_HISR_TCIF (DMA_HISR_TCIF5)
#define DMA_HIFCR_CTCIF (DMA_HIFCR_CTCIF5) /* We want an interrupt when the buffer is complete*/   
#define DMA_HIFCR_CHTIF (DMA_HIFCR_CHTIF5) /* We want an interrupt when the buffer is half complete */  

#define DMA_IRQN (DMA1_Stream5_IRQn)       /* The interrupt number and interrupt handler function */   
#define DMA_IRQ_HANDLER DMA1_Stream5_IRQHandler

/* API */
bool board_init(void);

#endif /* __BOARD_H__ */