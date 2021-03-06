/*  RSound - A PCM audio client/server
 *  Copyright (C) 2010-2011 - Hans-Kristian Arntzen
 * 
 *  RSound is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RSound is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RSound.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "audio.h"
#include "endian.h"
#include <assert.h>

static const int16_t MULAWTable[256] = { 
     -32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956, 
     -23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764, 
     -15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412, 
     -11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316, 
      -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140, 
      -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092, 
      -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004, 
      -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980, 
      -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436, 
      -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924, 
       -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652, 
       -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396, 
       -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260, 
       -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132, 
       -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64, 
        -56,   -48,   -40,   -32,   -24,   -16,    -8,     0, 
      32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956, 
      23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764, 
      15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412, 
      11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316, 
       7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140, 
       5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092, 
       3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004, 
       2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980, 
       1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436, 
       1372,  1308,  1244,  1180,  1116,  1052,   988,   924, 
        876,   844,   812,   780,   748,   716,   684,   652, 
        620,   588,   556,   524,   492,   460,   428,   396, 
        372,   356,   340,   324,   308,   292,   276,   260, 
        244,   228,   212,   196,   180,   164,   148,   132, 
        120,   112,   104,    96,    88,    80,    72,    64, 
         56,    48,    40,    32,    24,    16,     8,     0 
}; 

static const int16_t ALAWTable[256] =  { 
     -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736, 
     -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784, 
     -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368, 
     -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392, 
     -22016,-20992,-24064,-23040,-17920,-16896,-19968,-18944, 
     -30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136, 
     -11008,-10496,-12032,-11520,-8960, -8448, -9984, -9472, 
     -15104,-14592,-16128,-15616,-13056,-12544,-14080,-13568, 
     -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296, 
     -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424, 
     -88,   -72,   -120,  -104,  -24,   -8,    -56,   -40, 
     -216,  -200,  -248,  -232,  -152,  -136,  -184,  -168, 
     -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184, 
     -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696, 
     -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592, 
     -944,  -912,  -1008, -976,  -816,  -784,  -880,  -848, 
      5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736, 
      7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784, 
      2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368, 
      3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392, 
      22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944, 
      30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136, 
      11008, 10496, 12032, 11520, 8960,  8448,  9984,  9472, 
      15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568, 
      344,   328,   376,   360,   280,   264,   312,   296, 
      472,   456,   504,   488,   408,   392,   440,   424, 
      88,    72,   120,   104,    24,     8,    56,    40, 
      216,   200,   248,   232,   152,   136,   184,   168, 
      1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184, 
      1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696, 
      688,   656,   752,   720,   560,   528,   624,   592, 
      944,   912,  1008,   976,   816,   784,   880,   848 
}; 

const char* rsnd_format_to_string(enum rsd_format fmt)
{
   switch(fmt)
   {
      case RSD_S32_LE:
         return "Signed 32-bit little-endian";
      case RSD_S32_BE:
         return "Signed 32-bit big-endian";
      case RSD_U32_LE:
         return "Unsigned 32-bit little-endian";
      case RSD_U32_BE:
         return "Unsigned 32-bit big-endian";
      case RSD_S16_LE:
         return "Signed 16-bit little-endian";
      case RSD_S16_BE:
         return "Signed 16-bit big-endian";
      case RSD_U16_LE:
         return "Unsigned 16-bit little-endian";
      case RSD_U16_BE:
         return "Unsigned 16-bit big-endian";
      case RSD_U8:
         return "Unsigned 8-bit";
      case RSD_S8:
         return "Signed 8-bit";
      case RSD_ALAW:
         return "a-law";
      case RSD_MULAW:
         return "mu-law";
      case RSD_UNSPEC:
         break;
   }
   return "Unknown format";
}

int rsnd_format_to_bytes(enum rsd_format fmt)
{
   switch(fmt)
   {
      case RSD_S32_LE:
      case RSD_S32_BE:
      case RSD_U32_LE:
      case RSD_U32_BE:
         return 4;
      case RSD_S16_LE:
      case RSD_S16_BE:
      case RSD_U16_LE:
      case RSD_U16_BE:
         return 2;
      case RSD_U8:
      case RSD_S8:
      case RSD_ALAW:
      case RSD_MULAW:
         return 1;
      case RSD_UNSPEC:
         break;
   }
   return -1;
}

inline static void swap_bytes16(uint16_t *data, size_t bytes)
{
   for (int i = 0; i < (int)bytes/2; i++)
   {
      swap_endian_16(&data[i]);
   }
}

inline static void swap_bytes32(uint32_t *data, size_t bytes)
{
   for (int i = 0; i < (int)bytes/4; i++)
   {
      swap_endian_32(&data[i]);
   }
}

inline static void s32_to_float(void *data, size_t bytes)
{
   assert(sizeof(float) == 4);
   size_t samples = bytes / sizeof(int32_t);
   union
   {
      float *f;
      int32_t *i;
   } u;
   u.i = data;
   for (int i = samples - 1; i >= 0; i--)
      u.f[i] = (float)u.i[i] / 0x80000000UL;
}

inline static void s16_to_float(void *data, size_t bytes)
{
   assert(sizeof(float) == 4);
   size_t samples = bytes / sizeof(int16_t);
   union 
   {
      float *f;
      int16_t *i;
   } u;
   u.i = data;

   for (int i = samples - 1; i >= 0; i--)
      u.f[i] = (float)u.i[i] / 0x8000;
}

inline static void alaw_to_s16(void *data, size_t bytes)
{
   union
   {
      uint8_t *u8;
      int16_t *i16;
   } u;

   u.u8 = data;

   for ( int i = (int)bytes - 1; i >= 0; i-- )
      u.i16[i] = ALAWTable[u.u8[i]];
}

inline static void mulaw_to_s16(void *data, size_t bytes)
{
   union
   {
      uint8_t *u8;
      int16_t *i16;
   } u;

   u.u8 = data;

   for ( int i = (int)bytes - 1; i >= 0; i-- )
      u.i16[i] = MULAWTable[u.u8[i]];
}

inline static void s8_to_s16(void *data, size_t samples)
{
   union
   {
      int8_t *i8;
      int16_t *i16;
   } u;

   u.i8 = data;

   for ( int i = (int)samples - 1; i >= 0; i-- )
      u.i16[i] = ((int16_t)u.i8[i]) << 8;
}

inline static void s32_to_s16(void *data, size_t samples)
{
   union
   {
      int16_t *i16;
      int32_t *i32;
   } u;
   u.i16 = data;

   for (int i = 0; i < (int)samples; i++)
      u.i16[i] = (int16_t)(u.i32[i] >> 16);
}

void audio_converter(void* data, enum rsd_format fmt, int operation, size_t bytes)
{
   if ( operation == RSD_NULL )
      return;
   
   // Temporarily hold the data that is to be sign-flipped.
   int swapped = 0;
   int bits = rsnd_format_to_bytes(fmt) * 8;

   uint8_t buffer[bytes*2];
   
   // Fancy union to make the conversions more clean looking ;)
   union
   {
      uint8_t *u8;
      int8_t *s8;
      uint16_t *u16;
      int16_t *s16;
      uint32_t *u32;
      int32_t *s32;
      void *ptr;
   } u;

   memcpy(buffer, data, bytes);
   u.ptr = buffer;

   if ( operation & RSD_ALAW_TO_S16 )
   {
      alaw_to_s16(buffer, bytes);
      bytes *= 2;
      fmt = (is_little_endian()) ? RSD_S16_LE : RSD_S16_BE;
   }

   if ( operation & RSD_MULAW_TO_S16 )
   {
      mulaw_to_s16(buffer, bytes);
      bytes *= 2;
      fmt = (is_little_endian()) ? RSD_S16_LE : RSD_S16_BE;
   }

   int i = 0;
   if ( is_little_endian() )
      i++;
   if ( fmt & (RSD_S16_BE | RSD_U16_BE | RSD_S32_BE | RSD_U32_BE) )
      i++;

   // If we're gonna operate on the data, we better make sure that we are in native byte order
   if ( (i % 2) == 0 )
   {
      if (bits == 16)
      {
         swap_bytes16(u.u16, bytes);
         swapped = 1;
      }
      else if (bits == 32)
      {
         swap_bytes32(u.u32, bytes);
         swapped = 1;
      }
   }

   if ( operation & RSD_S_TO_U )
   {
      if ( bits == 8 )
      {
         for ( int i = 0; i < (int)bytes; i++ )
         {
            int32_t temp32 = (u.s8)[i];
            temp32 += (1 << 7);
            (u.u8)[i] = (uint8_t)temp32;
         }
      }

      else if ( bits == 16 )
      {
         for ( int i = 0; i < (int)bytes/2; i++ )
         {
            int32_t temp32 = (u.s16)[i];
            temp32 += (1 << 15);
            (u.u16)[i] = (uint16_t)temp32;
         }
      }

      else if ( bits == 32 )
      {
         for ( int i = 0; i < (int)bytes/4; i++ )
         {
            int64_t temp64 = (u.s32)[i];
            temp64 += (1 << 31);
            (u.u32)[i] = (uint32_t)temp64;
         }
      }
   }

   else if ( operation & RSD_U_TO_S )
   {
      if ( bits == 8 )
      {
         for ( int i = 0; i < (int)bytes; i++ )
         {
            int32_t temp32 = (u.u8)[i];
            temp32 -= (1 << 7);
            (u.s8)[i] = (int8_t)temp32;
         }
      }

      else if ( bits == 16 )
      {
         for ( int i = 0; i < (int)bytes/2; i++ )
         {
            int32_t temp32 = (u.u16)[i];
            temp32 -= (1 << 15);
            (u.s16)[i] = (int16_t)temp32;
         }
      }

      else if ( bits == 32 )
      {
         for ( int i = 0; i < (int)bytes/4; i++ )
         {
            int64_t temp64 = (u.u32)[i];
            temp64 -= (1 << 31);
            (u.s32)[i] = (int32_t)temp64;
         }
      }
   }

   if ( operation & RSD_S8_TO_S16 )
   {
      s8_to_s16(u.ptr, bytes);
      bytes *= 2;
      fmt = (is_little_endian()) ? RSD_S16_LE : RSD_S16_BE;
   }
   else if ( operation & RSD_S32_TO_S16 )
   {
      s32_to_s16(u.ptr, bytes);
      bytes /= 2;
      fmt = (is_little_endian()) ? RSD_S16_LE : RSD_S16_BE;
   }


   if ( operation & RSD_SWAP_ENDIAN )
   {
      if ( !swapped && bits == 16 )
      {
         swap_bytes16(u.u16, bytes);
      }
      else if ( !swapped && bits == 32 )
      {
         swap_bytes32(u.u32, bytes);
      }
   }

   else if ( swapped ) // We need to flip back. Kinda expensive, but hey. An endian-flipping-less algorithm will be more complex. :)
   {
      if ( bits == 16 )
         swap_bytes16(u.u16, bytes);
      else if ( bits == 32 )
         swap_bytes32(u.u32, bytes);
   }

   if ( operation & RSD_S16_TO_FLOAT )
   {
      s16_to_float(u.ptr, bytes);
      bytes *= sizeof(float) / sizeof(int16_t);
   }
   else if ( operation & RSD_S32_TO_FLOAT )
   {
      s32_to_float(u.ptr, bytes);
      bytes *= sizeof(float) / sizeof(int32_t);
   }

   memcpy(data, buffer, bytes);
}

int converter_fmt_to_s16ne(enum rsd_format format)
{
   int conversion = RSD_NULL;
   switch (format)
   {
      case RSD_S32_LE:
         conversion |= RSD_S32_TO_S16;
      case RSD_S16_LE:
         if ( !is_little_endian() )
            conversion |= RSD_SWAP_ENDIAN;
         break;
      case RSD_S32_BE:
         conversion |= RSD_S32_TO_S16;
      case RSD_S16_BE:
         if ( is_little_endian() )
            conversion |= RSD_SWAP_ENDIAN;
         break;
      case RSD_U32_LE:
         conversion |= RSD_S32_TO_S16;
      case RSD_U16_LE:
         conversion |= RSD_U_TO_S;
         if ( !is_little_endian() )
            conversion |= RSD_SWAP_ENDIAN;
         break;
      case RSD_U32_BE:
         conversion |= RSD_S32_TO_S16;
      case RSD_U16_BE:
         conversion |= RSD_U_TO_S;
         if ( is_little_endian() )
            conversion |= RSD_SWAP_ENDIAN;
         break;
      case RSD_U8:
         conversion |= RSD_U_TO_S;
      case RSD_S8:
         conversion |= RSD_S8_TO_S16;
         break;
      case RSD_ALAW:
         conversion |= RSD_ALAW_TO_S16;
         break;
      case RSD_MULAW:
         conversion |= RSD_MULAW_TO_S16;
         break;
      case RSD_UNSPEC:
      default:
         return -1;
   }

   return conversion;
}

int converter_fmt_to_s32ne(enum rsd_format format)
{
   int conversion = RSD_NULL;
   switch (format)
   {
      case RSD_S32_LE:
         if ( !is_little_endian() )
            conversion |= RSD_SWAP_ENDIAN;
         break;
      case RSD_S32_BE:
         if ( is_little_endian() )
            conversion |= RSD_SWAP_ENDIAN;
         break;
      case RSD_U32_LE:
         conversion |= RSD_U_TO_S;
         if ( !is_little_endian() )
            conversion |= RSD_SWAP_ENDIAN;
         break;
      case RSD_U32_BE:
         conversion |= RSD_U_TO_S;
         if ( is_little_endian() )
            conversion |= RSD_SWAP_ENDIAN;
         break;
      default:
         return -1;
   }

   return conversion;
}


#ifdef HAVE_SAMPLERATE
long resample_callback(void *cb_data, float **data)
#else
size_t resample_callback(void *cb_data, float **data)
#endif
{
   resample_cb_state_t *state = cb_data;

   int conversion = RSD_NULL;

   if (rsnd_format_to_bytes(state->format) == 4)
      conversion = converter_fmt_to_s32ne(state->format);
   else
      conversion = converter_fmt_to_s16ne(state->format);
   if (conversion == -1)
   {
      *data = NULL;
      return 0;
   }
   
   assert(sizeof(float) == 4);
   size_t bufsize = sizeof(state->buffer)/sizeof(state->buffer[0]);
   uint32_t buf[bufsize];
   union
   {
      int16_t *i16;
      int32_t *i32;
      void *ptr;
   } inbuffer;
   inbuffer.ptr = buf;

   int channels = state->framesize / rsnd_format_to_bytes(state->format);
   size_t read_size = (bufsize / channels) * state->framesize;

   int rc = receive_data(state->data, state->conn, inbuffer.ptr, read_size);
   if ( rc <= 0 )
   {
      *data = NULL;
      return 0;
   }

   audio_converter(inbuffer.ptr, state->format, conversion, read_size);
#ifdef HAVE_SAMPLERATE
   if (rsnd_format_to_bytes(state->format) == 4)
      src_int_to_float_array(inbuffer.i32, state->buffer, bufsize);
   else
      src_short_to_float_array(inbuffer.i16, state->buffer, bufsize);
#else
   if (rsnd_format_to_bytes(state->format) == 4)
      resampler_s32_to_float(state->buffer, inbuffer.i32, bufsize);
   else
      resampler_s16_to_float(state->buffer, inbuffer.i16, bufsize);
#endif

   *data = state->buffer;
   return rc / state->framesize;
}

