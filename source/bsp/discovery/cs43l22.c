b/*
   MIT License
   Copyright (c) 2025 Jason Wilden

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/
#include "board.h"
#include "stm32f4xx_ll_i2c.h"

/* Control Memory Address Pointers (registers) for CS32L22 DAC */
#define CS43L22_REG_CHIP_ID 0x01
#define CS43L22_REG_POWER_CTL1 0x02
#define CS43L22_REG_POWER_CTL2 0x04
#define CS43L22_REG_CLOCKING_CTL 0x05
#define CS43L22_REG_INTERFACE_CTL1 0x06
#define CS43L22_REG_PLAYBACK_CTL2 0x0F
#define CS43L22_REG_MASTER_A_VOL 0x20
#define CS43L22_REG_MASTER_B_VOL 0x21
#define CS43L22_REG_SPEAKER_A_VOL 0x24
#define CS43L22_REG_SPEAKER_B_VOL 0x25

/* Configuration Values (parameters) */
#define OUTPUT_DEVICE_AUTO 0x05
#define CODEC_STANDARD 0x04
#define SPKR_ATTENUATION 0x00
#define SPKR_MODE 0x06
#define VOLUME 80

/* Define timeout for I2C operations */
#define I2C_TIMEOUT_COUNT  10000

/* I2C address */
#define CS43L22_I2C_ADDR (0x94U)


/* Initialise i2c1, this is used to configure the CS43L22 DAC. */
static bool i2c1_init()
{
  /* Enable peripheral clocks */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* Configure GPIOB PB6 & PB9 as I2C1 pins (SCL and SDA) */
  LL_GPIO_InitTypeDef io =
      {
          .Pin = LL_GPIO_PIN_6 | LL_GPIO_PIN_9,
          .Mode = LL_GPIO_MODE_ALTERNATE,
          .Speed = LL_GPIO_SPEED_FREQ_HIGH,
          .OutputType = LL_GPIO_OUTPUT_OPENDRAIN,
          .Pull = LL_GPIO_PULL_UP,
          .Alternate = LL_GPIO_AF_4,
      };

  if(LL_GPIO_Init(GPIOB, &io) != SUCCESS)
  {
    return false;
  }

  /* Configure GPIOD PD4 as output (Reset Codec active) */
  LL_GPIO_InitTypeDef reset =
      {
          .Pin = LL_GPIO_PIN_4,
          .Mode = LL_GPIO_MODE_OUTPUT,
          .Speed = LL_GPIO_SPEED_FREQ_LOW,
          .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
          .Pull = LL_GPIO_PULL_NO,
      };

  if(LL_GPIO_Init(GPIOD, &reset) != SUCCESS)
  {
    return false;
  }

  /* Reset the device */
  LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_4);

  /* Reset I2C Peripheral */
  LL_I2C_EnableReset(I2C1);
  LL_I2C_DisableReset(I2C1);

  /* Set frequency parameters Standard Mode (SM) 100 KHz */
  LL_I2C_InitTypeDef i2c =
      {
          .PeripheralMode = LL_I2C_MODE_I2C,
          .ClockSpeed = 100000, /* 100 KHz */
          .DutyCycle = LL_I2C_DUTYCYCLE_2,
          .OwnAddress1 = 0,
          .TypeAcknowledge = LL_I2C_ACK,
          .OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT};

  if(LL_I2C_Init(I2C1, &i2c) != SUCCESS)
  {
    return false;
  }

  /* Enable error interrupts */
  LL_I2C_EnableIT_ERR(I2C1);
  NVIC_SetPriority(I2C1_ER_IRQn, 10);
  NVIC_EnableIRQ(I2C1_ER_IRQn);

  /* Enable I2C1 peripheral */
  LL_I2C_Enable(I2C1);

  return true;
}

/* Only need to be able to write data to I2C */
static bool i2c_write(uint8_t device_addr, uint8_t addr, uint8_t data)
{
  uint32_t timeout = I2C_TIMEOUT_COUNT;

  /* Wait for I2C to become inactive */
  while(LL_I2C_IsActiveFlag_BUSY(I2C1))
  {
    if(timeout-- == 0)
      return false;
  }

  /* Generate start condition */
  LL_I2C_GenerateStartCondition(I2C1);
  while(!LL_I2C_IsActiveFlag_SB(I2C1))
  {
    if(timeout-- == 0)
      return false;
  }

  /* Send the address for write */
  LL_I2C_TransmitData8(I2C1, device_addr);
  while(!LL_I2C_IsActiveFlag_ADDR(I2C1))
  {
    if(timeout-- == 0)
      return false;
  }

  /* Clear the address flag */
  LL_I2C_ClearFlag_ADDR(I2C1);

   /* Send memory address (register) */
   LL_I2C_TransmitData8(I2C1, addr);
   while(!LL_I2C_IsActiveFlag_BTF(I2C1))
   {
    if(timeout-- == 0)
      return false;
  }

  /* Send data */
   LL_I2C_TransmitData8(I2C1, data);
   while(!LL_I2C_IsActiveFlag_BTF(I2C1))
   {
    if(timeout-- == 0)
      return false;
  }

   /* Stop */
   LL_I2C_GenerateStopCondition(I2C1);
   while(LL_I2C_IsActiveFlag_BUSY(I2C1))
   {
    if(timeout-- == 0)
      return false;
  }

   return true;
}

static void cs43l22_set_volume(uint8_t volume)
{
  /* CS43L22 volume: 0x00 = +12dB, 0xFF = -102dB (mute) */
  /* Map 0-100 scale to appropriate CS43L22 volume range (e.g., 0-100%) */
  uint8_t cs43l22_vol;
  
  if (volume > 100)
    volume = 100;
    
  /* For 0-100% volume: 0% = mute (0xE0), 100% = max (0x00) */
  if (volume == 0)
    cs43l22_vol = 0xFF; /* Mute */
  else
    cs43l22_vol = 0x18 - ((volume * 0x18) / 100); /* Map to approx +12dB to -50dB range */
  
  /* Set the Master volume for both channels */
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_MASTER_A_VOL, cs43l22_vol);
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_MASTER_B_VOL, cs43l22_vol);
}


void cs43l22_init(void)
{
  i2c1_init();

  /* Power down for configuration */
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_POWER_CTL1, 0x01);
  
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_POWER_CTL2, 0x0A);
  
  /* Clock configuration - auto detect, div by 2 */
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_CLOCKING_CTL, 0x81);
  
  /* I2S audio standard, 16-bit */
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_INTERFACE_CTL1, 0x04);
  
  /* Disable digital soft ramp and other DSP features */
  i2c_write(CS43L22_I2C_ADDR, 0x0A, 0x00);
  
  /* Disable digital EQ effects */
  i2c_write(CS43L22_I2C_ADDR, 0x14, 0x00);
  
  /* Bypass tone controls (no bass/treble boost) */
  i2c_write(CS43L22_I2C_ADDR, 0x1A, 0x00); /* Tone control off */
  i2c_write(CS43L22_I2C_ADDR, 0x1B, 0x00); /* Bass=0, Treble=0 */
  
  /* Configure limiter with gentle settings - just for hardware protection */
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_PLAYBACK_CTL2, 0x02); /* Limiter ON, PCM mixer OFF */
  
  /* Limiter threshold - set to avoid clipping but allow dynamic range */
  i2c_write(CS43L22_I2C_ADDR, 0x27, 0x00); /* Default attack/release */
  i2c_write(CS43L22_I2C_ADDR, 0x1C, 0x90); /* Higher threshold (-1.5dB) */
  
  /* Speaker volumes */
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_SPEAKER_A_VOL, 0x00); /* No attenuation */
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_SPEAKER_B_VOL, 0x00);
  
  /* Headphone volumes */
  i2c_write(CS43L22_I2C_ADDR, 0x22, 0x00); /* HP-A: No attenuation */
  i2c_write(CS43L22_I2C_ADDR, 0x23, 0x00); /* HP-B: No attenuation */
  
  /* Set master volume */
  cs43l22_set_volume(100); 
  
  /* Power on with all systems enabled */
  i2c_write(CS43L22_I2C_ADDR, CS43L22_REG_POWER_CTL1, 0x9E);
}