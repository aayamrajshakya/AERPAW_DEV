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

#include "ChargingData.h"
#include "Helpers.h"

#include <sstream>

namespace oai::model::pcf {

ChargingData::ChargingData() {
  m_ChgId                     = "";
  m_MeteringMethodIsSet       = false;
  m_Offline                   = false;
  m_OfflineIsSet              = false;
  m_Online                    = false;
  m_OnlineIsSet               = false;
  m_SdfHandl                  = false;
  m_SdfHandlIsSet             = false;
  m_RatingGroup               = 0;
  m_RatingGroupIsSet          = false;
  m_ReportingLevelIsSet       = false;
  m_ServiceId                 = 0;
  m_ServiceIdIsSet            = false;
  m_SponsorId                 = "";
  m_SponsorIdIsSet            = false;
  m_AppSvcProvId              = "";
  m_AppSvcProvIdIsSet         = false;
  m_AfChargingIdentifier      = 0;
  m_AfChargingIdentifierIsSet = false;
  m_AfChargId                 = "";
  m_AfChargIdIsSet            = false;
}

void ChargingData::validate() const {
  std::stringstream msg;
  if (!validate(msg)) {
    throw oai::model::common::helpers::ValidationException(msg.str());
  }
}

bool ChargingData::validate(std::stringstream& msg) const {
  return validate(msg, "");
}

bool ChargingData::validate(
    std::stringstream& /* msg */, const std::string& /* pathPrefix */) const {
  bool success = true;
  /*
  const std::string _pathPrefix =
      pathPrefix.empty() ? "ChargingData" : pathPrefix;
  }
  */
  return success;
}

bool ChargingData::operator==(const ChargingData& rhs) const {
  return

      (getChgId() == rhs.getChgId()) &&

      ((!meteringMethodIsSet() && !rhs.meteringMethodIsSet()) ||
       (meteringMethodIsSet() && rhs.meteringMethodIsSet() &&
        getMeteringMethod() == rhs.getMeteringMethod())) &&

      ((!offlineIsSet() && !rhs.offlineIsSet()) ||
       (offlineIsSet() && rhs.offlineIsSet() &&
        isOffline() == rhs.isOffline())) &&

      ((!onlineIsSet() && !rhs.onlineIsSet()) ||
       (onlineIsSet() && rhs.onlineIsSet() && isOnline() == rhs.isOnline())) &&

      ((!sdfHandlIsSet() && !rhs.sdfHandlIsSet()) ||
       (sdfHandlIsSet() && rhs.sdfHandlIsSet() &&
        isSdfHandl() == rhs.isSdfHandl())) &&

      ((!ratingGroupIsSet() && !rhs.ratingGroupIsSet()) ||
       (ratingGroupIsSet() && rhs.ratingGroupIsSet() &&
        getRatingGroup() == rhs.getRatingGroup())) &&

      ((!reportingLevelIsSet() && !rhs.reportingLevelIsSet()) ||
       (reportingLevelIsSet() && rhs.reportingLevelIsSet() &&
        getReportingLevel() == rhs.getReportingLevel())) &&

      ((!serviceIdIsSet() && !rhs.serviceIdIsSet()) ||
       (serviceIdIsSet() && rhs.serviceIdIsSet() &&
        getServiceId() == rhs.getServiceId())) &&

      ((!sponsorIdIsSet() && !rhs.sponsorIdIsSet()) ||
       (sponsorIdIsSet() && rhs.sponsorIdIsSet() &&
        getSponsorId() == rhs.getSponsorId())) &&

      ((!appSvcProvIdIsSet() && !rhs.appSvcProvIdIsSet()) ||
       (appSvcProvIdIsSet() && rhs.appSvcProvIdIsSet() &&
        getAppSvcProvId() == rhs.getAppSvcProvId())) &&

      ((!afChargingIdentifierIsSet() && !rhs.afChargingIdentifierIsSet()) ||
       (afChargingIdentifierIsSet() && rhs.afChargingIdentifierIsSet() &&
        getAfChargingIdentifier() == rhs.getAfChargingIdentifier())) &&

      ((!afChargIdIsSet() && !rhs.afChargIdIsSet()) ||
       (afChargIdIsSet() && rhs.afChargIdIsSet() &&
        getAfChargId() == rhs.getAfChargId()))

          ;
}

bool ChargingData::operator!=(const ChargingData& rhs) const {
  return !(*this == rhs);
}

void to_json(nlohmann::json& j, const ChargingData& o) {
  j          = nlohmann::json();
  j["chgId"] = o.m_ChgId;
  if (o.meteringMethodIsSet()) j["meteringMethod"] = o.m_MeteringMethod;
  if (o.offlineIsSet()) j["offline"] = o.m_Offline;
  if (o.onlineIsSet()) j["online"] = o.m_Online;
  if (o.sdfHandlIsSet()) j["sdfHandl"] = o.m_SdfHandl;
  if (o.ratingGroupIsSet()) j["ratingGroup"] = o.m_RatingGroup;
  if (o.reportingLevelIsSet()) j["reportingLevel"] = o.m_ReportingLevel;
  if (o.serviceIdIsSet()) j["serviceId"] = o.m_ServiceId;
  if (o.sponsorIdIsSet()) j["sponsorId"] = o.m_SponsorId;
  if (o.appSvcProvIdIsSet()) j["appSvcProvId"] = o.m_AppSvcProvId;
  if (o.afChargingIdentifierIsSet())
    j["afChargingIdentifier"] = o.m_AfChargingIdentifier;
  if (o.afChargIdIsSet()) j["afChargId"] = o.m_AfChargId;
}

void from_json(const nlohmann::json& j, ChargingData& o) {
  j.at("chgId").get_to(o.m_ChgId);
  if (j.find("meteringMethod") != j.end()) {
    j.at("meteringMethod").get_to(o.m_MeteringMethod);
    o.m_MeteringMethodIsSet = true;
  }
  if (j.find("offline") != j.end()) {
    j.at("offline").get_to(o.m_Offline);
    o.m_OfflineIsSet = true;
  }
  if (j.find("online") != j.end()) {
    j.at("online").get_to(o.m_Online);
    o.m_OnlineIsSet = true;
  }
  if (j.find("sdfHandl") != j.end()) {
    j.at("sdfHandl").get_to(o.m_SdfHandl);
    o.m_SdfHandlIsSet = true;
  }
  if (j.find("ratingGroup") != j.end()) {
    j.at("ratingGroup").get_to(o.m_RatingGroup);
    o.m_RatingGroupIsSet = true;
  }
  if (j.find("reportingLevel") != j.end()) {
    j.at("reportingLevel").get_to(o.m_ReportingLevel);
    o.m_ReportingLevelIsSet = true;
  }
  if (j.find("serviceId") != j.end()) {
    j.at("serviceId").get_to(o.m_ServiceId);
    o.m_ServiceIdIsSet = true;
  }
  if (j.find("sponsorId") != j.end()) {
    j.at("sponsorId").get_to(o.m_SponsorId);
    o.m_SponsorIdIsSet = true;
  }
  if (j.find("appSvcProvId") != j.end()) {
    j.at("appSvcProvId").get_to(o.m_AppSvcProvId);
    o.m_AppSvcProvIdIsSet = true;
  }
  if (j.find("afChargingIdentifier") != j.end()) {
    j.at("afChargingIdentifier").get_to(o.m_AfChargingIdentifier);
    o.m_AfChargingIdentifierIsSet = true;
  }
  if (j.find("afChargId") != j.end()) {
    j.at("afChargId").get_to(o.m_AfChargId);
    o.m_AfChargIdIsSet = true;
  }
}

std::string ChargingData::getChgId() const {
  return m_ChgId;
}
void ChargingData::setChgId(std::string const& value) {
  m_ChgId = value;
}
oai::model::pcf::MeteringMethod ChargingData::getMeteringMethod() const {
  return m_MeteringMethod;
}
void ChargingData::setMeteringMethod(
    oai::model::pcf::MeteringMethod const& value) {
  m_MeteringMethod      = value;
  m_MeteringMethodIsSet = true;
}
bool ChargingData::meteringMethodIsSet() const {
  return m_MeteringMethodIsSet;
}
void ChargingData::unsetMeteringMethod() {
  m_MeteringMethodIsSet = false;
}
bool ChargingData::isOffline() const {
  return m_Offline;
}
void ChargingData::setOffline(bool const value) {
  m_Offline      = value;
  m_OfflineIsSet = true;
}
bool ChargingData::offlineIsSet() const {
  return m_OfflineIsSet;
}
void ChargingData::unsetOffline() {
  m_OfflineIsSet = false;
}
bool ChargingData::isOnline() const {
  return m_Online;
}
void ChargingData::setOnline(bool const value) {
  m_Online      = value;
  m_OnlineIsSet = true;
}
bool ChargingData::onlineIsSet() const {
  return m_OnlineIsSet;
}
void ChargingData::unsetOnline() {
  m_OnlineIsSet = false;
}
bool ChargingData::isSdfHandl() const {
  return m_SdfHandl;
}
void ChargingData::setSdfHandl(bool const value) {
  m_SdfHandl      = value;
  m_SdfHandlIsSet = true;
}
bool ChargingData::sdfHandlIsSet() const {
  return m_SdfHandlIsSet;
}
void ChargingData::unsetSdfHandl() {
  m_SdfHandlIsSet = false;
}
uint32_t ChargingData::getRatingGroup() const {
  return m_RatingGroup;
}
void ChargingData::setRatingGroup(uint32_t const value) {
  m_RatingGroup      = value;
  m_RatingGroupIsSet = true;
}
bool ChargingData::ratingGroupIsSet() const {
  return m_RatingGroupIsSet;
}
void ChargingData::unsetRatingGroup() {
  m_RatingGroupIsSet = false;
}
oai::model::pcf::ReportingLevel ChargingData::getReportingLevel() const {
  return m_ReportingLevel;
}
void ChargingData::setReportingLevel(
    oai::model::pcf::ReportingLevel const& value) {
  m_ReportingLevel      = value;
  m_ReportingLevelIsSet = true;
}
bool ChargingData::reportingLevelIsSet() const {
  return m_ReportingLevelIsSet;
}
void ChargingData::unsetReportingLevel() {
  m_ReportingLevelIsSet = false;
}
uint32_t ChargingData::getServiceId() const {
  return m_ServiceId;
}
void ChargingData::setServiceId(uint32_t const value) {
  m_ServiceId      = value;
  m_ServiceIdIsSet = true;
}
bool ChargingData::serviceIdIsSet() const {
  return m_ServiceIdIsSet;
}
void ChargingData::unsetServiceId() {
  m_ServiceIdIsSet = false;
}
std::string ChargingData::getSponsorId() const {
  return m_SponsorId;
}
void ChargingData::setSponsorId(std::string const& value) {
  m_SponsorId      = value;
  m_SponsorIdIsSet = true;
}
bool ChargingData::sponsorIdIsSet() const {
  return m_SponsorIdIsSet;
}
void ChargingData::unsetSponsorId() {
  m_SponsorIdIsSet = false;
}
std::string ChargingData::getAppSvcProvId() const {
  return m_AppSvcProvId;
}
void ChargingData::setAppSvcProvId(std::string const& value) {
  m_AppSvcProvId      = value;
  m_AppSvcProvIdIsSet = true;
}
bool ChargingData::appSvcProvIdIsSet() const {
  return m_AppSvcProvIdIsSet;
}
void ChargingData::unsetAppSvcProvId() {
  m_AppSvcProvIdIsSet = false;
}
uint32_t ChargingData::getAfChargingIdentifier() const {
  return m_AfChargingIdentifier;
}
void ChargingData::setAfChargingIdentifier(uint32_t const value) {
  m_AfChargingIdentifier      = value;
  m_AfChargingIdentifierIsSet = true;
}
bool ChargingData::afChargingIdentifierIsSet() const {
  return m_AfChargingIdentifierIsSet;
}
void ChargingData::unsetAfChargingIdentifier() {
  m_AfChargingIdentifierIsSet = false;
}
std::string ChargingData::getAfChargId() const {
  return m_AfChargId;
}
void ChargingData::setAfChargId(std::string const& value) {
  m_AfChargId      = value;
  m_AfChargIdIsSet = true;
}
bool ChargingData::afChargIdIsSet() const {
  return m_AfChargIdIsSet;
}
void ChargingData::unsetAfChargId() {
  m_AfChargIdIsSet = false;
}

}  // namespace oai::model::pcf
