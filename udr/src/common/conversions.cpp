/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this
 *file except in compliance with the License. You may obtain a copy of the
 *License at
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

#include <arpa/inet.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstring>

//#include <boost/algorithm/string.hpp>

//------------------------------------------------------------------------------
std::string conv::toString(const struct in_addr& inaddr) {
  std::string s              = {};
  char str[INET6_ADDRSTRLEN] = {};
  if (inet_ntop(AF_INET, (const void*) &inaddr, str, INET6_ADDRSTRLEN) ==
      NULL) {
    s.append("Error in_addr");
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
    s.append("Error in6_addr");
  } else {
    s.append(str);
  }
  return s;
}

//------------------------------------------------------------------------------
struct in_addr conv::fromString(const std::string addr4) {
  unsigned char buf[sizeof(struct in6_addr)] = {};
  int s              = inet_pton(AF_INET, addr4.c_str(), buf);
  struct in_addr* ia = (struct in_addr*) buf;
  return *ia;
}

//------------------------------------------------------------------------------
struct in6_addr conv::fromStringV6(const std::string& addr6) {
  unsigned char buf[sizeof(struct in6_addr)] = {};
  struct in6_addr ipv6_addr {};
  if (inet_pton(AF_INET6, addr6.c_str(), buf) == 1) {
    memcpy(&ipv6_addr, buf, sizeof(struct in6_addr));
  }
  return ipv6_addr;
}
