/*
  ------------------------------------------------------------------------------
   SynthCoreF4
   Author: ydigikat
  ------------------------------------------------------------------------------
   MIT License
   Copyright (c) 2025 YDigiKat

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
  ------------------------------------------------------------------------------
*/
#ifndef __UI_H__
#define __UI_H__

#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "board.h"

bool ui_start(UBaseType_t priority);



#endif /* __UI_H__ */