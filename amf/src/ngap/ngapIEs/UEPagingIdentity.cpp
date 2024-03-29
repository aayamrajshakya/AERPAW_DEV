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

#include "UEPagingIdentity.hpp"

namespace ngap {

//------------------------------------------------------------------------------
UEPagingIdentity::UEPagingIdentity() {}

//------------------------------------------------------------------------------
UEPagingIdentity::~UEPagingIdentity() {}

//------------------------------------------------------------------------------
void UEPagingIdentity::setUEPagingIdentity(
    const std::string& set_id, const std::string& pointer,
    const std::string& tmsi) {
  fiveGSTmsi.set(set_id, pointer, tmsi);
}

//------------------------------------------------------------------------------
void UEPagingIdentity::getUEPagingIdentity(std::string& _5g_s_tmsi) {
  fiveGSTmsi.getTmsi(_5g_s_tmsi);
}

//------------------------------------------------------------------------------
void UEPagingIdentity::getUEPagingIdentity(
    std::string& set_id, std::string& pointer, std::string& tmsi) {
  fiveGSTmsi.get(set_id, pointer, tmsi);
}

//------------------------------------------------------------------------------
bool UEPagingIdentity::encode(Ngap_UEPagingIdentity_t& pdu) {
  pdu.present = Ngap_UEPagingIdentity_PR_fiveG_S_TMSI;
  Ngap_FiveG_S_TMSI_t* ie =
      (Ngap_FiveG_S_TMSI_t*) calloc(1, sizeof(Ngap_FiveG_S_TMSI_t));
  pdu.choice.fiveG_S_TMSI = ie;
  if (!fiveGSTmsi.encode(*pdu.choice.fiveG_S_TMSI)) return false;

  return true;
}

//------------------------------------------------------------------------------
bool UEPagingIdentity::decode(const Ngap_UEPagingIdentity_t& pdu) {
  if (pdu.present != Ngap_UEPagingIdentity_PR_fiveG_S_TMSI) return false;
  if (!fiveGSTmsi.decode(*pdu.choice.fiveG_S_TMSI)) return false;

  return true;
}
}  // namespace ngap
