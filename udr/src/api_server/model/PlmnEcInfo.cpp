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

#include "PlmnEcInfo.h"

namespace oai::udr::model {

PlmnEcInfo::PlmnEcInfo() {
  m_EcRestrictionDataWbIsSet = false;
  m_EcRestrictionDataNb      = false;
  m_EcRestrictionDataNbIsSet = false;
}

PlmnEcInfo::~PlmnEcInfo() {}

void PlmnEcInfo::validate() {
  // TODO: implement validation
}

void to_json(nlohmann::json& j, const PlmnEcInfo& o) {
  j           = nlohmann::json();
  j["plmnId"] = o.m_PlmnId;
  if (o.ecRestrictionDataWbIsSet())
    j["ecRestrictionDataWb"] = o.m_EcRestrictionDataWb;
  if (o.ecRestrictionDataNbIsSet())
    j["ecRestrictionDataNb"] = o.m_EcRestrictionDataNb;
}

void from_json(const nlohmann::json& j, PlmnEcInfo& o) {
  j.at("plmnId").get_to(o.m_PlmnId);
  if (j.find("ecRestrictionDataWb") != j.end()) {
    j.at("ecRestrictionDataWb").get_to(o.m_EcRestrictionDataWb);
    o.m_EcRestrictionDataWbIsSet = true;
  }
  if (j.find("ecRestrictionDataNb") != j.end()) {
    j.at("ecRestrictionDataNb").get_to(o.m_EcRestrictionDataNb);
    o.m_EcRestrictionDataNbIsSet = true;
  }
}

oai::model::common::PlmnId PlmnEcInfo::getPlmnId() const {
  return m_PlmnId;
}
void PlmnEcInfo::setPlmnId(oai::model::common::PlmnId const& value) {
  m_PlmnId = value;
}
EcRestrictionDataWb PlmnEcInfo::getEcRestrictionDataWb() const {
  return m_EcRestrictionDataWb;
}
void PlmnEcInfo::setEcRestrictionDataWb(EcRestrictionDataWb const& value) {
  m_EcRestrictionDataWb      = value;
  m_EcRestrictionDataWbIsSet = true;
}
bool PlmnEcInfo::ecRestrictionDataWbIsSet() const {
  return m_EcRestrictionDataWbIsSet;
}
void PlmnEcInfo::unsetEcRestrictionDataWb() {
  m_EcRestrictionDataWbIsSet = false;
}
bool PlmnEcInfo::isEcRestrictionDataNb() const {
  return m_EcRestrictionDataNb;
}
void PlmnEcInfo::setEcRestrictionDataNb(bool const value) {
  m_EcRestrictionDataNb      = value;
  m_EcRestrictionDataNbIsSet = true;
}
bool PlmnEcInfo::ecRestrictionDataNbIsSet() const {
  return m_EcRestrictionDataNbIsSet;
}
void PlmnEcInfo::unsetEcRestrictionDataNb() {
  m_EcRestrictionDataNbIsSet = false;
}

}  // namespace oai::udr::model
