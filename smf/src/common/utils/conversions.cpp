/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the
 * License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file conversions.cpp
 \brief
 \author Sebastien ROUX
 \company Eurecom
 */
#include "conversions.hpp"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "logger.hpp"

extern "C" {
#include "dynamic_memory_check.h"
}

static const char hex_to_ascii_table[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static const signed char ascii_to_hex_table[0x100] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,
    9,  -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1};

void conv::hexa_to_ascii(uint8_t* from, char* to, size_t length) {
  size_t i;

  for (i = 0; i < length; i++) {
    uint8_t upper = (from[i] & 0xf0) >> 4;
    uint8_t lower = from[i] & 0x0f;

    to[2 * i]     = hex_to_ascii_table[upper];
    to[2 * i + 1] = hex_to_ascii_table[lower];
  }
}

int conv::ascii_to_hex(uint8_t* dst, const char* h) {
  const unsigned char* hex = (const unsigned char*) h;
  unsigned i               = 0;

  for (;;) {
    int high, low;

    while (*hex && isspace(*hex)) hex++;

    if (!*hex) return 1;

    high = ascii_to_hex_table[*hex++];

    if (high < 0) return 0;

    while (*hex && isspace(*hex)) hex++;

    if (!*hex) return 0;

    low = ascii_to_hex_table[*hex++];

    if (low < 0) return 0;

    dst[i++] = (high << 4) | low;
  }
}

//------------------------------------------------------------------------------
std::string conv::mccToString(
    const uint8_t digit1, const uint8_t digit2, const uint8_t digit3) {
  std::string s  = {};
  uint16_t mcc16 = digit1 * 100 + digit2 * 10 + digit3;
  // s.append(std::to_string(digit1)).append(std::to_string(digit2)).append(std::to_string(digit3));
  s.append(std::to_string(mcc16));
  return s;
}

//------------------------------------------------------------------------------
std::string conv::mncToString(
    const uint8_t digit1, const uint8_t digit2, const uint8_t digit3) {
  std::string s  = {};
  uint16_t mcc16 = 0;

  if (digit3 == 0x0F) {
    mcc16 = digit1 * 10 + digit2;
  } else {
    mcc16 = digit1 * 100 + digit2 * 10 + digit3;
  }
  s.append(std::to_string(mcc16));
  return s;
}

//------------------------------------------------------------------------------
bool conv::plmnFromString(
    plmn_t& p, const std::string mcc, const std::string mnc) {
  // MCC
  if (isdigit(mcc[0]))
    p.mcc_digit1 = mcc[0] - '0';
  else
    return false;
  if (isdigit(mcc[1]))
    p.mcc_digit2 = mcc[1] - '0';
  else
    return false;

  if (isdigit(mcc[2]))
    p.mcc_digit3 = mcc[2] - '0';
  else
    return false;

  // MNC
  if (isdigit(mnc[0]))
    p.mnc_digit1 = mnc[0] - '0';
  else
    return false;
  if (isdigit(mnc[1]))
    p.mnc_digit2 = mnc[1] - '0';
  else
    return false;
  if (mnc.length() > 2) {
    if (isdigit(mnc[2]))
      p.mnc_digit3 = mnc[2] - '0';
    else
      return false;
  } else {
    p.mnc_digit3 = 0x0;
  }
  return true;
}

//------------------------------------------------------------------------------
void conv::plmnToMccMnc(
    const plmn_t& plmn, std::string& mcc, std::string& mnc) {
  int m_mcc = plmn.mcc_digit1 * 100 + plmn.mcc_digit2 * 10 + plmn.mcc_digit3;
  mcc       = std::to_string(m_mcc);
  if ((plmn.mcc_digit2 == 0) and (plmn.mcc_digit1 == 0)) {
    mcc = "00" + mcc;
  } else if (plmn.mcc_digit1 == 0) {
    mcc = "0" + mcc;
  }

  int m_mnc = 0;
  if (plmn.mnc_digit3 == 0xf) {
    m_mnc = plmn.mnc_digit1 * 10 + plmn.mnc_digit2;
    if (plmn.mnc_digit1 == 0) {
      mnc = "0" + std::to_string(m_mnc);
      return;
    }
  } else {
    m_mnc = plmn.mnc_digit3 * 100 + plmn.mnc_digit1 * 10 + plmn.mnc_digit2;
    mnc   = std::to_string(m_mnc);
    if ((plmn.mnc_digit2 == 0) and (plmn.mnc_digit1 == 0)) {
      mnc = "00" + mnc;
    } else if (plmn.mnc_digit1 == 0) {
      mnc = "0" + mnc;
    }
  }

  return;
}

//------------------------------------------------------------------------------
struct in_addr conv::fromString(const std::string& addr4) {
  unsigned char buf[sizeof(struct in6_addr)] = {};
  struct in_addr ipv4_addr;
  ipv4_addr.s_addr = INADDR_ANY;
  if (inet_pton(AF_INET, addr4.c_str(), buf) == 1) {
    memcpy(&ipv4_addr, buf, sizeof(struct in_addr));
  }
  return ipv4_addr;
}

struct in6_addr conv::fromStringV6(const std::string& addr6) {
  unsigned char buf[sizeof(struct in6_addr)] = {};
  struct in6_addr ipv6_addr {};
  if (inet_pton(AF_INET6, addr6.c_str(), buf) == 1) {
    memcpy(&ipv6_addr, buf, sizeof(struct in6_addr));
  }
  return ipv6_addr;
}

//------------------------------------------------------------------------------
std::string conv::toString(const struct in_addr& inaddr) {
  std::string s              = {};
  char str[INET6_ADDRSTRLEN] = {};
  if (inet_ntop(AF_INET, (const void*) &inaddr, str, INET6_ADDRSTRLEN) ==
      NULL) {
    s.append("");
  } else {
    s.append(str);
  }
  return s;
}
//------------------------------------------------------------------------------
std::string conv::toString(const struct in6_addr& in6addr) {
  std::string s              = {};
  char str[INET6_ADDRSTRLEN] = {};
  if (inet_ntop(AF_INET6, (const void*) &in6addr, str, INET6_ADDRSTRLEN) ==
      nullptr) {
    s.append("");
  } else {
    s.append(str);
  }
  return s;
}

//---------------------------------------------------------------------------------------------
void conv::convert_string_2_hex(
    const std::string& input_str, std::string& output_str) {
  Logger::smf_app().debug("Convert string to Hex");
  unsigned char* data = (unsigned char*) malloc(input_str.length() + 1);
  memset(data, 0, input_str.length() + 1);
  memcpy((void*) data, (void*) input_str.c_str(), input_str.length());

  Logger::smf_app().debug("Input: ");
  if (Logger::should_log(spdlog::level::debug)) {
    for (int i = 0; i < input_str.length(); i++) {
      printf("%02x ", data[i]);
    }
    printf("\n");
  }

  char* datahex = (char*) malloc(input_str.length() * 2 + 1);
  memset(datahex, 0, input_str.length() * 2 + 1);

  for (int i = 0; i < input_str.length(); i++)
    sprintf(datahex + i * 2, "%02x", data[i]);

  output_str = reinterpret_cast<char*>(datahex);
  Logger::smf_app().debug("Output: \n %s ", output_str.c_str());

  // free memory
  free_wrapper((void**) &data);
  free_wrapper((void**) &datahex);
}
