/*
   MIT License
   Copyright (c) 2025 Jason Wilden

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/
#ifndef DAE_H
#define DAE_H

#include <math.h>
#include <stdalign.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "midi.h"

#include "trace.h"

#define PING (0)
#define PONG (1)

/* Control plane API */
bool dae_start(UBaseType_t priority);
void dae_ready_for_audio(uint8_t buffer_idx);


/* Data plane callback functions */
void dae_prepare_for_play(float sample_rate, size_t block_size, uint8_t *channel);
void dae_process_block(float *left, float *right);
void dae_handle_midi(struct midi_msg *msg);

#endif /* DAE_H */