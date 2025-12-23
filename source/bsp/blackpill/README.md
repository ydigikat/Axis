## Hardware Configuration

### Clock Setup
- System clock: 100MHz (using 25MHz HSE with PLL)
- APB1: 50MHz
- APB2: 100MHz
- I2S PLL configured based on requested sample rate

### Performance
- ART (Adaptive Real-Time) memory accelerator is enabled with prefetch and instruction/data caches


### Audio Interface
- I2S2 peripheral in master transmit mode
- 32-bit data format, Philips standard
- DMA1 Stream4 configured for circular buffer operation
- GPIO pins: MCK(PA3), CK(PB10), WS(PB12), SDO(PB15)

### MIDI Interface
- USART1 in receive-only mode
- 31250 baud, 8N1 configuration
- RX pin: PA10

### Debug and Development
- User LED on PC13 (active low, open-drain)
- User button on PA0 (with pull-up)
- Debug probe pins on PB2-PB9

