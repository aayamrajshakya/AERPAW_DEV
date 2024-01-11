/**
 * Nsmf_EventExposure
 *
 * UsageReport.cpp
 */

#include "UsageReport.h"
// #include <sstream>

namespace oai {
namespace smf_server {
namespace model {

UsageReport::UsageReport() {
  m_SEndID        = 0;
  m_SEndIDIsSet   = false;
  m_urSeqN        = 0;
  m_urSeqNIsSet   = false;
  m_duration      = 0;
  m_durationIsSet = false;
  m_totNoP        = 0;
  m_totNoPIsSet   = false;
  m_ulNoP         = 0;
  m_ulNoPIsSet    = false;
  m_dlNoP         = 0;
  m_dlNoPIsSet    = false;
  m_totVol        = 0;
  m_totVolIsSet   = false;
  m_ulVol         = 0;
  m_ulVolIsSet    = false;
  m_dlVol         = 0;
  m_dlVolIsSet    = false;
  m_urTrig        = {};
  m_urTrigIsSet   = false;
}

UsageReport::~UsageReport() {}

void UsageReport::validate() {
  // TODO: implement validation
}

void to_json(nlohmann::json& j, const UsageReport& o) {
  j = nlohmann::json();
  if (o.SEndIDIsSet()) j["SEID"] = o.m_SEndID;
  if (o.urSeqNIsSet()) j["UR-SEQN"] = o.m_urSeqN;
  if (o.urTriggerIsSet()) {
    if (o.m_urTrig.perio) j["Trigger"] = "Periodic Reporting";
    if (o.m_urTrig.volth) j["Trigger"] = "Volume Threshold";
    if (o.m_urTrig.timth) j["Trigger"] = "Time Threshold";
    if (o.m_urTrig.volqu) j["Trigger"] = "Volume Quota";
    if (o.m_urTrig.timqu) j["Trigger"] = "Time Quota";
    /*
    switch (o.m_urTrig) {
      case :
        j["Trigger"] = "Periodic Reporting";
        break;
      case UsageReportTrigger::VOLTH:
        j["Trigger"] = "Volume Threshold";
        break;
      case UsageReportTrigger::TIMTH:
        j["Trigger"] = "Time Threshold";
        break;
      case UsageReportTrigger::VOLQU:
        j["Trigger"] = "Volume Quota";
        break;
      case UsageReportTrigger::TIMQU:
        j["Trigger"] = "Time Quota";
        break;
    }
    */
  }
  if (o.durationIsSet()) j["Duration"] = o.m_duration;
  if (o.totNoPIsSet()) j["NoP"]["Total"] = o.m_totNoP;
  if (o.ulNoPIsSet()) j["NoP"]["Uplink"] = o.m_ulNoP;
  if (o.dlNoPIsSet()) j["NoP"]["Downlink"] = o.m_dlNoP;
  if (o.totVolIsSet()) j["Volume"]["Total"] = o.m_totVol;
  if (o.ulVolIsSet()) j["Volume"]["Uplink"] = o.m_ulVol;
  if (o.dlVolIsSet()) j["Volume"]["Downlink"] = o.m_dlVol;
}

void from_json(const nlohmann::json& j, UsageReport& o) {
  if (j.find("SEID") != j.end()) {
    j.at("SEID").get_to(o.m_SEndID);
    o.m_SEndIDIsSet = true;
  }
  if (j.find("UR-SEQN") != j.end()) {
    j.at("UR-SEQN").get_to(o.m_urSeqN);
    o.m_urSeqNIsSet = true;
  }
  if (j.find("Trigger") != j.end()) {
    o.m_urTrigIsSet = true;
    auto s          = j.get<std::string>();
    s               = j.at("Trigger");
    if (s == "Periodic Reporting") o.m_urTrig.perio = 1;
    if (s == "Volume Threshold") o.m_urTrig.volth = 1;
    if (s == "Time Threshold") o.m_urTrig.timth = 1;
    if (s == "Volume Quota") o.m_urTrig.volqu = 1;
    if (s == "Time Quota")
      o.m_urTrig.timqu = 1;
    else {
      o.m_urTrigIsSet = false;
      // TODO: Handle invalid JSON
    }
  }
  if (j.find("Duration") != j.end()) {
    j.at("Duration").get_to(o.m_duration);
    o.m_durationIsSet = true;
  }
  if (j.find("NoP") != j.end()) {
    if (j.find("Total") != j.end()) {
      j.at("Total").get_to(o.m_totNoP);
      o.m_totNoPIsSet = true;
    }
    if (j.find("Uplink") != j.end()) {
      j.at("Uplink").get_to(o.m_ulNoP);
      o.m_ulNoPIsSet = true;
    }
    if (j.find("Downlink") != j.end()) {
      j.at("Downlink").get_to(o.m_dlNoP);
      o.m_dlNoPIsSet = true;
    }
  }
  if (j.find("Volume") != j.end()) {
    if (j.find("Total") != j.end()) {
      j.at("Total").get_to(o.m_totVol);
      o.m_totVolIsSet = true;
    }
    if (j.find("Uplink") != j.end()) {
      j.at("Uplink").get_to(o.m_ulVol);
      o.m_ulVolIsSet = true;
    }
    if (j.find("Downlink") != j.end()) {
      j.at("Downlink").get_to(o.m_dlVol);
      o.m_dlVolIsSet = true;
    }
  }
}

int64_t UsageReport::getSEndID() const {
  return m_SEndID;
}
void UsageReport::setSEndID(int64_t const& value) {
  m_SEndID      = value;
  m_SEndIDIsSet = true;
}
bool UsageReport::SEndIDIsSet() const {
  return m_SEndIDIsSet;
}
void UsageReport::unsetSEndID() {
  m_SEndIDIsSet = false;
}
int32_t UsageReport::geturSeqN() const {
  return m_urSeqN;
}
void UsageReport::seturSeqN(int32_t const& value) {
  m_urSeqN      = value;
  m_urSeqNIsSet = true;
}
bool UsageReport::urSeqNIsSet() const {
  return m_urSeqNIsSet;
}
void UsageReport::unseturSeqN() {
  m_urSeqNIsSet = false;
}
int32_t UsageReport::getDuration() const {
  return m_duration;
}
void UsageReport::setDuration(int32_t const& value) {
  m_duration      = value;
  m_durationIsSet = true;
}
bool UsageReport::durationIsSet() const {
  return m_durationIsSet;
}
void UsageReport::unsetDuration() {
  m_durationIsSet = false;
}
int64_t UsageReport::getTotNoP() const {
  return m_totNoP;
}
void UsageReport::setTotNoP(int64_t const& value) {
  m_totNoP      = value;
  m_totNoPIsSet = true;
}
bool UsageReport::totNoPIsSet() const {
  return m_totNoPIsSet;
}
void UsageReport::unsetTotNoP() {
  m_totNoPIsSet = false;
}
int64_t UsageReport::getUlNoP() const {
  return m_ulNoP;
}
void UsageReport::setUlNoP(int64_t const& value) {
  m_ulNoP      = value;
  m_ulNoPIsSet = true;
}
bool UsageReport::ulNoPIsSet() const {
  return m_ulNoPIsSet;
}
void UsageReport::unsetUlNoP() {
  m_ulNoPIsSet = false;
}
int64_t UsageReport::getDlNoP() const {
  return m_dlNoP;
}
void UsageReport::setDlNoP(int64_t const& value) {
  m_dlNoP      = value;
  m_dlNoPIsSet = true;
}
bool UsageReport::dlNoPIsSet() const {
  return m_dlNoPIsSet;
}
void UsageReport::unsetDlNoP() {
  m_dlNoPIsSet = false;
}
int64_t UsageReport::getTotVol() const {
  return m_totVol;
}
void UsageReport::setTotVol(int64_t const& value) {
  m_totVol      = value;
  m_totVolIsSet = true;
}
bool UsageReport::totVolIsSet() const {
  return m_totVolIsSet;
}
void UsageReport::unsetTotVol() {
  m_totVolIsSet = false;
}
int64_t UsageReport::getUlVol() const {
  return m_ulVol;
}
void UsageReport::setUlVol(int64_t const& value) {
  m_ulVol      = value;
  m_ulVolIsSet = true;
}
bool UsageReport::ulVolIsSet() const {
  return m_ulVolIsSet;
}
void UsageReport::unsetUlVol() {
  m_ulVolIsSet = false;
}
int64_t UsageReport::getDlVol() const {
  return m_dlVol;
}
void UsageReport::setDlVol(int64_t const& value) {
  m_dlVol      = value;
  m_dlVolIsSet = true;
}
bool UsageReport::dlVolIsSet() const {
  return m_dlVolIsSet;
}
void UsageReport::unsetDlVol() {
  m_dlVolIsSet = false;
}
pfcp::usage_report_trigger_t UsageReport::getURTrigger() const {
  return m_urTrig;
}
void UsageReport::setURTrigger(pfcp::usage_report_trigger_t const& value) {
  m_urTrig      = value;
  m_urTrigIsSet = true;
}
bool UsageReport::urTriggerIsSet() const {
  return m_urTrigIsSet;
}
void UsageReport::unsetURTrigger() {
  m_urTrigIsSet = false;
}

}  // namespace model
}  // namespace smf_server
}  // namespace oai
