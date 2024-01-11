/**
 * Namf_Communication
 * AMF Communication Service © 2019, 3GPP Organizational Partners (ARIB, ATIS,
 * CCSA, ETSI, TSDSI, TTA, TTC). All rights reserved.
 *
 * The version of the OpenAPI document: 1.1.0.alpha-1
 *
 *
 * NOTE: This class is auto generated by OpenAPI Generator
 * (https://openapi-generator.tech). https://openapi-generator.tech Do not edit
 * the class manually.
 */
/*
 * SubscriptionData.h
 *
 *
 */

#ifndef SubscriptionData_H_
#define SubscriptionData_H_

#include <string>
#include <vector>
#include "Guami.h"
#include <nlohmann/json.hpp>

namespace oai {
namespace amf {
namespace model {

/// <summary>
///
/// </summary>
class SubscriptionData {
 public:
  SubscriptionData();
  virtual ~SubscriptionData();

  void validate();

  /////////////////////////////////////////////
  /// SubscriptionData members

  /// <summary>
  ///
  /// </summary>
  std::string getAmfStatusUri() const;
  void setAmfStatusUri(std::string const& value);
  /// <summary>
  ///
  /// </summary>
  std::vector<oai::model::common::Guami>& getGuamiList();
  bool guamiListIsSet() const;
  void unsetGuamiList();

  friend void to_json(nlohmann::json& j, const SubscriptionData& o);
  friend void from_json(const nlohmann::json& j, SubscriptionData& o);

 protected:
  std::string m_AmfStatusUri;

  std::vector<oai::model::common::Guami> m_GuamiList;
  bool m_GuamiListIsSet;
};

}  // namespace model
}  // namespace amf
}  // namespace oai

#endif /* SubscriptionData_H_ */
