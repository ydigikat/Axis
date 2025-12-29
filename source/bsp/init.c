/*
   MIT License
   Copyright (c) 2025 YDigiKat

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/
#include "trace.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_usart.h"

/* This will include the header for specific board we're using */
#include "board.h"

/**
 * clock_init
 * \brief initialises the STM32F4xx clock tree
 * \note we use the maximum 100MHz for sythesisers since power usage
 *       is not a concern.
 */
static void clock_init()
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
static void fpu_init()
{
  /* Enable */
  SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));

  /* Subnormals as zeroes */
  __set_FPSCR(__get_FPSCR() | (1 << 24));
  __ISB();
  __DSB();
}

/**
 * \brief Initialise the I2S peripheral and associated pins
 * \note This only partially initialises the peripheral and
 *       does not start it, this is done later by the audio
 *       start call.
 * \return true if success, false otherwise
 */
static bool i2s_init()
{
  /* Pin configuration, output, low frequency, alternate function (peripheral mode) */
  LL_GPIO_InitTypeDef io =
      {
          .Speed = LL_GPIO_SPEED_FREQ_LOW,
          .Pull = LL_GPIO_PULL_DOWN,
          .Mode = LL_GPIO_MODE_ALTERNATE,
          .Alternate = I2S_AF,
      };

  /* Serial Clock (SCK, aka BCLK) - the bit clock */
  io.Pin = I2S_SCK_PIN;
  if (LL_GPIO_Init(I2S_SCK_PORT, &io) != SUCCESS)
  {
    return false;
  }

  /* Serial Data Out (SDO, aka SD) - the audio data */
  io.Pin = I2S_SDO_PIN;
  if (LL_GPIO_Init(I2S_SDO_PORT, &io) != SUCCESS)
  {
    return false;
  }

  /* Word Select (WS, aka LRCK) - indicates left/right channel */
  io.Pin = I2S_WS_PIN;
  if (LL_GPIO_Init(I2S_WS_PORT, &io) != SUCCESS)
  {
    return false;
  }

#ifdef MCLK_PIN
  /* Some codecs require a master clock signal */
  io.Pin = MCLK_PIN;
  if (LL_GPIO_Init(MCLK_PORT, &io) != SUCCESS)
  {
    return false;
  }
#endif

/* 
  Partially configure the I2S peripheral, the remaining configuration is done by
  the audio_start call.
*/
if (LL_I2S_Init(I2S, &(LL_I2S_InitTypeDef){  
  .ClockPolarity = LL_I2S_POLARITY_LOW,
  .DataFormat = LL_I2S_DATAFORMAT_32B,
  .MCLKOutput = LL_I2S_MCLK_OUTPUT_DISABLE,
  .Mode = LL_I2S_MODE_MASTER_TX,
  .Standard = LL_I2S_STANDARD_PHILIPS}) != SUCCESS)
  {
    return false;
  }

  return true;
}

/**
 * \brief initialises the DMA transfer for the I2S peripheral
 * \note this is only partial configuration and the DMA is not started,
 *       we complete these operations in the audio_start call.
 * \return true if success, false otherwise
 */
 static bool dma_init()
{
  if (LL_DMA_Init(DMA, DMA_STREAM, &(LL_DMA_InitTypeDef){
         .Channel = DMA_CHANNEL,                               
        .Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH,       
        .PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT,   
        .MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT,     
        .PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD, 
        .MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD, 
        .Mode = LL_DMA_MODE_CIRCULAR,                         
        .Priority = LL_DMA_PRIORITY_HIGH,                     
        .FIFOMode = LL_DMA_FIFOMODE_DISABLE
  }) != SUCCESS)
  {
    return false;
  }

  /* Enable interrupts at half-transfer (HT) and transfer-complete (TC) */
  LL_DMA_EnableIT_HT(DMA, DMA_STREAM); 
  LL_DMA_EnableIT_TC(DMA, DMA_STREAM); 

  NVIC_SetPriority(DMA_IRQN, 10);
  NVIC_EnableIRQ(DMA_IRQN);      

  /* Connnect DMA to I2S peripheral */
  LL_SPI_EnableDMAReq_TX(I2S);

  return true;
}

/**
 * \brief Configure the UART for MIDI input
 * \note  MIDI runs at 31250 baud, 8N1, RX only since the synth does not
 *        ever send MIDI.
 * 1\return true if successful, false if initialisation failed.
 */
static bool uart_init()
{
  
  LL_GPIO_InitTypeDef gpio =
      {
          .Mode = LL_GPIO_MODE_ALTERNATE,   
          .Alternate = UART_AF,             
          .Pin = UART_RX_PIN,               
          .Speed = LL_GPIO_SPEED_FREQ_HIGH, 
          .Pull = LL_GPIO_PULL_UP};         

  if (LL_GPIO_Init(UART_RX_PORT, &gpio) != SUCCESS)
  {
    return false; 
  }

  /* USART config  31250, 8N1 RX only */
  LL_USART_InitTypeDef usart =
      {
          .BaudRate = 31250,                           
          .OverSampling = LL_USART_OVERSAMPLING_16,    
          .DataWidth = LL_USART_DATAWIDTH_8B,          
          .Parity = LL_USART_PARITY_NONE,              
          .StopBits = LL_USART_STOPBITS_1,             
          .TransferDirection = LL_USART_DIRECTION_RX}; 

  if (LL_USART_Init(UART, &usart) != SUCCESS)
  {
    return false; 
  }

  /* Interrupt handling, this will signal arrival of MIDI data */
  NVIC_EnableIRQ(UART_IRQN);      
  NVIC_SetPriority(UART_IRQN, 6); 

  LL_USART_EnableIT_RXNE(UART); 

  /* Enable */
  LL_USART_Enable(UART); 
  while (!LL_USART_IsEnabled(UART))
    ; 

  return true;
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
  clock_init(); /* Initialise clocks  */
  fpu_init();   /* Initialise the FPU */

  if (!board_init()) /* Board specific initialisation */
  {
    return false;
  }

  /* Remaining shared initialisation goes here */
  i2s_init();
  dma_init();
  uart_init();

  return true;
}


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
 *       the dae_ready_for_audio() function.
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

/**
 * UART Interrupt Handler
 *
 * This gets called whenever a MIDI byte arrives at the UART.
 * It simply passes the data to the Digital Audio Engine for processing.
 */
void UART_IRQ_HANDLER(void)
{   
  dae_midi_received((uint8_t)UART->DR); // Send the received MIDI byte to the DAE
}

/**
 * \brief Hard fault exception handler
 * \note If the processor hits a serious error, this gets called.
 *       It logs the error and hangs the system (better than random behavior).
 */
void HardFault_Handler(void)
{
  RTT_LOG("HALTED: Hard fault.\n");
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
  RTT_LOG("HALTED: Memory fault.\n");
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
  RTT_LOG("HALTED: Bus fault.\n");
  while (1)
    ;
}

/**
 * \brief Usage fault exception handler
 * \note Called on various CPU usage errors like trying to execute invalid instructions.
 */
void UsageFault_Handler(void)
{
  RTT_LOG("HALTED: Usage fault.\n");
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
  RTT_LOG("HALTED: Non maskable interrupt.\n");
  while (1)
    ;
}