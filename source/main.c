/*
   MIT License
   Copyright (c) 2025 Jason Wilden

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/

#include "FreeRTOS.h"
#include "task.h"

#include "trace.h"
#include "ui.h"
#include "dae.h"


/* Import the hardware initialisation function */
extern bool init(void);

int main(void)
{
    init();    

    /* Create the user interface thread */
    if (!ui_start(tskIDLE_PRIORITY + 1))
    {
        RTT_LOG("UI task failed to start\n");
        while (1)
            ;
    }

    /* Create the digital audio engine thread, this has a higher priority than the UI.*/
    if(!dae_start(tskIDLE_PRIORITY + 5))
    {
        RTT_LOG("DAE task failed to start\n");
        while (1)
            ;
    }

    /* Start the task scheduler, this should block */
    vTaskStartScheduler();

    /* We shouldn't get here, so the scheduler must have failed to start. */
    RTT_LOG("%sFailed to start scheduler\n", RTT_CTRL_TEXT_BRIGHT_RED);
    while (1)
        ;

    return 0;
}

/* This will be called if RTOS detects that we're out of stack space for threads, this
   is only used if the corresponding configuration item is set in FreeRTOSConfig.h  */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    RTT_LOG("%sStack overflow, task:%s\n", RTT_CTRL_TEXT_BRIGHT_RED, pcTaskName);
    while (1)
        ;
}

/* Callbacks to connect DAE to audio generator */

/**
 * \brief called once by the DAE at start up
 * \param the the sample rate 
 * \param block_size the size of the sample blocks
 * \param midi_channel the default midi channel, 1-16 or OMNI
 */
void dae_prepare_for_play(float sample_rate, size_t block_size, uint8_t *midi_channel)
{
  /* Call the relevant function in your audio generator */
}

/**
 * \brief a MIDI message needs to be handled.  These are always complete and validated messages.
 * \midi_msg the MIDI message structure.  These include realtime messages.
 */
void dae_handle_midi(struct midi_msg *msg)
{  
  /* Call the relevant function in your audio generator */
}

/**
 * \brief called every time a new audio block needs to be generated, the block size is the
 *        value passed in the call dae_prepare_for_play()
 * \param left the left channel (samples)
 * \param right the right channel (samples)
 */
// void dae_process_block(float *left, float *right)
// { 
//   /* Call the relevant function in your audio generator */
// }

