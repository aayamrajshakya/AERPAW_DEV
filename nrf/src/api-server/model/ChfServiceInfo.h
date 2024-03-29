/**
 * NRF NFManagement Service
 * NRF NFManagement Service. © 2019, 3GPP Organizational Partners (ARIB, ATIS,
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
 * ChfServiceInfo.h
 *
 *
 */

#ifndef ChfServiceInfo_H_
#define ChfServiceInfo_H_

#include <string>
#include <nlohmann/json.hpp>

namespace oai::model::nrf {

/// <summary>
///
/// </summary>
class ChfServiceInfo {
 public:
  ChfServiceInfo();
  virtual ~ChfServiceInfo();

  void validate();

  /////////////////////////////////////////////
  /// ChfServiceInfo members

  /// <summary>
  ///
  /// </summary>
  std::string getPrimaryChfServiceInstance() const;
  void setPrimaryChfServiceInstance(std::string const& value);
  bool primaryChfServiceInstanceIsSet() const;
  void unsetPrimaryChfServiceInstance();
  /// <summary>
  ///
  /// </summary>
  std::string getSecondaryChfServiceInstance() const;
  void setSecondaryChfServiceInstance(std::string const& value);
  bool secondaryChfServiceInstanceIsSet() const;
  void unsetSecondaryChfServiceInstance();

  friend void to_json(nlohmann::json& j, const ChfServiceInfo& o);
  friend void from_json(const nlohmann::json& j, ChfServiceInfo& o);

 protected:
  std::string m_PrimaryChfServiceInstance;
  bool m_PrimaryChfServiceInstanceIsSet;
  std::string m_SecondaryChfServiceInstance;
  bool m_SecondaryChfServiceInstanceIsSet;
};

}  // namespace oai::model::nrf

#endif /* ChfServiceInfo_H_ */
