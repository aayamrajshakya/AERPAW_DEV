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
/**
 * Nudr_DataRepository API OpenAPI file
 * Unified Data Repository Service. © 2020, 3GPP Organizational Partners (ARIB,
 * ATIS, CCSA, ETSI, TSDSI, TTA, TTC). All rights reserved.
 *
 * The version of the OpenAPI document: 2.1.2
 *
 *
 * NOTE: This class is auto generated by OpenAPI Generator
 * (https://openapi-generator.tech). https://openapi-generator.tech Do not edit
 * the class manually.
 */
/*
 * TrafficInfluDataPatch.h
 *
 *
 */

#ifndef TrafficInfluDataPatch_H_
#define TrafficInfluDataPatch_H_

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "EthFlowDescription.h"
#include "FlowInfo.h"
#include "NetworkAreaInfo_2.h"
#include "RouteToLocation.h"
#include "Snssai.h"
#include "TemporalValidity.h"

namespace oai::udr::model {

/// <summary>
///
/// </summary>
class TrafficInfluDataPatch {
 public:
  TrafficInfluDataPatch();
  virtual ~TrafficInfluDataPatch();

  void validate();

  /////////////////////////////////////////////
  /// TrafficInfluDataPatch members

  /// <summary>
  /// Contains the Notification Correlation Id allocated by the NEF for the UP
  /// path change notification.
  /// </summary>
  std::string getUpPathChgNotifCorreId() const;
  void setUpPathChgNotifCorreId(std::string const& value);
  bool upPathChgNotifCorreIdIsSet() const;
  void unsetUpPathChgNotifCorreId();
  /// <summary>
  /// Identifies whether an application can be relocated once a location of the
  /// application has been selected.
  /// </summary>
  bool isAppReloInd() const;
  void setAppReloInd(bool const value);
  bool appReloIndIsSet() const;
  void unsetAppReloInd();
  /// <summary>
  ///
  /// </summary>
  std::string getDnn() const;
  void setDnn(std::string const& value);
  bool dnnIsSet() const;
  void unsetDnn();
  /// <summary>
  /// Identifies Ethernet packet filters. Either \&quot;trafficFilters\&quot; or
  /// \&quot;ethTrafficFilters\&quot; shall be included if applicable.
  /// </summary>
  std::vector<EthFlowDescription>& getEthTrafficFilters();
  void setEthTrafficFilters(std::vector<EthFlowDescription> const& value);
  bool ethTrafficFiltersIsSet() const;
  void unsetEthTrafficFilters();
  /// <summary>
  ///
  /// </summary>
  oai::model::common::Snssai getSnssai() const;
  void setSnssai(oai::model::common::Snssai const& value);
  bool snssaiIsSet() const;
  void unsetSnssai();
  /// <summary>
  ///
  /// </summary>
  std::string getInternalGroupId() const;
  void setInternalGroupId(std::string const& value);
  bool internalGroupIdIsSet() const;
  void unsetInternalGroupId();
  /// <summary>
  ///
  /// </summary>
  std::string getSupi() const;
  void setSupi(std::string const& value);
  bool supiIsSet() const;
  void unsetSupi();
  /// <summary>
  /// Identifies IP packet filters. Either \&quot;trafficFilters\&quot; or
  /// \&quot;ethTrafficFilters\&quot; shall be included if applicable.
  /// </summary>
  std::vector<FlowInfo>& getTrafficFilters();
  void setTrafficFilters(std::vector<FlowInfo> const& value);
  bool trafficFiltersIsSet() const;
  void unsetTrafficFilters();
  /// <summary>
  /// Identifies the N6 traffic routing requirement.
  /// </summary>
  std::vector<oai::model::common::RouteToLocation>& getTrafficRoutes();
  void setTrafficRoutes(
      std::vector<oai::model::common::RouteToLocation> const& value);
  bool trafficRoutesIsSet() const;
  void unsetTrafficRoutes();
  /// <summary>
  ///
  /// </summary>
  bool isTraffCorreInd() const;
  void setTraffCorreInd(bool const value);
  bool traffCorreIndIsSet() const;
  void unsetTraffCorreInd();
  /// <summary>
  ///
  /// </summary>
  std::string getValidStartTime() const;
  void setValidStartTime(std::string const& value);
  bool validStartTimeIsSet() const;
  void unsetValidStartTime();
  /// <summary>
  ///
  /// </summary>
  std::string getValidEndTime() const;
  void setValidEndTime(std::string const& value);
  bool validEndTimeIsSet() const;
  void unsetValidEndTime();
  /// <summary>
  /// Identifies the temporal validities for the N6 traffic routing requirement.
  /// </summary>
  std::vector<TemporalValidity>& getTempValidities();
  void setTempValidities(std::vector<TemporalValidity> const& value);
  bool tempValiditiesIsSet() const;
  void unsetTempValidities();
  /// <summary>
  ///
  /// </summary>
  NetworkAreaInfo_2 getNwAreaInfo() const;
  void setNwAreaInfo(NetworkAreaInfo_2 const& value);
  bool nwAreaInfoIsSet() const;
  void unsetNwAreaInfo();
  /// <summary>
  ///
  /// </summary>
  std::string getUpPathChgNotifUri() const;
  void setUpPathChgNotifUri(std::string const& value);
  bool upPathChgNotifUriIsSet() const;
  void unsetUpPathChgNotifUri();
  /// <summary>
  ///
  /// </summary>
  std::vector<std::string>& getHeaders();
  void setHeaders(std::vector<std::string> const& value);
  bool headersIsSet() const;
  void unsetHeaders();
  /// <summary>
  ///
  /// </summary>
  bool isAfAckInd() const;
  void setAfAckInd(bool const value);
  bool afAckIndIsSet() const;
  void unsetAfAckInd();
  /// <summary>
  ///
  /// </summary>
  bool isAddrPreserInd() const;
  void setAddrPreserInd(bool const value);
  bool addrPreserIndIsSet() const;
  void unsetAddrPreserInd();

  friend void to_json(nlohmann::json& j, const TrafficInfluDataPatch& o);
  friend void from_json(const nlohmann::json& j, TrafficInfluDataPatch& o);

 protected:
  std::string m_UpPathChgNotifCorreId;
  bool m_UpPathChgNotifCorreIdIsSet;
  bool m_AppReloInd;
  bool m_AppReloIndIsSet;
  std::string m_Dnn;
  bool m_DnnIsSet;
  std::vector<EthFlowDescription> m_EthTrafficFilters;
  bool m_EthTrafficFiltersIsSet;
  oai::model::common::Snssai m_Snssai;
  bool m_SnssaiIsSet;
  std::string m_InternalGroupId;
  bool m_InternalGroupIdIsSet;
  std::string m_Supi;
  bool m_SupiIsSet;
  std::vector<FlowInfo> m_TrafficFilters;
  bool m_TrafficFiltersIsSet;
  std::vector<oai::model::common::RouteToLocation> m_TrafficRoutes;
  bool m_TrafficRoutesIsSet;
  bool m_TraffCorreInd;
  bool m_TraffCorreIndIsSet;
  std::string m_ValidStartTime;
  bool m_ValidStartTimeIsSet;
  std::string m_ValidEndTime;
  bool m_ValidEndTimeIsSet;
  std::vector<TemporalValidity> m_TempValidities;
  bool m_TempValiditiesIsSet;
  NetworkAreaInfo_2 m_NwAreaInfo;
  bool m_NwAreaInfoIsSet;
  std::string m_UpPathChgNotifUri;
  bool m_UpPathChgNotifUriIsSet;
  std::vector<std::string> m_Headers;
  bool m_HeadersIsSet;
  bool m_AfAckInd;
  bool m_AfAckIndIsSet;
  bool m_AddrPreserInd;
  bool m_AddrPreserIndIsSet;
};

}  // namespace oai::udr::model

#endif /* TrafficInfluDataPatch_H_ */
