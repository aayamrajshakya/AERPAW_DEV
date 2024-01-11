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

/*! \file smf_context.cpp
 \brief
 \author  Lionel GAUTHIER, Tien-Thinh NGUYEN
 \company Eurecom
 \date 2019
 \email: lionel.gauthier@eurecom.fr, tien-thinh.nguyen@eurecom.fr
 */

#include "smf_context.hpp"

#include <algorithm>
#include <boost/algorithm/string.hpp>

#include "3gpp_24.501.h"
#include "3gpp_29.500.h"
#include "3gpp_29.502.h"
#include "SmContextCreatedData.h"
#include "RefToBinaryData.h"
#include "SmContextUpdateError.h"
#include "itti.hpp"
#include "logger.hpp"
#include "smf_app.hpp"
#include "smf_config.hpp"
#include "smf_event.hpp"
#include "smf_n1.hpp"
#include "smf_sbi.hpp"
#include "smf_n2.hpp"
#include "smf_n7.hpp"
#include "smf_paa_dynamic.hpp"
#include "smf_pfcp_association.hpp"
#include "smf_procedure.hpp"
#include "3gpp_conversions.hpp"
#include "string.hpp"
#include "EventNotification.h"
#include "SmPolicyContextData.h"
#include "SmPolicyDecision.h"
#include "SmPolicyDeleteData.h"
#include "PlmnId.h"
#include "Snssai.h"
#include "PduSessionType.h"

extern "C" {
#include "Ngap_AssociatedQosFlowItem.h"
#include "Ngap_GTPTunnel.h"
#include "Ngap_PDUSessionResourceModifyResponseTransfer.h"
#include "Ngap_PDUSessionResourceReleaseResponseTransfer.h"
#include "Ngap_PDUSessionResourceSetupResponseTransfer.h"
#include "Ngap_PDUSessionResourceSetupUnsuccessfulTransfer.h"
#include "Ngap_QosFlowAddOrModifyResponseItem.h"
#include "Ngap_QosFlowAddOrModifyResponseList.h"
#include "Ngap_PathSwitchRequestTransfer.h"
#include "Ngap_PathSwitchRequestSetupFailedTransfer.h"
#include "Ngap_QosFlowAcceptedItem.h"
#include "Ngap_HandoverRequiredTransfer.h"
#include "Ngap_HandoverRequestAcknowledgeTransfer.h"
#include "Ngap_QosFlowItemWithDataForwarding.h"
#include "Ngap_HandoverResourceAllocationUnsuccessfulTransfer.h"
#include "Ngap_SecondaryRATDataUsageReportTransfer.h"
#include "dynamic_memory_check.h"
}

using namespace smf;

extern itti_mw* itti_inst;
extern smf::smf_app* smf_app_inst;
extern std::unique_ptr<oai::config::smf::smf_config> smf_cfg;

//------------------------------------------------------------------------------
void smf_pdu_session::get_pdu_session_id(uint32_t& psi) const {
  psi = pdu_session_id;
}

//------------------------------------------------------------------------------
uint32_t smf_pdu_session::get_pdu_session_id() const {
  return pdu_session_id;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set(const paa_t& paa) {
  switch (paa.pdu_session_type.pdu_session_type) {
    case PDU_SESSION_TYPE_E_IPV4:
      ipv4                              = true;
      ipv6                              = false;
      ipv4_address                      = paa.ipv4_address;
      pdu_session_type.pdu_session_type = paa.pdu_session_type.pdu_session_type;
      break;
    case PDU_SESSION_TYPE_E_IPV6:
      ipv4                              = false;
      ipv6                              = true;
      ipv6_address                      = paa.ipv6_address;
      pdu_session_type.pdu_session_type = paa.pdu_session_type.pdu_session_type;
      break;
    case PDU_SESSION_TYPE_E_IPV4V6:
      ipv4                              = true;
      ipv6                              = true;
      ipv4_address                      = paa.ipv4_address;
      ipv6_address                      = paa.ipv6_address;
      pdu_session_type.pdu_session_type = paa.pdu_session_type.pdu_session_type;
      break;
    case PDU_SESSION_TYPE_E_UNSTRUCTURED:
    case PDU_SESSION_TYPE_E_ETHERNET:
    case PDU_SESSION_TYPE_E_RESERVED:
      ipv4                              = false;
      ipv6                              = false;
      pdu_session_type.pdu_session_type = paa.pdu_session_type.pdu_session_type;
      break;
    default:
      Logger::smf_app().error(
          "smf_pdu_session::set(paa_t) Unknown PDN type %d",
          paa.pdu_session_type.pdu_session_type);
  }
}

//------------------------------------------------------------------------------
void smf_pdu_session::get_paa(paa_t& paa) {
  switch (pdu_session_type.pdu_session_type) {
    case PDU_SESSION_TYPE_E_IPV4:
      ipv4             = true;
      ipv6             = false;
      paa.ipv4_address = ipv4_address;
      break;
    case PDU_SESSION_TYPE_E_IPV6:
      ipv4             = false;
      ipv6             = true;
      paa.ipv6_address = ipv6_address;
      break;
    case PDU_SESSION_TYPE_E_IPV4V6:
      ipv4             = true;
      ipv6             = true;
      paa.ipv4_address = ipv4_address;
      paa.ipv6_address = ipv6_address;
      break;
    case PDU_SESSION_TYPE_E_UNSTRUCTURED:
    case PDU_SESSION_TYPE_E_ETHERNET:
    case PDU_SESSION_TYPE_E_RESERVED:
      ipv4 = false;
      ipv6 = false;
      break;
    default:
      Logger::smf_app().error(
          "smf_pdu_session::get_paa (paa_t) Unknown PDN type %d",
          pdu_session_type.pdu_session_type);
  }
  paa.pdu_session_type.pdu_session_type = pdu_session_type.pdu_session_type;
}

//------------------------------------------------------------------------------
void smf_pdu_session::add_qos_flow(const smf_qos_flow& flow) {
  if ((flow.qfi.qfi >= QOS_FLOW_IDENTIFIER_FIRST) and
      (flow.qfi.qfi <= QOS_FLOW_IDENTIFIER_LAST)) {
    Logger::smf_app().trace(
        "QoS Flow (flow Id %d) has been added successfully", flow.qfi.qfi);
    std::unique_lock lock(m_pdu_session_mutex);
    qos_flows.erase(flow.qfi.qfi);
    qos_flows.insert(
        std::pair<uint8_t, smf_qos_flow>((uint8_t) flow.qfi.qfi, flow));
  } else {
    Logger::smf_app().error(
        "Failed to add QoS flow (flow Id %d), invalid QFI", flow.qfi.qfi);
  }
}

//------------------------------------------------------------------------------
bool smf_pdu_session::get_qos_flow(
    const pfcp::pdr_id_t& pdr_id, smf_qos_flow& q) {
  std::shared_lock lock(m_pdu_session_mutex);
  for (auto it : qos_flows) {
    if (it.second.pdr_id_ul.rule_id == pdr_id.rule_id) {
      q = it.second;
      return true;
    }
    if (it.second.pdr_id_dl.rule_id == pdr_id.rule_id) {
      q = it.second;
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
bool smf_pdu_session::get_qos_flow(
    const pfcp::far_id_t& far_id, smf_qos_flow& q) {
  std::shared_lock lock(m_pdu_session_mutex);
  for (auto it : qos_flows) {
    if ((it.second.far_id_ul.first) &&
        (it.second.far_id_ul.second.far_id == far_id.far_id)) {
      q = it.second;
      return true;
    }
    if ((it.second.far_id_dl.first) &&
        (it.second.far_id_dl.second.far_id == far_id.far_id)) {
      q = it.second;
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
bool smf_pdu_session::get_qos_flow(const pfcp::qfi_t& qfi, smf_qos_flow& q) {
  std::shared_lock lock(m_pdu_session_mutex);
  for (auto it : qos_flows) {
    if (it.second.qfi == qfi) {
      q = it.second;
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set_default_qos_flow(const pfcp::qfi_t& qfi) {
  default_qfi.qfi = qfi.qfi;
}

//------------------------------------------------------------------------------
bool smf_pdu_session::get_default_qos_flow(smf_qos_flow& flow) {
  Logger::smf_app().debug("Get default QoS Flow of this PDU session.");
  return get_qos_flow(default_qfi, flow);
}

//------------------------------------------------------------------------------
void smf_pdu_session::get_qos_flows(std::vector<smf_qos_flow>& flows) {
  std::shared_lock lock(m_pdu_session_mutex);
  flows.clear();
  for (auto it : qos_flows) {
    flows.push_back(it.second);
  }
}

//------------------------------------------------------------------------------
bool smf_pdu_session::find_qos_flow(
    const pfcp::pdr_id_t& pdr_id, smf_qos_flow& flow) {
  std::shared_lock lock(m_pdu_session_mutex);
  for (std::map<uint8_t, smf_qos_flow>::iterator it = qos_flows.begin();
       it != qos_flows.end(); ++it) {
    if ((it->second.pdr_id_ul == pdr_id) || (it->second.pdr_id_dl == pdr_id)) {
      flow = it->second;
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
void smf_pdu_session::remove_qos_flow(const pfcp::qfi_t& qfi) {
  std::unique_lock lock(m_pdu_session_mutex);
  smf_qos_flow& flow = qos_flows[qfi.qfi];
  flow.deallocate_ressources();
  qos_flows.erase(qfi.qfi);
}

//------------------------------------------------------------------------------
void smf_pdu_session::remove_qos_flow(smf_qos_flow& flow) {
  std::unique_lock lock(m_pdu_session_mutex);
  pfcp::qfi_t qfi = {};
  qfi.qfi         = flow.qfi.qfi;
  flow.deallocate_ressources();
  qos_flows.erase(qfi.qfi);
}

//------------------------------------------------------------------------------
void smf_pdu_session::remove_qos_flows() {
  std::unique_lock lock(m_pdu_session_mutex);
  for (std::map<uint8_t, smf_qos_flow>::iterator it = qos_flows.begin();
       it != qos_flows.end(); ++it) {
    it->second.deallocate_ressources();
  }
  qos_flows.clear();
}

//------------------------------------------------------------------------------
void smf_pdu_session::deallocate_ressources(const std::string& dnn) {
  for (std::map<uint8_t, smf_qos_flow>::iterator it = qos_flows.begin();
       it != qos_flows.end(); ++it) {
    // TODO: release FAR_ID, PDR_ID
    // release_pdr_id(it->second.pdr_id_dl);
    // release_pdr_id(it->second.pdr_id_ul);
    // release_far_id(it->second.far_id_dl.second);
    // release_far_id(it->second.far_id_ul.second);
    it->second.deallocate_ressources();
  }
  if (ipv4) {
    paa_dynamic::get_instance().release_paa(dnn, ipv4_address);
  }
  clear();  // including qos_flows.clear()
  Logger::smf_app().info(
      "Resources associated with this PDU Session have been released");
}

//------------------------------------------------------------------------------
void smf_pdu_session::generate_seid() {}

void smf_pdu_session::set_seid(const uint64_t& s) {
  seid = s;
}

//------------------------------------------------------------------------------
void smf_pdu_session::generate_teid(pfcp::fteid_t& local_fteid) {
  local_fteid.teid = teid_generator.get_uid();
}

//------------------------------------------------------------------------------
// TODO check if urr_id should be unique in the UPF or in the context of a pdn
// connection
void smf_pdu_session::generate_urr_id(pfcp::urr_id_t& urr_id) {
  urr_id.urr_id = urr_id_generator.get_uid();
}

//------------------------------------------------------------------------------
void smf_pdu_session::release_urr_id(const pfcp::urr_id_t& urr_id) {
  urr_id_generator.free_uid(urr_id.urr_id);
}

//------------------------------------------------------------------------------
// TODO check if far_id should be uniq in the UPF or in the context of a pdn
// connection
void smf_pdu_session::generate_far_id(pfcp::far_id_t& far_id) {
  far_id.far_id = far_id_generator.get_uid();
}

//------------------------------------------------------------------------------
void smf_pdu_session::release_far_id(const pfcp::far_id_t& far_id) {
  far_id_generator.free_uid(far_id.far_id);
}

//------------------------------------------------------------------------------
// TODO check if prd_id should be unique in the UPF or in the context of a pdn
// connection
void smf_pdu_session::generate_pdr_id(pfcp::pdr_id_t& pdr_id) {
  pdr_id.rule_id = pdr_id_generator.get_uid();
}

//------------------------------------------------------------------------------
// TODO check if prd_id should be unique in the UPF or in the context of a pdn
// connection
void smf_pdu_session::release_pdr_id(const pfcp::pdr_id_t& pdr_id) {
  pdr_id_generator.free_uid(pdr_id.rule_id);
}

//------------------------------------------------------------------------------
void smf_pdu_session::generate_qos_rule_id(uint8_t& rule_id) {
  rule_id = qos_rule_id_generator.get_uid();
}

//------------------------------------------------------------------------------
void smf_pdu_session::release_qos_rule_id(const uint8_t& rule_id) {
  qos_rule_id_generator.free_uid(rule_id);
}

//------------------------------------------------------------------------------
std::string smf_pdu_session::toString() const {
  std::string s     = {};
  smf_qos_flow flow = {};

  bool is_released = false;
  if (pdu_session_status == pdu_session_status_e::PDU_SESSION_INACTIVE)
    is_released = true;
  if (!is_released) {
    s.append("\tPDU Session ID:\t\t\t")
        .append(std::to_string((uint8_t) pdu_session_id))
        .append("\n");
    s.append("\tDNN:\t\t\t").append(dnn).append("\n");
    s.append("\tS-NSSAI:\t\t\t").append(snssai.toString()).append("\n");
    s.append("\tPDN type:\t\t")
        .append(pdu_session_type.to_string())
        .append("\n");
  }
  if (ipv4)
    s.append("\tPAA IPv4:\t\t")
        .append(conv::toString(ipv4_address))
        .append("\n");
  if (ipv6)
    s.append("\tPAA IPv6:\t\t")
        .append(conv::toString(ipv6_address))
        .append("\n");
  if (default_qfi.qfi) {
    s.append("\tDefault QFI:\t\t")
        .append(std::to_string(default_qfi.qfi))
        .append("\n");
  } else {
    s.append("\tDefault QFI:\t\t").append("No QFI available").append("\n");
  }
  if (!is_released) {
    s.append("\tSEID:\t\t\t").append(std::to_string(seid)).append("\n");
  }

  if (sessions_graph) {
    s.append(sessions_graph->to_string("\t"));
  }

  if (policy_ptr) {
    s.append("\t Policy Decision:").append("\n");
    s.append(policy_ptr->toString());
  }

  return s;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set_pdu_session_status(
    const pdu_session_status_e& status) {
  // TODO: Should consider congestion handling
  Logger::smf_app().info(
      "Set PDU Session Status to %s",
      pdu_session_status_e2str.at(static_cast<int>(status)).c_str());
  std::unique_lock lock(m_pdu_session_mutex);
  pdu_session_status = status;
}

//------------------------------------------------------------------------------
pdu_session_status_e smf_pdu_session::get_pdu_session_status() const {
  std::shared_lock lock(m_pdu_session_mutex);
  return pdu_session_status;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set_upCnx_state(const upCnx_state_e& state) {
  Logger::smf_app().info(
      "Set upCnxState to %s",
      upCnx_state_e2str.at(static_cast<int>(state)).c_str());
  std::unique_lock lock(m_pdu_session_mutex);
  upCnx_state = state;
}

//------------------------------------------------------------------------------
upCnx_state_e smf_pdu_session::get_upCnx_state() const {
  std::shared_lock lock(m_pdu_session_mutex);
  return upCnx_state;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set_ho_state(const ho_state_e& state) {
  Logger::smf_app().info(
      "Set HOState to %s", ho_state_e2str.at(static_cast<int>(state)).c_str());
  std::unique_lock lock(m_pdu_session_mutex);
  ho_state = state;
}

//------------------------------------------------------------------------------
ho_state_e smf_pdu_session::get_ho_state() const {
  std::shared_lock lock(m_pdu_session_mutex);
  return ho_state;
}

//------------------------------------------------------------------------------
pdu_session_type_t smf_pdu_session::get_pdu_session_type() const {
  return pdu_session_type;
}

//------------------------------------------------------------------------------
void smf_pdu_session::get_qos_rules_to_be_synchronised(
    std::vector<QOSRulesIE>& rules) const {
  std::shared_lock lock(m_pdu_session_mutex);
  for (auto it : qos_rules_to_be_synchronised) {
    if (qos_rules.count(it) > 0) rules.push_back(qos_rules.at(it));
  }
}

//------------------------------------------------------------------------------
void smf_pdu_session::get_qos_rules(
    const pfcp::qfi_t& qfi, std::vector<QOSRulesIE>& rules) const {
  Logger::smf_app().info(
      "Get QoS Rules associated with Flow with QFI %d", qfi.qfi);
  std::shared_lock lock(m_pdu_session_mutex);
  for (auto it : qos_rules) {
    if (it.second.qosflowidentifer == qfi.qfi)
      rules.push_back(qos_rules.at(it.first));
  }
}

//------------------------------------------------------------------------------
bool smf_pdu_session::get_default_qos_rule(QOSRulesIE& qos_rule) const {
  Logger::smf_app().info(
      "Get default QoS Rule this PDU Session (ID %d)", pdu_session_id);
  std::shared_lock lock(m_pdu_session_mutex);
  for (auto it : qos_rules) {
    if (it.second.dqrbit == THE_QOS_RULE_IS_DEFAULT_QOS_RULE) {
      qos_rule = it.second;
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
bool smf_pdu_session::get_qos_rule(
    const uint8_t rule_id, QOSRulesIE& qos_rule) const {
  Logger::smf_app().info("Find QoS Rule with Rule Id %d", (uint8_t) rule_id);
  std::shared_lock lock(m_pdu_session_mutex);
  if (qos_rules.count(rule_id) > 0) {
    qos_rule = qos_rules.at(rule_id);
  }
  return false;
}

//------------------------------------------------------------------------------
void smf_pdu_session::update_qos_rule(const QOSRulesIE& qos_rule) {
  std::unique_lock lock(
      m_pdu_session_mutex,
      std::defer_lock);  // Do not lock it first

  Logger::smf_app().info(
      "Update QoS Rule with Rule Id %d", (uint8_t) qos_rule.qosruleidentifer);
  uint8_t rule_id = qos_rule.qosruleidentifer;
  if ((rule_id >= QOS_RULE_IDENTIFIER_FIRST) and
      (rule_id <= QOS_RULE_IDENTIFIER_LAST)) {
    if (qos_rules.count(rule_id) > 0) {
      lock.lock();  // Lock it here
      qos_rules.erase(rule_id);
      qos_rules.insert(std::pair<uint8_t, QOSRulesIE>(rule_id, qos_rule));
      // marked to be synchronised with UE
      qos_rules_to_be_synchronised.push_back(rule_id);
      Logger::smf_app().trace("Update QoS rule (%d) success", rule_id);
    } else {
      Logger::smf_app().error(
          "Update QoS Rule (%d) failed, rule does not existed", rule_id);
    }

  } else {
    Logger::smf_app().error(
        "Update QoS rule (%d) failed, invalid Rule Id", rule_id);
  }
}

//------------------------------------------------------------------------------
void smf_pdu_session::mark_qos_rule_to_be_synchronised(const uint8_t rule_id) {
  std::unique_lock lock(
      m_pdu_session_mutex,
      std::defer_lock);  // Do not lock it first
  if ((rule_id >= QOS_RULE_IDENTIFIER_FIRST) and
      (rule_id <= QOS_RULE_IDENTIFIER_LAST)) {
    if (qos_rules.count(rule_id) > 0) {
      lock.lock();  // Lock it here
      qos_rules_to_be_synchronised.push_back(rule_id);
      Logger::smf_app().trace(
          "smf_pdu_session::mark_qos_rule_to_be_synchronised(%d) success",
          rule_id);
    } else {
      Logger::smf_app().error(
          "smf_pdu_session::mark_qos_rule_to_be_synchronised(%d) failed, rule "
          "does not existed",
          rule_id);
    }

  } else {
    Logger::smf_app().error(
        "smf_pdu_session::mark_qos_rule_to_be_synchronised(%d) failed, invalid "
        "Rule Id",
        rule_id);
  }
}

//------------------------------------------------------------------------------
void smf_pdu_session::add_qos_rule(const QOSRulesIE& qos_rule) {
  std::unique_lock lock(
      m_pdu_session_mutex,
      std::defer_lock);  // Do not lock it first
  Logger::smf_app().info(
      "Add QoS Rule with Rule Id %d", (uint8_t) qos_rule.qosruleidentifer);
  uint8_t rule_id = qos_rule.qosruleidentifer;

  if ((rule_id >= QOS_RULE_IDENTIFIER_FIRST) and
      (rule_id <= QOS_RULE_IDENTIFIER_LAST)) {
    if (qos_rules.count(rule_id) > 0) {
      Logger::smf_app().error(
          "Failed to add rule (Id %d), rule existed", rule_id);
    } else {
      lock.lock();  // Lock it here
      qos_rules.insert(std::pair<uint8_t, QOSRulesIE>(rule_id, qos_rule));
      Logger::smf_app().trace(
          "Rule (Id %d) has been added successfully", rule_id);
    }

  } else {
    Logger::smf_app().error(
        "Failed to add rule (Id %d) failed: invalid rule Id", rule_id);
  }
}

std::shared_ptr<upf_graph> smf_pdu_session::get_sessions_graph() const {
  return sessions_graph;
}

void smf_pdu_session::set_sessions_graph(
    const std::shared_ptr<upf_graph> upf_graph) {
  sessions_graph = upf_graph;
}

//-----------------------------------------------------------------------------
void smf_pdu_session::set_urr_id(const uint32_t& urrId) {
  urr_Id = urrId;
}

//-----------------------------------------------------------------------------
uint32_t smf_pdu_session::get_urr_id() const {
  return urr_Id;
}

//-----------------------------------------------------------------------------
void smf_pdu_session::set_nwi_access(const std::string& nwiAccess) {
  nwi_access = nwiAccess;
}

//-----------------------------------------------------------------------------
std::string smf_pdu_session::get_nwi_access() const {
  return nwi_access;
}

//-----------------------------------------------------------------------------
void smf_pdu_session::set_nwi_core(const std::string& nwiCore) {
  nwi_core = nwiCore;
}

//-----------------------------------------------------------------------------
std::string smf_pdu_session::get_nwi_core() const {
  return nwi_core;
}

//-----------------------------------------------------------------------------
std::string smf_pdu_session::get_dnn() const {
  return dnn;
}

//-----------------------------------------------------------------------------
snssai_t smf_pdu_session::get_snssai() const {
  return snssai;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set_dnn(const std::string& d) {
  dnn = d;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set_snssai(const snssai_t s) {
  snssai = s;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set_pending_n11_msg(
    const std::shared_ptr<itti_n11_msg>& msg) {
  pending_n11_msg = msg;
}

//------------------------------------------------------------------------------
void smf_pdu_session::get_pending_n11_msg(
    std::shared_ptr<itti_n11_msg>& msg) const {
  msg = pending_n11_msg;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set_number_retransmission_T3591(const uint8_t& n) {
  number_retransmission_T3591 = n;
}

//------------------------------------------------------------------------------
void smf_pdu_session::get_number_retransmission_T3591(uint8_t& n) const {
  n = number_retransmission_T3591;
}

//------------------------------------------------------------------------------
uint8_t smf_pdu_session::get_number_retransmission_T3591() const {
  return number_retransmission_T3591;
}

//------------------------------------------------------------------------------
void smf_pdu_session::set_number_retransmission_T3592(const uint8_t& n) {
  number_retransmission_T3592 = n;
}

//------------------------------------------------------------------------------
void smf_pdu_session::get_number_retransmission_T3592(uint8_t& n) const {
  n = number_retransmission_T3592;
}

//------------------------------------------------------------------------------
uint8_t smf_pdu_session::get_number_retransmission_T3592() const {
  return number_retransmission_T3592;
}

//------------------------------------------------------------------------------
void session_management_subscription::insert_dnn_configuration(
    const std::string& dnn,
    std::shared_ptr<dnn_configuration_t>& dnn_configuration) {
  std::unique_lock lock(m_mutex);
  dnn_configurations.insert(
      std::pair<std::string, std::shared_ptr<dnn_configuration_t>>(
          dnn, dnn_configuration));
}

//------------------------------------------------------------------------------
void session_management_subscription::find_dnn_configuration(
    const std::string& dnn,
    std::shared_ptr<dnn_configuration_t>& dnn_configuration) const {
  Logger::smf_app().info("Find DNN configuration with DNN %s", dnn.c_str());
  std::shared_lock lock(m_mutex);
  if (dnn_configurations.count(dnn) > 0) {
    dnn_configuration = dnn_configurations.at(dnn);
  }
}

//------------------------------------------------------------------------------
bool session_management_subscription::dnn_configuration(
    const std::string& dnn) const {
  std::shared_lock lock(m_mutex);
  if (dnn_configurations.count(dnn) > 0) {
    return true;
  } else {
    return false;
  }
}

//------------------------------------------------------------------------------
void smf_context::insert_procedure(std::shared_ptr<smf_procedure>& sproc) {
  std::unique_lock<std::recursive_mutex> lock(m_context);
  pending_procedures.push_back(sproc);
}

//------------------------------------------------------------------------------
bool smf_context::find_procedure(
    const uint64_t& trxn_id, std::shared_ptr<smf_procedure>& proc) {
  std::unique_lock<std::recursive_mutex> lock(m_context);
  auto found = std::find_if(
      pending_procedures.begin(), pending_procedures.end(),
      [trxn_id](const std::shared_ptr<smf_procedure>& i) -> bool {
        return i->trxn_id == trxn_id;
      });
  if (found != pending_procedures.end()) {
    proc = *found;
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
void smf_context::remove_procedure(smf_procedure* proc) {
  std::unique_lock<std::recursive_mutex> lock(m_context);
  auto found = std::find_if(
      pending_procedures.begin(), pending_procedures.end(),
      [proc](const std::shared_ptr<smf_procedure>& i) {
        return i.get() == proc;
      });
  if (found != pending_procedures.end()) {
    pending_procedures.erase(found);
  }
}

//------------------------------------------------------------------------------
void smf_context::handle_itti_msg(
    itti_n4_session_establishment_response& seresp) {
  std::shared_ptr<smf_procedure> proc = {};
  if (find_procedure(seresp.trxn_id, proc)) {
    Logger::smf_app().debug(
        "Received N4 Session Establishment Response sender teid " TEID_FMT
        "  pfcp_tx_id %" PRIX64 "",
        seresp.seid, seresp.trxn_id);
    smf_procedure_code res = proc->handle_itti_msg(seresp, shared_from_this());
    if (res != smf_procedure_code::CONTINUE) {
      std::shared_ptr<session_create_sm_context_procedure> proc_session_create =
          std::static_pointer_cast<session_create_sm_context_procedure>(proc);
      send_pdu_session_create_response(
          proc_session_create->n11_triggered_pending);
      remove_procedure(proc.get());
    }
  } else {
    Logger::smf_app().debug(
        "Received N4 Session Establishment Response sender teid " TEID_FMT
        "  pfcp_tx_id %" PRIX64 ", smf_procedure not found, discarded!",
        seresp.seid, seresp.trxn_id);
  }
}

//------------------------------------------------------------------------------
void smf_context::handle_itti_msg(
    itti_n4_session_modification_response& smresp) {
  std::shared_ptr<smf_procedure> proc = {};
  if (find_procedure(smresp.trxn_id, proc)) {
    Logger::smf_app().debug(
        "Received N4 Session Modification Response sender teid " TEID_FMT
        "  pfcp_tx_id %" PRIX64 " ",
        smresp.seid, smresp.trxn_id);
    smf_procedure_code res = proc->handle_itti_msg(smresp, shared_from_this());
    if (res != smf_procedure_code::CONTINUE) {
      std::shared_ptr<session_update_sm_context_procedure> proc_session_update =
          std::static_pointer_cast<session_update_sm_context_procedure>(proc);
      send_pdu_session_update_response(
          proc_session_update->n11_trigger,
          proc_session_update->n11_triggered_pending,
          proc_session_update->session_procedure_type,
          proc_session_update->sps);
      remove_procedure(proc.get());
    }
  } else {
    Logger::smf_app().debug(
        "Received N4 Session Modification Response sender teid " TEID_FMT
        "  pfcp_tx_id %" PRIX64 ", smf_procedure not found, discarded!",
        smresp.seid, smresp.trxn_id);
  }
  Logger::smf_app().info("Handle N4 Session Modification Response");
}

//------------------------------------------------------------------------------
void smf_context::handle_itti_msg(itti_n4_session_deletion_response& sdresp) {
  std::shared_ptr<smf_procedure> proc = {};
  if (find_procedure(sdresp.trxn_id, proc)) {
    Logger::smf_app().debug(
        "Received N4 Session Deletion Response sender teid " TEID_FMT
        "  pfcp_tx_id %" PRIX64 " ",
        sdresp.seid, sdresp.trxn_id);
    smf_procedure_code res = proc->handle_itti_msg(sdresp, shared_from_this());
    if (res != smf_procedure_code::CONTINUE) {
      auto proc_session_delete =
          std::static_pointer_cast<session_release_sm_context_procedure>(proc);
      send_pdu_session_release_response(
          proc_session_delete->n11_trigger,
          proc_session_delete->n11_triggered_pending,
          proc_session_delete->session_procedure_type,
          proc_session_delete->sps);
      remove_procedure(proc.get());
    }
  } else {
    Logger::smf_app().debug(
        "Received N4 Session Deletion Response sender teid " TEID_FMT
        "  pfcp_tx_id %" PRIX64 ", smf_procedure not found, discarded!",
        sdresp.seid, sdresp.trxn_id);
  }

  Logger::smf_app().info("Handle N4 Session Deletion Response");
}

//------------------------------------------------------------------------------
void smf_context::handle_itti_msg(
    std::shared_ptr<itti_n4_session_report_request>& req) {
  pfcp::report_type_t report_type;
  if (req->pfcp_ies.get(report_type)) {
    pfcp::pdr_id_t pdr_id;
    // Downlink Data Report
    if (report_type.dldr) {
      pfcp::downlink_data_report data_report;
      if (req->pfcp_ies.get(data_report)) {
        pfcp::pdr_id_t pdr_id;
        if (data_report.get(pdr_id)) {
          std::shared_ptr<smf_pdu_session> sp = {};
          pfcp::qfi_t qfi                     = {};
          if (find_pdu_session(pdr_id, qfi, sp)) {
            // Step 1. send N4 Data Report Ack to UPF
            std::shared_ptr<itti_n4_session_report_response> n4_report_ack =
                std::make_shared<itti_n4_session_report_response>(
                    TASK_SMF_APP, TASK_SMF_N4);
            n4_report_ack->seid       = req->seid;
            n4_report_ack->trxn_id    = req->trxn_id;
            n4_report_ack->r_endpoint = req->r_endpoint;

            Logger::smf_app().info(
                "Sending ITTI message %s to task TASK_SMF_N4",
                n4_report_ack->get_msg_name());
            int ret = itti_inst->send_msg(n4_report_ack);
            if (RETURNok != ret) {
              Logger::smf_app().error(
                  "Could not send ITTI message %s to task TASK_SMF_N4",
                  n4_report_ack->get_msg_name());
              return;
            }

            // Step 2. Send N1N2MessageTranfer to AMF
            pdu_session_report_response session_report_msg = {};
            // set the required IEs
            session_report_msg.set_supi(supi);
            session_report_msg.set_snssai(sp.get()->get_snssai());
            session_report_msg.set_dnn(sp.get()->get_dnn());
            session_report_msg.set_pdu_session_type(
                sp.get()->get_pdu_session_type().pdu_session_type);
            // get supi and put into URL
            std::string supi_prefix = {};
            get_supi_prefix(supi_prefix);
            std::string supi_str =
                smf_get_supi_with_prefix(supi_prefix, smf_supi_to_string(supi));
            std::string api_version =
                smf_cfg->get_nf(oai::config::AMF_CONFIG_NAME)
                    ->get_sbi()
                    .get_api_version();
            std::string url = get_amf_addr() + NAMF_COMMUNICATION_BASE +
                              api_version +
                              fmt::format(
                                  NAMF_COMMUNICATION_N1N2_MESSAGE_TRANSFER_URL,
                                  supi_str.c_str());
            session_report_msg.set_amf_url(url);
            // seid and trxn_id to be used in Failure indication
            session_report_msg.set_seid(req->seid);
            session_report_msg.set_trxn_id(req->trxn_id);

            // QFIs, QoS profiles, CN Tunnel
            smf_qos_flow flow = {};
            sp.get()->get_qos_flow(qfi, flow);
            // ADD QoS Flow to be updated
            qos_flow_context_updated qcu = {};
            qcu.set_qfi(qfi);
            qcu.set_ul_fteid(flow.ul_fteid);
            qcu.set_qos_profile(flow.qos_profile);
            session_report_msg.add_qos_flow_context_updated(qcu);

            // Create N2 SM Information: PDU Session Resource Setup Request
            // Transfer IE
            // N2 SM Information
            std::string n2_sm_info, n2_sm_info_hex;
            smf_n2::get_instance()
                .create_n2_pdu_session_resource_setup_request_transfer(
                    session_report_msg, n2_sm_info_type_e::PDU_RES_SETUP_REQ,
                    n2_sm_info);

            conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);
            session_report_msg.set_n2_sm_information(n2_sm_info_hex);

            // Fill the json part
            nlohmann::json json_data = {};
            json_data["n2InfoContainer"]["n2InformationClass"] =
                N1N2_MESSAGE_CLASS;
            json_data["n2InfoContainer"]["smInfo"]["PduSessionId"] =
                session_report_msg.get_pdu_session_id();
            // N2InfoContent (section 6.1.6.2.27@3GPP TS 29.518)
            json_data["n2InfoContainer"]["smInfo"]["n2InfoContent"]
                     ["ngapIeType"] = "PDU_RES_SETUP_REQ";  // NGAP message type
            json_data["n2InfoContainer"]["smInfo"]["n2InfoContent"]["ngapData"]
                     ["contentId"] = N2_SM_CONTENT_ID;  // NGAP part
            json_data["n2InfoContainer"]["smInfo"]["sNssai"]["sst"] =
                session_report_msg.get_snssai().sst;
            json_data["n2InfoContainer"]["smInfo"]["sNssai"]["sd"] =
                std::to_string(session_report_msg.get_snssai().sd);

            session_report_msg.set_json_data(json_data);

            std::shared_ptr<itti_n11_session_report_request> itti_n11_report =
                std::make_shared<itti_n11_session_report_request>(
                    TASK_SMF_APP, TASK_SMF_SBI);
            itti_n11_report->http_version = 1;  // use HTTPv1 for the moment
            itti_n11_report->res          = session_report_msg;
            // send ITTI message to N11 interface to trigger N1N2MessageTransfer
            // towards AMFs
            Logger::smf_app().info(
                "Sending ITTI message %s to task TASK_SMF_SBI",
                itti_n11_report->get_msg_name());

            ret = itti_inst->send_msg(itti_n11_report);
            if (RETURNok != ret) {
              Logger::smf_app().error(
                  "Could not send ITTI message %s to task TASK_SMF_SBI",
                  itti_n11_report->get_msg_name());
            }
          }
        }
      }
    }
    // Usage Report
    if (report_type.usar) {
      // TODO
      // Step 1. send N4 Data Report Ack to UPF
      pfcp::usage_report_within_pfcp_session_report_request ur;
      if (req->pfcp_ies.get(ur)) {
        pfcp::volume_measurement_t vm;
        pfcp::duration_measurement_t dm;
        pfcp::ur_seqn_t seqn;
        pfcp::usage_report_trigger_t trig;

        if (ur.get(vm)) {
          Logger::smf_app().info("\t\t SEID            -> %lld", req->seid);
          if (ur.get(seqn))
            Logger::smf_app().info("\t\t UR-SEQN         -> %ld", seqn.ur_seqn);
          if (ur.get(trig))
            if (trig.perio)
              Logger::smf_app().info(
                  "\t\t Trigger         -> Periodic Reporting");
          if (trig.timqu)
            Logger::smf_app().info("\t\t Trigger         -> Time Quota");
          if (trig.timth)
            Logger::smf_app().info("\t\t Trigger         -> Time Threshold");
          if (trig.volqu)
            Logger::smf_app().info("\t\t Trigger         -> Volume Quota");
          if (trig.volth)
            Logger::smf_app().info("\t\t Trigger         -> Volume Threshold");
          if (ur.get(dm))
            Logger::smf_app().info("\t\t Duration        -> %ld", dm.duration);
          Logger::smf_app().info("\t\t NoP    Total    -> %lld", vm.total_nop);
          Logger::smf_app().info("\t\t        Uplink   -> %lld", vm.uplink_nop);
          Logger::smf_app().info(
              "\t\t        Downlink -> %lld", vm.downlink_nop);
          Logger::smf_app().info(
              "\t\t Volume Total    -> %lld", vm.total_volume);
          Logger::smf_app().info(
              "\t\t        Uplink   -> %lld", vm.uplink_volume);
          Logger::smf_app().info(
              "\t\t        Downlink -> %lld", vm.downlink_volume);
        }

        // Trigger QoS Monitoring Event report notification
        std::shared_ptr<smf_context> pc = {};
        if (smf_app_inst->seid_2_smf_context(req->seid, pc)) {
          oai::smf_server::model::EventNotification ev_notif = {};
          oai::smf_server::model::UsageReport ur_model       = {};
          if (ur.get(vm)) {
            ur_model.setSEndID(req->seid);
            if (ur.get(seqn)) ur_model.seturSeqN(seqn.ur_seqn);
            if (ur.get(dm)) ur_model.setDuration(dm.duration);
            ur_model.setTotNoP(vm.total_nop);
            ur_model.setUlNoP(vm.uplink_nop);
            ur_model.setDlNoP(vm.downlink_nop);
            ur_model.setTotVol(vm.total_volume);
            ur_model.setUlVol(vm.uplink_volume);
            ur_model.setDlVol(vm.downlink_volume);
          }
          if (ur.usage_report_trigger.first)
            ur_model.setURTrigger(ur.usage_report_trigger.second);
          ev_notif.setUsageReport(ur_model);
          pc.get()->trigger_qos_monitoring(req->seid, ev_notif, 1);
        } else {
          Logger::smf_app().debug(
              "No SFM context found for SEID " TEID_FMT
              ". Unable to notify QoS Monitoring Event Report.",
              req->seid);
        }
      }

      std::shared_ptr<itti_n4_session_report_response> n4_report_ack =
          std::make_shared<itti_n4_session_report_response>(
              TASK_SMF_APP, TASK_SMF_N4);
      n4_report_ack->seid    = req->seid;
      n4_report_ack->trxn_id = req->trxn_id;
      pfcp::cause_t cause = {.cause_value = pfcp::CAUSE_VALUE_REQUEST_ACCEPTED};
      n4_report_ack->pfcp_ies.set(cause);
      n4_report_ack->r_endpoint = req->r_endpoint;

      Logger::smf_app().info(
          "Sending ITTI message %s to task TASK_SMF_N4",
          n4_report_ack->get_msg_name());
      int ret = itti_inst->send_msg(n4_report_ack);
      if (RETURNok != ret) {
        Logger::smf_app().error(
            "Could not send ITTI message %s to task TASK_SMF_N4",
            n4_report_ack->get_msg_name());
        return;
      }
    }
    // Error Indication Report
    if (report_type.erir) {
      // TODO
      Logger::smf_app().debug(
          "PFCP_SESSION_REPORT_REQUEST/Error Indication Report");
    }
    // User Plane Inactivity Report
    if (report_type.upir) {
      // TODO
      Logger::smf_app().debug(
          "PFCP_SESSION_REPORT_REQUEST/User Plane Inactivity Report");
    }
  }
}

//------------------------------------------------------------------------------
std::string smf_context::toString() const {
  std::unique_lock<std::recursive_mutex> lock(m_context);
  std::string s = {};
  s.append("\n");
  s.append("SMF CONTEXT:\n");
  s.append("SUPI:\t\t\t\t")
      .append(smf_supi_to_string(supi).c_str())
      .append("\n");
  s.append("PDU SESSION:\t\t\t\t").append("\n");
  for (auto it : pdu_sessions) {
    s.append(it.second->toString());
    s.append("\n");
  }
  return s;
}

//------------------------------------------------------------------------------
void smf_context::get_default_qos(
    const snssai_t& snssai, const std::string& dnn,
    subscribed_default_qos_t& default_qos) {
  Logger::smf_app().info(
      "Get default QoS for a PDU Session, key %d", (uint8_t) snssai.sst);
  // get the default QoS profile
  std::shared_ptr<session_management_subscription> ss = {};
  std::shared_ptr<dnn_configuration_t> sdc            = {};
  find_dnn_subscription(snssai, ss);

  if (nullptr != ss.get()) {
    ss.get()->find_dnn_configuration(dnn, sdc);
    if (nullptr != sdc.get()) {
      default_qos = sdc.get()->_5g_qos_profile;
    }
  }
}

//------------------------------------------------------------------------------
void smf_context::get_default_qos_rule(
    QOSRulesIE& qos_rule, uint8_t pdu_session_type) {
  // TODO, update according to PDU Session type
  Logger::smf_app().info(
      "Get default QoS rule for a PDU Session (PDU session type %d)",
      pdu_session_type);
  // see section 9.11.4.13 @ 3GPP TS 24.501 and section 5.7.1.4 @ 3GPP TS 23.501
  qos_rule.qosruleidentifer  = 0x01;  // be updated later on
  qos_rule.ruleoperationcode = CREATE_NEW_QOS_RULE;
  qos_rule.dqrbit            = THE_QOS_RULE_IS_DEFAULT_QOS_RULE;
  if ((pdu_session_type == PDU_SESSION_TYPE_E_IPV4) or
      (pdu_session_type == PDU_SESSION_TYPE_E_IPV4V6) or
      (pdu_session_type == PDU_SESSION_TYPE_E_IPV6) or
      (pdu_session_type == PDU_SESSION_TYPE_E_ETHERNET)) {
    qos_rule.numberofpacketfilters = 1;
    qos_rule.packetfilterlist.create_modifyandadd_modifyandreplace =
        (Create_ModifyAndAdd_ModifyAndReplace*) calloc(
            1, sizeof(Create_ModifyAndAdd_ModifyAndReplace));
    qos_rule.packetfilterlist.create_modifyandadd_modifyandreplace[0]
        .packetfilterdirection = 0b10;  // TODO: Uplink only!
    qos_rule.packetfilterlist.create_modifyandadd_modifyandreplace[0]
        .packetfilteridentifier = 1;
    qos_rule.packetfilterlist.create_modifyandadd_modifyandreplace[0]
        .packetfiltercontents.component_type = QOS_RULE_MATCHALL_TYPE;
    // qos_rule.packetfilterlist.create_modifyandadd_modifyandreplace[0].packetfiltercontents.component_value
    // = bfromcstralloc(2, "\0");
    qos_rule.qosruleprecedence = 0x01;
  }

  if (pdu_session_type == PDU_SESSION_TYPE_E_UNSTRUCTURED) {
    qos_rule.numberofpacketfilters = 0;
    qos_rule.qosruleprecedence     = 0xff;
  }

  qos_rule.segregation      = SEGREGATION_NOT_REQUESTED;
  qos_rule.qosflowidentifer = DEFAULT_QFI;

  Logger::smf_app().debug(
      "Default QoSRules: %x %x %x %x %x %x %x %x %x", qos_rule.qosruleidentifer,
      qos_rule.ruleoperationcode, qos_rule.dqrbit,
      qos_rule.numberofpacketfilters,
      qos_rule.packetfilterlist.create_modifyandadd_modifyandreplace
          ->packetfilterdirection,
      qos_rule.packetfilterlist.create_modifyandadd_modifyandreplace
          ->packetfilteridentifier,
      qos_rule.packetfilterlist.create_modifyandadd_modifyandreplace
          ->packetfiltercontents.component_type,
      qos_rule.qosruleprecedence, qos_rule.segregation,
      qos_rule.qosflowidentifer);
}

//------------------------------------------------------------------------------
void smf_context::get_default_qos_flow_description(
    QOSFlowDescriptionsContents& qos_flow_description, uint8_t pdu_session_type,
    const pfcp::qfi_t& qfi) {
  // TODO, update according to PDU Session type
  Logger::smf_app().info(
      "Get default QoS Flow Description (PDU session type %d)",
      pdu_session_type);
  qos_flow_description.qfi                = qfi.qfi;
  qos_flow_description.operationcode      = CREATE_NEW_QOS_FLOW_DESCRIPTION;
  qos_flow_description.e                  = PARAMETERS_LIST_IS_INCLUDED;
  qos_flow_description.numberofparameters = 1;
  qos_flow_description.parameterslist =
      (ParametersList*) calloc(3, sizeof(ParametersList));
  qos_flow_description.parameterslist[0].parameteridentifier =
      PARAMETER_IDENTIFIER_5QI;
  qos_flow_description.parameterslist[0].parametercontents._5qi = DEFAULT_5QI;
  /*
   qos_flow_description.parameterslist[1].parameteridentifier =
   PARAMETER_IDENTIFIER_GFBR_UPLINK;
   qos_flow_description.parameterslist[1].parametercontents
   .gfbrormfbr_uplinkordownlink.uint =
   GFBRORMFBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1MBPS;
   qos_flow_description.parameterslist[1].parametercontents
   .gfbrormfbr_uplinkordownlink.value = 0x10;
   qos_flow_description.parameterslist[2].parameteridentifier =
   PARAMETER_IDENTIFIER_GFBR_DOWNLINK;
   qos_flow_description.parameterslist[2].parametercontents
   .gfbrormfbr_uplinkordownlink.uint =
   GFBRORMFBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1MBPS;
   qos_flow_description.parameterslist[2].parametercontents
   .gfbrormfbr_uplinkordownlink.value = 0x10;
   */

  Logger::smf_app().debug(
      "Default Qos Flow Description: %x %x %x %x %x %x",
      qos_flow_description.qfi, qos_flow_description.operationcode,
      qos_flow_description.e, qos_flow_description.numberofparameters,
      qos_flow_description.parameterslist[0].parameteridentifier,
      qos_flow_description.parameterslist[0].parametercontents._5qi
      /*      qos_flow_description.parameterslist[1].parameteridentifier,
       qos_flow_description.parameterslist[1].parametercontents
       .gfbrormfbr_uplinkordownlink.uint,
       qos_flow_description.parameterslist[1].parametercontents
       .gfbrormfbr_uplinkordownlink.value,
       qos_flow_description.parameterslist[2].parameteridentifier,
       qos_flow_description.parameterslist[2].parametercontents
       .gfbrormfbr_uplinkordownlink.uint,
       qos_flow_description.parameterslist[2].parametercontents
       .gfbrormfbr_uplinkordownlink.value
       */);
}

//------------------------------------------------------------------------------
void smf_context::get_session_ambr(
    SessionAMBR& session_ambr, const snssai_t& snssai, const std::string& dnn) {
  Logger::smf_app().debug(
      "Get AMBR info from the subscription information (DNN %s)", dnn.c_str());

  std::shared_ptr<session_management_subscription> ss = {};
  std::shared_ptr<dnn_configuration_t> sdc            = {};
  find_dnn_subscription(snssai, ss);
  if (nullptr != ss.get()) {
    ss.get()->find_dnn_configuration(dnn, sdc);
    if (nullptr != sdc.get()) {
      Logger::smf_app().debug(
          "Default AMBR info from the subscription information, downlink %s, "
          "uplink %s",
          (sdc.get()->session_ambr).downlink.c_str(),
          (sdc.get()->session_ambr).uplink.c_str());

      // Downlink
      size_t leng_of_session_ambr_dl =
          (sdc.get()->session_ambr).downlink.length();
      try {
        std::string session_ambr_dl_unit =
            (sdc.get()->session_ambr)
                .downlink.substr(
                    leng_of_session_ambr_dl -
                    4);  // 4 last characters stand for mbps, kbps, ..
        if (session_ambr_dl_unit.compare("Kbps") == 0)
          session_ambr.uint_for_session_ambr_for_downlink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1KBPS;
        if (session_ambr_dl_unit.compare("Mbps") == 0)
          session_ambr.uint_for_session_ambr_for_downlink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1MBPS;
        if (session_ambr_dl_unit.compare("Gbps") == 0)
          session_ambr.uint_for_session_ambr_for_downlink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1GBPS;
        if (session_ambr_dl_unit.compare("Tbps") == 0)
          session_ambr.uint_for_session_ambr_for_downlink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1TBPS;
        if (session_ambr_dl_unit.compare("Pbps") == 0)
          session_ambr.uint_for_session_ambr_for_downlink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1PBPS;

        session_ambr.session_ambr_for_downlink =
            std::stoi((sdc.get()->session_ambr)
                          .downlink.substr(0, leng_of_session_ambr_dl - 4));
      } catch (const std::exception& e) {
        Logger::smf_app().warn("Undefined error: %s", e.what());
        // assign default value
        session_ambr.session_ambr_for_downlink = 1;
        session_ambr.uint_for_session_ambr_for_downlink =
            AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1MBPS;
      }

      // Uplink
      size_t leng_of_session_ambr_ul =
          (sdc.get()->session_ambr).uplink.length();
      try {
        std::string session_ambr_ul_unit =
            (sdc.get()->session_ambr)
                .uplink.substr(
                    leng_of_session_ambr_ul -
                    4);  // 4 last characters stand for mbps, kbps, ..
        if (session_ambr_ul_unit.compare("Kbps") == 0)
          session_ambr.uint_for_session_ambr_for_uplink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1KBPS;
        if (session_ambr_ul_unit.compare("Mbps") == 0)
          session_ambr.uint_for_session_ambr_for_uplink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1MBPS;
        if (session_ambr_ul_unit.compare("Gbps") == 0)
          session_ambr.uint_for_session_ambr_for_uplink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1GBPS;
        if (session_ambr_ul_unit.compare("Tbps") == 0)
          session_ambr.uint_for_session_ambr_for_uplink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1TBPS;
        if (session_ambr_ul_unit.compare("Pbps") == 0)
          session_ambr.uint_for_session_ambr_for_uplink =
              AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1PBPS;

        session_ambr.session_ambr_for_uplink =
            std::stoi((sdc.get()->session_ambr)
                          .uplink.substr(0, leng_of_session_ambr_ul - 4));
      } catch (const std::exception& e) {
        Logger::smf_app().warn("Undefined error: %s", e.what());
        // assign default value
        session_ambr.session_ambr_for_uplink = 1;
        session_ambr.uint_for_session_ambr_for_uplink =
            AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1MBPS;
      }
    }
  } else {
    Logger::smf_app().debug(
        "Could not get default info from the subscription information, use "
        "default value instead.");
    // use default value
    session_ambr.session_ambr_for_downlink = 1;
    session_ambr.uint_for_session_ambr_for_downlink =
        AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1MBPS;
    session_ambr.session_ambr_for_uplink = 1;
    session_ambr.uint_for_session_ambr_for_uplink =
        AMBR_VALUE_IS_INCREMENTED_IN_MULTIPLES_OF_1MBPS;
  }
}

//------------------------------------------------------------------------------
void smf_context::get_session_ambr(
    Ngap_PDUSessionAggregateMaximumBitRate_t& session_ambr,
    const snssai_t& snssai, const std::string& dnn) {
  std::shared_ptr<session_management_subscription> ss = {};
  std::shared_ptr<dnn_configuration_t> sdc            = {};
  find_dnn_subscription(snssai, ss);

  uint32_t bit_rate_dl = {110000000};  // TODO: to be updated
  uint32_t bit_rate_ul = {110000000};  // TODO: to be updated

  session_ambr.pDUSessionAggregateMaximumBitRateDL.size = 4;
  session_ambr.pDUSessionAggregateMaximumBitRateDL.buf =
      (uint8_t*) calloc(4, sizeof(uint8_t));
  session_ambr.pDUSessionAggregateMaximumBitRateUL.size = 4;
  session_ambr.pDUSessionAggregateMaximumBitRateUL.buf =
      (uint8_t*) calloc(4, sizeof(uint8_t));

  if (nullptr != ss.get()) {
    ss.get()->find_dnn_configuration(dnn, sdc);

    if (nullptr != sdc.get()) {
      Logger::smf_app().debug(
          "Default AMBR info from the DNN configuration, uplink %s, downlink "
          "%s",
          (sdc.get()->session_ambr).uplink.c_str(),
          (sdc.get()->session_ambr).downlink.c_str());
      // Downlink
      size_t leng_of_session_ambr_dl =
          (sdc.get()->session_ambr).downlink.length();
      try {
        bit_rate_dl =
            std::stoi((sdc.get()->session_ambr)
                          .downlink.substr(0, leng_of_session_ambr_dl - 4));
        std::string session_ambr_dl_unit =
            (sdc.get()->session_ambr)
                .downlink.substr(
                    leng_of_session_ambr_dl -
                    4);  // 4 last characters stand for mbps, kbps, ..
        if (session_ambr_dl_unit.compare("Kbps") == 0) bit_rate_dl *= 1000;
        if (session_ambr_dl_unit.compare("Mbps") == 0) bit_rate_dl *= 1000000;
        if (session_ambr_dl_unit.compare("Gbps") == 0)
          bit_rate_dl *= 1000000000;
        INT32_TO_BUFFER(
            bit_rate_dl, session_ambr.pDUSessionAggregateMaximumBitRateDL.buf);
      } catch (const std::exception& e) {
        Logger::smf_app().warn("Undefined error: %s", e.what());
        // assign default value
        bit_rate_dl = 1;
        INT32_TO_BUFFER(
            bit_rate_dl, session_ambr.pDUSessionAggregateMaximumBitRateDL.buf);
      }

      // Uplink
      size_t leng_of_session_ambr_ul =
          (sdc.get()->session_ambr).uplink.length();
      try {
        bit_rate_ul =
            std::stoi((sdc.get()->session_ambr)
                          .uplink.substr(0, leng_of_session_ambr_ul - 4));
        std::string session_ambr_ul_unit =
            (sdc.get()->session_ambr)
                .uplink.substr(
                    leng_of_session_ambr_ul -
                    4);  // 4 last characters stand for mbps, kbps, ..
        if (session_ambr_ul_unit.compare("Kbps") == 0) bit_rate_ul *= 1000;
        if (session_ambr_ul_unit.compare("Mbps") == 0) bit_rate_ul *= 1000000;
        if (session_ambr_ul_unit.compare("Gbps") == 0)
          bit_rate_ul *= 1000000000;
        INT32_TO_BUFFER(
            bit_rate_ul, session_ambr.pDUSessionAggregateMaximumBitRateUL.buf);
      } catch (const std::exception& e) {
        Logger::smf_app().warn("Undefined error: %s", e.what());
        // assign default value
        bit_rate_ul = 1;
        INT32_TO_BUFFER(
            bit_rate_ul, session_ambr.pDUSessionAggregateMaximumBitRateUL.buf);
      }
    }
  } else {
    INT32_TO_BUFFER(
        bit_rate_dl, session_ambr.pDUSessionAggregateMaximumBitRateDL.buf);
    INT32_TO_BUFFER(
        bit_rate_ul, session_ambr.pDUSessionAggregateMaximumBitRateUL.buf);
  }

  Logger::smf_app().debug(
      "Get AMBR info from the subscription information (DNN %s), uplink %d "
      "downlink %d",
      dnn.c_str(), bit_rate_ul, bit_rate_dl);
}

//------------------------------------------------------------------------------
void smf_context::handle_pdu_session_create_sm_context_request(
    std::shared_ptr<itti_n11_create_sm_context_request> smreq) {
  Logger::smf_app().info(
      "Handle a PDU Session Create SM Context Request message from AMF (HTTP "
      "version %d)",
      smreq->http_version);

  std::string n1_sm_message = {};
  std::string n1_sm_msg_hex = {};
  bool request_accepted     = true;

  // Step 1. Get necessary information
  std::string dnn         = smreq->req.get_dnn();
  snssai_t snssai         = smreq->req.get_snssai();
  supi64_t supi64         = smf_supi_to_u64(smreq->req.get_supi());
  uint32_t pdu_session_id = smreq->req.get_pdu_session_id();

  // Step 2. Check the validity of the UE request, if valid send PDU Session
  // Accept, otherwise send PDU Session Reject to AMF
  if (!verify_sm_context_request(smreq)) {
    Logger::smf_app().warn(
        "Received a PDU Session Create SM Context Request, the request is not "
        "valid!");
    send_pdu_session_establishment_response_reject(
        smreq,
        cause_value_5gsm_e::
            CAUSE_29_USER_AUTHENTICATION_OR_AUTHORIZATION_FAILED,
        pdu_session_application_error_e::
            PDU_SESSION_APPLICATION_ERROR_SUBSCRIPTION_DENIED,
        http_status_code_e::HTTP_STATUS_CODE_401_UNAUTHORIZED);
    // TODO:
    // SMF unsubscribes to the modifications of Session Management Subscription
    // data for (SUPI, DNN, S-NSSAI)  using Nudm_SDM_Unsubscribe()
    return;
  }

  // Store HttpResponse and session-related information to be used when
  // receiving the response from UPF
  std::shared_ptr<itti_n11_create_sm_context_response> sm_context_resp_pending =
      std::make_shared<itti_n11_create_sm_context_response>(
          TASK_SMF_APP, TASK_SMF_SBI, smreq->pid);

  // Assign necessary information for the response
  xgpp_conv::create_sm_context_response_from_ctx_request(
      smreq, sm_context_resp_pending);

  // Step 3.2. Create PDU session if not exist
  std::shared_ptr<smf_pdu_session> sp = {};
  bool find_pdu                       = find_pdu_session(pdu_session_id, sp);

  if (nullptr == sp.get()) {
    Logger::smf_app().debug("Create a new PDU session");
    sp = std::shared_ptr<smf_pdu_session>(new smf_pdu_session(pdu_session_id));
    sp.get()->pdu_session_type.pdu_session_type =
        smreq->req.get_pdu_session_type();
    sp.get()->set_dnn(dnn);
    sp.get()->set_snssai(snssai);
    add_pdu_session(pdu_session_id, sp);
  } else {
    Logger::smf_app().warn("PDU session is already existed!");
  }

  // TODO: if "Integrity Protection is required", check UE Integrity Protection
  // Maximum Data Rate
  // TODO: (Optional) Secondary authentication/authorization

  // Step 5. Create SM Policy Association with PCF or local PCC rules

  std::string smContextRef = std::to_string(smreq->scid);
  sp.get()->policy_ptr     = std::make_shared<n7::policy_association>();
  bool use_pcf_policy      = false;
  sp.get()->policy_ptr->set_context(
      smf_supi_to_string_without_nulls(smreq->req.get_supi()),
      smreq->req.get_supi_prefix(), smreq->req.get_dnn(), snssai, plmn,
      smreq->req.get_pdu_session_id(), smreq->req.get_pdu_session_type());

  // TODO what is the exact meaning of SCID? Is this unique per registration
  // or unique per PDU session?
  sp.get()->policy_ptr->id = smreq->scid;

  n7::sm_policy_status_code status =
      n7::smf_n7::get_instance().create_sm_policy_association(*sp->policy_ptr);
  if (status != n7::sm_policy_status_code::CREATED) {
    Logger::smf_n7().info(
        "PCF SM Policy Association Creation was not successful. Continue "
        "using default rules");
    use_pcf_policy = false;
    sp->policy_ptr.reset();
    // Here, the standard says that we could reject the PDU session or allow
    // the PDU session applying local policies 29.512 Chapter 4.2.2.2
    // TODO I propose to have this behavior configurable, for now we
    // continue
  } else {
    use_pcf_policy = true;
  }
  // TODO use the PCC rules also for QoS and other policy information

  // Step 6. PCO
  // section 6.2.4.2, TS 24.501
  // If the UE wants to use DHCPv4 for IPv4 address assignment, it shall
  // indicate that to the network within the Extended  protocol configuration
  // options IE in the PDU SESSION ESTABLISHMENT REQUEST  Extended protocol
  // configuration options: See subclause 10.5.6.3A in 3GPP TS 24.008.

  protocol_configuration_options_t pco_req = {};
  smreq->req.get_epco(pco_req);
  protocol_configuration_options_t pco_resp    = {};
  protocol_configuration_options_ids_t pco_ids = {
      .pi_ipcp                                     = 0,
      .ci_dns_server_ipv4_address_request          = 0,
      .ci_ip_address_allocation_via_nas_signalling = 0,
      .ci_ipv4_address_allocation_via_dhcpv4       = 0,
      .ci_ipv4_link_mtu_request                    = 0,
      .ci_dns_server_ipv6_address_request          = 0,
      .ci_ipv6_p_cscf_request                      = 0,
      .ci_ipv4_p_cscf_request                      = 0,
      .ci_selected_bearer_control_mode             = 0};

  smf_app_inst->process_pco_request(pco_req, dnn, pco_resp, pco_ids);
  sm_context_resp_pending->res.set_epco(pco_resp);

  // Step 7. Address allocation based on PDN type, IP Address pool is controlled
  // by SMF
  bool set_paa = false;
  paa_t paa    = {};
  Logger::smf_app().debug("UE Address Allocation");
  bool paa_static_ip = false;

  std::shared_ptr<session_management_subscription> ss = {};
  std::shared_ptr<dnn_configuration_t> sdc            = {};
  find_dnn_subscription(snssai, ss);
  if (nullptr != ss.get()) {
    ss.get()->find_dnn_configuration(dnn, sdc);
    if (nullptr != sdc.get()) {
      paa.pdu_session_type.pdu_session_type =
          sdc.get()
              ->pdu_session_types.default_session_type
              .pdu_session_type;  // TODO: Verified if use default session
                                  // type or requested session type
                                  // Static IP address allocation
      for (auto addr : sdc.get()->static_ip_addresses) {
        if ((sp->pdu_session_type.pdu_session_type ==
             PDU_SESSION_TYPE_E_IPV4V6) or
            (sp->pdu_session_type.pdu_session_type ==
             PDU_SESSION_TYPE_E_IPV4)) {
          if (addr.ip_address_type == IP_ADDRESS_TYPE_IPV4_ADDRESS) {
            Logger::smf_app().debug(
                "Static IP Address with IPv4 %s",
                inet_ntoa(*((struct in_addr*) &addr.u1.ipv4_address)));
            paa.ipv4_address.s_addr = addr.u1.ipv4_address.s_addr;
            paa.pdu_session_type = pdu_session_type_e::PDU_SESSION_TYPE_E_IPV4;
            set_paa              = true;
            paa_static_ip        = true;
          }
        } else if (
            sp->pdu_session_type.pdu_session_type == PDU_SESSION_TYPE_E_IPV6) {
          paa.pdu_session_type = pdu_session_type_e::PDU_SESSION_TYPE_E_IPV6;
          if (addr.ip_address_type == IP_ADDRESS_TYPE_IPV6_ADDRESS) {
            paa.ipv6_address = addr.u1.ipv6_address;
          } else if (addr.ip_address_type == IP_ADDRESS_TYPE_IPV6_PREFIX) {
            paa.ipv6_address = addr.u1.ipv6_prefix.prefix;
            // TODO: prefix length
          }
          char str_addr6[INET6_ADDRSTRLEN];
          if (inet_ntop(
                  AF_INET6, &paa.ipv6_address, str_addr6, sizeof(str_addr6))) {
            Logger::smf_app().debug(
                "Static IP Address with IPv6 %s", str_addr6);
          }
          set_paa       = true;
          paa_static_ip = true;
        }
      }
    }
  }

  switch (sp->pdu_session_type.pdu_session_type) {
    case PDU_SESSION_TYPE_E_IPV4V6: {
      Logger::smf_app().debug(
          "PDU Session Type IPv4v6, select PDU Session Type IPv4");
      // TODO: use requested PDU Session Type?
      //     paa.pdu_session_type.pdu_session_type = PDU_SESSION_TYPE_E_IPV4V6;
      if ((not paa_static_ip) || (not paa.is_ip_assigned())) {
        bool success = paa_dynamic::get_instance().get_free_paa(dnn, paa);
        if (success) {
          set_paa = true;
        } else {
          // ALL_DYNAMIC_ADDRESSES_ARE_OCCUPIED;
          send_pdu_session_establishment_response_reject(
              smreq, cause_value_5gsm_e::CAUSE_26_INSUFFICIENT_RESOURCES,
              pdu_session_application_error_e::
                  PDU_SESSION_APPLICATION_ERROR_INSUFFICIENT_RESOURCES_SLICE_DNN,
              http_status_code_e::HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR);
          return;
        }
      } else if ((paa_static_ip) && (paa.is_ip_assigned())) {
        set_paa = true;
      }
      Logger::smf_app().info(
          "PAA, Ipv4 Address: %s",
          inet_ntoa(*((struct in_addr*) &paa.ipv4_address)));

      char str_addr6[INET6_ADDRSTRLEN];
      if (inet_ntop(
              AF_INET6, &paa.ipv6_address, str_addr6, sizeof(str_addr6))) {
        Logger::smf_app().info("PAA, IPv6 Address: %s", str_addr6);
      }

    }; break;
    case PDU_SESSION_TYPE_E_IPV4: {
      Logger::smf_app().debug("PDU Session Type IPv4");
      if (!pco_ids.ci_ipv4_address_allocation_via_dhcpv4) {
        // use SM NAS signalling
        if ((not paa_static_ip) || (not paa.is_ip_assigned())) {
          bool success = paa_dynamic::get_instance().get_free_paa(dnn, paa);
          if (success) {
            set_paa = true;
          } else {
            send_pdu_session_establishment_response_reject(
                smreq, cause_value_5gsm_e::CAUSE_26_INSUFFICIENT_RESOURCES,
                pdu_session_application_error_e::
                    PDU_SESSION_APPLICATION_ERROR_INSUFFICIENT_RESOURCES_SLICE_DNN,
                http_status_code_e::HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR);
            return;
          }

        } else if ((paa_static_ip) && (paa.is_ip_assigned())) {
          set_paa = true;
        }
        Logger::smf_app().info(
            "PAA, Ipv4 Address: %s",
            inet_ntoa(*((struct in_addr*) &paa.ipv4_address)));
      } else {  // use DHCP
        Logger::smf_app().info(
            "UE requests to use DHCPv4 for IPv4 address assignment, this "
            "feature has not been supported yet!");
        // TODO maybe find a better response code
        send_pdu_session_establishment_response_reject(
            smreq, cause_value_5gsm_e::CAUSE_28_UNKNOWN_PDU_SESSION_TYPE,
            pdu_session_application_error_e::
                PDU_SESSION_APPLICATION_ERROR_PDUTYPE_NOT_SUPPORTED,
            http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN);
        return;
        // TODO
      }

    } break;

    case PDU_SESSION_TYPE_E_IPV6: {
      // TODO:
      Logger::smf_app().warn("IPv6 is not supported yet!");
      // PDU Session Establishment Reject
      send_pdu_session_establishment_response_reject(
          smreq, cause_value_5gsm_e::CAUSE_28_UNKNOWN_PDU_SESSION_TYPE,
          pdu_session_application_error_e::
              PDU_SESSION_APPLICATION_ERROR_PDUTYPE_NOT_SUPPORTED,
          http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN);
      return;
    } break;

    default: {
      Logger::smf_app().error(
          "Unknown PDN type %d", sp->pdu_session_type.pdu_session_type);
      // PDU Session Establishment Reject
      send_pdu_session_establishment_response_reject(
          smreq, cause_value_5gsm_e::CAUSE_28_UNKNOWN_PDU_SESSION_TYPE,
          pdu_session_application_error_e::
              PDU_SESSION_APPLICATION_ERROR_PDUTYPE_NOT_SUPPORTED,
          http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN);
      // sm_context_resp_pending->res.set_cause(static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_28_UNKNOWN_PDU_SESSION_TYPE));
      return;
    }
  }

  // Store AMF callback URI and subscribe to the status notification: AMF will
  // be notified when SM context changes
  std::string amf_status_uri = smreq->req.get_sm_context_status_uri();
  set_amf_status_uri(amf_status_uri);

  // Get and Store AMF Addr if available
  std::vector<std::string> split_result;
  std::string amf_addr_str =
      smf_cfg->get_nf(oai::config::AMF_CONFIG_NAME)->get_sbi().get_url();

  boost::split(split_result, amf_status_uri, boost::is_any_of("/"));
  if (split_result.size() >= 3) {
    std::string full_addr = split_result[2];
    // Check if the AMF addr is valid
    std::size_t found_port = full_addr.find(":");

    std::string addr = {};  // Addr without port
    if (found_port != std::string::npos) {
      addr = full_addr.substr(0, found_port);
    } else {
      addr = full_addr;
    }
    struct in_addr amf_ipv4_addr;
    if (inet_pton(AF_INET, util::trim(addr).c_str(), &amf_ipv4_addr) == 0) {
      Logger::smf_api_server().warn("Bad IPv4 for AMF");
    } else {
      amf_addr_str = "http://" + full_addr;
      Logger::smf_api_server().debug("AMF IP Addr %s", amf_addr_str.c_str());
    }
  }

  set_amf_addr(amf_addr_str);

  // TODO: Step 8. SMF-initiated SM Policy Modification (with PCF)

  // Step 9. Create session establishment procedure and run the procedure
  // if request is accepted
  if (set_paa) {
    sm_context_resp_pending->res.set_paa(paa);
    sp->set(paa);
  }

  // Trigger SMF APP to send response to SMF-HTTP-API-SERVER (Step
  // 5, 4.3.2.2.1 TS 23.502)
  Logger::smf_app().debug(
      "Send ITTI msg to SMF APP to trigger the response of Server");

  pdu_session_create_sm_context_response sm_context_response = {};
  // headers: Location: contains the URI of the newly created resource,
  // according to the structure:
  // {apiRoot}/nsmf-pdusession/{apiVersion}/sm-contexts/{smContextRef}
  std::string smf_context_uri = smreq->req.get_api_root() + "/" + smContextRef;
  sm_context_response.set_smf_context_uri(smf_context_uri);
  sm_context_response.set_cause(static_cast<uint8_t>(
      cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));  // TODO

  nlohmann::json json_data          = {};
  json_data["smfServiceInstanceId"] = smf_app_inst->get_smf_instance_id();
  sm_context_response.set_json_data(json_data);

  sm_context_response.set_http_code(
      http_status_code_e::HTTP_STATUS_CODE_201_CREATED);

  smf_app_inst->trigger_session_create_sm_context_response(
      sm_context_response, smreq->pid);

  // TODO: PDU Session authentication/authorization (Optional)
  // see section 4.3.2.3@3GPP TS 23.502 and section 6.3.1@3GPP TS 24.501

  Logger::smf_app().info("Create a procedure to process this message.");
  auto proc = std::make_shared<session_create_sm_context_procedure>(sp);
  std::shared_ptr<smf_procedure> sproc = proc;

  insert_procedure(sproc);
  if (proc->run(smreq, sm_context_resp_pending, shared_from_this()) ==
      smf_procedure_code::ERROR) {
    // error !
    Logger::smf_app().info(
        "PDU Session Establishment Request: Create SM Context Request "
        "procedure failed");
    remove_procedure(sproc.get());
    // Set cause to error to trigger PDU session establishment reject (step
    // 10)
    sm_context_resp_pending->res.set_cause(
        PDU_SESSION_APPLICATION_ERROR_PEER_NOT_RESPONDING);
  }

  // Step 10. if error when establishing the pdu session,
  // send ITTI message to APP to trigger N1N2MessageTransfer towards AMFs (PDU
  // Session Establishment Reject)
  if (sm_context_resp_pending->res.get_cause() !=
      static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED)) {
    // clear pco, ambr

    // free paa
    paa_t free_paa = {};
    free_paa       = sm_context_resp_pending->res.get_paa();
    if (free_paa.is_ip_assigned()) {
      switch (sp->pdu_session_type.pdu_session_type) {
        case PDU_SESSION_TYPE_E_IPV4:
        case PDU_SESSION_TYPE_E_IPV4V6:
          // paa_dynamic::get_instance().release_paa(
          //    dnn, free_paa.ipv4_address);
          // break;
        case PDU_SESSION_TYPE_E_IPV6:
          paa_dynamic::get_instance().release_paa(dnn, free_paa);
          break;
        case PDU_SESSION_TYPE_E_UNSTRUCTURED:
        case PDU_SESSION_TYPE_E_ETHERNET:
        case PDU_SESSION_TYPE_E_RESERVED:
        default:;
      }
    }
    // clear the created context??
    // TODO:

    // Create PDU Session Establishment Reject and embedded in
    // Namf_Communication_N1N2MessageTransfer Request
    Logger::smf_app().debug("Create PDU Session Establishment Reject");
    cause_value_5gsm_e cause_n1 = static_cast<cause_value_5gsm_e>(
        sm_context_resp_pending->res.get_cause());

    smf_n1::get_instance().create_n1_pdu_session_establishment_reject(
        sm_context_resp_pending->res, n1_sm_message, cause_n1);
    conv::convert_string_2_hex(n1_sm_message, n1_sm_msg_hex);
    sm_context_resp_pending->res.set_n1_sm_message(n1_sm_msg_hex);

    // Get SUPI and put into URL
    std::string supi_str = {};
    supi_t supi          = sm_context_resp_pending->res.get_supi();
    supi_str             = smf_get_supi_with_prefix(
        sm_context_resp_pending->res.get_supi_prefix(),
        smf_supi_to_string(supi));
    std::string api_version = smf_cfg->get_nf(oai::config::AMF_CONFIG_NAME)
                                  ->get_sbi()
                                  .get_api_version();
    std::string url =
        get_amf_addr() + NAMF_COMMUNICATION_BASE + api_version +
        fmt::format(
            NAMF_COMMUNICATION_N1N2_MESSAGE_TRANSFER_URL, supi_str.c_str());
    sm_context_resp_pending->res.set_amf_url(url);

    // Fill the json part
    nlohmann::json json_data                          = {};
    json_data["n1MessageContainer"]["n1MessageClass"] = N1N2_MESSAGE_CLASS;
    json_data["n1MessageContainer"]["n1MessageContent"]["contentId"] =
        N1_SM_CONTENT_ID;
    json_data["pduSessionId"] =
        sm_context_resp_pending->res.get_pdu_session_id();
    sm_context_resp_pending->res.set_json_data(json_data);

    // Send ITTI message to N11 to trigger N1N2MessageTransfer towards AMFs
    Logger::smf_app().info(
        "Sending ITTI message %s to task TASK_SMF_SBI",
        sm_context_resp_pending->get_msg_name());
    int ret = itti_inst->send_msg(sm_context_resp_pending);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_SBI",
          sm_context_resp_pending->get_msg_name());
    }
  }
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_modification_request(
    nas_message_t& nas_msg,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  Logger::smf_app().debug("PDU_SESSION_MODIFICATION_REQUEST");

  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::
          PDU_SESSION_MODIFICATION_UE_INITIATED_STEP1;

  // check if the PDU Session Release Command is already sent for this
  // message (see section 6.3.3.5 @3GPP TS 24.501)
  if (sp.get()->get_pdu_session_status() ==
      pdu_session_status_e::PDU_SESSION_INACTIVE_PENDING) {
    // Ignore the message
    Logger::smf_app().info(
        "A PDU Session Release Command has been sent for this session "
        "(session ID %d), ignore the message!",
        nas_msg.plain.sm.header.pdu_session_identity);
    return false;
  }

  // check if the session is in state Modification pending, SMF will
  // ignore this message (see section 6.3.2.5 @3GPP TS 24.501)
  if (sp.get()->get_pdu_session_status() ==
      pdu_session_status_e::PDU_SESSION_MODIFICATION_PENDING) {
    // Ignore the message
    Logger::smf_app().info(
        "This PDU session is in MODIFICATION_PENDING State (session ID "
        "%d), ignore the message!",
        nas_msg.plain.sm.header.pdu_session_identity);
    return false;
  }

  // See section 6.4.2 - UE-requested PDU Session modification procedure@
  // 3GPP TS 24.501  Verify PDU Session Identity
  if (sm_context_request.get()->req.get_pdu_session_id() !=
      nas_msg.plain.sm.header.pdu_session_identity) {
    // TODO: PDU Session ID mismatch
  }

  // PTI
  Logger::smf_app().info(
      "PTI %d", nas_msg.plain.sm.header.procedure_transaction_identity);
  procedure_transaction_id_t pti = {
      .procedure_transaction_id =
          nas_msg.plain.sm.header.procedure_transaction_identity};
  sm_context_resp.get()->res.set_pti(pti);

  // Message Type
  // TODO: _5GSMCapability
  // TODO: Cause
  // TODO: maximum_number_of_supported_packet_filters
  // TODO: AlwaysonPDUSessionRequested
  // TODO: IntergrityProtectionMaximumDataRate

  // Process QoS rules and Qos Flow descriptions
  update_qos_info(sp, sm_context_resp.get()->res, nas_msg);

  // TODO: MappedEPSBearerContexts
  // TODO: ExtendedProtocolConfigurationOptions

  // section 6.3.2. Network-requested PDU Session modification procedure @
  // 3GPP TS 24.501  requested QoS rules (including packet filters) and/or
  // requested QoS flow descriptions  session-AMBR, session TMBR
  // PTI or UE capability

  // Create a N1 SM (PDU Session Modification Command) and N2 SM (PDU
  // Session Resource Modify Request Transfer IE)
  std::string n1_sm_msg_to_be_created, n1_sm_msg_hex_to_be_created;
  std::string n2_sm_info_to_be_created, n2_sm_info_hex_to_be_created;
  // N1 SM (PDU Session Modification Command)
  if (not smf_n1::get_instance().create_n1_pdu_session_modification_command(
          sm_context_resp.get()->res, n1_sm_msg_to_be_created,
          cause_value_5gsm_e::CAUSE_0_UNKNOWN) or
      // N2 SM (PDU Session Resource Modify Request Transfer IE)
      not smf_n2::get_instance()
              .create_n2_pdu_session_resource_modify_request_transfer(
                  sm_context_resp.get()->res,
                  n2_sm_info_type_e::PDU_RES_MOD_REQ,
                  n2_sm_info_to_be_created)) {
    smf_app_inst->trigger_http_response(
        http_status_code_e::HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR,
        sm_context_request.get()->pid, N11_SESSION_UPDATE_SM_CONTEXT_RESPONSE);

    free_wrapper((void**) &nas_msg.plain.sm.pdu_session_modification_request
                     .qosflowdescriptions.qosflowdescriptionscontents);
    return false;
  }

  conv::convert_string_2_hex(
      n1_sm_msg_to_be_created, n1_sm_msg_hex_to_be_created);
  conv::convert_string_2_hex(
      n2_sm_info_to_be_created, n2_sm_info_hex_to_be_created);

  sm_context_resp.get()->res.set_n1_sm_message(n1_sm_msg_hex_to_be_created);
  sm_context_resp.get()->res.set_n2_sm_information(
      n2_sm_info_hex_to_be_created);
  sm_context_resp.get()->res.set_n2_sm_info_type("PDU_RES_MOD_REQ");

  // Fill the json part with SmContextUpdatedData
  nlohmann::json json_data           = {};
  json_data["n1SmMsg"]["contentId"]  = N1_SM_CONTENT_ID;
  json_data["n2SmInfo"]["contentId"] = N2_SM_CONTENT_ID;
  json_data["n2SmInfoType"]          = "PDU_RES_MOD_REQ";  // NGAP message

  sm_context_resp.get()->res.set_json_data(json_data);
  // Update PDU Session status
  sp.get()->set_pdu_session_status(
      pdu_session_status_e::PDU_SESSION_MODIFICATION_PENDING);

  scid_t scid = {};
  try {
    scid = std::stoi(sm_context_request.get()->scid);
  } catch (const std::exception& err) {
    Logger::smf_app().warn(
        "Couldn't retrieve "
        "the corresponding SMF context, ignore message!");
    return false;
  }

  // Store the context for the timer handling
  sp.get()->set_pending_n11_msg(
      std::dynamic_pointer_cast<itti_n11_msg>(sm_context_resp));
  // start timer T3591
  // get smf_pdu_session and set the corresponding timer
  sp.get()->timer_T3591 = itti_inst->timer_setup(
      T3591_TIMER_VALUE_SEC, 0, TASK_SMF_APP, TASK_SMF_APP_TRIGGER_T3591, scid);

  free_wrapper((void**) &nas_msg.plain.sm.pdu_session_modification_request
                   .qosflowdescriptions.qosflowdescriptionscontents);
  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_modification_complete(
    nas_message_t& nas_msg,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  /* see section 6.3.2.3@3GPP TS 24.501 V16.1.0
   Upon receipt of a PDU SESSION MODIFICATION COMPLETE message, the SMF
   shall stop timer T3591 and shall consider the PDU session as modified.
   If the selected SSC mode of the PDU session is "SSC mode 3" and the PDU
   SESSION MODIFICATION COMMAND message included 5GSM cause #39
   "reactivation requested", the SMF shall start timer T3593. If the PDU
   Session Address Lifetime value is sent to the UE in the PDU SESSION
   MODIFICATION COMMAND message then timer T3593 shall be started with the
   same value, otherwise it shall use a default value.
   */
  // Update PDU Session status -> ACTIVE
  sp.get()->set_pdu_session_status(pdu_session_status_e::PDU_SESSION_ACTIVE);
  // stop T3591
  itti_inst->timer_remove(sp.get()->timer_T3591);

  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::
          PDU_SESSION_MODIFICATION_UE_INITIATED_STEP3;
  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_modification_command_reject(
    nas_message_t& nas_msg,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::
          PDU_SESSION_MODIFICATION_UE_INITIATED_STEP3;

  // Verify PDU Session Identity
  if (sm_context_request.get()->req.get_pdu_session_id() !=
      nas_msg.plain.sm.header.pdu_session_identity) {
    // TODO: PDU Session ID mismatch
  }

  Logger::smf_app().info(
      "PTI %d", nas_msg.plain.sm.header.procedure_transaction_identity);
  procedure_transaction_id_t pti = {
      .procedure_transaction_id =
          nas_msg.plain.sm.header.procedure_transaction_identity};
  sm_context_resp.get()->res.set_pti(pti);

  // Message Type
  //_5GSMCause
  // presence
  // ExtendedProtocolConfigurationOptions

  if (nas_msg.plain.sm.pdu_session_modification_command_reject._5gsmcause ==
      static_cast<uint8_t>(
          cause_value_5gsm_e::CAUSE_43_INVALID_PDU_SESSION_IDENTITY)) {
    // Update PDU Session status -> INACTIVE
    sp.get()->set_pdu_session_status(
        pdu_session_status_e::PDU_SESSION_INACTIVE);
    // TODO: Release locally the existing PDU Session (see
    // section 6.3.2.5@3GPP TS 24.501)
  } else if (
      sp.get()->get_pdu_session_status() ==
      pdu_session_status_e::PDU_SESSION_MODIFICATION_PENDING) {
    // Update PDU Session status -> ACTIVE
    sp.get()->set_pdu_session_status(pdu_session_status_e::PDU_SESSION_ACTIVE);
  }

  // stop T3591
  itti_inst->timer_remove(sp.get()->timer_T3591);

  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_release_request(
    nas_message_t& nas_msg,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  std::string n1_sm_msg, n1_sm_msg_hex;
  std::string n2_sm_info, n2_sm_info_hex;

  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::
          PDU_SESSION_RELEASE_UE_REQUESTED_STEP1;

  // verify PDU Session ID
  if (sm_context_request.get()->req.get_pdu_session_id() !=
      nas_msg.plain.sm.header.pdu_session_identity) {
    // TODO: PDU Session ID mismatch
  }

  // Abnormal cases in network side (see section 6.4.3.6 @3GPP TS 24.501)
  if (sp.get()->get_pdu_session_status() ==
      pdu_session_status_e::PDU_SESSION_INACTIVE) {
    Logger::smf_app().warn(
        "PDU Session status: INACTIVE, send PDU Session Release Reject "
        "to UE!");
    if (smf_n1::get_instance().create_n1_pdu_session_release_reject(
            sm_context_request.get()->req, n1_sm_msg,
            cause_value_5gsm_e::CAUSE_43_INVALID_PDU_SESSION_IDENTITY)) {
      conv::convert_string_2_hex(n1_sm_msg, n1_sm_msg_hex);
      // trigger to send reply to AMF
      smf_app_inst->trigger_update_context_error_response(
          http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
          PDU_SESSION_APPLICATION_ERROR_NETWORK_FAILURE, n1_sm_msg_hex,
          sm_context_request.get()->pid);
    } else {
      smf_app_inst->trigger_http_response(
          http_status_code_e::HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR,
          sm_context_request.get()->pid,
          N11_SESSION_UPDATE_SM_CONTEXT_RESPONSE);
    }
    return false;
  }
  // Abnormal cases in network side (see section 6.3.3.5 @3GPP TS 24.501)
  if (sp.get()->get_pdu_session_status() ==
      pdu_session_status_e::PDU_SESSION_INACTIVE_PENDING) {
    // Ignore the message
    Logger::smf_app().info(
        "A PDU Session Release Command has been sent for this session "
        "(session ID %d), ignore the message!",
        nas_msg.plain.sm.header.pdu_session_identity);
    return false;
  }

  procedure_transaction_id_t pti = {
      .procedure_transaction_id =
          nas_msg.plain.sm.header.procedure_transaction_identity};

  Logger::smf_app().info("PTI %d", pti.procedure_transaction_id);
  sm_context_resp.get()->res.set_pti(pti);

  // Message Type
  // Presence
  // 5GSM Cause
  // Extended Protocol Configuration Options

  // Release the resources related to this PDU Session (in Procedure)

  // get the associated QoS flows: to be used for PFCP Session
  // Modification procedure
  std::vector<smf_qos_flow> qos_flows;
  sp.get()->get_qos_flows(qos_flows);
  for (auto i : qos_flows) {
    sm_context_request.get()->req.add_qfi(i.qfi.qfi);
  }

  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_release_complete(
    nas_message_t& nas_msg,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::
          PDU_SESSION_RELEASE_UE_REQUESTED_STEP3;

  // verify PDU Session ID
  if (sm_context_request.get()->req.get_pdu_session_id() !=
      nas_msg.plain.sm.header.pdu_session_identity) {
    // TODO: PDU Session ID mismatch
  }

  Logger::smf_app().info(
      "PTI %d", nas_msg.plain.sm.header.procedure_transaction_identity);
  procedure_transaction_id_t pti = {
      .procedure_transaction_id =
          nas_msg.plain.sm.header.procedure_transaction_identity};
  if (nas_msg.plain.sm.header.message_type != PDU_SESSION_RELEASE_COMPLETE) {
    // TODO: Message Type mismatch
  }
  // 5GSM Cause
  // Extended Protocol Configuration Options

  // TODO: SMF invokes Nsmf_PDUSession_SMContextStatusNotify to notify AMF
  // that the SM context for this PDU Session is released
  scid_t scid = {};
  try {
    scid = std::stoi(sm_context_request.get()->scid);
  } catch (const std::exception& err) {
    Logger::smf_app().warn(
        "Received a PDU Session Update SM Context Request, couldn't "
        "retrieve the corresponding SMF context, ignore message!");
    // TODO: return;
  }

  Logger::smf_app().debug("Signal the SM Context Status Change");
  std::string status = "RELEASED";
  event_sub.sm_context_status(
      scid, status, sm_context_request.get()->http_version);

  // TODO: Notify AMF that the SM context for this PDU session is released
  if (sp.get()->get_pdu_session_status() ==
      pdu_session_status_e::PDU_SESSION_ACTIVE) {
    Logger::smf_app().debug(
        "Signal the PDU Session Release Event notification");
    trigger_pdu_session_release(scid, 1);
  }

  // SM Policy Association termination
  if (sp->policy_ptr) {
    oai::model::pcf::SmPolicyDeleteData delete_data;
    // TODO set data such as release cause, usage reports etc
    n7::smf_n7::get_instance().remove_sm_policy_association(
        *sp->policy_ptr, delete_data);
  }
  // TODO: SMF un-subscribes from Session Management Subscription data
  // changes notification from UDM by invoking Numd_SDM_Unsubscribe

  // TODO: should check if sd context exist

  if (get_number_pdu_sessions() == 0) {
    Logger::smf_app().debug(
        "Unsubscribe from Session Management Subscription data changes "
        "notification from UDM");
    // TODO: unsubscribes from Session Management Subscription data
    // changes notification from UDM
  }

  // TODO: Invoke Nudm_UECM_Deregistration

  // Update PDU Session status -> INACTIVE
  sp.get()->set_pdu_session_status(pdu_session_status_e::PDU_SESSION_INACTIVE);
  // Stop timer T3592
  itti_inst->timer_remove(sp.get()->timer_T3592);
  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_resource_setup_response_transfer(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request) {
  std::string n1_sm_msg, n1_sm_msg_hex;

  // Ngap_PDUSessionResourceSetupResponseTransfer
  std::shared_ptr<Ngap_PDUSessionResourceSetupResponseTransfer_t> decoded_msg =
      std::make_shared<Ngap_PDUSessionResourceSetupResponseTransfer_t>();
  int decode_status = smf_n2::get_instance().decode_n2_sm_information(
      decoded_msg, n2_sm_information);
  if (decode_status == RETURNerror) {
    // error, send error to AMF
    Logger::smf_app().warn(
        "Decode N2 SM (Ngap_PDUSessionResourceSetupResponseTransfer) "
        "failed!");
    // PDU Session Establishment Reject
    // 24.501: response with a 5GSM STATUS message including cause "#95
    // Semantically incorrect message"
    if (smf_n1::get_instance().create_n1_pdu_session_establishment_reject(
            sm_context_request.get()->req, n1_sm_msg,
            cause_value_5gsm_e::CAUSE_95_SEMANTICALLY_INCORRECT_MESSAGE)) {
      conv::convert_string_2_hex(n1_sm_msg, n1_sm_msg_hex);
      // trigger to send reply to AMF
      smf_app_inst->trigger_update_context_error_response(
          http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
          PDU_SESSION_APPLICATION_ERROR_N2_SM_ERROR,
          sm_context_request.get()->pid);

    } else {
      smf_app_inst->trigger_http_response(
          http_status_code_e::HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR,
          sm_context_request.get()->pid,
          N11_SESSION_UPDATE_SM_CONTEXT_RESPONSE);
    }
    return false;
  }

  // store AN Tunnel Info + list of accepted QFIs
  pfcp::fteid_t dl_teid = {};
  memcpy(
      &dl_teid.teid,
      decoded_msg->dLQosFlowPerTNLInformation.uPTransportLayerInformation.choice
          .gTPTunnel->gTP_TEID.buf,
      TEID_GRE_KEY_LENGTH);
  memcpy(
      &dl_teid.ipv4_address,
      decoded_msg->dLQosFlowPerTNLInformation.uPTransportLayerInformation.choice
          .gTPTunnel->transportLayerAddress.buf,
      4);

  dl_teid.teid = ntohl(dl_teid.teid);
  dl_teid.v4   = 1;  // Only V4 for now
  sm_context_request.get()->req.set_dl_fteid(dl_teid);

  Logger::smf_app().debug(
      "DL GTP F-TEID (AN F-TEID) "
      "0x%" PRIx32 " ",
      dl_teid.teid);
  Logger::smf_app().debug(
      "uPTransportLayerInformation (AN IP Addr) %s",
      conv::toString(dl_teid.ipv4_address).c_str());

  for (int i = 0;
       i <
       decoded_msg->dLQosFlowPerTNLInformation.associatedQosFlowList.list.count;
       i++) {
    pfcp::qfi_t qfi((uint8_t) (decoded_msg->dLQosFlowPerTNLInformation
                                   .associatedQosFlowList.list.array[i])
                        ->qosFlowIdentifier);
    sm_context_request.get()->req.add_qfi(qfi);
    Logger::smf_app().debug(
        "QoSFlowPerTNLInformation, AssociatedQosFlowList, QFI %d",
        (decoded_msg->dLQosFlowPerTNLInformation.associatedQosFlowList.list
             .array[i])
            ->qosFlowIdentifier);
  }
  return true;
}
//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_resource_setup_unsuccessful_transfer(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request) {
  std::string n1_sm_msg, n1_sm_msg_hex;

  // Ngap_PDUSessionResourceSetupUnsuccessfulTransfer
  std::shared_ptr<Ngap_PDUSessionResourceSetupUnsuccessfulTransfer_t>
      decoded_msg = std::make_shared<
          Ngap_PDUSessionResourceSetupUnsuccessfulTransfer_t>();
  int decode_status = smf_n2::get_instance().decode_n2_sm_information(
      decoded_msg, n2_sm_information);

  if (decode_status == RETURNerror) {
    Logger::smf_app().warn(
        "Decode N2 SM (Ngap_PDUSessionResourceSetupUnsuccessfulTransfer) "
        "failed!");
    // trigger to send reply to AMF
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
        PDU_SESSION_APPLICATION_ERROR_N2_SM_ERROR,
        sm_context_request.get()->pid);
    return false;
  }

  // PDU Session Establishment Reject, 24.501 cause "#26 Insufficient resources"
  if (smf_n1::get_instance().create_n1_pdu_session_establishment_reject(
          sm_context_request.get()->req, n1_sm_msg,
          cause_value_5gsm_e::CAUSE_26_INSUFFICIENT_RESOURCES)) {
    conv::convert_string_2_hex(n1_sm_msg, n1_sm_msg_hex);
    // trigger to send reply to AMF
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
        PDU_SESSION_APPLICATION_ERROR_UE_NOT_RESPONDING, n1_sm_msg_hex,
        sm_context_request.get()->pid);

    // TODO: Need release established resources?
  } else {
    smf_app_inst->trigger_http_response(
        http_status_code_e::HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR,
        sm_context_request.get()->pid, N11_SESSION_UPDATE_SM_CONTEXT_RESPONSE);
  }
  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_resource_modify_response_transfer(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request) {
  std::string n1_sm_msg, n1_sm_msg_hex;

  // Ngap_PDUSessionResourceModifyResponseTransfer
  std::shared_ptr<Ngap_PDUSessionResourceModifyResponseTransfer_t> decoded_msg =
      std::make_shared<Ngap_PDUSessionResourceModifyResponseTransfer_t>();
  int decode_status = smf_n2::get_instance().decode_n2_sm_information(
      decoded_msg, n2_sm_information);

  if (decode_status == RETURNerror) {
    Logger::smf_app().warn(
        "Decode N2 SM (Ngap_PDUSessionResourceModifyResponseTransfer) "
        "failed!");
    // trigger to send reply to AMF
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
        PDU_SESSION_APPLICATION_ERROR_N2_SM_ERROR,
        sm_context_request.get()->pid);
    return false;
  }

  // see section 8.2.3 (PDU Session Resource Modify) @3GPP TS 38.413
  // if dL_NGU_UP_TNLInformation is included, it shall be considered as
  // the new DL transport layer addr for the PDU session (should be
  // verified)
  // TODO: may include uL_NGU_UP_TNLInformation (mapping between each new
  // DL transport layer address and the corresponding UL transport layer
  // address)
  pfcp::fteid_t dl_teid;
  memcpy(
      &dl_teid.teid,
      decoded_msg->dL_NGU_UP_TNLInformation->choice.gTPTunnel->gTP_TEID.buf,
      TEID_GRE_KEY_LENGTH);
  memcpy(
      &dl_teid.ipv4_address,
      decoded_msg->dL_NGU_UP_TNLInformation->choice.gTPTunnel
          ->transportLayerAddress.buf,
      4);

  dl_teid.teid = ntohl(dl_teid.teid);
  dl_teid.v4   = 1;  // Only v4 for now
  sm_context_request.get()->req.set_dl_fteid(dl_teid);

  // list of Qos Flows which have been successfully setup or modified
  if (decoded_msg->qosFlowAddOrModifyResponseList) {
    for (int i = 0; i < decoded_msg->qosFlowAddOrModifyResponseList->list.count;
         i++) {
      sm_context_request.get()->req.add_qfi(
          (decoded_msg->qosFlowAddOrModifyResponseList->list.array[i])
              ->qosFlowIdentifier);
    }
  }

  // TODO: list of QoS Flows which have failed to be modified,
  // qosFlowFailedToAddOrModifyList
  // TODO: additionalDLQosFlowPerTNLInformation
  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_resource_release_response_transfer(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<smf_pdu_session>& sp) {
  std::string n1_sm_msg, n1_sm_msg_hex;

  // TODO: SMF does nothing (Step 7, section 4.3.4.2@3GPP TS 23.502)
  // Ngap_PDUSessionResourceReleaseResponseTransfer
  std::shared_ptr<Ngap_PDUSessionResourceReleaseResponseTransfer_t>
      decoded_msg =
          std::make_shared<Ngap_PDUSessionResourceReleaseResponseTransfer_t>();
  int decode_status = smf_n2::get_instance().decode_n2_sm_information(
      decoded_msg, n2_sm_information);
  if (decode_status == RETURNerror) {
    Logger::smf_app().warn(
        "Decode N2 SM (Ngap_PDUSessionResourceReleaseResponseTransfer) "
        "failed!");
    // trigger to send reply to AMF
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
        PDU_SESSION_APPLICATION_ERROR_N2_SM_ERROR,
        sm_context_request.get()->pid);

    return false;
  }

  scid_t scid = {};
  try {
    scid = std::stoi(sm_context_request->scid);
  } catch (const std::exception& err) {
    Logger::smf_app().warn(
        "Couldn't retrieve "
        "the corresponding SMF context, ignore message!");
    return false;
  }

  // Notify AMF that the SM context for this PDU session is released
  if (sp.get()->get_pdu_session_status() ==
      pdu_session_status_e::PDU_SESSION_ACTIVE) {
    trigger_pdu_session_release(scid, 1);
  }

  smf_app_inst->trigger_http_response(
      http_status_code_e::HTTP_STATUS_CODE_200_OK,
      sm_context_request.get()->pid, N11_SESSION_UPDATE_SM_CONTEXT_RESPONSE);

  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_service_request(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  std::string n2_sm_info, n2_sm_info_hex;

  // Update upCnxState
  sp.get()->set_upCnx_state(upCnx_state_e::UPCNX_STATE_ACTIVATING);

  // get QFIs associated with PDU session ID
  std::vector<smf_qos_flow> qos_flows = {};
  sp.get()->get_qos_flows(qos_flows);
  for (auto i : qos_flows) {
    sm_context_request.get()->req.add_qfi(i.qfi.qfi);

    qos_flow_context_updated qcu = {};
    qcu.set_cause(
        static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
    qcu.set_qfi(i.qfi);
    qcu.set_ul_fteid(i.ul_fteid);
    qcu.set_qos_profile(i.qos_profile);
    sm_context_resp.get()->res.add_qos_flow_context_updated(qcu);
  }

  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::SERVICE_REQUEST_UE_TRIGGERED_STEP1;

  // Create N2 SM Information: PDU Session Resource Setup Request Transfer IE
  // N2 SM Information
  smf_n2::get_instance().create_n2_pdu_session_resource_setup_request_transfer(
      sm_context_resp.get()->res, n2_sm_info_type_e::PDU_RES_SETUP_REQ,
      n2_sm_info);

  conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);
  sm_context_resp.get()->res.set_n2_sm_information(n2_sm_info_hex);

  // fill the content of SmContextUpdatedData
  nlohmann::json json_data = {};

  json_data["n2SmInfo"]["contentId"] = N2_SM_CONTENT_ID;
  json_data["n2SmInfoType"]          = "PDU_RES_SETUP_REQ";  // NGAP message
  json_data["upCnxState"]            = "ACTIVATING";
  sm_context_resp.get()->res.set_json_data(json_data);

  // Update upCnxState to ACTIVATING
  sp.get()->set_upCnx_state(upCnx_state_e::UPCNX_STATE_ACTIVATING);

  // TODO: If new UPF is used, need to send N4 Session Modification
  // Request/Response to new/old UPF

  // Accept the activation of UP connection and continue to using the current
  // UPF
  // TODO: Accept the activation of UP connection and select a new UPF
  // Reject the activation of UP connection
  // SMF fails to find a suitable I-UPF: i) trigger re-establishment of PDU
  // Session;  or ii) keep PDU session but reject the activation of UP
  // connection;  or iii) release PDU session

  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_an_release(
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  // Get QFIs associated with PDU session ID
  std::vector<smf_qos_flow> qos_flows = {};
  sp.get()->get_qos_flows(qos_flows);
  for (auto i : qos_flows) {
    sm_context_request.get()->req.add_qfi(i.qfi.qfi);

    qos_flow_context_updated qcu = {};
    qcu.set_cause(
        static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
    qcu.set_qfi(i.qfi);
    qcu.set_ul_fteid(i.ul_fteid);
    qcu.set_qos_profile(i.qos_profile);
    sm_context_resp.get()->res.add_qos_flow_context_updated(qcu);
  }

  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::PDU_SESSION_RELEASE_AN_INITIATED;

  // Update upCnxState
  sp.get()->set_upCnx_state(upCnx_state_e::UPCNX_STATE_DEACTIVATED);
  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_pdu_session_update_sm_context_request(
    std::shared_ptr<itti_n11_update_sm_context_request> smreq) {
  Logger::smf_app().info(
      "Handle a PDU Session Update SM Context Request message from an AMF "
      "(HTTP version %d)",
      smreq->http_version);
  pdu_session_update_sm_context_request sm_context_req_msg = smreq->req;
  std::string n1_sm_msg                                    = {};
  std::string n1_sm_msg_hex                                = {};
  std::string n2_sm_info                                   = {};
  std::string n2_sm_info_hex                               = {};
  bool update_upf                                          = false;
  bool pdu_session_release_procedure                       = false;
  session_management_procedures_type_e procedure_type(
      session_management_procedures_type_e::
          PDU_SESSION_ESTABLISHMENT_UE_REQUESTED);

  // Step 1. get SMF PDU session context. At this stage, pdu_session must be
  // existed
  std::shared_ptr<smf_pdu_session> sp = {};
  if (!find_pdu_session(sm_context_req_msg.get_pdu_session_id(), sp)) {
    // error
    Logger::smf_app().warn("PDU session context does not exist!");
    // trigger to send reply to AMF
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_404_NOT_FOUND,
        PDU_SESSION_APPLICATION_ERROR_CONTEXT_NOT_FOUND, smreq->pid);
    return false;
  }

  std::string dnn = sp.get()->get_dnn();
  if ((dnn.compare(sm_context_req_msg.get_dnn()) != 0) or
      (!(sp.get()->get_snssai() == sm_context_req_msg.get_snssai()))) {
    // error
    Logger::smf_n1().warn("DNN/SNSSAI doesn't matched with this session!");
    // trigger to send reply to AMF
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_404_NOT_FOUND,
        PDU_SESSION_APPLICATION_ERROR_CONTEXT_NOT_FOUND, smreq->pid);
    return false;
  }

  // we need to store HttpResponse and session-related information to be used
  // when receiving the response from UPF
  std::shared_ptr<itti_n11_update_sm_context_response> sm_context_resp_pending =
      std::make_shared<itti_n11_update_sm_context_response>(
          TASK_SMF_SBI, TASK_SMF_APP, smreq->pid);

  sm_context_resp_pending->res.set_pdu_session_type(
      sp.get()->get_pdu_session_type().pdu_session_type);

  // Assign necessary information for the response
  xgpp_conv::update_sm_context_response_from_ctx_request(
      smreq, sm_context_resp_pending);

  // Step 2.1. Decode N1 (if content is available)
  if (sm_context_req_msg.n1_sm_msg_is_set()) {
    nas_message_t decoded_nas_msg = {};

    // Decode NAS and get the necessary information
    n1_sm_msg = sm_context_req_msg.get_n1_sm_message();
    memset(&decoded_nas_msg, 0, sizeof(nas_message_t));

    int decoder_rc = smf_n1::get_instance().decode_n1_sm_container(
        decoded_nas_msg, n1_sm_msg);
    if (decoder_rc != RETURNok) {
      // error, send reply to AMF with error code!!
      Logger::smf_app().warn("N1 SM container cannot be decoded correctly!");
      smf_app_inst->trigger_update_context_error_response(
          http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
          PDU_SESSION_APPLICATION_ERROR_N1_SM_ERROR, smreq->pid);
      return false;
    }

    uint8_t message_type = decoded_nas_msg.plain.sm.header.message_type;
    switch (message_type) {
      case PDU_SESSION_MODIFICATION_REQUEST: {
        // PDU Session Modification procedure (UE-initiated, step 1.a,
        // Section 4.3.3.2@3GPP TS 23.502). UE initiated PDU session
        // modification request (Step 1)

        procedure_type = session_management_procedures_type_e::
            PDU_SESSION_MODIFICATION_UE_INITIATED_STEP1;
        if (!handle_pdu_session_modification_request(
                decoded_nas_msg, smreq, sm_context_resp_pending, sp)) {
          // TODO
          return false;
        }

        // don't need to create a procedure to update UPF
      } break;

      case PDU_SESSION_MODIFICATION_COMPLETE: {
        // PDU Session Modification procedure (UE-initiated/Network-requested)
        // (step 3)  PDU Session  Modification Command Complete
        Logger::smf_app().debug("PDU_SESSION_MODIFICATION_COMPLETE");

        procedure_type = session_management_procedures_type_e::
            PDU_SESSION_MODIFICATION_UE_INITIATED_STEP3;
        if (!handle_pdu_session_modification_complete(
                decoded_nas_msg, smreq, sm_context_resp_pending, sp)) {
          // TODO:
          return false;
        }
        // don't need to create a procedure to update UPF
      } break;

      case PDU_SESSION_MODIFICATION_COMMAND_REJECT: {
        // PDU Session Modification procedure (Section 4.3.3.2@3GPP TS 23.502)
        Logger::smf_app().debug("PDU_SESSION_MODIFICATION_COMMAND_REJECT");

        procedure_type = session_management_procedures_type_e::
            PDU_SESSION_MODIFICATION_UE_INITIATED_STEP3;

        if (!handle_pdu_session_modification_command_reject(
                decoded_nas_msg, smreq, sm_context_resp_pending, sp)) {
          // TODO:
          return false;
        }
        // don't need to create a procedure to update UPF
      } break;

      case PDU_SESSION_RELEASE_REQUEST: {
        // PDU Session Release procedure (Section 4.3.4@3GPP TS 23.502)
        // PDU Session Release UE-Initiated (Step 1)

        Logger::smf_app().debug("PDU_SESSION_RELEASE_REQUEST");
        Logger::smf_app().info(
            "PDU Session Release (UE-Initiated), processing N1 SM Information");
        procedure_type = session_management_procedures_type_e::
            PDU_SESSION_RELEASE_UE_REQUESTED_STEP1;

        if (!handle_pdu_session_release_request(
                decoded_nas_msg, smreq, sm_context_resp_pending, sp)) {
          // TODO
          return false;
        }
        // need to update UPF accordingly
        update_upf                    = true;
        pdu_session_release_procedure = true;
      } break;

      case PDU_SESSION_RELEASE_COMPLETE: {
        // PDU Session Release procedure
        // PDU Session Release UE-Initiated (Step 3)

        Logger::smf_app().debug("PDU_SESSION_RELEASE_COMPLETE");
        Logger::smf_app().info(
            "PDU Session Release Complete (UE-Initiated), processing N1 SM "
            "Information");
        procedure_type = session_management_procedures_type_e::
            PDU_SESSION_RELEASE_UE_REQUESTED_STEP3;

        if (!handle_pdu_session_release_complete(
                decoded_nas_msg, smreq, sm_context_resp_pending, sp)) {
          // TODO:
          return false;
        }

        // display info
        Logger::smf_app().info("SMF context: \n %s", toString().c_str());

        // don't need to create a procedure to update UPF
        pdu_session_release_procedure = true;

      } break;
      default: {
        Logger::smf_app().warn("Unknown message type %d", message_type);
        // TODO:
      }
    }  // end switch
  }

  // Step 2.2. Decode N2 (if content is available)
  std::string n2_sm_info_type_str   = {};
  std::string n2_sm_information     = {};
  n2_sm_info_type_e n2_sm_info_type = {};

  if (sm_context_req_msg.n2_sm_info_is_set()) {
    // get necessary information (N2 SM information)
    n2_sm_info_type_str = smreq->req.get_n2_sm_info_type();
    n2_sm_information   = smreq->req.get_n2_sm_information();
    n2_sm_info_type = smf_app_inst->n2_sm_info_type_str2e(n2_sm_info_type_str);

    // decode N2 SM Info
    switch (n2_sm_info_type) {
      case n2_sm_info_type_e::PDU_RES_SETUP_RSP: {
        // PDU Session Resource Setup Response Transfer is included in the
        // following procedures:  1 - UE-Requested PDU Session Establishment
        // procedure (Section 4.3.2.2.1@3GPP TS 23.502)  2 - UE Triggered
        // Service Request Procedure (step 2)

        Logger::smf_app().info("PDU Session Resource Setup Response Transfer");

        if (!handle_pdu_session_resource_setup_response_transfer(
                n2_sm_information, smreq)) {
          // TODO:
          return false;
        }

        if (sp->get_upCnx_state() == upCnx_state_e::UPCNX_STATE_ACTIVATING) {
          procedure_type = session_management_procedures_type_e::
              SERVICE_REQUEST_UE_TRIGGERED_STEP2;
          Logger::smf_app().info(
              "UE-Triggered Service Request, processing N2 SM Information");
        } else {
          procedure_type = session_management_procedures_type_e::
              PDU_SESSION_ESTABLISHMENT_UE_REQUESTED;
          Logger::smf_app().info(
              "PDU Session Establishment Request, processing N2 SM "
              "Information");
        }

        // need to update UPF accordingly
        update_upf = true;
      } break;

      case n2_sm_info_type_e::PDU_RES_SETUP_FAIL: {
        // PDU Session Establishment procedure
        // PDU Session Resource Setup Unsuccessful Transfer

        Logger::smf_app().info(
            "PDU Session Resource Setup Unsuccessful Transfer");

        if (!handle_pdu_session_resource_setup_unsuccessful_transfer(
                n2_sm_information, smreq)) {
          // TODO:
          return false;
        }
        // don't need to update UPF
      } break;

      case n2_sm_info_type_e::PDU_RES_MOD_RSP: {
        // PDU Session Modification procedure (UE-initiated,
        // Section 4.3.3.2@3GPP TS 23.502 or SMF-Requested)(Step 2)

        Logger::smf_app().info(
            "PDU Session Modification Procedure, processing N2 SM Information");
        procedure_type = session_management_procedures_type_e::
            PDU_SESSION_MODIFICATION_UE_INITIATED_STEP2;

        if (!handle_pdu_session_resource_modify_response_transfer(
                n2_sm_information, smreq)) {
          // TODO:
          return false;
        }
        // need to update UPF accordingly
        update_upf = true;
      } break;

      case n2_sm_info_type_e::PDU_RES_MOD_FAIL: {
        // PDU Session Modification procedure

        Logger::smf_app().info("PDU_RES_MOD_FAIL");
        // TODO: To be completed
      } break;

      case n2_sm_info_type_e::PDU_RES_REL_RSP: {
        // PDU Session Release procedure (UE-initiated, Section 4.3.4.2@3GPP
        // TS 23.502 or SMF-Requested)(Step 2)
        Logger::smf_app().info(
            "PDU Session Release (UE-initiated), processing N2 SM Information");

        procedure_type = session_management_procedures_type_e::
            PDU_SESSION_RELEASE_UE_REQUESTED_STEP2;

        if (!handle_pdu_session_resource_release_response_transfer(
                n2_sm_information, smreq, sp)) {
          // TODO:
          return false;
        }

        sm_context_resp_pending->session_procedure_type =
            session_management_procedures_type_e::
                PDU_SESSION_RELEASE_UE_REQUESTED_STEP2;

        // Update PDU session status to PDU_SESSION_INACTIVE
        sp.get()->set_pdu_session_status(
            pdu_session_status_e::PDU_SESSION_INACTIVE);

        // don't need to create a procedure to update UPF
        pdu_session_release_procedure = true;
      } break;

      // Xn Handover
      case n2_sm_info_type_e::PATH_SWITCH_REQ: {
        // Xn based inter NG-RAN handover (Section 4.9.1.2@3GPP TS 23.502
        // V16.0.0)

        Logger::smf_app().info(
            "Xn based inter NG-RAN Handover, processing N2 SM Information");
        procedure_type =
            session_management_procedures_type_e::HO_PATH_SWITCH_REQ;

        if (!handle_ho_path_switch_req(
                n2_sm_information, smreq, sm_context_resp_pending, sp)) {
          // TODO:
          return false;
        }
        // need to update UPF accordingly
        update_upf = true;
      } break;

      // N2 Handover
      case n2_sm_info_type_e::HANDOVER_REQUIRED: {
        // Inter NG-RAN node N2 based handover (Section 4.9.1.3@3GPP TS 23.502
        // V16.0.0)

        Logger::smf_app().info(
            "Inter NG-RAN node N2 based handover (Handover Preparation, Step "
            "1), processing N2 SM "
            "Information");
        procedure_type =
            session_management_procedures_type_e::N2_HO_PREPARATION_PHASE_STEP1;

        if (!handle_ho_preparation_request(
                n2_sm_information, smreq, sm_context_resp_pending, sp)) {
          // TODO:
          return false;
        }
        // Don't need to update UPF since we use the same UPF for now
        // TODO: use another UPF
        update_upf = false;
      } break;

      case n2_sm_info_type_e::HANDOVER_REQ_ACK: {
        // Inter NG-RAN node N2 based handover (Section 4.9.1.3@3GPP TS 23.502
        // V16.0.0)

        Logger::smf_app().info(
            "Inter NG-RAN node N2 based handover (Handover Preparation, Step "
            "2), processing N2 SM "
            "Information");
        procedure_type =
            session_management_procedures_type_e::N2_HO_PREPARATION_PHASE_STEP2;

        if (!handle_ho_preparation_request_ack(
                n2_sm_information, smreq, sm_context_resp_pending, sp)) {
          // TODO:
          return false;
        }
        // Update UPF with new DL Tunnel
        update_upf = true;
      } break;

      case n2_sm_info_type_e::HANDOVER_RES_ALLOC_FAIL: {
        // Inter NG-RAN node N2 based handover (Section 4.9.1.3@3GPP TS 23.502
        // V16.0.0)

        Logger::smf_app().info(
            "Inter NG-RAN node N2 based handover (Handover Preparation, Step "
            "2), processing N2 SM "
            "Information");
        procedure_type =
            session_management_procedures_type_e::N2_HO_PREPARATION_PHASE_STEP2;

        if (!handle_ho_preparation_request_fail(
                n2_sm_information, smreq, sm_context_resp_pending, sp)) {
          // TODO:
          return false;
        }

        // TODO:
        // Update UPF with new DL Tunnel
        update_upf = false;
      } break;

      case n2_sm_info_type_e::SECONDARY_RAT_USAGE: {
        // Inter NG-RAN node N2 based handover (Section 4.9.1.3@3GPP TS 23.502
        // V16.0.0)
        if (sm_context_req_msg.ho_state_is_set()) {
          std::string ho_state = sm_context_req_msg.get_ho_state();
          if (ho_state.compare("COMPLETED") == 0) {
            // TODO:
          }
        }
      } break;

      case n2_sm_info_type_e::PDU_RES_NTY: {
        // PDU Session Resource Notify (from AN to AMF/SMF, Section 8.2.4 @3GPP
        // TS 38.413) PDU Session Resource Notify Transfer
        // TODO: to be completed
      } break;

      case n2_sm_info_type_e::PDU_RES_NTY_REL: {
        // PDU Session Resource Notify (from AN to AMF/SMF, Section 8.2.4 @3GPP
        // TS 38.413) PDU Session Resource Notify Released Transfer
        // TODO: to be completed
      } break;

      default: {
        Logger::smf_app().warn(
            "Unknown N2 SM info type %d", (int) n2_sm_info_type);
      }

    }  // end switch
  }

  // Step 3. For Service Request
  if (!sm_context_req_msg.n1_sm_msg_is_set() and
      !sm_context_req_msg.n2_sm_info_is_set() and
      sm_context_req_msg.upCnx_state_is_set()) {
    std::string up_cnx_state = {};
    sm_context_req_msg.get_upCnx_state(up_cnx_state);

    if (boost::iequals(up_cnx_state, "DEACTIVATED")) {
      Logger::smf_app().info(
          "Deactivation of User Plane connectivity of a PDU session");
      procedure_type = session_management_procedures_type_e::
          PDU_SESSION_RELEASE_AN_INITIATED;
      if (!handle_an_release(smreq, sm_context_resp_pending, sp)) {
        // TODO:
        return false;
      }
    } else if (boost::iequals(up_cnx_state, "ACTIVATING")) {
      Logger::smf_app().info("Service Request (UE-triggered, step 1)");
      procedure_type = session_management_procedures_type_e::
          SERVICE_REQUEST_UE_TRIGGERED_STEP1;
      if (!handle_service_request(
              n2_sm_info, smreq, sm_context_resp_pending, sp)) {
        // TODO:
        return false;
      }
    } else {
      // TODO:
      Logger::smf_app().warn(
          "Invalid value for UpCnxState %s", up_cnx_state.c_str());
      return false;
    }

    // do not need update UPF
    update_upf = true;
  }

  // Step 4. For AMF-initiated Session Release (with release indication)
  if (sm_context_req_msg.release_is_set()) {
    procedure_type =
        session_management_procedures_type_e::PDU_SESSION_RELEASE_AMF_INITIATED;
    // get QFIs associated with PDU session ID
    std::vector<smf_qos_flow> qos_flows = {};
    sp.get()->get_qos_flows(qos_flows);
    for (auto i : qos_flows) {
      smreq->req.add_qfi(i.qfi.qfi);
    }
    // need update UPF
    update_upf                    = true;
    pdu_session_release_procedure = true;
  }

  // Step 5. N2 Handover Execution/Cancellation
  if (sm_context_req_msg.ho_state_is_set() or
      sm_context_req_msg.n2_sm_info_is_set()) {
    std::string ho_state = sm_context_req_msg.get_ho_state();

    // Handover Execution
    if (ho_state.compare("COMPLETED") == 0 or
        n2_sm_info_type == n2_sm_info_type_e::SECONDARY_RAT_USAGE) {
      Logger::smf_app().info(
          "Inter NG-RAN node N2 based handover (Handover execution, "
          "processing N2 SM Information");
      procedure_type =
          session_management_procedures_type_e::N2_HO_EXECUTION_PHASE;

      if (!handle_ho_execution(
              n2_sm_information, smreq, sm_context_resp_pending, sp)) {
        // TODO:
        return false;
      }

      // TODO:
      // Update UPF with new DL Tunnel
      update_upf = false;
    }

    // Handover Cancellation
    if (ho_state.compare("CANCELLED") == 0) {
      if (!handle_ho_cancellation(
              n2_sm_information, smreq, sm_context_resp_pending, sp)) {
        // TODO:
        return false;
      }
      update_upf = false;
    }
  }

  // Step 5. Create a procedure for update SM context and let the procedure
  // handle the request if necessary
  if (update_upf) {
    if (!pdu_session_release_procedure) {
      auto proc = std::make_shared<session_update_sm_context_procedure>(sp);
      std::shared_ptr<smf_procedure> sproc = proc;
      proc->session_procedure_type         = procedure_type;

      insert_procedure(sproc);
      if (proc->run(smreq, sm_context_resp_pending, shared_from_this()) ==
          smf_procedure_code::ERROR) {
        // error
        Logger::smf_app().info(
            "PDU Update SM Context Request procedure failed (session procedure "
            "type %s)",
            session_management_procedures_type_e2str
                .at(static_cast<int>(procedure_type))
                .c_str());
        remove_procedure(sproc.get());

        // send error to AMF according to the procedure
        switch (procedure_type) {
          case session_management_procedures_type_e::
              PDU_SESSION_ESTABLISHMENT_UE_REQUESTED: {
            // PDU Session Establishment Reject
            if (smf_n1::get_instance()
                    .create_n1_pdu_session_establishment_reject(
                        sm_context_req_msg, n1_sm_msg,
                        cause_value_5gsm_e::CAUSE_38_NETWORK_FAILURE)) {
              conv::convert_string_2_hex(n1_sm_msg, n1_sm_msg_hex);
              // trigger to send reply to AMF
              smf_app_inst->trigger_update_context_error_response(
                  http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
                  PDU_SESSION_APPLICATION_ERROR_PEER_NOT_RESPONDING,
                  smreq->pid);
            } else {
              smf_app_inst->trigger_http_response(
                  http_status_code_e::
                      HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR,
                  smreq->pid, N11_SESSION_UPDATE_SM_CONTEXT_RESPONSE);
            }
          } break;

          case session_management_procedures_type_e::
              SERVICE_REQUEST_UE_TRIGGERED_STEP1:
          case session_management_procedures_type_e::
              PDU_SESSION_MODIFICATION_SMF_REQUESTED:
          case session_management_procedures_type_e::
              PDU_SESSION_MODIFICATION_AN_REQUESTED:
          case session_management_procedures_type_e::
              PDU_SESSION_MODIFICATION_UE_INITIATED_STEP2: {
            // trigger the reply to AMF
            smf_app_inst->trigger_update_context_error_response(
                http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
                PDU_SESSION_APPLICATION_ERROR_PEER_NOT_RESPONDING, smreq->pid);
          } break;

          default: {
            // trigger the reply to AMF
            smf_app_inst->trigger_update_context_error_response(
                http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
                PDU_SESSION_APPLICATION_ERROR_PEER_NOT_RESPONDING, smreq->pid);
          }
        }
        return false;
      }
    } else {
      // UE-triggered PDU Session Release
      pdu_session_release_sm_context_request sm_context_rel_req_msg = {};

      sm_context_rel_req_msg.set_supi(sm_context_req_msg.get_supi());
      sm_context_rel_req_msg.set_supi_prefix(
          sm_context_req_msg.get_supi_prefix());
      sm_context_rel_req_msg.set_pdu_session_id(
          sm_context_req_msg.get_pdu_session_id());
      sm_context_rel_req_msg.set_snssai(sm_context_req_msg.get_snssai());
      sm_context_rel_req_msg.set_dnn(sm_context_req_msg.get_dnn());
      sm_context_rel_req_msg.set_pti(sm_context_resp_pending->res.get_pti());

      // check if update message contain N2 SM info
      if (sm_context_req_msg.n2_sm_info_is_set()) {
        // get necessary information (N2 SM information)
        sm_context_rel_req_msg.set_n2_sm_information(
            smreq->req.get_n2_sm_information());
        sm_context_rel_req_msg.set_n2_sm_info_type(
            smreq->req.get_n2_sm_info_type());
      }

      // check if update message contain N1 SM Msg
      if (sm_context_req_msg.n1_sm_msg_is_set()) {
        sm_context_rel_req_msg.set_n1_sm_message(
            smreq->req.get_n1_sm_message());
      }

      // Create an itti_n11_release_sm_context_request message and handling it
      // accordingly
      std::shared_ptr<itti_n11_release_sm_context_request> smreq_release =
          std::make_shared<itti_n11_release_sm_context_request>(
              TASK_SMF_APP, TASK_SMF_APP, smreq->pid, smreq->scid);
      smreq_release->req          = sm_context_rel_req_msg;
      smreq_release->http_version = 1;

      std::shared_ptr<itti_n11_release_sm_context_response>
          sm_context_rel_resp_pending =
              std::make_shared<itti_n11_release_sm_context_response>(
                  TASK_SMF_APP, TASK_SMF_APP, smreq_release->pid);

      sm_context_rel_resp_pending->res.set_http_code(
          http_status_code_e::HTTP_STATUS_CODE_200_OK);
      sm_context_rel_resp_pending->res.set_supi(
          sm_context_rel_req_msg.get_supi());
      sm_context_rel_resp_pending->res.set_supi_prefix(
          sm_context_rel_req_msg.get_supi_prefix());
      sm_context_rel_resp_pending->res.set_cause(
          static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
      sm_context_rel_resp_pending->res.set_pdu_session_id(
          sm_context_rel_req_msg.get_pdu_session_id());
      sm_context_rel_resp_pending->res.set_snssai(
          sm_context_rel_req_msg.get_snssai());
      sm_context_rel_resp_pending->res.set_dnn(
          sm_context_rel_req_msg.get_dnn());
      sm_context_rel_resp_pending->res.set_pti(
          sm_context_rel_req_msg.get_pti());

      auto proc = std::make_shared<session_release_sm_context_procedure>(sp);
      std::shared_ptr<smf_procedure> sproc = proc;
      proc->session_procedure_type         = procedure_type;

      insert_procedure(sproc);

      if (proc->run(
              smreq_release, sm_context_rel_resp_pending, shared_from_this()) ==
          smf_procedure_code::ERROR) {
        Logger::smf_app().info(
            "PDU Release SM Context Request procedure failed");

        remove_procedure(sproc.get());
        // Trigger to send reply to AMF
        smf_app_inst->trigger_http_response(
            http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
            smreq_release->pid, N11_SESSION_RELEASE_SM_CONTEXT_RESPONSE);
        // TODO: set cause PDU_SESSION_APPLICATION_ERROR_PEER_NOT_RESPONDING

        return false;
      }
    }

  } else {
    Logger::smf_app().info(
        "Sending ITTI message %s to task TASK_SMF_APP to trigger response",
        sm_context_resp_pending->get_msg_name());
    int ret = itti_inst->send_msg(sm_context_resp_pending);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_SBI",
          sm_context_resp_pending->get_msg_name());
    }
    return true;
  }

  // TODO, Step 6
  /*  If the PDU Session establishment is not successful, the SMF informs the
   AMF by invoking Nsmf_PDUSession_SMContextStatusNotify (Release). The SMF also
   releases any N4 session(s) created, any PDU Session address if allocated
   (e.g. IP address) and releases the association with PCF, if any. In this
   case, step 19 is skipped. see step 18, section 4.3.2.2.1@3GPP TS 23.502)
   */
  return true;
}

//-------------------------------------------------------------------------------------
void smf_context::handle_pdu_session_release_sm_context_request(
    std::shared_ptr<itti_n11_release_sm_context_request> smreq) {
  Logger::smf_app().info("Handle a PDU Session Release SM Context Request");

  // Step 1. get SMF PDU session context. At this stage, pdu_session must be
  // existed
  std::shared_ptr<smf_pdu_session> sp = {};
  if (!find_pdu_session(smreq->req.get_pdu_session_id(), sp)) {
    // error
    Logger::smf_app().warn("PDU session context does not exist!");
    // trigger to send reply to AMF
    smf_app_inst->trigger_http_response(
        http_status_code_e::HTTP_STATUS_CODE_404_NOT_FOUND, smreq->pid,
        N11_SESSION_RELEASE_SM_CONTEXT_RESPONSE);
    return;
  }

  std::string dnn = sp.get()->get_dnn();
  if ((dnn.compare(smreq->req.get_dnn()) != 0) or
      (!(sp.get()->get_snssai() == smreq->req.get_snssai()))) {
    // error
    Logger::smf_n1().warn("DNN/SNSSAI doesn't matched with this session!");
    // trigger to send reply to AMF
    smf_app_inst->trigger_http_response(
        http_status_code_e::HTTP_STATUS_CODE_404_NOT_FOUND, smreq->pid,
        N11_SESSION_RELEASE_SM_CONTEXT_RESPONSE);
    return;
  }

  std::shared_ptr<itti_n11_release_sm_context_response>
      sm_context_resp_pending =
          std::make_shared<itti_n11_release_sm_context_response>(
              TASK_SMF_SBI, TASK_SMF_APP, smreq->pid);

  sm_context_resp_pending->res.set_http_code(
      http_status_code_e::HTTP_STATUS_CODE_200_OK);
  sm_context_resp_pending->res.set_supi(smreq->req.get_supi());
  sm_context_resp_pending->res.set_supi_prefix(smreq->req.get_supi_prefix());
  sm_context_resp_pending->res.set_cause(
      static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
  sm_context_resp_pending->res.set_pdu_session_id(
      smreq->req.get_pdu_session_id());
  sm_context_resp_pending->res.set_snssai(smreq->req.get_snssai());
  sm_context_resp_pending->res.set_dnn(smreq->req.get_dnn());

  auto proc = std::make_shared<session_release_sm_context_procedure>(sp);
  std::shared_ptr<smf_procedure> sproc = proc;

  insert_procedure(sproc);
  http_status_code_e http_response_code =
      http_status_code_e::HTTP_STATUS_CODE_204_NO_CONTENT;

  if (proc->run(smreq, sm_context_resp_pending, shared_from_this()) ==
      smf_procedure_code::ERROR) {
    Logger::smf_app().info("PDU Release SM Context Request procedure failed");

    remove_procedure(sproc.get());
    http_response_code =
        http_status_code_e::HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;
  } else {
    http_response_code = http_status_code_e::HTTP_STATUS_CODE_204_NO_CONTENT;
  }
  // Trigger to send reply to AMF
  smf_app_inst->trigger_http_response(
      http_response_code, smreq->pid, N11_SESSION_RELEASE_SM_CONTEXT_RESPONSE);
}

//------------------------------------------------------------------------------
void smf_context::handle_pdu_session_modification_network_requested(
    std::shared_ptr<itti_nx_trigger_pdu_session_modification> itti_msg) {
  Logger::smf_app().info(
      "Handle a PDU Session Modification Request (SMF-Requested)");

  std::string n1_sm_msg      = {};
  std::string n1_sm_msg_hex  = {};
  std::string n2_sm_info     = {};
  std::string n2_sm_info_hex = {};

  // Step 1. get SMF PDU session context. At this stage, pdu_session must be
  // existed
  std::shared_ptr<smf_pdu_session> sp = {};

  if (!find_pdu_session(itti_msg->msg.get_pdu_session_id(), sp)) {
    Logger::smf_app().warn("PDU session context does not exist!");
    return;
  }

  std::vector<pfcp::qfi_t> list_qfis_to_be_updated;
  itti_msg->msg.get_qfis(list_qfis_to_be_updated);

  // add QFI(s), QoS Profile(s), QoS Rules
  for (auto it : list_qfis_to_be_updated) {
    Logger::smf_app().debug("QFI to be updated: %d", it.qfi);

    std::vector<QOSRulesIE> qos_rules;
    sp.get()->get_qos_rules(it, qos_rules);
    // mark QoS rule to be updated for all rules associated with the QFIs
    for (auto r : qos_rules) {
      sp.get()->mark_qos_rule_to_be_synchronised(r.qosruleidentifer);
    }

    // list of QFIs and QoS profiles
    smf_qos_flow flow = {};
    if (sp.get()->get_qos_flow(it, flow)) {
      qos_flow_context_updated qcu = {};
      qcu.set_qfi(flow.qfi);
      qcu.set_qos_profile(flow.qos_profile);
      qcu.set_ul_fteid(flow.ul_fteid);
      qcu.set_dl_fteid(flow.dl_fteid);
      itti_msg->msg.add_qos_flow_context_updated(qcu);
    }
  }

  // Step 2. prepare information for N1N2MessageTransfer to send to AMF
  Logger::smf_app().debug(
      "Prepare N1N2MessageTransfer message and send to AMF");

  // TODO: handle encode N1, N2 failure
  // N1: PDU_SESSION_MODIFICATION_COMMAND
  smf_n1::get_instance().create_n1_pdu_session_modification_command(
      itti_msg->msg, n1_sm_msg, cause_value_5gsm_e::CAUSE_0_UNKNOWN);
  conv::convert_string_2_hex(n1_sm_msg, n1_sm_msg_hex);
  itti_msg->msg.set_n1_sm_message(n1_sm_msg_hex);

  // N2: PDU Session Resource Modify Response Transfer
  smf_n2::get_instance().create_n2_pdu_session_resource_modify_request_transfer(
      itti_msg->msg, n2_sm_info_type_e::PDU_RES_MOD_REQ, n2_sm_info);

  conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);
  itti_msg->msg.set_n2_sm_information(n2_sm_info_hex);

  // Fill N1N2MesasgeTransferRequestData
  // get supi and put into URL
  supi_t supi          = itti_msg->msg.get_supi();
  std::string supi_str = smf_get_supi_with_prefix(
      itti_msg->msg.get_supi_prefix(), smf_supi_to_string(supi));
  std::string api_version = smf_cfg->get_nf(oai::config::AMF_CONFIG_NAME)
                                ->get_sbi()
                                .get_api_version();
  std::string url =
      get_amf_addr() + NAMF_COMMUNICATION_BASE + api_version +
      fmt::format(
          NAMF_COMMUNICATION_N1N2_MESSAGE_TRANSFER_URL, supi_str.c_str());
  itti_msg->msg.set_amf_url(url);
  Logger::smf_app().debug(
      "N1N2MessageTransfer will be sent to AMF with URL: %s", url.c_str());

  // Fill the json part
  nlohmann::json json_data = {};
  // N1SM
  json_data["n1MessageContainer"]["n1MessageClass"] = N1N2_MESSAGE_CLASS;
  json_data["n1MessageContainer"]["n1MessageContent"]["contentId"] =
      N1_SM_CONTENT_ID;  // NAS part
  // N2SM
  json_data["n2InfoContainer"]["n2InformationClass"] = N1N2_MESSAGE_CLASS;
  json_data["n2InfoContainer"]["smInfo"]["PduSessionId"] =
      itti_msg->msg.get_pdu_session_id();
  // N2InfoContent (section 6.1.6.2.27@3GPP TS 29.518)
  json_data["n2InfoContainer"]["smInfo"]["n2InfoContent"]["ngapIeType"] =
      "PDU_RES_MOD_REQ";  // NGAP message type
  json_data["n2InfoContainer"]["smInfo"]["n2InfoContent"]["ngapData"]
           ["contentId"] = N2_SM_CONTENT_ID;  // NGAP part
  json_data["n2InfoContainer"]["smInfo"]["sNssai"]["sst"] =
      itti_msg->msg.get_snssai().sst;
  json_data["n2InfoContainer"]["smInfo"]["sNssai"]["sd"] =
      std::to_string(itti_msg->msg.get_snssai().sd);
  json_data["pduSessionId"] = itti_msg->msg.get_pdu_session_id();
  itti_msg->msg.set_json_data(json_data);

  // Step 3. Send ITTI message to N11 interface to trigger N1N2MessageTransfer
  // towards AMFs
  Logger::smf_app().info(
      "Sending ITTI message %s to task TASK_SMF_SBI", itti_msg->get_msg_name());

  int ret = itti_inst->send_msg(itti_msg);
  if (RETURNok != ret) {
    Logger::smf_app().error(
        "Could not send ITTI message %s to task TASK_SMF_SBI",
        itti_msg->get_msg_name());
  }
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_ho_path_switch_req(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  std::string n1_sm_msg     = {};
  std::string n1_sm_msg_hex = {};

  // If the PDU session is requested to be switched to a new N3 endpoint
  if (sm_context_request->req.get_to_be_switched()) {
    // Ngap_PathSwitchRequestTransfer
    std::shared_ptr<Ngap_PathSwitchRequestTransfer_t> decoded_msg =
        std::make_shared<Ngap_PathSwitchRequestTransfer_t>();
    int decode_status = smf_n2::get_instance().decode_n2_sm_information(
        decoded_msg, n2_sm_information);
    if (decode_status == RETURNerror) {
      // error, send error to AMF
      Logger::smf_app().warn(
          "Decode N2 SM (Ngap_PathSwitchRequestTransfer) "
          "failed!");
      // trigger to send reply to AMF
      // TODO: to be updated with correct status/cause
      smf_app_inst->trigger_update_context_error_response(
          http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
          PDU_SESSION_APPLICATION_ERROR_N2_SM_ERROR,
          sm_context_request.get()->pid);
      return false;
    }

    // store AN Tunnel Info + list of accepted QFIs
    pfcp::fteid_t dl_teid = {};

    if (decoded_msg->dL_NGU_UP_TNLInformation.present ==
        Ngap_UPTransportLayerInformation_PR_gTPTunnel) {
      memcpy(
          &dl_teid.teid,
          decoded_msg->dL_NGU_UP_TNLInformation.choice.gTPTunnel->gTP_TEID.buf,
          TEID_GRE_KEY_LENGTH);
      memcpy(
          &dl_teid.ipv4_address,
          decoded_msg->dL_NGU_UP_TNLInformation.choice.gTPTunnel
              ->transportLayerAddress.buf,
          4);

      dl_teid.teid = ntohl(dl_teid.teid);
      dl_teid.v4   = 1;  // Only V4 for now
      sm_context_request.get()->req.set_dl_fteid(dl_teid);

      Logger::smf_app().debug(
          "DL GTP F-TEID (AN F-TEID) "
          "0x%" PRIx32 " ",
          dl_teid.teid);
      Logger::smf_app().debug(
          "dL_NGU_UP_TNLInformation (AN IP Addr) %s",
          conv::toString(dl_teid.ipv4_address).c_str());

      for (int i = 0; i < decoded_msg->qosFlowAcceptedList.list.count; i++) {
        pfcp::qfi_t qfi(
            (uint8_t) (decoded_msg->qosFlowAcceptedList.list.array[i]
                           ->qosFlowIdentifier));
        sm_context_request.get()->req.add_qfi(qfi);
        Logger::smf_app().debug(
            "QoSFlowAcceptedList, QFI % d ",
            (uint8_t) (decoded_msg->qosFlowAcceptedList.list.array[i]
                           ->qosFlowIdentifier));
      }
    }

    // DL NG-U TNL Information Reused IE
    if (decoded_msg->dL_NGU_TNLInformationReused != nullptr &&
        (*decoded_msg->dL_NGU_TNLInformationReused ==
         Ngap_DL_NGU_TNLInformationReused_true)) {
      // TODO:
    }

    // TODO: User Plane Security Information IE
    return true;
  }

  // if the PDU session failed to be setup in the target RAN
  // Release this session
  if (sm_context_request->req.get_failed_to_be_switched()) {
    // TODO:
  }

  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_ho_preparation_request(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  std::string n2_sm_info     = {};
  std::string n2_sm_info_hex = {};

  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::N2_HO_PREPARATION_PHASE_STEP1;

  // Ngap_HandoverRequiredTransfer
  std::shared_ptr<Ngap_HandoverRequiredTransfer_t> decoded_msg =
      std::make_shared<Ngap_HandoverRequiredTransfer_t>();
  int decode_status = smf_n2::get_instance().decode_n2_sm_information(
      decoded_msg, n2_sm_information);
  if (decode_status == RETURNerror) {
    // error, send error to AMF
    Logger::smf_app().warn(
        "Decode N2 SM (Ngap_HandoverRequiredTransfer) "
        "failed!");
    // trigger to send reply to AMF
    // TODO: to be updated with correct status/cause
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
        PDU_SESSION_APPLICATION_ERROR_N2_SM_ERROR,
        sm_context_request.get()->pid);
    return false;
  }

  if (decoded_msg->directForwardingPathAvailability != nullptr) {
    Logger::smf_app().debug(
        "Ngap_HandoverRequiredTransfer, directForwardingPathAvailability");
    // TODO:
  } else {
    Logger::smf_app().debug(
        "Ngap_HandoverRequiredTransfer, In directForwardingPathAvailability");
    // TODO:
  }

  ng_ran_target_id_t ran_target_id = {};
  sm_context_request->req.get_target_id(ran_target_id);

  pdu_session_id_t pdu_session_id =
      sm_context_request->req.get_pdu_session_id();

  // TODO: Check Target ID whether N2 Handover for the indicated PDU Session can
  // be accepted Select UPF (should be done in Procedure)
  if (!check_handover_possibility(ran_target_id, pdu_session_id)) {
    // TODO:
    return false;
  }

  if (!sp->get_sessions_graph()) {
    // Abnormal condition when the PDU Session has no associate graph
    // TODO: Check correct return code/error
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
        PDU_SESSION_APPLICATION_ERROR_NETWORK_FAILURE,
        sm_context_request.get()->pid);
    return false;
  }

  edge access_upf = sp->get_sessions_graph()->get_access_edge();

  // Retrieve QoS Flows from the access UPF
  // TODO: Check PDU Session id cast (uint32 -> uint8)
  std::vector<std::shared_ptr<smf_qos_flow>> flows = {};
  access_upf.get_qos_flows(sp->pdu_session_id, flows);

  for (const auto& flow : flows) {
    qos_flow_context_updated qos_flow = {};
    qos_flow.set_qfi(flow->qfi);
    qos_flow.set_ul_fteid(flow->ul_fteid);
    qos_flow.set_dl_fteid(flow->dl_fteid);
    qos_flow.set_qos_profile(flow->qos_profile);
    sm_context_resp->res.add_qos_flow_context_updated(qos_flow);
  }

  smf_n2::get_instance().create_n2_pdu_session_resource_setup_request_transfer(
      sm_context_resp->res, n2_sm_info_type_e::PDU_RES_SETUP_REQ, n2_sm_info);

  conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);
  sm_context_resp.get()->res.set_n2_sm_information(n2_sm_info_hex);

  // Fill the content of SmContextUpdatedData
  nlohmann::json json_data           = {};
  json_data["n2SmInfo"]["contentId"] = N2_SM_CONTENT_ID;
  json_data["n2SmInfoType"]          = "PDU_RES_SETUP_REQ";  // NGAP message
  json_data["hoState"]               = "PREPARING";
  sm_context_resp.get()->res.set_json_data(json_data);
  sm_context_resp.get()->res.set_http_code(
      http_status_code_e::HTTP_STATUS_CODE_200_OK);

  // Set HOStatus to PREPARING
  sp.get()->set_ho_state(ho_state_e::HO_STATE_PREPARING);

  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_ho_preparation_request_ack(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  std::string n2_sm_info     = {};
  std::string n2_sm_info_hex = {};

  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::N2_HO_PREPARATION_PHASE_STEP2;

  // Ngap_HandoverRequestAcknowledgeTransfer
  std::shared_ptr<Ngap_HandoverRequestAcknowledgeTransfer_t> decoded_msg =
      std::make_shared<Ngap_HandoverRequestAcknowledgeTransfer_t>();
  int decode_status = smf_n2::get_instance().decode_n2_sm_information(
      decoded_msg, n2_sm_information);
  if (decode_status == RETURNerror) {
    // Error, send error to AMF
    Logger::smf_app().warn(
        "Decode N2 SM (Ngap_HandoverRequestAcknowledgeTransfer) "
        "failed!");
    // Trigger to send reply to AMF
    // TODO: to be updated with correct status/cause
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
        PDU_SESSION_APPLICATION_ERROR_N2_SM_ERROR,
        sm_context_request.get()->pid);
    return false;
  }

  if (decoded_msg->dL_NGU_UP_TNLInformation.present) {
    // Store AN Tunnel Info + list of accepted QFIs
    pfcp::fteid_t dl_teid = {};

    memcpy(
        &dl_teid.teid,
        decoded_msg->dL_NGU_UP_TNLInformation.choice.gTPTunnel->gTP_TEID.buf,
        TEID_GRE_KEY_LENGTH);
    memcpy(
        &dl_teid.ipv4_address,
        decoded_msg->dL_NGU_UP_TNLInformation.choice.gTPTunnel
            ->transportLayerAddress.buf,
        4);

    dl_teid.teid = ntohl(dl_teid.teid);
    dl_teid.v4   = 1;  // Only V4 for now
    dl_teid.v6   = 0;
    sm_context_request.get()->req.set_dl_fteid(dl_teid);

    Logger::smf_app().debug(
        "DL GTP F-TEID (AN F-TEID) "
        "0x%" PRIx32 " ",
        dl_teid.teid);
    Logger::smf_app().debug(
        "uPTransportLayerInformation (AN IP Addr) %s",
        conv::toString(dl_teid.ipv4_address).c_str());
  }

  for (int i = 0; i < decoded_msg->qosFlowSetupResponseList.list.count; i++) {
    pfcp::qfi_t qfi(
        (uint8_t) (decoded_msg->qosFlowSetupResponseList.list.array[i])
            ->qosFlowIdentifier);
    sm_context_request.get()->req.add_qfi(qfi);
    Logger::smf_app().debug(
        "QoSFlowPerTNLInformation, AssociatedQosFlowList, QFI %d",
        (decoded_msg->qosFlowSetupResponseList.list.array[i])
            ->qosFlowIdentifier);
  }
  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_ho_preparation_request_fail(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  std::string n2_sm_info     = {};
  std::string n2_sm_info_hex = {};

  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::N2_HO_PREPARATION_PHASE_STEP2;

  // Ngap_HandoverResourceAllocationUnsuccessfulTransfer
  std::shared_ptr<Ngap_HandoverResourceAllocationUnsuccessfulTransfer_t>
      decoded_msg = std::make_shared<
          Ngap_HandoverResourceAllocationUnsuccessfulTransfer_t>();
  int decode_status = smf_n2::get_instance().decode_n2_sm_information(
      decoded_msg, n2_sm_information);
  if (decode_status == RETURNerror) {
    // error, send error to AMF
    Logger::smf_app().warn(
        "Decode N2 SM (Ngap_HandoverResourceAllocationUnsuccessfulTransfer) "
        "failed!");
    // trigger to send reply to AMF
    // TODO: to be updated with correct status/cause
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
        PDU_SESSION_APPLICATION_ERROR_N2_SM_ERROR,
        sm_context_request.get()->pid);
    return false;
  }

  // decoded_msg->cause
  // set HoState to NONE
  sp.get()->set_ho_state(ho_state_e::HO_STATE_NONE);
  // Release resource ??
  // Create Handover Preparation Unsuccessful Transfer IE
  smf_n2::get_instance().create_n2_handover_preparation_unsuccessful_transfer(
      sm_context_resp->res, n2_sm_info_type_e::HANDOVER_RES_ALLOC_FAIL,
      n2_sm_info);

  conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);

  // Prepare SmContextUpdateError
  oai::smf_server::model::SmContextUpdateError sm_context = {};
  oai::model::common::ProblemDetails problem_details      = {};
  oai::model::common::RefToBinaryData refToBinaryData     = {};
  Logger::smf_app().warn("Create SmContextCreateError");
  problem_details.setCause(pdu_session_application_error_e2str.at(
      PDU_SESSION_APPLICATION_ERROR_HANDOVER_RESOURCE_ALLOCATION_FAILURE));
  sm_context.setError(problem_details);
  refToBinaryData.setContentId(N2_SM_CONTENT_ID);
  sm_context.setN2SmInfo(refToBinaryData);
  nlohmann::json json_data = {};
  to_json(json_data, sm_context);
  sm_context_resp.get()->res.set_json_data(json_data);
  sm_context_resp.get()->res.set_json_format("application/problem+json");
  sm_context_resp.get()->res.set_http_code(
      http_status_code_e::HTTP_STATUS_CODE_406_NOT_ACCEPTABLE);  // To be
                                                                 // verified
  sm_context_resp.get()->res.set_n2_sm_information(n2_sm_info_hex);

  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_ho_execution(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  std::string n2_sm_info     = {};
  std::string n2_sm_info_hex = {};

  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::N2_HO_EXECUTION_PHASE;

  // Ngap_SecondaryRATDataUsageReportTransfer
  std::shared_ptr<Ngap_SecondaryRATDataUsageReportTransfer_t> decoded_msg =
      std::make_shared<Ngap_SecondaryRATDataUsageReportTransfer_t>();
  int decode_status = smf_n2::get_instance().decode_n2_sm_information(
      decoded_msg, n2_sm_information);
  if (decode_status == RETURNerror) {
    // error, send error to AMF
    Logger::smf_app().warn(
        "Decode N2 SM (Ngap_SecondaryRATDataUsageReportTransfer) "
        "failed!");
    // trigger to send reply to AMF
    smf_app_inst->trigger_update_context_error_response(
        http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
        PDU_SESSION_APPLICATION_ERROR_N2_SM_ERROR,
        sm_context_request.get()->pid);
    return false;
  }
  // TODO: process Ngap_SecondaryRATDataUsageReportTransfer

  // Fill the content of SmContextUpdatedData
  nlohmann::json json_data = {};
  json_data["hoState"]     = "COMPLETED";
  sm_context_resp.get()->res.set_json_data(json_data);
  sm_context_resp.get()->res.set_http_code(
      http_status_code_e::HTTP_STATUS_CODE_200_OK);

  // set HoState to NONE
  sp.get()->set_ho_state(ho_state_e::HO_STATE_COMPLETED);
  return true;
}

//-------------------------------------------------------------------------------------
bool smf_context::handle_ho_cancellation(
    std::string& n2_sm_information,
    std::shared_ptr<itti_n11_update_sm_context_request>& sm_context_request,
    std::shared_ptr<itti_n11_update_sm_context_response>& sm_context_resp,
    std::shared_ptr<smf_pdu_session>& sp) {
  sm_context_resp.get()->session_procedure_type =
      session_management_procedures_type_e::N2_HO_CANCELLATION_PHASE;

  // set HoState to CANCELLED
  sp.get()->set_ho_state(ho_state_e::HO_STATE_CANCELLED);
  // TODO: release resources ...
  sp.get()->set_ho_state(ho_state_e::HO_STATE_NONE);
  // Delete targetServingNfId

  return true;
}

//------------------------------------------------------------------------------
void smf_context::get_snssai_key(const snssai_t& snssai, uint32_t& key) {
  key = (snssai.sd << 8 | snssai.sst);
}

//------------------------------------------------------------------------------
void smf_context::insert_dnn_subscription(
    const snssai_t& snssai,
    std::shared_ptr<session_management_subscription>& ss) {
  // Get a unique key from S-NSSAI
  uint32_t key = 0;
  get_snssai_key(snssai, key);

  std::unique_lock<std::recursive_mutex> lock(m_context);
  dnn_subscriptions[key] = ss;
  Logger::smf_app().info(
      "Inserted DNN Subscription, key: %ld (SST %d, SD %ld (0x%x))", key,
      snssai.sst, snssai.sd, snssai.sd);
}

//------------------------------------------------------------------------------
void smf_context::insert_dnn_subscription(
    const snssai_t& snssai, const std::string& dnn,
    std::shared_ptr<session_management_subscription>& ss) {
  // Get a unique key from S-NSSAI
  uint32_t key = 0;
  get_snssai_key(snssai, key);

  std::unique_lock<std::recursive_mutex> lock(m_context);
  if (dnn_subscriptions.count(key) > 0) {
    std::shared_ptr<session_management_subscription> old_ss =
        dnn_subscriptions.at(key);
    std::shared_ptr<dnn_configuration_t> dnn_configuration = {};
    ss.get()->find_dnn_configuration(dnn, dnn_configuration);
    if (dnn_configuration != nullptr) {
      old_ss.get()->insert_dnn_configuration(dnn, dnn_configuration);
    }

  } else {
    dnn_subscriptions[key] = ss;
  }
  Logger::smf_app().info(
      "Inserted DNN Subscription, key: %ld (SST %d, SD %ld (0x%x)), dnn %s",
      key, snssai.sst, snssai.sd, snssai.sd, dnn.c_str());
}

//------------------------------------------------------------------------------
bool smf_context::is_dnn_snssai_subscription_data(
    const std::string& dnn, const snssai_t& snssai) {
  // Get a unique key from S-NSSAI
  uint32_t key = 0;
  get_snssai_key(snssai, key);

  std::unique_lock<std::recursive_mutex> lock(m_context);
  if (dnn_subscriptions.count(key) > 0) {
    std::shared_ptr<session_management_subscription> ss =
        dnn_subscriptions.at(key);
    if (ss.get()->dnn_configuration(dnn))
      return true;
    else
      return false;
  }
  return false;
}

//------------------------------------------------------------------------------
bool smf_context::find_dnn_subscription(
    const snssai_t& snssai,
    std::shared_ptr<session_management_subscription>& ss) {
  // Get a unique key from S-NSSAI
  uint32_t key = 0;
  get_snssai_key(snssai, key);

  Logger::smf_app().info(
      "Find a DNN Subscription with key: %ld (SST %d, SD %ld (0x%x)), map size "
      "%ld",
      key, (uint8_t) snssai.sst, snssai.sd, snssai.sd,
      dnn_subscriptions.size());

  std::unique_lock<std::recursive_mutex> lock(m_context);
  if (dnn_subscriptions.count(key) > 0) {
    ss = dnn_subscriptions.at(key);
    return true;
  }

  Logger::smf_app().info(
      "DNN subscription (SST %d, SD %ld (0x%x)) not found",
      (uint8_t) snssai.sst, snssai.sd, snssai.sd);
  return false;
}

//------------------------------------------------------------------------------
bool smf_context::verify_sm_context_request(
    std::shared_ptr<itti_n11_create_sm_context_request> smreq) {
  // check the validity of the UE request according to the user subscription or
  // local policies
  // TODO: need to be implemented
  return true;
}

//-----------------------------------------------------------------------------
supi_t smf_context::get_supi() const {
  return supi;
}

//-----------------------------------------------------------------------------
void smf_context::set_supi(const supi_t& s) {
  supi = s;
}

//-----------------------------------------------------------------------------
void smf_context::get_supi_prefix(std::string& prefix) const {
  prefix = supi_prefix;
}

//-----------------------------------------------------------------------------
void smf_context::set_supi_prefix(std::string const& prefix) {
  supi_prefix = prefix;
}

//------------------------------------------------------------------------------
bool smf_context::find_pdu_session(
    const pdu_session_id_t& psi, std::shared_ptr<smf_pdu_session>& sp) const {
  Logger::smf_app().info("Find PDU Session with ID %d", psi);
  std::shared_lock lock(m_pdu_sessions_mutex);
  if (pdu_sessions.count(psi) > 0) {
    sp = pdu_sessions.at(psi);
    if (sp) return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
bool smf_context::find_pdu_session(
    const pfcp::pdr_id_t& pdr_id, pfcp::qfi_t& qfi,
    std::shared_ptr<smf_pdu_session>& sp) {
  std::shared_lock lock(m_pdu_sessions_mutex);
  for (auto it : pdu_sessions) {
    smf_qos_flow flow = {};
    if (it.second->find_qos_flow(pdr_id, flow)) {
      qfi.qfi = flow.qfi.qfi;
      sp      = it.second;
      if (sp) return true;
      break;
    }
  }
  return false;
}

//------------------------------------------------------------------------------
bool smf_context::add_pdu_session(
    const pdu_session_id_t& psi, const std::shared_ptr<smf_pdu_session>& sp) {
  std::unique_lock lock(
      m_pdu_sessions_mutex,
      std::defer_lock);  // Do not lock it first
  Logger::smf_app().info("Add PDU Session with Id %d", psi);

  if (((uint8_t) psi >= PDU_SESSION_IDENTITY_FIRST) and
      ((uint8_t) psi <= PDU_SESSION_IDENTITY_LAST)) {
    if (pdu_sessions.count(psi) > 0) {
      Logger::smf_app().error(
          "Failed to add PDU Session (Id %d), existed", psi);
      return false;
    } else {
      lock.lock();  // Lock it here
      pdu_sessions.insert(
          std::pair<pdu_session_id_t, std::shared_ptr<smf_pdu_session>>(
              psi, sp));
      Logger::smf_app().debug(
          "PDU Session Id (%d) has been added successfully", psi);
      return true;
    }

  } else {
    Logger::smf_app().error(
        "Failed to add PDU Session (Id %d) failed: invalid Id", psi);
    return false;
  }
}

//------------------------------------------------------------------------------
bool smf_context::remove_pdu_session(const pdu_session_id_t& psi) {
  Logger::smf_app().debug(
      "Failed to add PDU Session (Id %d) failed: invalid Id", psi);
  std::unique_lock lock(m_pdu_sessions_mutex);
  return (pdu_sessions.erase(psi) > 0);
}

//------------------------------------------------------------------------------
size_t smf_context::get_number_pdu_sessions() const {
  std::shared_lock lock(m_pdu_sessions_mutex);
  return pdu_sessions.size();
}

//------------------------------------------------------------------------------
void smf_context::get_pdu_sessions(
    std::map<pdu_session_id_t, std::shared_ptr<smf_pdu_session>>& sessions) {
  std::shared_lock lock(m_pdu_sessions_mutex);
  for (auto it : pdu_sessions) {
    sessions.insert(
        std::pair<pdu_session_id_t, std::shared_ptr<smf_pdu_session>>(
            it.first, it.second));
  }
}

//------------------------------------------------------------------------------
bool smf_context::get_pdu_session_info(
    const scid_t& scid, supi64_t& supi,
    pdu_session_id_t& pdu_session_id) const {
  Logger::smf_app().debug(
      "Get PDU Session information related to SMF Context ID " SCID_FMT " ",
      scid);
  std::shared_ptr<smf_context_ref> scf = {};

  if (smf_app_inst->is_scid_2_smf_context(scid)) {
    scf = smf_app_inst->scid_2_smf_context(scid);
  } else {
    Logger::smf_app().warn(
        "SM Context associated with this id " SCID_FMT " does not exit!", scid);
    return false;
  }

  std::shared_ptr<smf_pdu_session> sp = {};
  if (!find_pdu_session(scf.get()->pdu_session_id, sp)) {
    if (sp.get() == nullptr) {
      Logger::smf_n1().warn("PDU session context does not exist!");
      return false;
    }
  }

  supi64_t supi64 = smf_supi_to_u64(scf.get()->supi);

  // Verify if SMF context exist
  std::shared_ptr<smf_context> sc = {};

  if (smf_app_inst->is_supi_2_smf_context(supi64)) {
    sc = smf_app_inst->supi_2_smf_context(supi64);
  } else {
    Logger::smf_app().warn(
        "Could not retrieve the corresponding SMF context with "
        "Supi " SUPI_64_FMT "!",
        supi64);
    return false;
  }

  supi           = supi64;
  pdu_session_id = scf.get()->pdu_session_id;
  return true;
}

//------------------------------------------------------------------------------
bool smf_context::get_pdu_session_info(
    const scid_t& scid, supi64_t& supi,
    std::shared_ptr<smf_pdu_session>& sp) const {
  Logger::smf_app().debug(
      "Get PDU Session information related to SMF Context ID " SCID_FMT " ",
      scid);

  pdu_session_id_t pdu_session_id = {};
  if (!get_pdu_session_info(scid, supi, pdu_session_id)) {
    return false;
  }

  if (!find_pdu_session(pdu_session_id, sp)) {
    Logger::smf_app().warn(
        "Could not retrieve the PDU Session Info with PDU Session ID %d!",
        pdu_session_id);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
void smf_context::handle_sm_context_status_change(
    const scid_t& scid, const std::string& status,
    const uint8_t& http_version) const {
  Logger::smf_app().debug(
      "Send request to N11 to triger SM Context Status Notification to AMF, "
      "SMF Context ID " SCID_FMT " ",
      scid);
  std::shared_ptr<smf_context_ref> scf = {};

  if (smf_app_inst->is_scid_2_smf_context(scid)) {
    scf = smf_app_inst->scid_2_smf_context(scid);
  } else {
    Logger::smf_app().warn(
        "SM Context associated with this id " SCID_FMT " does not exit!", scid);
    return;
  }

  std::shared_ptr<smf_pdu_session> sp = {};
  if (!find_pdu_session(scf.get()->pdu_session_id, sp)) {
    if (sp.get() == nullptr) {
      Logger::smf_n1().warn("PDU session context does not exist!");
      return;
    }
  }

  // Send request to N11 to trigger the notification
  Logger::smf_app().debug(
      "Send ITTI msg to SMF N11 to trigger the status notification");
  std::shared_ptr<itti_n11_notify_sm_context_status> itti_msg =
      std::make_shared<itti_n11_notify_sm_context_status>(
          TASK_SMF_APP, TASK_SMF_SBI);
  itti_msg->scid              = scid;
  itti_msg->sm_context_status = status;
  itti_msg->amf_status_uri    = get_amf_status_uri();
  itti_msg->http_version      = http_version;

  int ret = itti_inst->send_msg(itti_msg);
  if (RETURNok != ret) {
    Logger::smf_app().error(
        "Could not send ITTI message %s to task TASK_SMF_SBI",
        itti_msg->get_msg_name());
  }
}

//------------------------------------------------------------------------------
void smf_context::trigger_pdu_session_release(
    const scid_t& scid, const uint8_t& http_version) const {
  event_sub.ee_pdu_session_release(scid, http_version);
}

//------------------------------------------------------------------------------
void smf_context::handle_ee_pdu_session_release(
    const scid_t& scid, const uint8_t& http_version) const {
  Logger::smf_app().debug(
      "Send request to N11 to triger PDU Session Release Notification, "
      "SMF Context ID " SCID_FMT " ",
      scid);

  supi64_t supi64                 = {};
  pdu_session_id_t pdu_session_id = {};
  if (!get_pdu_session_info(scid, supi64, pdu_session_id)) {
    Logger::smf_app().debug(
        "Could not retrieve info with "
        "SMF Context ID " SCID_FMT " ",
        scid);
    return;
  }

  std::vector<std::shared_ptr<smf_subscription>> subscriptions = {};
  smf_app_inst->get_ee_subscriptions(
      smf_event_t::SMF_EVENT_PDU_SES_REL, subscriptions);

  if (subscriptions.size() > 0) {
    // Send request to N11 to trigger the notification to the subscribed event
    Logger::smf_app().debug(
        "Send ITTI msg to SMF N11 to trigger the event notification");
    std::shared_ptr<itti_n11_notify_subscribed_event> itti_msg =
        std::make_shared<itti_n11_notify_subscribed_event>(
            TASK_SMF_APP, TASK_SMF_SBI);

    for (auto i : subscriptions) {
      event_notification ev_notif = {};
      ev_notif.set_supi(supi64);
      ev_notif.set_pdu_session_id(pdu_session_id);
      ev_notif.set_smf_event(smf_event_t::SMF_EVENT_PDU_SES_REL);
      ev_notif.set_notif_uri(i.get()->notif_uri);
      ev_notif.set_notif_id(i.get()->notif_id);
      // custom json e.g., for FlexCN
      // nlohmann::json cj = {};
      // cj["ue_ipv4_addr"]  = "12.1.1.2";
      // cj[""]
      itti_msg->event_notifs.push_back(ev_notif);
    }

    itti_msg->http_version = http_version;

    int ret = itti_inst->send_msg(itti_msg);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_SBI",
          itti_msg->get_msg_name());
    }
  } else {
    Logger::smf_app().debug("No subscription available for this event");
  }
}

//------------------------------------------------------------------------------
void smf_context::trigger_ddds(
    const scid_t& scid, const uint8_t& http_version) const {
  event_sub.ee_ddds(scid, http_version);
}

//------------------------------------------------------------------------------
void smf_context::handle_ddds(
    const scid_t& scid, const uint8_t& http_version) const {
  Logger::smf_app().debug(
      "Send request to N11 to triger FlexCN, "
      "SMF Context ID " SCID_FMT " ",
      scid);

  supi64_t supi64                 = {};
  pdu_session_id_t pdu_session_id = {};
  if (!get_pdu_session_info(scid, supi64, pdu_session_id)) {
    Logger::smf_app().debug(
        "Could not retrieve info with "
        "SMF Context ID " SCID_FMT " ",
        scid);
    return;
  }

  std::vector<std::shared_ptr<smf_subscription>> subscriptions = {};
  smf_app_inst->get_ee_subscriptions(
      smf_event_t::SMF_EVENT_DDDS, subscriptions);

  if (subscriptions.size() > 0) {
    // Send request to N11 to trigger the notification to the subscribed event
    Logger::smf_app().debug(
        "Send ITTI msg to SMF N11 to trigger the event notification");
    std::shared_ptr<itti_n11_notify_subscribed_event> itti_msg =
        std::make_shared<itti_n11_notify_subscribed_event>(
            TASK_SMF_APP, TASK_SMF_SBI);

    for (auto i : subscriptions) {
      event_notification ev_notif = {};
      ev_notif.set_supi(supi64);  // SUPI
      // ev_notif.set_pdu_session_id(pdu_session_id);  // PDU session ID
      ev_notif.set_smf_event(smf_event_t::SMF_EVENT_DDDS);
      ev_notif.set_notif_uri(i.get()->notif_uri);
      ev_notif.set_notif_id(i.get()->notif_id);
      // timestamp
      std::time_t time_epoch_ntp = std::time(nullptr);
      uint64_t tv_ntp            = time_epoch_ntp + SECONDS_SINCE_FIRST_EPOCH;
      ev_notif.set_timestamp(std::to_string(tv_ntp));

      // DDDS Status
      // TODO: where to get this information in SMF???
      oai::smf_server::model::DddStatus ddds =
          oai::smf_server::model::DddStatus();
      ev_notif.set_Ddds(ddds);
      itti_msg->event_notifs.push_back(ev_notif);
    }

    itti_msg->http_version = http_version;

    int ret = itti_inst->send_msg(itti_msg);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_SBI",
          itti_msg->get_msg_name());
    }
  } else {
    Logger::smf_app().debug("No subscription available for this event");
  }
}

//------------------------------------------------------------------------------
void smf_context::handle_ue_ip_change(
    const scid_t& scid, const uint8_t& http_version) const {
  Logger::smf_app().debug(
      "Send request to N11 to triger FlexCN, "
      "SMF Context ID " SCID_FMT " ",
      scid);

  supi64_t supi64 = {};
  // get smf_pdu_session
  std::shared_ptr<smf_pdu_session> sp = {};
  if (!get_pdu_session_info(scid, supi64, sp)) {
    Logger::smf_app().debug(
        "Could not retrieve info with "
        "SMF Context ID " SCID_FMT " ",
        scid);
    return;
  }

  Logger::smf_app().debug(
      "Send request to N11 to triger FlexCN (Event "
      "Exposure), SUPI " SUPI_64_FMT " , PDU Session ID %u, HTTP version %u",
      supi64, sp->get_pdu_session_id(), http_version);

  std::vector<std::shared_ptr<smf_subscription>> subscriptions = {};
  smf_app_inst->get_ee_subscriptions(
      smf_event_t::SMF_EVENT_UE_IP_CH, subscriptions);

  if (subscriptions.size() > 0) {
    // Send request to N11 to trigger the notification to the subscribed event
    Logger::smf_app().debug(
        "Send ITTI msg to SMF N11 to trigger the event notification");
    std::shared_ptr<itti_n11_notify_subscribed_event> itti_msg =
        std::make_shared<itti_n11_notify_subscribed_event>(
            TASK_SMF_APP, TASK_SMF_SBI);

    for (auto i : subscriptions) {
      event_notification ev_notif = {};
      ev_notif.set_supi(supi64);                              // SUPI
      ev_notif.set_pdu_session_id(sp->get_pdu_session_id());  // PDU session ID
      ev_notif.set_smf_event(smf_event_t::SMF_EVENT_UE_IP_CH);
      ev_notif.set_notif_uri(i.get()->notif_uri);
      ev_notif.set_notif_id(i.get()->notif_id);
      // timestamp
      std::time_t time_epoch_ntp = std::time(nullptr);
      uint64_t tv_ntp            = time_epoch_ntp + SECONDS_SINCE_FIRST_EPOCH;
      ev_notif.set_timestamp(std::to_string(tv_ntp));

      // New UE IPv4
      if (sp->ipv4) {
        ev_notif.set_ad_ipv4_addr(conv::toString(sp->ipv4_address));
      }
      // New UE IPv6 Prefix
      if (sp->ipv6) {
        char str_addr6[INET6_ADDRSTRLEN];
        if (inet_ntop(
                AF_INET6, &sp->ipv6_address, str_addr6, sizeof(str_addr6))) {
          // TODO
          // ev_notif.set_ad_ipv6_prefix(conv::toString(sp->ipv4_address));
        }
      }

      // TODO: Release UE IP address/prefix as "reIpv4Addr", "reIpv6Prefix"

      itti_msg->event_notifs.push_back(ev_notif);
    }

    itti_msg->http_version = http_version;

    int ret = itti_inst->send_msg(itti_msg);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_SBI",
          itti_msg->get_msg_name());
    }
  } else {
    Logger::smf_app().debug("No subscription available for this event");
  }
}

//------------------------------------------------------------------------------
void smf_context::trigger_ue_ip_change(
    const scid_t& scid, const uint8_t& http_version) const {
  event_sub.ee_ue_ip_change(scid, http_version);
}

//------------------------------------------------------------------------------
void smf_context::handle_qos_monitoring(
    const seid_t& seid,
    const oai::smf_server::model::EventNotification& ev_notif_model,
    const uint8_t& http_version) const {
  Logger::smf_app().debug(
      "Send request to N11 to trigger QoS Monitoring (Usage Report) Event, "
      "SMF Context-related SEID  " SEID_FMT,
      seid);

  // Get the smf context
  std::shared_ptr<smf_context> pc = {};
  if (!smf_app_inst->seid_2_smf_context(seid, pc)) {
    Logger::smf_app().warn(
        "Context associated with this SEID " SEID_FMT " does not exit!", seid);
    return;
  }

  supi_t supi     = pc.get()->supi;
  supi64_t supi64 = smf_supi_to_u64(supi);

  std::vector<std::shared_ptr<smf_subscription>> subscriptions = {};
  smf_app_inst->get_ee_subscriptions(
      smf_event_t::SMF_EVENT_QOS_MON, subscriptions);

  if (subscriptions.size() > 0) {
    // Send request to N11 to trigger the notification to the subscribed event
    Logger::smf_app().debug(
        "Send ITTI msg to SMF N11 to trigger the event notification");
    std::shared_ptr<itti_n11_notify_subscribed_event> itti_msg =
        std::make_shared<itti_n11_notify_subscribed_event>(
            TASK_SMF_APP, TASK_SMF_SBI);

    for (auto i : subscriptions) {
      event_notification ev_notif = {};
      ev_notif.set_supi(supi64);
      ev_notif.set_smf_event(smf_event_t::SMF_EVENT_QOS_MON);
      ev_notif.set_notif_uri(i.get()->notif_uri);
      ev_notif.set_notif_id(i.get()->notif_id);
      // timestamp
      std::time_t time_epoch_ntp = std::time(nullptr);
      uint64_t tv_ntp            = time_epoch_ntp + SECONDS_SINCE_FIRST_EPOCH;
      ev_notif.set_timestamp(std::to_string(tv_ntp));

      // Custom json for Usage Report
      nlohmann::json cj = {};
      to_json(cj, ev_notif_model);

      ev_notif.set_custom_info(cj);
      itti_msg->event_notifs.push_back(ev_notif);
    }

    itti_msg->http_version = http_version;

    int ret = itti_inst->send_msg(itti_msg);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_SBI",
          itti_msg->get_msg_name());
    }
  } else {
    Logger::smf_app().debug("No subscription available for this event");
  }
}

//------------------------------------------------------------------------------
void smf_context::trigger_qos_monitoring(
    const seid_t& seid,
    const oai::smf_server::model::EventNotification& ev_notif_model,
    const uint8_t& http_version) const {
  event_sub.ee_qos_monitoring(seid, ev_notif_model, http_version);
}

//------------------------------------------------------------------------------
void smf_context::handle_flexcn_event(
    const scid_t& scid, const uint8_t& http_version) const {
  Logger::smf_app().debug(
      "Send request to N11 to triger FlexCN, "
      "SMF Context ID " SCID_FMT " ",
      scid);

  supi64_t supi64 = {};
  // get smf_pdu_session
  std::shared_ptr<smf_pdu_session> sp = {};
  if (!get_pdu_session_info(scid, supi64, sp)) {
    Logger::smf_app().debug(
        "Could not retrieve info with "
        "SMF Context ID " SCID_FMT " ",
        scid);
    return;
  }

  // Get SMF Context
  std::shared_ptr<smf_context> sc = {};
  if (smf_app_inst->is_supi_2_smf_context(supi64)) {
    sc = smf_app_inst->supi_2_smf_context(supi64);
  } else {
    Logger::smf_app().warn(
        "Could not retrieve the corresponding SMF context with "
        "Supi " SUPI_64_FMT "!",
        supi64);
    return;
  }

  Logger::smf_app().debug(
      "Send request to N11 to triger FlexCN (Event "
      "Exposure), SUPI " SUPI_64_FMT " , PDU Session ID %u, HTTP version  %u",
      supi64, sp->get_pdu_session_id(), http_version);

  std::vector<std::shared_ptr<smf_subscription>> subscriptions = {};
  smf_app_inst->get_ee_subscriptions(
      smf_event_t::SMF_EVENT_FLEXCN, subscriptions);

  if (subscriptions.size() > 0) {
    // Send request to N11 to trigger the notification to the subscribed event
    Logger::smf_app().debug(
        "Send ITTI msg to SMF N11 to trigger the event notification");
    std::shared_ptr<itti_n11_notify_subscribed_event> itti_msg =
        std::make_shared<itti_n11_notify_subscribed_event>(
            TASK_SMF_APP, TASK_SMF_SBI);

    for (auto i : subscriptions) {
      event_notification ev_notif = {};
      ev_notif.set_supi(supi64);                              // SUPI
      ev_notif.set_pdu_session_id(sp->get_pdu_session_id());  // PDU session ID
      ev_notif.set_smf_event(smf_event_t::SMF_EVENT_FLEXCN);
      ev_notif.set_notif_uri(i.get()->notif_uri);
      ev_notif.set_notif_id(i.get()->notif_id);
      // timestamp
      std::time_t time_epoch_ntp = std::time(nullptr);
      uint64_t tv_ntp            = time_epoch_ntp + SECONDS_SINCE_FIRST_EPOCH;
      ev_notif.set_timestamp(std::to_string(tv_ntp));

      // custom json e.g., for FlexCN
      nlohmann::json cj = {};
      // PLMN
      plmn_t plmn     = {};
      std::string mcc = {};
      std::string mnc = {};
      sc->get_plmn(plmn);
      conv::plmnToMccMnc(plmn, mcc, mnc);
      cj["plmn"]["mcc"] = mcc;
      cj["plmn"]["mnc"] = mnc;
      // UE IPv4
      if (sp->ipv4) {
        cj["ue_ipv4_addr"] = conv::toString(sp->ipv4_address);
      }
      // UE IPv6
      if (sp->ipv6) {
        char str_addr6[INET6_ADDRSTRLEN];
        if (inet_ntop(
                AF_INET6, &sp->ipv6_address, str_addr6, sizeof(str_addr6))) {
          cj["ue_ipv6_prefix"] = str_addr6;
        }
      }
      cj["pdu_session_type"] =
          sp->pdu_session_type.to_string();  // PDU Session Type
      // NSSAI
      cj["snssai"]["sst"] = sp->get_snssai().sst;
      cj["snssai"]["sd"]  = std::to_string(sp->get_snssai().sd);
      cj["dnn"]           = sp->get_dnn();       // DNN
      cj["amf_addr"]      = sc->get_amf_addr();  // Serving AMF addr

      // QoS flows associated with this session
      std::vector<smf_qos_flow> flows = {};
      sp->get_qos_flows(flows);

      if (flows.size() > 0) {
        cj["qos_flow"] = nlohmann::json::array();
        for (auto f : flows) {
          nlohmann::json tmp = {};
          tmp["qfi"]         = (uint8_t) f.qfi.qfi;
          // UL FTEID IPv4/IPv6 (UPF)
          if (f.ul_fteid.v4)
            tmp["upf_addr"]["ipv4"] = inet_ntoa(f.ul_fteid.ipv4_address);
          if (f.ul_fteid.v6) {
            char str_addr6[INET6_ADDRSTRLEN];
            if (inet_ntop(
                    AF_INET6, &f.ul_fteid.ipv6_address, str_addr6,
                    sizeof(str_addr6))) {
              tmp["upf_addr"]["ipv6"] = str_addr6;
            }
          }
          // DL FTEID Ipv4/v6 (AN)
          if (f.dl_fteid.v4)
            tmp["an_addr"]["ipv4"] = inet_ntoa(f.dl_fteid.ipv4_address);
          if (f.dl_fteid.v6) {
            char str_addr6[INET6_ADDRSTRLEN];
            if (inet_ntop(
                    AF_INET6, &f.dl_fteid.ipv6_address, str_addr6,
                    sizeof(str_addr6))) {
              tmp["an_addr"]["ipv6"] = str_addr6;
            }
          }
          cj["qos_flow"].push_back(tmp);
        }
      }

      ev_notif.set_custom_info(cj);
      itti_msg->event_notifs.push_back(ev_notif);
    }

    itti_msg->http_version = http_version;

    int ret = itti_inst->send_msg(itti_msg);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_SBI",
          itti_msg->get_msg_name());
    }
  } else {
    Logger::smf_app().debug("No subscription available for this event");
  }
}

//------------------------------------------------------------------------------
void smf_context::trigger_flexcn_event(
    const scid_t& scid, const uint8_t& http_version) const {
  event_sub.ee_flexcn(scid, http_version);
}

//------------------------------------------------------------------------------
void smf_context::handle_pdusesest(
    const scid_t& scid, const uint8_t& http_version) const {
  Logger::smf_app().debug(
      "Send request to N11 to triger pdusesest, "
      "SMF Context ID " SCID_FMT " ",
      scid);

  supi64_t supi64 = {};
  // get smf_pdu_session
  std::shared_ptr<smf_pdu_session> sp = {};
  if (!get_pdu_session_info(scid, supi64, sp)) {
    Logger::smf_app().debug(
        "Could not retrieve info with "
        "SMF Context ID " SCID_FMT " ",
        scid);
    return;
  }

  Logger::smf_app().debug(
      "Send request to N11 to triger PDU_SES_EST (Event "
      "Exposure), SUPI " SUPI_64_FMT " , PDU Session ID %u, HTTP version %u",
      supi64, sp->get_pdu_session_id(), http_version);

  std::vector<std::shared_ptr<smf_subscription>> subscriptions = {};
  smf_app_inst->get_ee_subscriptions(
      smf_event_t::SMF_EVENT_PDUSESEST, subscriptions);

  if (subscriptions.size() > 0) {
    // Send request to N11 to trigger the notification to the subscribed event
    Logger::smf_app().debug(
        "Send ITTI msg to SMF N11 to trigger the event notification");
    std::shared_ptr<itti_n11_notify_subscribed_event> itti_msg =
        std::make_shared<itti_n11_notify_subscribed_event>(
            TASK_SMF_APP, TASK_SMF_SBI);

    for (auto i : subscriptions) {
      event_notification ev_notif = {};
      ev_notif.set_supi(supi64);                              // SUPI
      ev_notif.set_pdu_session_id(sp->get_pdu_session_id());  // PDU session ID
      ev_notif.set_smf_event(smf_event_t::SMF_EVENT_PDUSESEST);
      ev_notif.set_notif_uri(i.get()->notif_uri);
      ev_notif.set_notif_id(i.get()->notif_id);
      // timestamp
      std::time_t time_epoch_ntp = std::time(nullptr);
      uint64_t tv_ntp            = time_epoch_ntp + SECONDS_SINCE_FIRST_EPOCH;
      ev_notif.set_timestamp(std::to_string(tv_ntp));

      //  UE IPv4
      if (sp->ipv4) {
        ev_notif.set_ad_ipv4_addr(conv::toString(sp->ipv4_address));
      }
      //  UE IPv6 Prefix
      if (sp->ipv6) {
        char str_addr6[INET6_ADDRSTRLEN];
        if (inet_ntop(
                AF_INET6, &sp->ipv6_address, str_addr6, sizeof(str_addr6))) {
          // TODO
          // ev_notif.set_ad_ipv6_prefix(conv::toString(sp->ipv4_address));
        }
      }
      ev_notif.set_pdu_session_type(
          sp->pdu_session_type.to_string());  // PDU Session Type
      ev_notif.set_sst(sp->get_snssai().sst);
      ev_notif.set_sd(std::to_string(sp->get_snssai().sd));
      ev_notif.set_dnn(sp->get_dnn());

      itti_msg->event_notifs.push_back(ev_notif);
    }

    itti_msg->http_version = http_version;

    int ret = itti_inst->send_msg(itti_msg);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_SBI",
          itti_msg->get_msg_name());
    }
  } else {
    Logger::smf_app().debug("No subscription available for this event");
  }
}

//------------------------------------------------------------------------------
void smf_context::trigger_pdusesest(
    const scid_t& scid, const uint8_t& http_version) const {
  event_sub.ee_pdusesest(scid, http_version);
}

//------------------------------------------------------------------------------
void smf_context::trigger_plmn_change(
    const scid_t& scid, const uint8_t& http_version) const {
  event_sub.ee_plmn_change(scid, http_version);
}

//------------------------------------------------------------------------------
void smf_context::handle_plmn_change(
    const scid_t& scid, const uint8_t& http_version) const {
  Logger::smf_app().debug(
      "Send request to N11 to triger FlexCN, "
      "SMF Context ID " SCID_FMT " ",
      scid);

  supi64_t supi64                 = {};
  pdu_session_id_t pdu_session_id = {};
  if (!get_pdu_session_info(scid, supi64, pdu_session_id)) {
    Logger::smf_app().debug(
        "Could not retrieve info with "
        "SMF Context ID " SCID_FMT " ",
        scid);
    return;
  }

  // Get SMF Context
  std::shared_ptr<smf_context> sc = {};
  if (smf_app_inst->is_supi_2_smf_context(supi64)) {
    sc = smf_app_inst->supi_2_smf_context(supi64);
  } else {
    Logger::smf_app().warn(
        "Could not retrieve the corresponding SMF context with "
        "Supi " SUPI_64_FMT "!",
        supi64);
    return;
  }

  std::vector<std::shared_ptr<smf_subscription>> subscriptions = {};
  smf_app_inst->get_ee_subscriptions(
      smf_event_t::SMF_EVENT_FLEXCN, subscriptions);

  if (subscriptions.size() > 0) {
    // Send request to N11 to trigger the notification to the subscribed event
    Logger::smf_app().debug(
        "Send ITTI msg to SMF N11 to trigger the event notification");
    std::shared_ptr<itti_n11_notify_subscribed_event> itti_msg =
        std::make_shared<itti_n11_notify_subscribed_event>(
            TASK_SMF_APP, TASK_SMF_SBI);

    for (auto i : subscriptions) {
      event_notification ev_notif = {};
      ev_notif.set_supi(supi64);  // SUPI
      // ev_notif.set_pdu_session_id(pdu_session_id);  // PDU session ID
      ev_notif.set_smf_event(smf_event_t::SMF_EVENT_PLMN_CH);
      ev_notif.set_notif_uri(i.get()->notif_uri);
      ev_notif.set_notif_id(i.get()->notif_id);
      // timestamp
      std::time_t time_epoch_ntp = std::time(nullptr);
      uint64_t tv_ntp            = time_epoch_ntp + SECONDS_SINCE_FIRST_EPOCH;
      ev_notif.set_timestamp(std::to_string(tv_ntp));

      // PLMN
      plmn_t plmn = {};
      std::string mcc, mnc;
      sc->get_plmn(plmn);
      conv::plmnToMccMnc(plmn, mcc, mnc);
      oai::model::common::PlmnId plmnid;
      plmnid.setMcc(mcc);
      plmnid.setMnc(mnc);
      ev_notif.set_PlmnId(plmnid);
      itti_msg->event_notifs.push_back(ev_notif);
    }

    itti_msg->http_version = http_version;

    int ret = itti_inst->send_msg(itti_msg);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_SBI",
          itti_msg->get_msg_name());
    }
  } else {
    Logger::smf_app().debug("No subscription available for this event");
  }
}

//------------------------------------------------------------------------------
void smf_context::update_qos_info(
    std::shared_ptr<smf_pdu_session>& sp,
    smf::pdu_session_update_sm_context_response& res,
    const nas_message_t& nas_msg) {
  // Process QoS rules and Qos Flow descriptions
  uint16_t length_of_rule_ie = nas_msg.plain.sm.pdu_session_modification_request
                                   .qosrules.lengthofqosrulesie;

  pfcp::qfi_t generated_qfi{};
  generated_qfi.qfi = 0;

  // QOSFlowDescriptions
  uint8_t number_of_flow_descriptions =
      nas_msg.plain.sm.pdu_session_modification_request.qosflowdescriptions
          .qosflowdescriptionsnumber;
  QOSFlowDescriptionsContents qos_flow_description_content = {};

  // Only one flow description for new requested QoS Flow
  if (number_of_flow_descriptions > 0) {
    for (int i = 0; i < number_of_flow_descriptions; i++) {
      if (nas_msg.plain.sm.pdu_session_modification_request.qosflowdescriptions
              .qosflowdescriptionscontents[i]
              .qfi == NO_QOS_FLOW_IDENTIFIER_ASSIGNED) {
        // TODO: generate new QFI
        generated_qfi.qfi = (uint8_t) 60;  // hardcoded for now
        qos_flow_description_content =
            nas_msg.plain.sm.pdu_session_modification_request
                .qosflowdescriptions.qosflowdescriptionscontents[i];
        qos_flow_description_content.qfi = generated_qfi.qfi;
        break;
      }
    }
  }

  int i              = 0;
  int length_of_rule = 0;
  while (length_of_rule_ie > 0) {
    QOSRulesIE qos_rules_ie = {};
    qos_rules_ie = nas_msg.plain.sm.pdu_session_modification_request.qosrules
                       .qosrulesie[i];
    uint8_t rule_id       = {0};
    pfcp::qfi_t qfi       = {};
    smf_qos_flow qos_flow = {};
    length_of_rule        = qos_rules_ie.LengthofQoSrule;

    // If UE requested a new GBR flow
    if ((qos_rules_ie.ruleoperationcode == CREATE_NEW_QOS_RULE) and
        (qos_rules_ie.segregation == SEGREGATION_REQUESTED)) {
      // Add a new QoS Flow
      if (qos_rules_ie.qosruleidentifer == NO_QOS_RULE_IDENTIFIER_ASSIGNED) {
        // Generate a new QoS rule
        sp.get()->generate_qos_rule_id(rule_id);
        Logger::smf_app().info("Create a new QoS rule (rule Id %d)", rule_id);
        qos_rules_ie.qosruleidentifer = rule_id;
      }
      sp.get()->add_qos_rule(qos_rules_ie);

      qfi.qfi      = generated_qfi.qfi;
      qos_flow.qfi = generated_qfi.qfi;

      // set qos_profile from qos_flow_description_content
      qos_flow.qos_profile = {};

      for (int j = 0; j < qos_flow_description_content.numberofparameters;
           j++) {
        if (qos_flow_description_content.parameterslist[j]
                .parameteridentifier == PARAMETER_IDENTIFIER_5QI) {
          qos_flow.qos_profile._5qi =
              qos_flow_description_content.parameterslist[j]
                  .parametercontents._5qi;
        } else if (
            qos_flow_description_content.parameterslist[j]
                .parameteridentifier == PARAMETER_IDENTIFIER_GFBR_UPLINK) {
          qos_flow.qos_profile.parameter.qos_profile_gbr.gfbr.uplink.unit =
              qos_flow_description_content.parameterslist[j]
                  .parametercontents.gfbrormfbr_uplinkordownlink.uint;
          qos_flow.qos_profile.parameter.qos_profile_gbr.gfbr.uplink.value =
              qos_flow_description_content.parameterslist[j]
                  .parametercontents.gfbrormfbr_uplinkordownlink.value;
        } else if (
            qos_flow_description_content.parameterslist[j]
                .parameteridentifier == PARAMETER_IDENTIFIER_GFBR_DOWNLINK) {
          qos_flow.qos_profile.parameter.qos_profile_gbr.gfbr.donwlink.unit =
              qos_flow_description_content.parameterslist[j]
                  .parametercontents.gfbrormfbr_uplinkordownlink.uint;
          qos_flow.qos_profile.parameter.qos_profile_gbr.gfbr.donwlink.value =
              qos_flow_description_content.parameterslist[j]
                  .parametercontents.gfbrormfbr_uplinkordownlink.value;
        } else if (
            qos_flow_description_content.parameterslist[j]
                .parameteridentifier == PARAMETER_IDENTIFIER_MFBR_UPLINK) {
          qos_flow.qos_profile.parameter.qos_profile_gbr.mfbr.uplink.unit =
              qos_flow_description_content.parameterslist[j]
                  .parametercontents.gfbrormfbr_uplinkordownlink.uint;
          qos_flow.qos_profile.parameter.qos_profile_gbr.mfbr.uplink.value =
              qos_flow_description_content.parameterslist[j]
                  .parametercontents.gfbrormfbr_uplinkordownlink.value;
        } else if (
            qos_flow_description_content.parameterslist[j]
                .parameteridentifier == PARAMETER_IDENTIFIER_MFBR_DOWNLINK) {
          qos_flow.qos_profile.parameter.qos_profile_gbr.mfbr.donwlink.unit =
              qos_flow_description_content.parameterslist[j]
                  .parametercontents.gfbrormfbr_uplinkordownlink.uint;
          qos_flow.qos_profile.parameter.qos_profile_gbr.mfbr.donwlink.value =
              qos_flow_description_content.parameterslist[j]
                  .parametercontents.gfbrormfbr_uplinkordownlink.value;
        }
      }

      Logger::smf_app().debug(
          "Add new QoS Flow with new QRI %d", qos_rules_ie.qosruleidentifer);
      // mark this rule to be synchronised with the UE
      sp.get()->update_qos_rule(qos_rules_ie);
      // Add new QoS flow
      sp.get()->add_qos_flow(qos_flow);

      // ADD QoS Flow to be updated
      qos_flow_context_updated qcu = {};
      qcu.set_qfi(pfcp::qfi_t(qos_flow.qfi));
      // qcu.set_ul_fteid(flow.ul_fteid);
      // qcu.set_dl_fteid(flow.dl_fteid);
      qcu.set_qos_profile(qos_flow.qos_profile);
      res.add_qos_flow_context_updated(qcu);

    } else {  // update existing QRI
      Logger::smf_app().debug(
          "Update existing QRI %d", qos_rules_ie.qosruleidentifer);
      qfi.qfi = qos_rules_ie.qosflowidentifer;
      if (sp.get()->get_qos_flow(qfi, qos_flow)) {
        sp.get()->update_qos_rule(qos_rules_ie);
        // update QoS flow
        sp.get()->add_qos_flow(qos_flow);

        // ADD QoS Flow to be updated
        qos_flow_context_updated qcu = {};
        qcu.set_qfi(pfcp::qfi_t(qos_flow.qfi));
        qcu.set_ul_fteid(qos_flow.ul_fteid);
        qcu.set_dl_fteid(qos_flow.dl_fteid);
        qcu.set_qos_profile(qos_flow.qos_profile);
        res.add_qos_flow_context_updated(qcu);
      }
    }
    length_of_rule_ie -= (length_of_rule + 3);  // 2 for Length of QoS
                                                // rules IE and 1 for QoS
                                                // rule identifier
    i++;
  }
}

//------------------------------------------------------------------------------
void smf_context::set_amf_addr(const std::string& addr) {
  amf_addr = addr;
}

//------------------------------------------------------------------------------
void smf_context::get_amf_addr(std::string& addr) const {
  addr = amf_addr;
}

//------------------------------------------------------------------------------
std::string smf_context::get_amf_addr() const {
  return amf_addr;
}

//------------------------------------------------------------------------------
void smf_context::set_amf_status_uri(const std::string& status_uri) {
  amf_status_uri = status_uri;
}

//------------------------------------------------------------------------------
void smf_context::get_amf_status_uri(std::string& status_uri) const {
  status_uri = amf_status_uri;
}

//------------------------------------------------------------------------------
std::string smf_context::get_amf_status_uri() const {
  return amf_status_uri;
}

void smf_context::set_target_amf(const std::string& amf) {
  target_amf = amf;
}
void smf_context::get_target_amf(std::string& amf) const {
  amf = target_amf;
}
std::string smf_context::get_target_amf() const {
  return target_amf;
}

//------------------------------------------------------------------------------
void smf_context::set_plmn(const plmn_t& plmn) {
  this->plmn = plmn;
}

//------------------------------------------------------------------------------
void smf_context::get_plmn(plmn_t& plmn) const {
  plmn = this->plmn;
}

//------------------------------------------------------------------------------
bool smf_context::check_handover_possibility(
    const ng_ran_target_id_t& ran_target_id,
    const pdu_session_id_t& pdu_session_id) const {
  // TODO:
  return true;
}

//------------------------------------------------------------------------------
void smf_context::send_pdu_session_establishment_response_reject(
    const std::shared_ptr<itti_n11_create_sm_context_request> smreq,
    cause_value_5gsm_e cause, pdu_session_application_error_e application_error,
    http_status_code_e http_status) {
  std::string n1_sm_message = {};
  std::string n1_sm_msg_hex = {};

  if (smf_n1::get_instance().create_n1_pdu_session_establishment_reject(
          smreq->req, n1_sm_message, cause)) {
    conv::convert_string_2_hex(n1_sm_message, n1_sm_msg_hex);
    // trigger to send reply to AMF
    smf_app_inst->trigger_create_context_error_response(
        http_status, application_error, n1_sm_msg_hex, smreq->pid);
  } else {
    smf_app_inst->trigger_http_response(
        http_status_code_e::HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR,
        smreq->pid, N11_SESSION_CREATE_SM_CONTEXT_RESPONSE);
  }

  // TODO this may be a good point to unsubscribe from UDM/PCF
}

//------------------------------------------------------------------------------
void smf_context::send_pdu_session_create_response(
    const std::shared_ptr<itti_n11_create_sm_context_response>& resp) {
  // fill content for N1N2MessageTransfer (including N1, N2 SM)
  // Create N1 SM container & N2 SM Information
  std::string n1_sm_msg       = {};
  std::string n1_sm_msg_hex   = {};
  std::string n2_sm_info      = {};
  std::string n2_sm_info_hex  = {};
  cause_value_5gsm_e cause_n1 = {cause_value_5gsm_e::CAUSE_0_UNKNOWN};

  if (resp->res.get_cause() !=
      static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED)) {
    // PDU Session Establishment Reject
    Logger::smf_app().debug(
        "Prepare a PDU Session Establishment Reject message and send to UE");
    cause_n1 = cause_value_5gsm_e::CAUSE_38_NETWORK_FAILURE;
    // TODO: Support IPv4 only for now
    if (resp->res.get_pdu_session_type() == PDU_SESSION_TYPE_E_IPV6) {
      resp->res.set_pdu_session_type(PDU_SESSION_TYPE_E_IPV4);
      cause_n1 =
          cause_value_5gsm_e::CAUSE_50_PDU_SESSION_TYPE_IPV4_ONLY_ALLOWED;
    }

    smf_n1::get_instance().create_n1_pdu_session_establishment_reject(
        resp->res, n1_sm_msg, cause_n1);
    conv::convert_string_2_hex(n1_sm_msg, n1_sm_msg_hex);
    resp->res.set_n1_sm_message(n1_sm_msg_hex);

  } else {  // PDU Session Establishment Accept
    Logger::smf_app().debug(
        "Prepare a PDU Session Establishment Accept message and send to UE");

    // TODO: Support IPv4 only for now
    if (resp->res.get_pdu_session_type() == PDU_SESSION_TYPE_E_IPV6) {
      resp->res.set_pdu_session_type(PDU_SESSION_TYPE_E_IPV4);
      cause_n1 =
          cause_value_5gsm_e::CAUSE_50_PDU_SESSION_TYPE_IPV4_ONLY_ALLOWED;
    }

    smf_n1::get_instance().create_n1_pdu_session_establishment_accept(
        resp->res, n1_sm_msg, cause_n1);
    conv::convert_string_2_hex(n1_sm_msg, n1_sm_msg_hex);
    resp->res.set_n1_sm_message(n1_sm_msg_hex);
    // N2 SM Information (Step 11, section 4.3.2.2.1 @ 3GPP TS 23.502):
    // PDUSessionRessourceSetupRequestTransfer IE
    smf_n2::get_instance()
        .create_n2_pdu_session_resource_setup_request_transfer(
            resp->res, n2_sm_info_type_e::PDU_RES_SETUP_REQ, n2_sm_info);

    conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);
    resp->res.set_n2_sm_information(n2_sm_info_hex);
  }

  // Fill N1N2MesasgeTransferRequestData
  // get SUPI and put into URL
  supi_t supi          = resp->res.get_supi();
  std::string supi_str = smf_get_supi_with_prefix(
      resp->res.get_supi_prefix(), smf_supi_to_string(supi));
  std::string api_version = smf_cfg->get_nf(oai::config::AMF_CONFIG_NAME)
                                ->get_sbi()
                                .get_api_version();
  std::string url =
      get_amf_addr() + NAMF_COMMUNICATION_BASE + api_version +
      fmt::format(
          NAMF_COMMUNICATION_N1N2_MESSAGE_TRANSFER_URL, supi_str.c_str());
  resp->res.set_amf_url(url);
  Logger::smf_app().debug(
      "N1N2MessageTransfer will be sent to AMF with URL: %s", url.c_str());

  // HTTP version
  resp->http_version = smf_cfg->http_version;

  // Fill the json part
  nlohmann::json json_data = {};
  // N1SM
  json_data["n1MessageContainer"]["n1MessageClass"] = N1N2_MESSAGE_CLASS;
  json_data["n1MessageContainer"]["n1MessageContent"]["contentId"] =
      N1_SM_CONTENT_ID;  // NAS part
  // N2SM
  if (resp->res.get_cause() ==
      static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED)) {
    json_data["n2InfoContainer"]["n2InformationClass"] = N1N2_MESSAGE_CLASS;
    json_data["n2InfoContainer"]["smInfo"]["pduSessionId"] =
        resp->res.get_pdu_session_id();
    // N2InfoContent (section 6.1.6.2.27@3GPP TS 29.518)
    json_data["n2InfoContainer"]["smInfo"]["n2InfoContent"]["ngapIeType"] =
        "PDU_RES_SETUP_REQ";  // NGAP message type
    json_data["n2InfoContainer"]["smInfo"]["n2InfoContent"]["ngapData"]
             ["contentId"] = N2_SM_CONTENT_ID;  // NGAP part
    json_data["n2InfoContainer"]["smInfo"]["sNssai"]["sst"] =
        resp->res.get_snssai().sst;
    json_data["n2InfoContainer"]["smInfo"]["sNssai"]["sd"] =
        std::to_string(resp->res.get_snssai().sd);
    // N1N2MsgTxfrFailureNotification
    std::string callback_uri =
        smf_cfg->local().get_sbi().get_url() + NSMF_PDU_SESSION_BASE +
        smf_cfg->local().get_sbi().get_api_version() +
        fmt::format(
            NSMF_CALLBACK_N1N2_MESSAGE_TRANSFER_FAILURE, supi_str.c_str());
    json_data["n1n2FailureTxfNotifURI"] = callback_uri.c_str();
  }
  // Others information
  // resp->res.n1n2_message_transfer_data["pti"] = 1;  //Don't
  // need this info for the moment
  json_data["pduSessionId"] = resp->res.get_pdu_session_id();

  resp->res.set_json_data(json_data);

  // send ITTI message to APP to trigger N1N2MessageTransfer towards AMFs
  Logger::smf_app().info(
      "Sending ITTI message %s to task TASK_SMF_APP", resp->get_msg_name());

  int ret = itti_inst->send_msg(resp);
  if (RETURNok != ret) {
    Logger::smf_app().error(
        "Could not send ITTI message %s to task TASK_SMF_SBI",
        resp->get_msg_name());
  }
}

//------------------------------------------------------------------------------
// TODO refactor: Break down this function and split logic (e.g. setting PDU
// session values) from actual response, we should only need resp here
void smf_context::send_pdu_session_update_response(
    const std::shared_ptr<itti_n11_update_sm_context_request>& req,
    const std::shared_ptr<itti_n11_update_sm_context_response>& resp,
    const session_management_procedures_type_e& session_procedure_type,
    const std::shared_ptr<smf_pdu_session>& sps) {
  std::string n1_sm_msg      = {};
  std::string n1_sm_msg_hex  = {};
  std::string n2_sm_info     = {};
  std::string n2_sm_info_hex = {};

  // TODO: check we got all responses vs
  // resp->res.flow_context_modified

  // TODO: Optional: send ITTI message to N10 to trigger UDM registration
  // (Nudm_UECM_Registration)  see TS29503_Nudm_UECM.yaml (
  // /{ueId}/registrations/smf-registrations/{pduSessionId}:)
  if (resp->res.get_cause() ==
      static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED)) {
    switch (session_procedure_type) {
      // PDU Session Establishment UE-Requested
      case session_management_procedures_type_e::
          PDU_SESSION_ESTABLISHMENT_UE_REQUESTED: {
        // No need to create N1/N2 Container, just Cause
        Logger::smf_app().info(
            "PDU Session Establishment Request (UE-Initiated)");
        nlohmann::json json_data = {};
        json_data["cause"]       = resp->res.get_cause();
        resp->res.set_json_data(json_data);

        // Update PDU session status to ACTIVE
        sps->set_pdu_session_status(pdu_session_status_e::PDU_SESSION_ACTIVE);

        // set UpCnxState to ACTIVATED
        sps->set_upCnx_state(upCnx_state_e::UPCNX_STATE_ACTIVATED);
        // Display UE Context Info
        Logger::smf_app().info("SMF context: \n %s", toString().c_str());

        // Trigger Event_exposure event
        std::string str_scid = req.get()->scid;
        // TODO: validate the str_scid

        scid_t scid = (scid_t) std::stoul(str_scid, nullptr, 0);
        trigger_ue_ip_change(scid, 1);
        trigger_plmn_change(scid, 1);
        trigger_ddds(scid, 1);
        trigger_pdusesest(scid, 1);
        trigger_flexcn_event(scid, 1);
      } break;

        // UE-Triggered Service Request Procedure (Step 1)
      case session_management_procedures_type_e::
          SERVICE_REQUEST_UE_TRIGGERED_STEP1: {
        // Create N2 SM Information: PDU Session Resource Setup Request
        // Transfer IE

        // N2 SM Information
        smf_n2::get_instance()
            .create_n2_pdu_session_resource_setup_request_transfer(
                resp->res, n2_sm_info_type_e::PDU_RES_SETUP_REQ, n2_sm_info);

        conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);
        resp->res.set_n2_sm_information(n2_sm_info_hex);

        // fill the content of SmContextUpdatedData
        nlohmann::json json_data = {};

        json_data["n2SmInfo"]["contentId"] = N2_SM_CONTENT_ID;
        json_data["n2SmInfoType"] = "PDU_RES_SETUP_REQ";  // NGAP message
        json_data["upCnxState"]   = "ACTIVATING";
        resp->res.set_json_data(json_data);
        // TODO: verify whether cause is needed (as in 23.502 but not in 3GPP
        // TS 29.502)

        // Update upCnxState to ACTIVATING
        sps->set_upCnx_state(upCnx_state_e::UPCNX_STATE_ACTIVATING);
      } break;

        // UE-triggered Service Request (Step 2)
      case session_management_procedures_type_e::
          SERVICE_REQUEST_UE_TRIGGERED_STEP2: {
        // No need to create N1/N2 Container, just Cause
        Logger::smf_app().info("UE Triggered Service Request (Step 2)");
        nlohmann::json json_data = {};
        json_data["cause"]       = resp->res.get_cause();
        json_data["upCnxState"]  = "ACTIVATED";
        resp->res.set_json_data(json_data);
        resp->res.set_http_code(http_status_code_e::HTTP_STATUS_CODE_200_OK);
      } break;

        // PDU Session Modification UE-initiated (Step 2)
      case session_management_procedures_type_e::
          PDU_SESSION_MODIFICATION_UE_INITIATED_STEP2: {
        // No need to create N1/N2 Container
        Logger::smf_app().info(
            "PDU Session Modification UE-initiated (Step 2)");
      } break;

        // PDU Session Modification UE-initiated (Step 3)
      case session_management_procedures_type_e::
          PDU_SESSION_MODIFICATION_UE_INITIATED_STEP3: {
        // No need to create N1/N2 Container
        Logger::smf_app().info(
            "PDU Session Modification UE-initiated (Step 3)");
        // TODO: To be completed
      } break;

      case session_management_procedures_type_e::HO_PATH_SWITCH_REQ: {
        // Create N2 SM Information: Path Switch Request Acknowledge Transfer
        // IE

        // N2 SM Information
        smf_n2::get_instance().create_n2_path_switch_request_ack(
            resp->res, n2_sm_info_type_e::PATH_SWITCH_REQ_ACK, n2_sm_info);

        conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);
        resp->res.set_n2_sm_information(n2_sm_info_hex);

        // fill the content of SmContextUpdatedData
        nlohmann::json json_data           = {};
        json_data["n2SmInfo"]["contentId"] = N2_SM_CONTENT_ID;
        json_data["n2SmInfoType"] = "PATH_SWITCH_REQ_ACK";  // NGAP message
        // NGAP message json_data["upCnxState"] ="ACTIVATING";
        resp->res.set_json_data(json_data);

      } break;

      case session_management_procedures_type_e::
          N2_HO_PREPARATION_PHASE_STEP2: {
        // Create N2 SM Information: Handover Command Transfer IE

        // N2 SM Information
        smf_n2::get_instance().create_n2_handover_command_transfer(
            resp->res, n2_sm_info_type_e::HANDOVER_CMD, n2_sm_info);

        conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);
        resp->res.set_n2_sm_information(n2_sm_info_hex);

        // fill the content of SmContextUpdatedData
        nlohmann::json json_data = {};

        json_data["n2SmInfo"]["contentId"] = N2_SM_CONTENT_ID;
        json_data["n2SmInfoType"]          = "HANDOVER_CMD";  // NGAP message
        json_data["hoState"]               = "PREPARED";
        resp->res.set_json_data(json_data);

        // Set HO State to prepared
        sps->set_ho_state(ho_state_e::HO_STATE_PREPARED);
      } break;

      default: {
        Logger::smf_app().info(
            "Unknown session procedure type %d", (int) session_procedure_type);
      }
    }
  } else {
    resp->res.set_http_code(
        http_status_code_e::HTTP_STATUS_CODE_406_NOT_ACCEPTABLE);
  }

  // send ITTI message to SMF_APP interface to trigger
  // SessionUpdateSMContextResponse towards AMFs
  Logger::smf_app().info(
      "Sending ITTI message %s to task TASK_SMF_APP", resp->get_msg_name());
  resp->session_procedure_type = session_procedure_type;
  int ret                      = itti_inst->send_msg(resp);
  if (RETURNok != ret) {
    Logger::smf_app().error(
        "Could not send ITTI message %s to task TASK_SMF_APP",
        resp->get_msg_name());
  }

  // The SMF may subscribe to the UE mobility event notification from the AMF
  // (e.g. location reporting, UE  moving into or out of Area Of Interest), by
  // invoking Namf_EventExposure_Subscribe service operation
  // For LADN, the SMF subscribes to the UE moving into or out of LADN service
  // area event notification by providing the LADN DNN as an indicator for the
  // Area Of Interest
  // see step 17@section 4.3.2.2.1@3GPP TS 23.502
}

//------------------------------------------------------------------------------
void smf_context::send_pdu_session_release_response(
    const std::shared_ptr<itti_n11_release_sm_context_request>& req,
    const std::shared_ptr<itti_n11_release_sm_context_response>& resp,
    const session_management_procedures_type_e& session_procedure_type,
    const std::shared_ptr<smf_pdu_session>& sps) {
  if (resp->res.get_cause() ==
      static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED)) {
    switch (session_procedure_type) {
      case session_management_procedures_type_e::
          PDU_SESSION_RELEASE_UE_REQUESTED_STEP1: {
        // UE-initiated PDU Session Release
        Logger::smf_app().info("PDU Session Release UE-initiated (Step 1))");
        std::shared_ptr<pdu_session_release_sm_context_response>
            session_release_msg =
                std::make_shared<pdu_session_release_sm_context_response>(
                    resp->res);

        // Create N1 SM message (PDU Session Release Command)
        std::string n1_sm_msg     = {};
        std::string n1_sm_msg_hex = {};
        smf_n1::get_instance().create_n1_pdu_session_release_command(
            session_release_msg, n1_sm_msg,
            cause_value_5gsm_e::CAUSE_36_REGULAR_DEACTIVATION);  // TODO:
                                                                 // check
                                                                 // Cause
        conv::convert_string_2_hex(n1_sm_msg, n1_sm_msg_hex);
        resp->res.set_n1_sm_message(n1_sm_msg_hex);

        // Create N2 SM info (if the UP connection of the PDU Session is
        // active)
        if (sps->get_upCnx_state() == upCnx_state_e::UPCNX_STATE_ACTIVATED) {
          // N2 SM Information
          std::string n2_sm_info     = {};
          std::string n2_sm_info_hex = {};
          smf_n2::get_instance()
              .create_n2_pdu_session_resource_release_command_transfer(
                  session_release_msg, n2_sm_info_type_e::PDU_RES_REL_CMD,
                  n2_sm_info);
          conv::convert_string_2_hex(n2_sm_info, n2_sm_info_hex);
          resp->res.set_n2_sm_information(n2_sm_info_hex);

          // Prepare response to send to AMF
          // (PDUSession_UpdateSMContextResponse)
          nlohmann::json sm_context_response_data           = {};
          sm_context_response_data["n1SmMsg"]["contentId"]  = N1_SM_CONTENT_ID;
          sm_context_response_data["n2SmInfo"]["contentId"] = N2_SM_CONTENT_ID;
          sm_context_response_data["n2SmInfoType"] =
              "PDU_RES_REL_CMD";  // NGAP message

          resp->res.set_json_data(sm_context_response_data);
        } else {
          // fill the content of SmContextUpdatedData
          nlohmann::json json_data          = {};
          json_data["n1SmMsg"]["contentId"] = N1_SM_CONTENT_ID;
          resp->res.set_json_data(json_data);
        }

        // Update PDU session status to PDU_SESSION_INACTIVE_PENDING
        sps->set_pdu_session_status(
            pdu_session_status_e::PDU_SESSION_INACTIVE_PENDING);

        // set UpCnxState to DEACTIVATED
        sps->set_upCnx_state(upCnx_state_e::UPCNX_STATE_DEACTIVATED);

        // TODO: To be completed
        // TODO: start timer T3592 (see Section 6.3.3@3GPP TS 24.501)
        // get smf_pdu_session and set the corresponding timer

        scid_t scid = {};
        try {
          scid = (scid_t) std::stoul(req->scid, nullptr, 10);
        } catch (const std::exception& e) {
          Logger::smf_n1().warn(
              "Error when converting from string to int for SCID, "
              "error: %s",
              e.what());
          // TODO Stefan: I could not find a better response code here
          smf_app_inst->trigger_update_context_error_response(
              http_status_code_e::HTTP_STATUS_CODE_403_FORBIDDEN,
              PDU_SESSION_APPLICATION_ERROR_NETWORK_FAILURE, resp->pid);
          return;
        }
        resp->res.set_http_code(http_status_code_e::HTTP_STATUS_CODE_200_OK);

        // Store the context for the timer handling
        sps.get()->set_pending_n11_msg(
            std::dynamic_pointer_cast<itti_n11_msg>(resp));

        sps->timer_T3592 = itti_inst->timer_setup(
            T3592_TIMER_VALUE_SEC, 0, TASK_SMF_APP, TASK_SMF_APP_TRIGGER_T3592,
            scid);
      } break;
        // PDU Session Release UE-initiated (Step 2)
      case session_management_procedures_type_e::
          PDU_SESSION_RELEASE_UE_REQUESTED_STEP2: {
        // No need to create N1/N2 Container
        Logger::smf_app().info("PDU Session Release UE-initiated (Step 2)");
        // TODO: To be completed
      } break;

        // PDU Session Release UE-initiated (Step 3)
      case session_management_procedures_type_e::
          PDU_SESSION_RELEASE_UE_REQUESTED_STEP3: {
        // No need to create N1/N2 Container
        Logger::smf_app().info("PDU Session Release UE-initiated (Step 3)");
        // TODO: To be completed
      } break;

      case session_management_procedures_type_e::
          PDU_SESSION_RELEASE_AMF_INITIATED: {
        Logger::smf_app().info("PDU Session Release AMF-initiated");
        // TODO: To be completed
      } break;

      default: {
        resp->res.set_http_code(
            http_status_code_e::HTTP_STATUS_CODE_204_NO_CONTENT);
      }
    }

  } else {
    resp->res.set_http_code(
        http_status_code_e::HTTP_STATUS_CODE_406_NOT_ACCEPTABLE);
  }

  // clear the resources including addresses allocated to this Session and
  // associated QoS flows
  sps->deallocate_ressources(resp->res.get_dnn());

  // send ITTI message to SMF_APP interface to trigger the response towards
  // AMFs
  Logger::smf_app().info(
      "Sending ITTI message %s to task TASK_SMF_APP", resp->get_msg_name());
  int ret = itti_inst->send_msg(resp);
  if (RETURNok != ret) {
    Logger::smf_app().error(
        "Could not send ITTI message %s to task TASK_SMF_APP",
        resp->get_msg_name());
  }
}
