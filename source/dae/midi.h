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
#ifndef __MIDI_H__
#define __MIDI_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIDI_OMNI 17 
#define MIDI_BUFFER_SIZE 16

struct midi_msg
{
  size_t len;     
  uint8_t data[3]; 
};

struct midi_port
{
  struct midi_msg *msg;  
  uint8_t channel;        
  uint8_t running_status;  
  bool third_byte_expected; 
  bool sysex_active;
};

enum midi_status_byte
{
  MIDI_STATUS_INVALID = 0x00,          
  MIDI_STATUS_NOTE_OFF = 0x80,         
  MIDI_STATUS_NOTE_ON = 0x90,          
  MIDI_STATUS_POLY_PRESSURE = 0xA0,    
  MIDI_STATUS_CONTROL_CHANGE = 0xB0,   
  MIDI_STATUS_PROGRAM_CHANGE = 0xC0,   
  MIDI_STATUS_CHANNEL_PRESSURE = 0xD0, 
  MIDI_STATUS_PITCH_BEND = 0xE0,       
  MIDI_STATUS_SYS_EX_START = 0xF0,     
  MIDI_STATUS_TIME_CODE = 0xF1,        
  MIDI_STATUS_SONG_POS = 0xF2,         
  MIDI_STATUS_SONG_SELECT = 0xF3,      
  MIDI_STATUS_TUNE_REQUEST = 0xF6,     
  MIDI_STATUS_SYS_EX_END = 0xF7,       
  MIDI_STATUS_CLOCK = 0xF8,            
  MIDI_STATUS_TICK = 0xF9,             
  MIDI_STATUS_START = 0xFA,            
  MIDI_STATUS_CONTINUE = 0xFB,         
  MIDI_STATUS_STOP = 0xFC,             
  MIDI_STATUS_ACTIVE_SENSE = 0xFE,     
  MIDI_STATUS_SYS_RESET = 0xFF         
};

enum midi_cc
{
  MIDI_CC_BANKSELECT = 0,                 
  MIDI_CC_MODULATIONWHEEL = 1,            
  MIDI_CC_BREATHCONTROLLER = 2,           
  MIDI_CC_FOOTCONTROLLER = 4,             
  MIDI_CC_PORTAMENTOTIME = 5,             
  MIDI_CC_DATAENTRYMSB = 6,               
  MIDI_CC_VOLUME = 7,                     
  MIDI_CC_BALANCE = 8,                    
  MIDI_CC_PAN = 10,                       
  MIDI_CC_EXPRESSIONCONTROLLER = 11,      
  MIDI_CC_EFFECTCONTROL1 = 12,            
  MIDI_CC_EFFECTCONTROL2 = 13,            
  MIDI_CC_GENERALPURPOSECONTROLLER1 = 16, 
  MIDI_CC_GENERALPURPOSECONTROLLER2 = 17, 
  MIDI_CC_GENERALPURPOSECONTROLLER3 = 18, 
  MIDI_CC_GENERALPURPOSECONTROLLER4 = 19, 

  MIDI_CC_HOLDPEDAL = 64,                
  MIDI_CC_PORTAMENTO = 65,               
  MIDI_CC_SOSTENUTO = 66,                
  MIDI_CC_SOFTPEDAL = 67,                
  MIDI_CC_LEGATO = 68,                   
  MIDI_CC_HOLD2 = 69,                    
  MIDI_CC_SOUNDVARIATION = 70,           
  MIDI_CC_RESONANCE = 71,                
  MIDI_CC_RELEASETIME = 72,              
  MIDI_CC_ATTACKTIME = 73,               
  MIDI_CC_FREQUENCYCUTOFF = 74,          
  MIDI_CC_SOUNDCONTROLLER6 = 75,         
  MIDI_CC_SOUNDCONTROLLER7 = 76,         
  MIDI_CC_SOUNDCONTROLLER8 = 77,         
  MIDI_CC_SOUNDCONTROLLER9 = 78,         
  MIDI_CC_SOUNDCONTROLLER10 = 79,        
  MIDI_CC_DECAYTIME = 80,                
  MIDI_CC_HIGHPASSFREQUENCY = 81,        
  MIDI_CC_GENERALPURPOSECONTROLLER7 = 82,
  MIDI_CC_GENERALPURPOSECONTROLLER8 = 83,
  MIDI_CC_PORTAMENTOAMOUNT = 84,         

  MIDI_CC_REVERBLEVEL = 91,               
  MIDI_CC_TREMOLOLEVEL = 92,              
  MIDI_CC_CHORUSLEVEL = 93,               
  MIDI_CC_DETUNELEVEL = 94,               
  MIDI_CC_PHASERLEVEL = 95,               
  MIDI_CC_DATAINCREMENT = 96,          
  MIDI_CC_DATADECREMENT = 97,          
  MIDI_CC_NRPNLSB = 98,                   
  MIDI_CC_NRPNMSB = 99,                   
  MIDI_CC_RPNLSB = 100,                   
  MIDI_CC_RPNMSB = 101,                   

  MIDI_CC_ALLSOUNDOFF = 120,           
  MIDI_CC_RESETALLCONTROLLERS = 121,   
  MIDI_CC_LOCALCONTROL = 122,          
  MIDI_CC_ALLNOTESOFF = 123,           
  MIDI_CC_OMNIMODEOFF = 124,           
  MIDI_CC_OMNIMODEON = 125,            
  MIDI_CC_MONOMODEON = 126,            
  MIDI_CC_POLYMODEON = 127,            
  MIDI_CC_UNSUPPORTED = 128            
};


extern const float MIDI_FREQ_TABLE[128];

struct midi_msg* midi_parse(struct midi_port *in, uint8_t byte);
void midi_buffer_write(uint8_t byte);
bool midi_buffer_read(uint8_t* byte);
float midi_to_attenuation(uint32_t midi_value);
float attenuation_to_midi(float atten);

#endif /* __MIDI_H__ */