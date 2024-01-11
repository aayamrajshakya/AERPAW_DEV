/**
 * Nsmf_EventExposure
 *
 * UsageReport.h
 */

#ifndef UsageReport_H_
#define UsageReport_H_

#include "msg_pfcp.hpp"
#include <nlohmann/json.hpp>

namespace oai {
namespace smf_server {
namespace model {

// TODO: Redefine in separate files
enum UsageReportTrigger { PERIO = 1, VOLTH, TIMTH, VOLQU, TIMQU };

/// <summary>
///
/// </summary>
class UsageReport {
 public:
  UsageReport();
  virtual ~UsageReport();

  void validate();

  /////////////////////////////////////////////
  /// UsageReport members

  /// <summary>
  ///
  /// </summary>
  int64_t getSEndID() const;
  void setSEndID(int64_t const& value);
  bool SEndIDIsSet() const;
  void unsetSEndID();

  /// <summary>
  ///
  /// </summary>
  int32_t geturSeqN() const;
  void seturSeqN(int32_t const& value);
  bool urSeqNIsSet() const;
  void unseturSeqN();

  /// <summary>
  ///
  /// </summary>
  int32_t getDuration() const;
  void setDuration(int32_t const& value);
  bool durationIsSet() const;
  void unsetDuration();

  /// <summary>
  ///
  /// </summary>
  int64_t getTotNoP() const;
  void setTotNoP(int64_t const& value);
  bool totNoPIsSet() const;
  void unsetTotNoP();

  /// <summary>
  ///
  /// </summary>
  int64_t getUlNoP() const;
  void setUlNoP(int64_t const& value);
  bool ulNoPIsSet() const;
  void unsetUlNoP();

  /// <summary>
  ///
  /// </summary>
  int64_t getDlNoP() const;
  void setDlNoP(int64_t const& value);
  bool dlNoPIsSet() const;
  void unsetDlNoP();

  /// <summary>
  ///
  /// </summary>
  int64_t getTotVol() const;
  void setTotVol(int64_t const& value);
  bool totVolIsSet() const;
  void unsetTotVol();

  /// <summary>
  ///
  /// </summary>
  int64_t getUlVol() const;
  void setUlVol(int64_t const& value);
  bool ulVolIsSet() const;
  void unsetUlVol();

  /// <summary>
  ///
  /// </summary>
  int64_t getDlVol() const;
  void setDlVol(int64_t const& value);
  bool dlVolIsSet() const;
  void unsetDlVol();

  /// <summary>
  ///
  /// </summary>
  pfcp::usage_report_trigger_t getURTrigger() const;
  void setURTrigger(pfcp::usage_report_trigger_t const& value);
  bool urTriggerIsSet() const;
  void unsetURTrigger();

  friend void to_json(nlohmann::json& j, const UsageReport& o);
  friend void from_json(const nlohmann::json& j, UsageReport& o);

 protected:
  int64_t m_SEndID;
  bool m_SEndIDIsSet;
  int32_t m_urSeqN;
  bool m_urSeqNIsSet;
  int32_t m_duration;
  bool m_durationIsSet;
  int64_t m_totNoP;
  bool m_totNoPIsSet;
  int64_t m_ulNoP;
  bool m_ulNoPIsSet;
  int64_t m_dlNoP;
  bool m_dlNoPIsSet;
  int64_t m_totVol;
  bool m_totVolIsSet;
  int64_t m_ulVol;
  bool m_ulVolIsSet;
  int64_t m_dlVol;
  bool m_dlVolIsSet;
  // UsageReportTrigger m_urTrig;
  pfcp::usage_report_trigger_t m_urTrig;
  bool m_urTrigIsSet;
};

}  // namespace model
}  // namespace smf_server
}  // namespace oai

#endif /* UsageReport_H_ */
