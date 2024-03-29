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
 * UsageMonDataScope.h
 *
 * Contains a SNSSAI and DNN combinations to which the UsageMonData instance
 * belongs to.
 */

#ifndef UsageMonDataScope_H_
#define UsageMonDataScope_H_

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "Snssai.h"

namespace oai::udr::model {

/// <summary>
/// Contains a SNSSAI and DNN combinations to which the UsageMonData instance
/// belongs to.
/// </summary>
class UsageMonDataScope {
 public:
  UsageMonDataScope();
  virtual ~UsageMonDataScope();

  void validate();

  /////////////////////////////////////////////
  /// UsageMonDataScope members

  /// <summary>
  ///
  /// </summary>
  oai::model::common::Snssai getSnssai() const;
  void setSnssai(oai::model::common::Snssai const& value);
  /// <summary>
  ///
  /// </summary>
  std::vector<std::string>& getDnn();
  void setDnn(std::vector<std::string> const& value);
  bool dnnIsSet() const;
  void unsetDnn();

  friend void to_json(nlohmann::json& j, const UsageMonDataScope& o);
  friend void from_json(const nlohmann::json& j, UsageMonDataScope& o);

 protected:
  oai::model::common::Snssai m_Snssai;

  std::vector<std::string> m_Dnn;
  bool m_DnnIsSet;
};

}  // namespace oai::udr::model

#endif /* UsageMonDataScope_H_ */
