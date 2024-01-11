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

#include "AdditionalInformation.hpp"

using namespace nas;

//------------------------------------------------------------------------------
AdditionalInformation::AdditionalInformation()
    : Type4NasIe(kIeiAdditionalInformation), value_() {
  SetLengthIndicator(1);  // Minimum 3 octets (-2 for header)
}

//------------------------------------------------------------------------------
AdditionalInformation::AdditionalInformation(const bstring& value) {
  value_ = bstrcpy(value);
  SetLengthIndicator(blength(value));
}

//------------------------------------------------------------------------------
AdditionalInformation::~AdditionalInformation() {}

//------------------------------------------------------------------------------
void AdditionalInformation::SetValue(const bstring& value) {
  value_ = bstrcpy(value);
}

//------------------------------------------------------------------------------
void AdditionalInformation::GetValue(bstring& value) const {
  value = bstrcpy(value_);
}

//------------------------------------------------------------------------------
int AdditionalInformation::Encode(uint8_t* buf, int len) {
  Logger::nas_mm().debug("Encoding %s", GetIeName().c_str());
  int ie_len = GetIeLength();

  if (len < ie_len) {
    Logger::nas_mm().error("Len is less than %d", ie_len);
    return KEncodeDecodeError;
  }

  int encoded_size = 0;
  // IEI and Length
  int encoded_header_size = Type4NasIe::Encode(buf + encoded_size, len);
  if (encoded_header_size == KEncodeDecodeError) return KEncodeDecodeError;
  encoded_size += encoded_header_size;

  // Value
  int size = encode_bstring(value_, (buf + encoded_size), len - encoded_size);
  encoded_size += size;

  Logger::nas_mm().debug(
      "Encoded %s, len (%d)", GetIeName().c_str(), encoded_size);
  return encoded_size;
}

//------------------------------------------------------------------------------
int AdditionalInformation::Decode(uint8_t* buf, int len, bool is_iei) {
  if (len < kAdditionalInformationMinimumLength) {
    Logger::nas_mm().error(
        "Buffer length is less than the minimum length of this IE (%d octet)",
        kAdditionalInformationMinimumLength);
    return KEncodeDecodeError;
  }

  uint8_t decoded_size = 0;
  uint8_t octet        = 0;
  Logger::nas_mm().debug("Decoding %s", GetIeName().c_str());

  // IEI and Length
  int decoded_header_size = Type4NasIe::Decode(buf + decoded_size, len, is_iei);
  if (decoded_header_size == KEncodeDecodeError) return KEncodeDecodeError;
  decoded_size += decoded_header_size;

  // Value
  uint8_t ie_len = GetLengthIndicator();
  decode_bstring(&value_, ie_len, (buf + decoded_size), len - decoded_size);
  decoded_size += ie_len;

  for (int i = 0; i < ie_len; i++) {
    Logger::nas_mm().debug("Decoded value 0x%x", (uint8_t) value_->data[i]);
  }

  Logger::nas_mm().debug(
      "Decoded %s, len (%d)", GetIeName().c_str(), decoded_size);
  return decoded_size;
}
