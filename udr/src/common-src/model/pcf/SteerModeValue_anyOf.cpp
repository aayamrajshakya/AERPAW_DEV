/**
 * Npcf_SMPolicyControl API
 * Session Management Policy Control Service © 2023, 3GPP Organizational
 * Partners (ARIB, ATIS, CCSA, ETSI, TSDSI, TTA, TTC). All rights reserved.
 *
 * The version of the OpenAPI document: 1.1.9
 *
 *
 * NOTE: This class is auto generated by OpenAPI Generator
 * (https://openapi-generator.tech). https://openapi-generator.tech Do not edit
 * the class manually.
 */

#include "SteerModeValue_anyOf.h"
#include "Helpers.h"
#include <stdexcept>
#include <sstream>

namespace oai::model::pcf {

SteerModeValue_anyOf::SteerModeValue_anyOf() {}

void SteerModeValue_anyOf::validate() const {
  std::stringstream msg;
  if (!validate(msg)) {
    throw oai::model::common::helpers::ValidationException(msg.str());
  }
}

bool SteerModeValue_anyOf::validate(std::stringstream& msg) const {
  return validate(msg, "");
}

bool SteerModeValue_anyOf::validate(
    std::stringstream& msg, const std::string& pathPrefix) const {
  bool success = true;
  const std::string _pathPrefix =
      pathPrefix.empty() ? "SteerModeValue_anyOf" : pathPrefix;

  if (m_value == SteerModeValue_anyOf::eSteerModeValue_anyOf::
                     INVALID_VALUE_OPENAPI_GENERATED) {
    success = false;
    msg << _pathPrefix << ": has no value;";
  }

  return success;
}

bool SteerModeValue_anyOf::operator==(const SteerModeValue_anyOf& rhs) const {
  return getValue() == rhs.getValue()

      ;
}

bool SteerModeValue_anyOf::operator!=(const SteerModeValue_anyOf& rhs) const {
  return !(*this == rhs);
}

void to_json(nlohmann::json& j, const SteerModeValue_anyOf& o) {
  j = nlohmann::json();

  switch (o.getValue()) {
    case SteerModeValue_anyOf::eSteerModeValue_anyOf::
        INVALID_VALUE_OPENAPI_GENERATED:
      j = "INVALID_VALUE_OPENAPI_GENERATED";
      break;
    case SteerModeValue_anyOf::eSteerModeValue_anyOf::ACTIVE_STANDBY:
      j = "ACTIVE_STANDBY";
      break;
    case SteerModeValue_anyOf::eSteerModeValue_anyOf::LOAD_BALANCING:
      j = "LOAD_BALANCING";
      break;
    case SteerModeValue_anyOf::eSteerModeValue_anyOf::SMALLEST_DELAY:
      j = "SMALLEST_DELAY";
      break;
    case SteerModeValue_anyOf::eSteerModeValue_anyOf::PRIORITY_BASED:
      j = "PRIORITY_BASED";
      break;
  }
}

void from_json(const nlohmann::json& j, SteerModeValue_anyOf& o) {
  auto s = j.get<std::string>();
  if (s == "ACTIVE_STANDBY") {
    o.setValue(SteerModeValue_anyOf::eSteerModeValue_anyOf::ACTIVE_STANDBY);
  } else if (s == "LOAD_BALANCING") {
    o.setValue(SteerModeValue_anyOf::eSteerModeValue_anyOf::LOAD_BALANCING);
  } else if (s == "SMALLEST_DELAY") {
    o.setValue(SteerModeValue_anyOf::eSteerModeValue_anyOf::SMALLEST_DELAY);
  } else if (s == "PRIORITY_BASED") {
    o.setValue(SteerModeValue_anyOf::eSteerModeValue_anyOf::PRIORITY_BASED);
  } else {
    std::stringstream ss;
    ss << "Unexpected value " << s << " in json"
       << " cannot be converted to enum of type"
       << " SteerModeValue_anyOf::eSteerModeValue_anyOf";
    throw std::invalid_argument(ss.str());
  }
}

SteerModeValue_anyOf::eSteerModeValue_anyOf SteerModeValue_anyOf::getValue()
    const {
  return m_value;
}
void SteerModeValue_anyOf::setValue(
    SteerModeValue_anyOf::eSteerModeValue_anyOf value) {
  m_value = value;
}

}  // namespace oai::model::pcf
