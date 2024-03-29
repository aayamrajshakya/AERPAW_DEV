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

#include "AdditionalAccessInfo.h"
#include "Helpers.h"

#include <sstream>

namespace oai::model::pcf {
using namespace oai::model::common;

AdditionalAccessInfo::AdditionalAccessInfo() {
  m_RatTypeIsSet = false;
}

void AdditionalAccessInfo::validate() const {
  std::stringstream msg;
  if (!validate(msg)) {
    throw oai::model::common::helpers::ValidationException(msg.str());
  }
}

bool AdditionalAccessInfo::validate(std::stringstream& msg) const {
  return validate(msg, "");
}

bool AdditionalAccessInfo::validate(
    std::stringstream& /* msg */, const std::string& pathPrefix) const {
  bool success = true;
  const std::string _pathPrefix =
      pathPrefix.empty() ? "AdditionalAccessInfo" : pathPrefix;

  return success;
}

bool AdditionalAccessInfo::operator==(const AdditionalAccessInfo& rhs) const {
  return

      (getAccessType() == rhs.getAccessType()) &&

      ((!ratTypeIsSet() && !rhs.ratTypeIsSet()) ||
       (ratTypeIsSet() && rhs.ratTypeIsSet() &&
        getRatType() == rhs.getRatType()))

          ;
}

bool AdditionalAccessInfo::operator!=(const AdditionalAccessInfo& rhs) const {
  return !(*this == rhs);
}

void to_json(nlohmann::json& j, const AdditionalAccessInfo& o) {
  j               = nlohmann::json();
  j["accessType"] = o.m_AccessType;
  if (o.ratTypeIsSet()) j["ratType"] = o.m_RatType;
}

void from_json(const nlohmann::json& j, AdditionalAccessInfo& o) {
  j.at("accessType").get_to(o.m_AccessType);
  if (j.find("ratType") != j.end()) {
    j.at("ratType").get_to(o.m_RatType);
    o.m_RatTypeIsSet = true;
  }
}

AccessType AdditionalAccessInfo::getAccessType() const {
  return m_AccessType;
}
void AdditionalAccessInfo::setAccessType(AccessType const& value) {
  m_AccessType = value;
}
RatType AdditionalAccessInfo::getRatType() const {
  return m_RatType;
}
void AdditionalAccessInfo::setRatType(RatType const& value) {
  m_RatType      = value;
  m_RatTypeIsSet = true;
}
bool AdditionalAccessInfo::ratTypeIsSet() const {
  return m_RatTypeIsSet;
}
void AdditionalAccessInfo::unsetRatType() {
  m_RatTypeIsSet = false;
}

}  // namespace oai::model::pcf
