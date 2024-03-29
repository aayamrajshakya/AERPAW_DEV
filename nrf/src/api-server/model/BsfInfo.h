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
 * BsfInfo.h
 *
 *
 */

#ifndef BsfInfo_H_
#define BsfInfo_H_

#include <string>
#include "Ipv4AddressRange.h"
#include <vector>
#include "Ipv6PrefixRange.h"
#include <nlohmann/json.hpp>

namespace oai::model::nrf {

/// <summary>
///
/// </summary>
class BsfInfo {
 public:
  BsfInfo();
  virtual ~BsfInfo();

  void validate();

  /////////////////////////////////////////////
  /// BsfInfo members

  /// <summary>
  ///
  /// </summary>
  std::vector<std::string>& getDnnList();
  void setDnnList(std::vector<std::string> const& value);
  bool dnnListIsSet() const;
  void unsetDnnList();
  /// <summary>
  ///
  /// </summary>
  std::vector<std::string>& getIpDomainList();
  void setIpDomainList(std::vector<std::string> const& value);
  bool ipDomainListIsSet() const;
  void unsetIpDomainList();
  /// <summary>
  ///
  /// </summary>
  std::vector<Ipv4AddressRange>& getIpv4AddressRanges();
  void setIpv4AddressRanges(std::vector<Ipv4AddressRange> const& value);
  bool ipv4AddressRangesIsSet() const;
  void unsetIpv4AddressRanges();
  /// <summary>
  ///
  /// </summary>
  std::vector<Ipv6PrefixRange>& getIpv6PrefixRanges();
  void setIpv6PrefixRanges(std::vector<Ipv6PrefixRange> const& value);
  bool ipv6PrefixRangesIsSet() const;
  void unsetIpv6PrefixRanges();

  friend void to_json(nlohmann::json& j, const BsfInfo& o);
  friend void from_json(const nlohmann::json& j, BsfInfo& o);

 protected:
  std::vector<std::string> m_DnnList;
  bool m_DnnListIsSet;
  std::vector<std::string> m_IpDomainList;
  bool m_IpDomainListIsSet;
  std::vector<Ipv4AddressRange> m_Ipv4AddressRanges;
  bool m_Ipv4AddressRangesIsSet;
  std::vector<Ipv6PrefixRange> m_Ipv6PrefixRanges;
  bool m_Ipv6PrefixRangesIsSet;
};

}  // namespace oai::model::nrf

#endif /* BsfInfo_H_ */
