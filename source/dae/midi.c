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
#include "midi.h"

#define IS_STATUS_BYTE(__BYTE__) ((__BYTE__ >> 7) == 0x1)
#define IS_REAL_TIME(__BYTE__) ((__BYTE__ >> 3) == 0x1f)
#define IS_SINGLE_BYTE_MSG(__BYTE__) ((__BYTE__ >> 2) == 0x3D)
#define CHANNEL_MASK (0x0F)


struct midi_ring_buffer
{
  uint8_t buffer[MIDI_BUFFER_SIZE];
  size_t head;
  size_t tail;
};

static struct midi_msg rt_msg;
static struct midi_msg midi_msg;
static struct midi_ring_buffer midi_buffer = {{0}, 0, 0};
static struct midi_msg *midi_parse_rt_msg(struct midi_port *midi_in, uint8_t byte);
static bool midi_parse_message(struct midi_port *midi_in, uint8_t byte);

/**
 * \brief Parse a MIDI byte and return a MIDI message
 * \param midi_in MIDI port data structure
 * \param byte MIDI byte to parse
 * \return MIDI message if a complete message is parsed, NULL otherwise.
 */
struct midi_msg *midi_parse(struct midi_port *midi_in, uint8_t byte)
{
  struct midi_msg *msg = NULL;

  msg = midi_parse_rt_msg(midi_in, byte);

  if (msg == NULL)
  {
    if (midi_parse_message(midi_in, byte))
    {
      msg = midi_in->msg;
      midi_in->msg = NULL;
    }
  }

  return msg;
}


static void midi_reset_msg(struct midi_msg *msg)
{
  msg->len = 0;
  msg->data[0] = 0;
  msg->data[1] = 0;
  msg->data[2] = 0;
}

/**
 * \brief Parse a real-time MIDI message
 * \param midi_in MIDI port data structure
 * \param byte MIDI byte to parse
 * \return MIDI message if a complete real-time message is parsed, NULL otherwise.
 * \note Real-time messages are single byte messages that can be sent at any time.
 * They are not part of the MIDI stream and can be sent between any other messages.
 * This function does not intefere with the state of any incomplete MIDI message.
 */
static struct midi_msg *midi_parse_rt_msg(struct midi_port *midi_in, uint8_t byte)
{
  struct midi_msg *msg = NULL;

  if (IS_STATUS_BYTE(byte) && IS_REAL_TIME(byte))
  {
    rt_msg.data[0] = byte;
    rt_msg.len = 1;
    msg = &rt_msg;
  }

  return msg;
}

/**
 * \brief parses a midi message incrementally from the passed in bytes
 * \param midi_in the MIDI port (state)
 * \param byte data byte
 * \return true if the message is complete and parsed.
 */
static bool midi_parse_message(struct midi_port *midi_in, uint8_t byte)
{
  struct midi_msg *msg = NULL;
  uint8_t running_status;

  if (midi_in->msg == NULL)
  {
    midi_in->msg = &midi_msg;
    midi_reset_msg(midi_in->msg);
  }

  running_status = midi_in->running_status;
  msg = midi_in->msg;

  if (midi_in->sysex_active)
  {
    if (byte == MIDI_STATUS_SYS_EX_END)
    {
      midi_in->sysex_active = false;
    }
    return false;
  }

  if (IS_STATUS_BYTE(byte))
  {
    midi_in->running_status = byte;
    midi_in->third_byte_expected = false;

    if (byte == MIDI_STATUS_SYS_EX_START)
    {
      midi_in->sysex_active = true;
      return false;
    }

    if (IS_SINGLE_BYTE_MSG(byte))
    {
      midi_in->msg->data[0] = byte;
      midi_in->msg->len = 1;
      return true;
    }

    return false;
  }


  if (midi_in->channel != MIDI_OMNI && (running_status & CHANNEL_MASK) != (midi_in->channel - 1))
  {
    return false;
  }

  if (midi_in->third_byte_expected)
  {
    midi_in->third_byte_expected = false;
    msg->data[2] = byte;
    msg->len = 3;
    return true;
  }

  if (running_status == MIDI_STATUS_INVALID)
  {
    return false;
  }

  switch (running_status)
  {
  case MIDI_STATUS_NOTE_ON:
  case MIDI_STATUS_NOTE_OFF:
  case MIDI_STATUS_CONTROL_CHANGE:
  case MIDI_STATUS_PITCH_BEND:
  case MIDI_STATUS_POLY_PRESSURE:
    midi_in->third_byte_expected = true;
    msg->data[0] = running_status;
    msg->data[1] = byte;
    msg->len = 2;
    return false;

  case MIDI_STATUS_PROGRAM_CHANGE:
  case MIDI_STATUS_CHANNEL_PRESSURE:
    midi_in->third_byte_expected = false;
    msg->data[0] = running_status;
    msg->data[1] = byte;
    msg->len = 2;
    return true;

  case MIDI_STATUS_SONG_POS:
    midi_in->third_byte_expected = true;
    msg->data[0] = running_status;
    msg->data[1] = byte;
    msg->len = 2;
    midi_in->running_status = MIDI_STATUS_INVALID;
    return false;

  case MIDI_STATUS_SONG_SELECT:
  case MIDI_STATUS_TIME_CODE:
    midi_in->third_byte_expected = true;
    msg->data[0] = running_status;
    msg->data[1] = byte;
    msg->len = 2;
    midi_in->running_status = MIDI_STATUS_INVALID;
    return true;
  }

  midi_in->running_status = MIDI_STATUS_INVALID;
  return false;
}

/**
 * \brief Write a byte to the MIDI buffer
 * \param byte Byte to write to the buffer
 * \note This function is called by the MIDI driver to write incoming MIDI bytes to the buffer.
 */
void midi_buffer_write(uint8_t byte)
{
  uint16_t nextHead = (midi_buffer.head + 1) % MIDI_BUFFER_SIZE;

  if (nextHead != midi_buffer.tail)
  {
    midi_buffer.buffer[midi_buffer.head] = byte;
    midi_buffer.head = nextHead;
  }
}

/**
 * \brief Read a byte from the MIDI buffer
 * \param data Pointer to a byte to store the read data
 * \return true if a byte was read, false otherwise.
 * \note This function is called by the DAE to read MIDI bytes from the buffer.
 */
bool midi_buffer_read(uint8_t *data)
{
  if (midi_buffer.head != midi_buffer.tail)
  {
    *data = midi_buffer.buffer[midi_buffer.tail];
    midi_buffer.tail = (midi_buffer.tail + 1) % MIDI_BUFFER_SIZE;
    return true;
  }
  return false;
}


float midi_to_attenuation(uint32_t midi_value)
{
  if (midi_value == 0)
    return 0;

  return ((float)midi_value * (float)midi_value) / (127.0f * 127.0f);
}

float attenuation_to_midi(float atten)
{
  if (atten == 0)
    return 0;

  return sqrtf(127.0f * atten);
}

/**
 * \brief LUT to convert a MIDI note number to a frequency
 */ 
const float MIDI_FREQ_TABLE[128] =
    {
        8.1757993698120117, 8.6619567871093750,
        9.1770238876342773, 9.7227182388305664, 10.3008613586425780,
        10.9133825302124020, 11.5623254776000980, 12.2498569488525390,
        12.9782714843750000, 13.7500000000000000, 14.5676174163818360,
        15.4338531494140630, 16.3515987396240230, 17.3239135742187500,
        18.3540477752685550, 19.4454364776611330, 20.6017227172851560,
        21.8267650604248050, 23.1246509552001950, 24.4997138977050780,
        25.9565429687500000, 27.5000000000000000, 29.1352348327636720,
        30.8677062988281250, 32.7031974792480470, 34.6478271484375000,
        36.7080955505371090, 38.8908729553222660, 41.2034454345703130,
        43.6535301208496090, 46.2493019104003910, 48.9994277954101560,
        51.9130859375000000, 55.0000000000000000, 58.2704696655273440,
        61.7354125976562500, 65.4063949584960940, 69.2956542968750000,
        73.4161911010742190, 77.7817459106445310, 82.4068908691406250,
        87.3070602416992190, 92.4986038208007810, 97.9988555908203130,
        103.8261718750000000, 110.0000000000000000, 116.5409393310546900,
        123.4708251953125000, 130.8127899169921900, 138.5913085937500000,
        146.8323822021484400, 155.5634918212890600, 164.8137817382812500,
        174.6141204833984400, 184.9972076416015600, 195.9977111816406200,
        207.6523437500000000, 220.0000000000000000, 233.0818786621093700,
        246.9416503906250000, 261.6255798339843700, 277.1826171875000000,
        293.6647644042968700, 311.1269836425781200, 329.6275634765625000,
        349.2282409667968700, 369.9944152832031200, 391.9954223632812500,
        415.3046875000000000, 440.0000000000000000, 466.1637573242187500,
        493.8833007812500000, 523.2511596679687500, 554.3652343750000000,
        587.3295288085937500, 622.2539672851562500, 659.2551269531250000,
        698.4564819335937500, 739.9888305664062500, 783.9908447265625000,
        830.6093750000000000, 880.0000000000000000, 932.3275146484375000,
        987.7666015625000000, 1046.5023193359375000, 1108.7304687500000000,
        1174.6590576171875000, 1244.5079345703125000, 1318.5102539062500000,
        1396.9129638671875000, 1479.9776611328125000, 1567.9816894531250000,
        1661.2187500000000000, 1760.0000000000000000, 1864.6550292968750000,
        1975.5332031250000000, 2093.0046386718750000, 2217.4609375000000000,
        2349.3181152343750000, 2489.0158691406250000, 2637.0205078125000000,
        2793.8259277343750000, 2959.9553222656250000, 3135.9633789062500000,
        3322.4375000000000000, 3520.0000000000000000, 3729.3100585937500000,
        3951.0664062500000000, 4186.0092773437500000, 4434.9218750000000000,
        4698.6362304687500000, 4978.0317382812500000, 5274.0410156250000000,
        5587.6518554687500000, 5919.9106445312500000, 6271.9267578125000000,
        6644.8750000000000000, 7040.0000000000000000, 7458.6201171875000000,
        7902.1328125000000000, 8372.0185546875000000, 8869.8437500000000000,
        9397.2724609375000000, 9956.0634765625000000, 10548.0820312500000000,
        11175.3037109375000000, 11839.8212890625000000, 12543.8535156250000000};
