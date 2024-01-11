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

#include "AuthenticationFailure.hpp"

using namespace nas;

//------------------------------------------------------------------------------
AuthenticationFailure::AuthenticationFailure()
    : NasMmPlainHeader(EPD_5GS_MM_MSG, AUTHENTICATION_FAILURE) {
  ie_authentication_failure_parameter = std::nullopt;
}

//------------------------------------------------------------------------------
AuthenticationFailure::~AuthenticationFailure() {}

//------------------------------------------------------------------------------
void AuthenticationFailure::SetHeader(uint8_t security_header_type) {
  NasMmPlainHeader::SetSecurityHeaderType(security_header_type);
}

//------------------------------------------------------------------------------
void AuthenticationFailure::Set5gmmCause(uint8_t value) {
  ie_5gmm_cause.SetValue(value);
}

//------------------------------------------------------------------------------
uint8_t AuthenticationFailure::Get5gmmCause() const {
  return ie_5gmm_cause.GetValue();
}

/*
//------------------------------------------------------------------------------
void AuthenticationFailure::SetAuthenticationFailureParameter(
    const uint8_t (&value)[kAuthenticationFailureParameterContentLength]) {
  ie_authentication_failure_parameter =
      std::make_optional<AuthenticationFailureParameter>(value);
}
bool AuthenticationFailure::GetAuthenticationFailureParameter(uint8_t
(&value)[kAuthenticationFailureParameterContentLength]) const{ if
(ie_authentication_failure_parameter.has_value()) {
    ie_authentication_failure_parameter.value().GetValue(value);
    return true;
  } else {
    return false;
  }
}
*/

//------------------------------------------------------------------------------
void AuthenticationFailure::SetAuthenticationFailureParameter(
    const bstring& value) {
  ie_authentication_failure_parameter =
      std::make_optional<AuthenticationFailureParameter>(value);
}

//------------------------------------------------------------------------------
bool AuthenticationFailure::GetAuthenticationFailureParameter(
    bstring& value) const {
  if (ie_authentication_failure_parameter.has_value()) {
    ie_authentication_failure_parameter.value().GetValue(value);
    return true;
  } else {
    return false;
  }
}

//------------------------------------------------------------------------------
int AuthenticationFailure::Encode(uint8_t* buf, int len) {
  Logger::nas_mm().debug("Encoding AuthenticationFailure message");
  int encoded_size    = 0;
  int encoded_ie_size = 0;

  // Header
  if ((encoded_ie_size = NasMmPlainHeader::Encode(buf, len)) ==
      KEncodeDecodeError) {
    Logger::nas_mm().error("Encoding NAS Header error");
    return KEncodeDecodeError;
  }
  encoded_size += encoded_ie_size;

  int size = ie_5gmm_cause.Encode(buf + encoded_size, len - encoded_size);
  if (size != KEncodeDecodeError) {
    encoded_size += size;
  } else {
    Logger::nas_mm().error(
        "Encoding %s error", _5gmmCause::GetIeName().c_str());
    return KEncodeDecodeError;
  }

  if (!ie_authentication_failure_parameter.has_value()) {
    Logger::nas_mm().debug(
        "IE %s is not available",
        AuthenticationFailureParameter::GetIeName().c_str());
  } else {
    size = ie_authentication_failure_parameter.value().Encode(
        buf + encoded_size, len - encoded_size);
    if (size != KEncodeDecodeError) {
      encoded_size += size;
    } else {
      Logger::nas_mm().error(
          "Encoding %s error",
          AuthenticationFailureParameter::GetIeName().c_str());
      return KEncodeDecodeError;
    }
  }

  Logger::nas_mm().debug(
      "Encoded AuthenticationFailure message len (%d)", encoded_size);
  return encoded_size;
}

//------------------------------------------------------------------------------
int AuthenticationFailure::Decode(uint8_t* buf, int len) {
  Logger::nas_mm().debug("Decoding AuthenticationFailure message");

  int decoded_size   = 0;
  int decoded_result = 0;

  // Header
  decoded_result = NasMmPlainHeader::Decode(buf, len);
  if (decoded_result == KEncodeDecodeError) {
    Logger::nas_mm().error("Decoding NAS Header error");
    return KEncodeDecodeError;
  }
  decoded_size += decoded_result;

  // 5GMM Cause
  if ((decoded_result = ie_5gmm_cause.Decode(
           buf + decoded_size, len - decoded_size, false)) ==
      KEncodeDecodeError) {
    Logger::nas_mm().error(
        "Decoding %s error", _5gmmCause::GetIeName().c_str());
    return KEncodeDecodeError;
  }
  decoded_size += decoded_result;

  Logger::nas_mm().debug("Decoded_size (%d)", decoded_size);

  // Decode other IEs
  uint8_t octet = 0x00;
  DECODE_U8_VALUE(buf + decoded_size, octet);
  while ((octet != 0x0)) {
    Logger::nas_mm().debug("IEI 0x%x", octet);
    switch (octet) {
      case kIeiAuthenticationFailureParameter: {
        Logger::nas_mm().debug(
            "Decoding IEI 0x%x", kIeiAuthenticationFailureParameter);
        AuthenticationFailureParameter ie_authentication_failure_parameter_tmp =
            {};
        if ((decoded_result = ie_authentication_failure_parameter_tmp.Decode(
                 buf + decoded_size, len - decoded_size, true)) ==
            KEncodeDecodeError)
          return KEncodeDecodeError;
        decoded_size += decoded_result;
        ie_authentication_failure_parameter =
            std::optional<AuthenticationFailureParameter>(
                ie_authentication_failure_parameter_tmp);
        DECODE_U8_VALUE(buf + decoded_size, octet);
        Logger::nas_mm().debug("Next IEI (0x%x)", octet);
      } break;

      default: {
        Logger::nas_mm().warn("Unknown IEI 0x%x, stop decoding...", octet);
        // Stop decoding
        octet = 0x00;
      } break;
    }
  }

  Logger::nas_mm().debug(
      "Decoded AuthenticationFailure message len (%d)", decoded_size);
  return decoded_size;
}
