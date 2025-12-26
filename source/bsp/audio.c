/*
   MIT License
   Copyright (c) 2025 YDigiKat

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/
#include "board.h"
#include "stddef.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_dma.h"

/* I2S PLL dividers are the same for all boards providing the oscillator is divided down
   to 1MHz or 2MHz, these come from Table 90 in the STM32F411xE reference manual. They
   differ depending on whether we're also using a MCLK.
*/
#ifdef DAE_USES_MCLK

#define I2S_44_N (271)
#define I2S_44_R (LL_RCC_PLLI2SR_DIV_2)

#define I2S_48_N (258)
#define I2S_48_R (LL_RCC_PLLI2SR_DIV_3)

#define I2S_96_N (344)
#define I2S_96_R (LL_RCC_PLLI2SR_DIV_2)

#else

#define I2S_44_N (429)
#define I2S_44_R (LL_RCC_PLLI2SR_DIV_4)

#define I2S_48_N (384)
#define I2S_48_R (LL_RCC_PLLI2SR_DIV_5)

#define I2S_96_N (424)
#define I2S_96_R (LL_RCC_PLLI2SR_DIV_3)

#endif

/* Import the functions we need to communicate with the DAE */
extern void dae_ready_for_audio(uint8_t buffer_idx);
extern void dae_midi_received(uint8_t byte);

/**
 * audio_start
 * \brief starts the audio hardware
 * \note this is called by the DAE to start the hardware audio layer, it
 *       supplies the buffer and other parameters. It finalises the configuration
 *       of the audio subsystem and starts it running.
 * \param audio_buffer the audio buffer
 * \param buf_len the number of bytes in the buffer
 * \param fsr sample rate
 */
void audio_start(int16_t audio_buffer[], size_t buf_len, uint32_t fsr)
{
    /* Set DMA transfer buffer */
    LL_DMA_SetDataLength(DMA, DMA_STREAM, buf_len);
    LL_DMA_ConfigAddresses(DMA, DMA_STREAM, (uint32_t)audio_buffer, LL_SPI_DMA_GetRegAddr(I2S), LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    /* Calculate the PLL speeds required for different sample rates */
    if (fsr == 48000)
    {
        LL_RCC_PLLI2S_ConfigDomain_I2S(LL_RCC_PLLSOURCE_HSE, I2S_PLL_M, I2S_48_N, I2S_48_R);
    }
    else if (fsr == 96000)
    {
        LL_RCC_PLLI2S_ConfigDomain_I2S(LL_RCC_PLLSOURCE_HSE, I2S_PLL_M, I2S_96_N, I2S_96_R);
    }
    else
    {
        LL_RCC_PLLI2S_ConfigDomain_I2S(LL_RCC_PLLSOURCE_HSE, I2S_PLL_M, I2S_44_N, I2S_44_R);
    }

    /* Re-enable PLLI2S */
    LL_RCC_PLLI2S_Enable();
    while (!LL_RCC_PLLI2S_IsReady())
        ;

    /* Update I2S peripheral with sample rate provided by audio generator */
    LL_I2S_InitTypeDef i2s =
        {
            .AudioFreq = fsr,
            .ClockPolarity = LL_I2S_POLARITY_LOW,
            .DataFormat = LL_I2S_DATAFORMAT_32B,
            .MCLKOutput = LL_I2S_MCLK_OUTPUT_DISABLE,
            .Mode = LL_I2S_MODE_MASTER_TX,
            .Standard = LL_I2S_STANDARD_PHILIPS};
            
#ifdef DAE_USES_MCLK            
    /* If we're using the MCLK enable the output*/
    i2s.MCLKOutput = LL_I2S_MCLK_OUTPUT_ENABLE;
#endif

    if (LL_I2S_Init(I2S, &i2s) != SUCCESS)
    {
        return;
    }

    /* Enable I2S */
    LL_I2S_Enable(I2S);
    while (!LL_I2S_IsEnabled(I2S))
        ;

    /* Enable DMA */
    LL_DMA_EnableStream(DMA, DMA_STREAM);
    while (!LL_DMA_IsEnabledStream(DMA, DMA_STREAM))
        ;
}


/**
 * \brief Audio DMA Interrupt Handler
 *
 * \note This gets called when the DMA has finished transferring a buffer and it can
 *       be refilled.  The DAE will be responsible for filling the buffer by implementing
 *       the refill_audio function.  We provide a weak version which generates a simple
 *       tone for testing the board.
 */
void DMA_IRQ_HANDLER(void)
{
  if (DMA->HISR & DMA_HISR_TCIF)
  {
    DMA->HIFCR = DMA_HIFCR_CTCIF; 
    dae_ready_for_audio(1);              
  }
  else
  {
    DMA->HIFCR = DMA_HIFCR_CHTIF; 
    dae_ready_for_audio(0);              
  }
}

