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

#include "IptvConfigData.h"

namespace oai::udr::model {

IptvConfigData::IptvConfigData() {
  m_Supi              = "";
  m_SupiIsSet         = false;
  m_InterGroupId      = "";
  m_InterGroupIdIsSet = false;
  m_Dnn               = "";
  m_DnnIsSet          = false;
  m_SnssaiIsSet       = false;
  m_AfAppId           = "";
  m_SuppFeat          = "";
  m_SuppFeatIsSet     = false;
  m_ResUri            = "";
  m_ResUriIsSet       = false;
}

IptvConfigData::~IptvConfigData() {}

void IptvConfigData::validate() {
  // TODO: implement validation
}

void to_json(nlohmann::json& j, const IptvConfigData& o) {
  j = nlohmann::json();
  if (o.supiIsSet()) j["supi"] = o.m_Supi;
  if (o.interGroupIdIsSet()) j["interGroupId"] = o.m_InterGroupId;
  if (o.dnnIsSet()) j["dnn"] = o.m_Dnn;
  if (o.snssaiIsSet()) j["snssai"] = o.m_Snssai;
  j["afAppId"]       = o.m_AfAppId;
  j["multiAccCtrls"] = o.m_MultiAccCtrls;
  if (o.suppFeatIsSet()) j["suppFeat"] = o.m_SuppFeat;
  if (o.resUriIsSet()) j["resUri"] = o.m_ResUri;
}

void from_json(const nlohmann::json& j, IptvConfigData& o) {
  if (j.find("supi") != j.end()) {
    j.at("supi").get_to(o.m_Supi);
    o.m_SupiIsSet = true;
  }
  if (j.find("interGroupId") != j.end()) {
    j.at("interGroupId").get_to(o.m_InterGroupId);
    o.m_InterGroupIdIsSet = true;
  }
  if (j.find("dnn") != j.end()) {
    j.at("dnn").get_to(o.m_Dnn);
    o.m_DnnIsSet = true;
  }
  if (j.find("snssai") != j.end()) {
    j.at("snssai").get_to(o.m_Snssai);
    o.m_SnssaiIsSet = true;
  }
  j.at("afAppId").get_to(o.m_AfAppId);
  j.at("multiAccCtrls").get_to(o.m_MultiAccCtrls);
  if (j.find("suppFeat") != j.end()) {
    j.at("suppFeat").get_to(o.m_SuppFeat);
    o.m_SuppFeatIsSet = true;
  }
  if (j.find("resUri") != j.end()) {
    j.at("resUri").get_to(o.m_ResUri);
    o.m_ResUriIsSet = true;
  }
}

std::string IptvConfigData::getSupi() const {
  return m_Supi;
}
void IptvConfigData::setSupi(std::string const& value) {
  m_Supi      = value;
  m_SupiIsSet = true;
}
bool IptvConfigData::supiIsSet() const {
  return m_SupiIsSet;
}
void IptvConfigData::unsetSupi() {
  m_SupiIsSet = false;
}
std::string IptvConfigData::getInterGroupId() const {
  return m_InterGroupId;
}
void IptvConfigData::setInterGroupId(std::string const& value) {
  m_InterGroupId      = value;
  m_InterGroupIdIsSet = true;
}
bool IptvConfigData::interGroupIdIsSet() const {
  return m_InterGroupIdIsSet;
}
void IptvConfigData::unsetInterGroupId() {
  m_InterGroupIdIsSet = false;
}
std::string IptvConfigData::getDnn() const {
  return m_Dnn;
}
void IptvConfigData::setDnn(std::string const& value) {
  m_Dnn      = value;
  m_DnnIsSet = true;
}
bool IptvConfigData::dnnIsSet() const {
  return m_DnnIsSet;
}
void IptvConfigData::unsetDnn() {
  m_DnnIsSet = false;
}
oai::model::common::Snssai IptvConfigData::getSnssai() const {
  return m_Snssai;
}
void IptvConfigData::setSnssai(oai::model::common::Snssai const& value) {
  m_Snssai      = value;
  m_SnssaiIsSet = true;
}
bool IptvConfigData::snssaiIsSet() const {
  return m_SnssaiIsSet;
}
void IptvConfigData::unsetSnssai() {
  m_SnssaiIsSet = false;
}
std::string IptvConfigData::getAfAppId() const {
  return m_AfAppId;
}
void IptvConfigData::setAfAppId(std::string const& value) {
  m_AfAppId = value;
}
std::map<std::string, MulticastAccessControl>&
IptvConfigData::getMultiAccCtrls() {
  return m_MultiAccCtrls;
}
void IptvConfigData::setMultiAccCtrls(
    std::map<std::string, MulticastAccessControl> const& value) {
  m_MultiAccCtrls = value;
}
std::string IptvConfigData::getSuppFeat() const {
  return m_SuppFeat;
}
void IptvConfigData::setSuppFeat(std::string const& value) {
  m_SuppFeat      = value;
  m_SuppFeatIsSet = true;
}
bool IptvConfigData::suppFeatIsSet() const {
  return m_SuppFeatIsSet;
}
void IptvConfigData::unsetSuppFeat() {
  m_SuppFeatIsSet = false;
}
std::string IptvConfigData::getResUri() const {
  return m_ResUri;
}
void IptvConfigData::setResUri(std::string const& value) {
  m_ResUri      = value;
  m_ResUriIsSet = true;
}
bool IptvConfigData::resUriIsSet() const {
  return m_ResUriIsSet;
}
void IptvConfigData::unsetResUri() {
  m_ResUriIsSet = false;
}

}  // namespace oai::udr::model
