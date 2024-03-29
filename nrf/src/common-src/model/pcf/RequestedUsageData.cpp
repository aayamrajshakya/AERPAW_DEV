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

#include "RequestedUsageData.h"
#include "Helpers.h"

#include <sstream>

namespace oai::model::pcf {

RequestedUsageData::RequestedUsageData() {
  m_RefUmIdsIsSet = false;
  m_AllUmIds      = false;
  m_AllUmIdsIsSet = false;
}

void RequestedUsageData::validate() const {
  std::stringstream msg;
  if (!validate(msg)) {
    throw oai::model::common::helpers::ValidationException(msg.str());
  }
}

bool RequestedUsageData::validate(std::stringstream& msg) const {
  return validate(msg, "");
}

bool RequestedUsageData::validate(
    std::stringstream& msg, const std::string& pathPrefix) const {
  bool success = true;
  const std::string _pathPrefix =
      pathPrefix.empty() ? "RequestedUsageData" : pathPrefix;

  if (refUmIdsIsSet()) {
    const std::vector<std::string>& value = m_RefUmIds;
    const std::string currentValuePath    = _pathPrefix + ".refUmIds";

    if (value.size() < 1) {
      success = false;
      msg << currentValuePath << ": must have at least 1 elements;";
    }
    /*
    {  // Recursive validation of array elements
      const std::string oldValuePath = currentValuePath;
      int i                          = 0;
      for (const std::string& value : value) {
        const std::string currentValuePath =
            oldValuePath + "[" + std::to_string(i) + "]";

        i++;
      }
    }
    */
  }

  return success;
}

bool RequestedUsageData::operator==(const RequestedUsageData& rhs) const {
  return

      ((!refUmIdsIsSet() && !rhs.refUmIdsIsSet()) ||
       (refUmIdsIsSet() && rhs.refUmIdsIsSet() &&
        getRefUmIds() == rhs.getRefUmIds())) &&

      ((!allUmIdsIsSet() && !rhs.allUmIdsIsSet()) ||
       (allUmIdsIsSet() && rhs.allUmIdsIsSet() &&
        isAllUmIds() == rhs.isAllUmIds()))

          ;
}

bool RequestedUsageData::operator!=(const RequestedUsageData& rhs) const {
  return !(*this == rhs);
}

void to_json(nlohmann::json& j, const RequestedUsageData& o) {
  j = nlohmann::json();
  if (o.refUmIdsIsSet() || !o.m_RefUmIds.empty()) j["refUmIds"] = o.m_RefUmIds;
  if (o.allUmIdsIsSet()) j["allUmIds"] = o.m_AllUmIds;
}

void from_json(const nlohmann::json& j, RequestedUsageData& o) {
  if (j.find("refUmIds") != j.end()) {
    j.at("refUmIds").get_to(o.m_RefUmIds);
    o.m_RefUmIdsIsSet = true;
  }
  if (j.find("allUmIds") != j.end()) {
    j.at("allUmIds").get_to(o.m_AllUmIds);
    o.m_AllUmIdsIsSet = true;
  }
}

std::vector<std::string> RequestedUsageData::getRefUmIds() const {
  return m_RefUmIds;
}
void RequestedUsageData::setRefUmIds(std::vector<std::string> const& value) {
  m_RefUmIds      = value;
  m_RefUmIdsIsSet = true;
}
bool RequestedUsageData::refUmIdsIsSet() const {
  return m_RefUmIdsIsSet;
}
void RequestedUsageData::unsetRefUmIds() {
  m_RefUmIdsIsSet = false;
}
bool RequestedUsageData::isAllUmIds() const {
  return m_AllUmIds;
}
void RequestedUsageData::setAllUmIds(bool const value) {
  m_AllUmIds      = value;
  m_AllUmIdsIsSet = true;
}
bool RequestedUsageData::allUmIdsIsSet() const {
  return m_AllUmIdsIsSet;
}
void RequestedUsageData::unsetAllUmIds() {
  m_AllUmIdsIsSet = false;
}

}  // namespace oai::model::pcf
