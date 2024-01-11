/*
 * Copyright (c) 2015, EURECOM (www.eurecom.fr)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the FreeBSD Project.
 */

#include "assertions.h"

#ifndef FILE_CONVERSIONS_SEEN
#define FILE_CONVERSIONS_SEEN

/* Endianness conversions for 16 and 32 bits integers from host to network order
 */
#if (BYTE_ORDER == LITTLE_ENDIAN)
#define hton_int32(x)                                                          \
  (((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8) |                        \
   ((x & 0x00FF0000) >> 8) | ((x & 0xFF000000) >> 24))

#define hton_int16(x)                                                          \
    (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8)

#define ntoh_int32_buf(bUF)                                                    \
  ((*(bUF)) << 24) | ((*((bUF) + 1)) << 16) | ((*((bUF) + 2)) << 8) |          \
      (*((bUF) + 3))
#else
#define hton_int32(x) (x)
#define hton_int16(x) (x)
#endif

#define IN_ADDR_TO_BUFFER(X, bUFF) INT32_TO_BUFFER((X).s_addr, (char*) bUFF)

#define IN6_ADDR_TO_BUFFER(X, bUFF)                                            \
  do {                                                                         \
    ((uint8_t*) (bUFF))[0]  = (X).s6_addr[0];                                  \
    ((uint8_t*) (bUFF))[1]  = (X).s6_addr[1];                                  \
    ((uint8_t*) (bUFF))[2]  = (X).s6_addr[2];                                  \
    ((uint8_t*) (bUFF))[3]  = (X).s6_addr[3];                                  \
    ((uint8_t*) (bUFF))[4]  = (X).s6_addr[4];                                  \
    ((uint8_t*) (bUFF))[5]  = (X).s6_addr[5];                                  \
    ((uint8_t*) (bUFF))[6]  = (X).s6_addr[6];                                  \
    ((uint8_t*) (bUFF))[7]  = (X).s6_addr[7];                                  \
    ((uint8_t*) (bUFF))[8]  = (X).s6_addr[8];                                  \
    ((uint8_t*) (bUFF))[9]  = (X).s6_addr[9];                                  \
    ((uint8_t*) (bUFF))[10] = (X).s6_addr[10];                                 \
    ((uint8_t*) (bUFF))[11] = (X).s6_addr[11];                                 \
    ((uint8_t*) (bUFF))[12] = (X).s6_addr[12];                                 \
    ((uint8_t*) (bUFF))[13] = (X).s6_addr[13];                                 \
    ((uint8_t*) (bUFF))[14] = (X).s6_addr[14];                                 \
    ((uint8_t*) (bUFF))[15] = (X).s6_addr[15];                                 \
  } while (0)

#define BUFFER_TO_INT8(buf, x) (x = ((buf)[0]))

#define INT8_TO_BUFFER(x, buf) ((buf)[0] = (x))

/* Convert an integer on 16 bits to the given bUFFER */
#define INT16_TO_BUFFER(x, buf)                                                \
  do {                                                                         \
    (buf)[0] = (x) >> 8;                                                       \
    (buf)[1] = (x);                                                            \
  } while (0)

/* Convert an array of char containing vALUE to x */
#define BUFFER_TO_INT16(buf, x)                                                \
  do {                                                                         \
    x = ((buf)[0] << 8) | ((buf)[1]);                                          \
  } while (0)

/* Convert an integer on 24 bits to the given bUFFER */
#define INT24_TO_BUFFER(x, buf)                                                \
  do {                                                                         \
    (buf)[0] = (x) >> 16;                                                      \
    (buf)[1] = (x) >> 8;                                                       \
    (buf)[2] = (x);                                                            \
  } while (0)

#define BUFFER_TO_INT24(buf, x)                                                                             \
  do {                                                                                                      \
    x = (int32_t) (((uint32_t) ((buf)[0]) << 16) | ((uint32_t) ((buf)[1]) << 8) | ((uint32_t) ((buf)[2]))); \
  } while (0)

/* Convert an integer on 32 bits to the given bUFFER */
#define INT32_TO_BUFFER(x, buf)                                                \
  do {                                                                         \
    (buf)[0] = (x) >> 24;                                                      \
    (buf)[1] = (x) >> 16;                                                      \
    (buf)[2] = (x) >> 8;                                                       \
    (buf)[3] = (x);                                                            \
  } while (0)

/* Convert an array of char containing vALUE to x */
#define BUFFER_TO_INT32(buf, x)                                                \
  do {                                                                         \
    x = ((buf)[0] << 24) | ((buf)[1] << 16) | ((buf)[2] << 8) | ((buf)[3]);    \
  } while (0)

/* Convert an integer on 32 bits to an octet string from aSN1c tool */
#define INT32_TO_OCTET_STRING(x, aSN)                                          \
  do {                                                                         \
    (aSN)->buf = (uint8_t*) calloc(4, sizeof(uint8_t));                        \
    INT32_TO_BUFFER(x, ((aSN)->buf));                                          \
    (aSN)->size = 4;                                                           \
  } while (0)

#define INT32_TO_BIT_STRING(x, aSN)                                            \
  do {                                                                         \
    INT32_TO_OCTET_STRING(x, aSN);                                             \
    (aSN)->bits_unused = 0;                                                    \
  } while (0)

#define INT24_TO_BIT_STRING(x, aSN, s)                                         \
  do {                                                                         \
    INT24_TO_OCTET_STRING(x, aSN);                                             \
    (aSN)->bits_unused = 24 - s;                                               \
  } while (0)

#define INT24_TO_OCTET_STRING(x, aSN)                                          \
  do {                                                                         \
    (aSN)->buf  = (uint8_t*) calloc(3, sizeof(uint8_t));                       \
    (aSN)->size = 3;                                                           \
    INT24_TO_BUFFER(x, (aSN)->buf);                                            \
  } while (0)

#define OCTET_STRING_TO_INT24(aSN, x)                                          \
  do {                                                                         \
    DevCheck((aSN)->size == 3, (aSN)->size, 0, 0);                             \
    BUFFER_TO_INT24((aSN)->buf, x);                                            \
  } while (0)

#define INT16_TO_OCTET_STRING(x, aSN)                                          \
  do {                                                                         \
    (aSN)->buf  = calloc(2, sizeof(uint8_t));                                  \
    (aSN)->size = 2;                                                           \
    INT16_TO_BUFFER(x, (aSN)->buf);                                            \
  } while (0)

#define INT16_TO_BIT_STRING(x, aSN)                                            \
  do {                                                                         \
    (aSN)->buf         = calloc(2, sizeof(uint8_t));                           \
    (aSN)->size        = 2;                                                    \
    (aSN)->bits_unused = 0;                                                    \
  } while (0)

#define INT8_TO_OCTET_STRING(x, aSN)                                           \
  do {                                                                         \
    (aSN)->buf  = calloc(1, sizeof(uint8_t));                                  \
    (aSN)->size = 1;                                                           \
    INT8_TO_BUFFER(x, (aSN)->buf);                                             \
  } while (0)

#define MME_CODE_TO_OCTET_STRING INT8_TO_OCTET_STRING
#define M_TMSI_TO_OCTET_STRING INT32_TO_OCTET_STRING
#define MME_GID_TO_OCTET_STRING INT16_TO_OCTET_STRING

#define OCTET_STRING_TO_INT8(aSN, x)                                           \
  do {                                                                         \
    DevCheck((aSN)->size == 1, (aSN)->size, 0, 0);                             \
    BUFFER_TO_INT8((aSN)->buf, x);                                             \
  } while (0)

#define OCTET_STRING_TO_INT16(aSN, x)                                          \
  do {                                                                         \
    DevCheck((aSN)->size == 2, (aSN)->size, 0, 0);                             \
    BUFFER_TO_INT16((aSN)->buf, x);                                            \
  } while (0)

#define OCTET_STRING_TO_INT32(aSN, x)                                          \
  do {                                                                         \
    DevCheck((aSN)->size == 4, (aSN)->size, 0, 0);                             \
    BUFFER_TO_INT32((aSN)->buf, x);                                            \
  } while (0)

#define BIT_STRING_TO_INT32(aSN, x)                                            \
  do {                                                                         \
    DevCheck((aSN)->bits_unused == 0, (aSN)->bits_unused, 0, 0);               \
    OCTET_STRING_TO_INT32(aSN, x);                                             \
  } while (0)

#define BIT_STRING_TO_INT24(aSN, x)                                            \
  do {                                                                         \
    OCTET_STRING_TO_INT24(aSN, x);                                             \
  } while (0)

#define BIT_STRING_TO_CELL_IDENTITY(aSN, vALUE)                                \
  do {                                                                         \
    DevCheck((aSN)->bits_unused == 4, (aSN)->bits_unused, 4, 0);               \
    vALUE.enb_id =                                                             \
        ((aSN)->buf[0] << 12) | ((aSN)->buf[1] << 4) | ((aSN)->buf[2] >> 4);   \
    vALUE.cell_id = ((aSN)->buf[2] << 4) | ((aSN)->buf[3] >> 4);               \
  } while (0)

#define MCC_HUNDREDS(vALUE) ((vALUE) / 100)
/* When MNC is only composed of 2 digits, set the hundreds unit to 0xf */
#define MNC_HUNDREDS(vALUE, mNCdIGITlENGTH)                                    \
  (mNCdIGITlENGTH == 2 ? 15 : (vALUE) / 100)
#define MCC_MNC_DECIMAL(vALUE) (((vALUE) / 10) % 10)
#define MCC_MNC_DIGIT(vALUE) ((vALUE) % 10)

#define MCC_TO_BUFFER(mCC, bUFFER)                                             \
  do {                                                                         \
    DevAssert(bUFFER != NULL);                                                 \
    (bUFFER)[0] = MCC_HUNDREDS(mCC);                                           \
    (bUFFER)[1] = MCC_MNC_DECIMAL(mCC);                                        \
    (bUFFER)[2] = MCC_MNC_DIGIT(mCC);                                          \
  } while (0)

/* Used to format an uint32_t containing an ipv4 address */
#define IPV4_ADDR "%u.%u.%u.%u"
#define IPV4_ADDR_FORMAT(aDDRESS)                                              \
  (uint8_t)((aDDRESS) &0x000000ff), (uint8_t) (((aDDRESS) &0x0000ff00) >> 8),  \
      (uint8_t) (((aDDRESS) &0x00ff0000) >> 16),                               \
      (uint8_t) (((aDDRESS) &0xff000000) >> 24)

#define IPV4_ADDR_DISPLAY_8(aDDRESS)                                           \
  (aDDRESS)[0], (aDDRESS)[1], (aDDRESS)[2], (aDDRESS)[3]

#endif /* FILE_CONVERSIONS_SEEN */
