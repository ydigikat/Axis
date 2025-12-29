/*
   MIT License
   Copyright (c) 2025 Jason Wilden

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/
#include "dae.h"

/* Default configuration, these can be overriden in the CMakeLists.txt file */
#ifndef DAE_SAMPLE_RATE
#define DAE_SAMPLE_RATE (48000)
#endif

#ifndef DAE_AUDIO_BLOCK_SIZE
#define DAE_AUDIO_BLOCK_SIZE (128)
#endif

#define DAE_AUDIO_BUFFER_SIZE (DAE_AUDIO_BLOCK_SIZE * 8)

/* Sample and audio buffers */
static float left_buffer[DAE_AUDIO_BLOCK_SIZE];
static float right_buffer[DAE_AUDIO_BLOCK_SIZE];
static int16_t audio_buffer[DAE_AUDIO_BUFFER_SIZE];

/* State variables */
static uint8_t active_buffer = PONG;
static TaskHandle_t dae_task_handle;
static struct midi_port midi_in;

/* Imported functions */
void audio_start(int16_t audio_buffer[], size_t buf_len, uint32_t sample_rate);

/* Private functions */
static void check_buffer(float *buffer, int sampleCount);
static void generate_test_tone(float *restrict left, float *restrict right, size_t block_size);

/**
 * dae_task
 * \brief This is the main audio processing thread (task)
 * \param pvParameters - unused
 */
static void dae_task(void *pvParameters)
{
  /* Starts the board audio subsystem (I2S and DMA peripherals) */
  audio_start(audio_buffer, DAE_AUDIO_BUFFER_SIZE, DAE_SAMPLE_RATE);  


  /* Configures the sound source for playing, passing it DAE parameters and obtaining the MIDI channel */
  dae_prepare_for_play(DAE_SAMPLE_RATE,DAE_AUDIO_BLOCK_SIZE, &midi_in.channel);

  while (1)
  {
    /* Sleep until the DMA signals us to refresh a buffer */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

     /* Pass the synthesiser any buffered MIDI at the start of the block, this is block level and
       messages are not time-stamped so MIDI slicing is not possible. */
    uint8_t byte;
    while (midi_buffer_read(&byte))
    {      
      struct midi_msg *msg = midi_parse(&midi_in, byte);
      if (msg != NULL)
      {
        dae_handle_midi(msg);
      }
    }

    /* Call audio source to generate the audio block */
    dae_process_block(left_buffer, right_buffer);

    /* Select the buffer to which audio is output */
    int16_t *restrict ptr = (active_buffer == PING) ? audio_buffer : audio_buffer + DAE_AUDIO_BUFFER_SIZE / 2;

/* Copy samples to audio buffer in I2S required format */
#pragma GCC unroll 4
    for (int i = 0; i < DAE_AUDIO_BLOCK_SIZE; i++)
    {
      int32_t l_sample = left_buffer[i] * INT32_MAX;
      int32_t r_sample = right_buffer[i] * INT32_MAX;

      *ptr++ = (int16_t)(r_sample >> 16);
      *ptr++ = (int16_t)(r_sample);
      *ptr++ = (int16_t)(l_sample >> 16);
      *ptr++ = (int16_t)(l_sample);
    }
  }
}

/**
 * dae_start
 * \brief This kicks off the DAE thread (RTOS task)
 * \param priority The FreeRTOS task priority.
 */
bool dae_start(UBaseType_t priority)
{
  if (xTaskCreate(dae_task, "DAE", configMINIMAL_STACK_SIZE * 4, NULL, priority, &dae_task_handle) != pdPASS)
  {
    return false;
  }

  return true;
}


/**
 * dae_ready_for_audio
 * \brief called by the audio hardware interrupt when a new buffer of audio sample is required.
 * \param buffer_idx the buffer to refill (PING or PONG)
 * \note This ISR wakes a sleeping task (context switch) so RTOS interrupt protection/handling is required.
 */
void dae_ready_for_audio(uint8_t buffer_idx)
{
  BaseType_t higher_task_woken = pdFALSE;

  active_buffer = buffer_idx;

  /* Notify the DAE task that it is ready to process audio */
  vTaskNotifyGiveFromISR(dae_task_handle, &higher_task_woken);

  /* If the DAE task has higher priority than the current interrupted task, yield */
  if (higher_task_woken)
  {
    portYIELD_FROM_ISR(higher_task_woken);
  }
}


/**
 * dae_midi_received
 * \brief called by the hardware when a MIDI byte is received.  The DAE adds these to the
 *        MIDI ring_buffer for processing at the start of the next audio block.
 * \param byte the data byte.
 * \note  This ISR does not use RTOS or yield. The MIDI buffer is a single-producer (ISR), 
 *        single-consumer (task) lock-free ring buffer where the ISR only updates the write index
 *        and the task only updates the read index. All accesses are atomic,
 *        so no RTOS or critical section protection is required.
 */
void dae_midi_received(uint8_t byte)
{   
  if (byte == MIDI_STATUS_ACTIVE_SENSE)
  {
    return;
  }

  // RTT_LOG("Storing byte %d\n",byte);

  /* Write the byte to the MIDI ring-buffer */
  midi_buffer_write(byte);  
}



/**
 * dae_prepare_to_play
 * \brief called by the DAE when it is starting the audio task.  
 * \param sample_rate the sample rate
 * \param block_size the audio block size for dynamic buffer allocation etc. 
 * \param midi_channel the midi channel that the synthesiser wants to use.
 */
__attribute__((weak)) void dae_prepare_for_play(float sample_rate, size_t block_size, uint8_t *midi_channel)
{
  /* Override this in your audio generator */
}

/**
 * dae_handle_midi()
 * \brief called by DAE to pass MIDI data to then synthesiser, this can be called multiple times at the
 *        start of a block.
 * \param msg a valid parsed MIDI message.
 */
__attribute__((weak)) void dae_handle_midi(struct midi_msg *msg)
{
  /* Override this to handle MIDI messages in your synthesiser*/
}


/**
 * dae_process_block()
 * \brief called by the DAE when it requires a new block of samples
 * \param left the left sample buffer
 * \param right the right sample buffer 
 */
__attribute__((weak)) void dae_process_block(float *left, float *right)
{
  /* Override this in your audio generator, the default call will generate a 440Hz continuous sine tone */
  generate_test_tone(left, right, DAE_AUDIO_BLOCK_SIZE);
}


/* Coefficients for test tone generator */
static const float test_tone_b_coeff = 1.27323954474f;
static const float test_tone_c_coeff = -0.40528473456f;
static const float test_tone_p_coeff = 0.225f;
static float test_tone_phase = 0;
static float test_tone_inc = 440.0f / DAE_SAMPLE_RATE;

/**
 * generate_test_tone();
 * \brief This generates a sine approximation at 440Hz for testing.
 * \param left buffer
 * \param right buffer
 * \param block_size size of buffer
 */
static void generate_test_tone(float *restrict left, float *restrict right, size_t block_size)
{
  RTT_ASSERT(left != NULL);
  RTT_ASSERT(right != NULL);
  RTT_ASSERT(block_size > 0);

  for (size_t i = 0; i < block_size; i++)
  {
    if (test_tone_phase > 1.0f)
    {
      test_tone_phase -= 1.0f;
    }

    float angle = -1.0f * (test_tone_phase * 2.0f * 3.14159265f - 3.14159265f);
    float y = test_tone_b_coeff * angle + test_tone_c_coeff * angle * fabsf(angle);

    left[i] = (test_tone_p_coeff * (y * fabsf(y) - y) + y);
    right[i] = left[i];
    test_tone_phase += test_tone_inc;
  }
}
