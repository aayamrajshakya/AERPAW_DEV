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

/*! \file smf_pfcp_association.cpp
 \brief
 \author  Lionel GAUTHIER
 \date 2019
 \email: lionel.gauthier@eurecom.fr
 */

#include "smf_pfcp_association.hpp"

#include "common_defs.h"
#include "logger.hpp"
#include "smf_n4.hpp"
#include "smf_procedure.hpp"
#include "smf_config.hpp"
#include "fqdn.hpp"
#include <arpa/inet.h>

using namespace smf;
using namespace std;
using namespace oai::model::pcf;

extern itti_mw* itti_inst;
extern smf_n4* smf_n4_inst;
extern std::unique_ptr<oai::config::smf::smf_config> smf_cfg;

//---------------------------------------------------------------------------------------------
edge edge::from_upf_info(const upf_info_t& upf_info) {
  edge e                             = {};
  snssai_upf_info_item_s snssai_item = {};

  Logger::smf_app().debug("UPF info: %s", upf_info.to_string().c_str());

  for (auto& snssai : upf_info.snssai_upf_info_list) {
    snssai_item.snssai            = snssai.snssai;
    snssai_item.dnn_upf_info_list = snssai.dnn_upf_info_list;
    bool found                    = false;
    for (auto& item : e.snssai_dnns) {
      if (item.snssai == snssai.snssai) {
        // Update item if exist
        found = true;
        snssai_item.dnn_upf_info_list.insert(
            item.dnn_upf_info_list.begin(), item.dnn_upf_info_list.end());
        item.dnn_upf_info_list = snssai_item.dnn_upf_info_list;
        // Logger::smf_app().debug(
        //    "Updated item info: %s", snssai_item.to_string().c_str());
        break;
      }
    }
    // Insert a new item otherwise
    if (!found) e.snssai_dnns.insert(snssai_item);
  }

  if (!e.snssai_dnns.empty()) {
    Logger::smf_app().debug("UPF link info:");
    for (const auto s : e.snssai_dnns) {
      Logger::smf_app().debug("%s", s.to_string().c_str());
    }
  } else {
    Logger::smf_app().debug("UPF link info empty");
  }
  return e;
}

//---------------------------------------------------------------------------------------------
edge edge::from_upf_info(
    const upf_info_t& upf_info, const interface_upf_info_item_t& interface) {
  // TODO: Bring updates from the previous funtion to this one
  edge e = {};
  e.type = pfcp_association::iface_type_from_string(interface.interface_type);
  e.ip_addr.s_addr = interface.ipv4_addresses[0].s_addr;
  // e.ip6_addr    = interface.ipv6_addresses[0];
  e.nw_instance = interface.network_instance;

  // we filter out the DNAIs which do not map to the given NW interface
  for (const auto& snssai_item : upf_info.snssai_upf_info_list) {
    snssai_upf_info_item_s new_snssai_item;
    new_snssai_item.snssai = snssai_item.snssai;

    for (const auto& dnn_item : snssai_item.dnn_upf_info_list) {
      dnn_upf_info_item_s new_dnn_item;
      new_dnn_item.dnn = dnn_item.dnn;
      if (!dnn_item.dnai_list.empty() &&
          !dnn_item.dnai_nw_instance_list.empty()) {
        for (const auto& dnai : dnn_item.dnai_list) {
          auto dnai_it = dnn_item.dnai_nw_instance_list.find(dnai);
          if (dnai_it != dnn_item.dnai_nw_instance_list.end()) {
            if (dnai_it->second == e.nw_instance) {
              new_dnn_item.dnai_list.insert(dnai);
              break;
            }
          }
        }
      } else {
        Logger::smf_app().debug(
            "DNAI List or DNAI NW Instance List is empty for this UPF.");
      }
      new_snssai_item.dnn_upf_info_list.insert(new_dnn_item);
    }
    e.snssai_dnns.insert(new_snssai_item);
  }

  return e;
}

//---------------------------------------------------------------------------------------------
bool edge::serves_network(
    const std::string& dnn, const snssai_t& snssai,
    const std::unordered_set<std::string>& dnais,
    std::string& matched_dnai) const {
  // Logger::smf_app().debug(
  //    "Serves Network, DNN %s, S-NSSAI %s", dnn.c_str(),
  //    snssai.toString().c_str());
  // just create a snssai_upf_info_item for fast lookup
  snssai_upf_info_item_s snssai_item = {};
  snssai_item.snssai                 = snssai;
  // For debugging purpose
  if (!snssai_dnns.empty()) {
    Logger::smf_app().debug("S-NSSAI UPF info list: ");
    for (const auto& s : snssai_dnns) {
      Logger::smf_app().debug("%s", s.to_string().c_str());
    }
  }

  for (const auto& snssai_it : snssai_dnns) {
    if (snssai_it.snssai == snssai_item.snssai) {
      // create temp item for fast lookup
      dnn_upf_info_item_s dnn_item = {};
      dnn_item.dnn                 = dnn;
      auto dnn_it                  = snssai_it.dnn_upf_info_list.find(dnn_item);
      if (dnn_it != snssai_it.dnn_upf_info_list.end()) {
        if (!dnais.empty()) {
          // should be only 1 DNAI
          for (const auto& dnai : dnn_it->dnai_list) {
            // O(1)
            auto found_dnai = dnais.find(dnai);
            if (found_dnai != dnais.end()) {
              matched_dnai = dnai;
              return true;
            }
          }
        } else {
          return true;
        }
      }
    }
  }

  Logger::smf_app().debug("Could not serve network, return false");
  return false;
}

//---------------------------------------------------------------------------------------------
bool edge::serves_network(
    const std::string& dnn, const snssai_t& snssai) const {
  std::unordered_set<string> set;
  std::string s = {};
  return serves_network(dnn, snssai, set, s);
}

//---------------------------------------------------------------------------------------------
bool edge::get_qos_flows(std::vector<std::shared_ptr<smf_qos_flow>>& flows) {
  flows.clear();
  for (const auto& flow : this->qos_flows) {
    flows.push_back(flow);
  }
  return flows.size() > 0;
}

//---------------------------------------------------------------------------------------------
bool edge::get_qos_flows(
    pdu_session_id_t pid, std::vector<std::shared_ptr<smf_qos_flow>>& flows) {
  flows.clear();
  for (const auto& flow : this->qos_flows) {
    if (flow->pdu_session_id == pid) flows.push_back(flow);
  }
  return flows.size() > 0;
}

//---------------------------------------------------------------------------------------------
std::shared_ptr<smf_qos_flow> edge::get_qos_flow(const pfcp::pdr_id_t& pdr_id) {
  // it may happen that 2 qos flows have the same PDR ID e.g. in an
  // UL CL scenario, but then they will also have the same FTEID
  for (auto& flow_it : qos_flows) {
    if (flow_it->pdr_id_ul == pdr_id || flow_it->pdr_id_dl == pdr_id) {
      return flow_it;
    }
  }
  return {};
}

//---------------------------------------------------------------------------------------------
std::shared_ptr<smf_qos_flow> edge::get_qos_flow(const pfcp::qfi_t& qfi) {
  for (auto& flow_it : qos_flows) {
    if (flow_it->qfi == qfi) {
      return flow_it;
    }
  }
  return {};
}

//---------------------------------------------------------------------------------------------
std::shared_ptr<smf_qos_flow> edge::get_qos_flow(const pfcp::far_id_t& far_id) {
  for (auto& flow_it : qos_flows) {
    if (flow_it->far_id_ul.second == far_id ||
        flow_it->far_id_dl.second == far_id) {
      return flow_it;
    }
  }
  return {};
}

//------------------------------------------------------------------------------
void smf_qos_flow::mark_as_released() {
  released = true;
}

std::string smf_qos_flow::toString(const std::string& indent) const {
  std::string s = {};
  s.append(indent).append("QoS Flow:\n");
  s.append(indent)
      .append("\tQFI:\t\t")
      .append(std::to_string((uint8_t) qfi.qfi))
      .append("\n");
  s.append(indent)
      .append("\tUL FTEID:\t")
      .append(ul_fteid.toString())
      .append("\n");
  s.append(indent)
      .append("\tDL FTEID:\t")
      .append(dl_fteid.toString())
      .append("\n");
  s.append(indent)
      .append("\tPDR ID UL:\t")
      .append(std::to_string(pdr_id_ul.rule_id))
      .append("\n");
  s.append(indent)
      .append("\tPDR ID DL:\t")
      .append(std::to_string(pdr_id_dl.rule_id))
      .append("\n");

  s.append(indent)
      .append("\tPrecedence:\t")
      .append(std::to_string(precedence.precedence))
      .append("\n");
  if (far_id_ul.first) {
    s.append(indent)
        .append("\tFAR ID UL:\t")
        .append(std::to_string(far_id_ul.second.far_id))
        .append("\n");
  }
  if (far_id_dl.first) {
    s.append(indent)
        .append("\tFAR ID DL:\t")
        .append(std::to_string(far_id_dl.second.far_id))
        .append("\n");
  }
  if (urr_id.urr_id != 0) {
    s.append(indent)
        .append("\tURR ID:\t")
        .append(std::to_string(urr_id.urr_id))
        .append("\n");
  }
  return s;
}

//------------------------------------------------------------------------------
std::string smf_qos_flow::toString() const {
  return toString("\t\t");
}
//------------------------------------------------------------------------------
void smf_qos_flow::deallocate_ressources() {
  clear();
  Logger::smf_app().info(
      "Resources associated with this QoS Flow (%d) have been released",
      (uint8_t) qfi.qfi);
}

//------------------------------------------------------------------------------
iface_type pfcp_association::iface_type_from_string(const std::string& input) {
  iface_type type_tmp = {};
  if (input == "N3") {
    type_tmp = iface_type::N3;
  } else if (input == "N6") {
    type_tmp = iface_type::N6;
  } else if (input == "N9") {
    type_tmp = iface_type::N9;
  } else {
    Logger::smf_app().error("Interface Type %s undefined!", input.c_str());
  }

  return type_tmp;
}
//------------------------------------------------------------------------------
std::string pfcp_association::string_from_iface_type(const iface_type& type) {
  std::string output = {};
  switch (type) {
    case iface_type::N3:
      return "N3";
    case iface_type::N6:
      return "N6";
    case iface_type::N9:
      return "N9";
    default:
      return "?";
  }
}

//------------------------------------------------------------------------------
void pfcp_association::notify_add_session(const pfcp::fseid_t& cp_fseid) {
  std::unique_lock<std::mutex> l(m_sessions);
  sessions.insert(cp_fseid);
}

//------------------------------------------------------------------------------
bool pfcp_association::has_session(const pfcp::fseid_t& cp_fseid) {
  std::unique_lock<std::mutex> l(m_sessions);
  auto it = sessions.find(cp_fseid);
  if (it != sessions.end()) {
    return true;
  } else {
    return false;
  }
}
//------------------------------------------------------------------------------
void pfcp_association::notify_del_session(const pfcp::fseid_t& cp_fseid) {
  std::unique_lock<std::mutex> l(m_sessions);
  sessions.erase(cp_fseid);
}

//------------------------------------------------------------------------------
void pfcp_association::del_sessions() {
  std::unique_lock<std::mutex> l(m_sessions);
  sessions.clear();
}

//------------------------------------------------------------------------------
void pfcp_association::restore_n4_sessions() {
  std::unique_lock<std::mutex> l(m_sessions);
  if (sessions.size()) {
    is_restore_sessions_pending = true;
    std::unique_ptr<n4_session_restore_procedure> restore_proc =
        std::make_unique<n4_session_restore_procedure>(sessions);

    restore_proc->run();
  }
}

//---------------------------------------------------------------------------------------------
bool pfcp_association::find_interface_edge(
    const iface_type& type_match, std::vector<edge>& edges) {
  if (!is_upf_profile_set()) {
    return false;
  }
  upf_info_t upf_info = {};

  upf_node_profile.get_upf_info(upf_info);
  for (const auto& iface : upf_info.interface_upf_info_list) {
    iface_type type = iface_type_from_string(iface.interface_type);
    if (type == type_match) {
      edges.emplace_back(edge::from_upf_info(upf_info, iface));
    }
  }
  // Because interfaceUpfInfoList is optional in TS 29.510 (why even?), we
  // just guess that this UPF has a N3 or N6 interface
  if (upf_info.interface_upf_info_list.empty()) {
    Logger::smf_app().info(
        "UPF Interface list is empty: Assume that the UPF has a N3 and a N6 "
        "interface.");
    edge e = edge::from_upf_info(upf_info);
    e.type = type_match;
    edges.emplace_back(e);
  }

  return !edges.empty();
}

//---------------------------------------------------------------------------------------------
bool pfcp_association::find_n3_edge(std::vector<edge>& edges) {
  return find_interface_edge(iface_type::N3, edges);
}

//---------------------------------------------------------------------------------------------
bool pfcp_association::find_n6_edge(std::vector<edge>& edges) {
  bool success = find_interface_edge(iface_type::N6, edges);
  if (success) {
    for (auto& e : edges) {
      e.uplink = true;
    }
  }
  return success;
}

//------------------------------------------------------------------------------
bool pfcp_association::find_upf_edge(
    const std::shared_ptr<pfcp_association>& other_upf, edge& out_edge) {
  if (!is_upf_profile_set() || !other_upf->is_upf_profile_set()) {
    return false;
  }

  upf_info_t upf_info;

  upf_node_profile.get_upf_info(upf_info);
  for (const auto& iface : upf_info.interface_upf_info_list) {
    if (iface.endpoint_fqdn == other_upf->get_upf_node_profile().get_fqdn()) {
      out_edge = edge::from_upf_info(upf_info, iface);
      return true;
    }
    for (auto current_ip : iface.ipv4_addresses) {
      std::vector<in_addr> other_ips;
      other_upf->get_upf_node_profile().get_nf_ipv4_addresses(other_ips);
      for (auto other_ip : other_ips) {
        if (current_ip.s_addr == other_ip.s_addr) {
          out_edge = edge::from_upf_info(upf_info, iface);
          return true;
        }
      }
    }
  }
  return false;
}

//------------------------------------------------------------------------------
std::string pfcp_association::get_printable_name() {
  if (upf_profile_is_set) {
    if (!upf_node_profile.get_fqdn().empty()) {
      return upf_node_profile.get_fqdn();
    }
    std::vector<in_addr> ipv4_addresses;
    upf_node_profile.get_nf_ipv4_addresses(ipv4_addresses);
    if (!ipv4_addresses.empty()) {
      std::string addresses;
      for (auto add : ipv4_addresses) {
        addresses.append(conv::toString(add));
        addresses.append(";");
      }
      return addresses;
    }
    return std::to_string(hash_node_id);
  } else {
    return std::to_string(hash_node_id);
  }
}

//------------------------------------------------------------------------------
void pfcp_association::display() {
  Logger::smf_app().debug("\tUPF Node Id: %s", node_id.toString().c_str());

  if (upf_profile_is_set) {
    Logger::smf_app().debug("\tUPF Node profile:");
    upf_node_profile.display();
  }
}

/******************************************************************************/
/***************************** PFCP ASSOCIATIONS ******************************/
/******************************************************************************/

std::shared_ptr<pfcp_association> pfcp_associations::check_association_on_add(
    pfcp::node_id_t& node_id, pfcp::recovery_time_stamp_t& recovery_time_stamp,
    bool& restore_n4_sessions, const bool use_function_features,
    pfcp::up_function_features_s& function_features) {
  std::shared_ptr<pfcp_association> sa = {};
  if (get_association(node_id, sa)) {
    itti_inst->timer_remove(sa->timer_heartbeat);
    if (sa->recovery_time_stamp == recovery_time_stamp) {
      restore_n4_sessions = false;
    } else {
      restore_n4_sessions = true;
    }
    sa->recovery_time_stamp = recovery_time_stamp;
    if (use_function_features) {
      sa->function_features = {};
    } else {
      sa->function_features.first  = true;
      sa->function_features.second = function_features;
    }

    trigger_heartbeat_request_procedure(sa);
    return sa;
  }
  return {};  // empty ptr
}

//---------------------------------------------------------------------------------------------
bool pfcp_associations::resolve_upf_hostname(pfcp::node_id_t& node_id) {
  // TODO why is this even here? We can see in the logs that UPF IP is requested
  // before, at least for NRF scenario
  //  Resolve FQDN to get UPF IP address if necessary
  if (node_id.node_id_type == pfcp::NODE_ID_TYPE_FQDN) {
    Logger::smf_app().info("Node ID Type FQDN: %s", node_id.fqdn.c_str());

    std::string ip_addr = {};
    uint32_t port       = {0};
    uint8_t addr_type   = {0};

    if (!fqdn::resolve(node_id.fqdn, ip_addr, port, addr_type)) {
      Logger::smf_app().warn(
          "Add association with node (FQDN) %s: cannot resolve the hostname!",
          node_id.fqdn.c_str());
      return false;
    }
    switch (addr_type) {
      case 0:
        node_id.u1.ipv4_address = conv::fromString(ip_addr);
        return true;
      case 1:
        // TODO
        Logger::smf_app().warn(
            "Node ID Type FQDN: %s. IPv6 Addr, this mode has not been "
            "supported yet!",
            node_id.fqdn.c_str());
        return false;
      default:
        Logger::smf_app().warn("Unknown Address type");
        return false;
    }
  }
  return true;  // no FQDN so we just continue
}

//---------------------------------------------------------------------------------------------
void pfcp_associations::associate_with_upf_profile(
    std::shared_ptr<pfcp_association>& sa, const pfcp::node_id_t& node_id) {
  // TODO wouldn't it be better to use a shared lock? because here we have only
  // read access
  std::lock_guard<std::mutex> lck(m_mutex);
  // Associate with UPF profile if exist
  for (const auto& it : pending_associations) {
    if (it->is_upf_profile_set() && ((it->node_id.fqdn == node_id.fqdn) ||
                                     (it->node_id.u1.ipv4_address.s_addr ==
                                      node_id.u1.ipv4_address.s_addr))) {
      Logger::smf_app().info("Associate with UPF profile");
      sa->set_upf_node_profile(it->get_upf_node_profile());
      return;
    }
  }
}

//------------------------------------------------------------------------------
bool pfcp_associations::add_association(
    pfcp::node_id_t& node_id, pfcp::recovery_time_stamp_t& recovery_time_stamp,
    bool& restore_n4_sessions) {
  pfcp::up_function_features_s tmp     = {};
  std::shared_ptr<pfcp_association> sa = check_association_on_add(
      node_id, recovery_time_stamp, restore_n4_sessions, false, tmp);
  if (sa) return true;

  if (!resolve_upf_hostname(node_id)) return false;

  restore_n4_sessions = false;
  sa = std::make_shared<pfcp_association>(node_id, recovery_time_stamp);
  sa->recovery_time_stamp  = recovery_time_stamp;
  std::size_t hash_node_id = std::hash<pfcp::node_id_t>{}(node_id);

  associate_with_upf_profile(sa, node_id);

  if (!associations_graph.full()) {
    associations_graph.insert_into_graph(sa);
    trigger_heartbeat_request_procedure(sa);
  } else {
    Logger::smf_app().info("What do we do if too many associations?");
  }
  return true;
}

//------------------------------------------------------------------------------
bool pfcp_associations::add_association(
    pfcp::node_id_t& node_id, pfcp::recovery_time_stamp_t& recovery_time_stamp,
    pfcp::up_function_features_s& function_features,
    bool& restore_n4_sessions) {
  std::shared_ptr<pfcp_association> sa = check_association_on_add(
      node_id, recovery_time_stamp, restore_n4_sessions, true,
      function_features);
  if (sa) return true;

  if (!resolve_upf_hostname(node_id)) return false;

  restore_n4_sessions = false;
  sa                  = std::make_shared<pfcp_association>(
      node_id, recovery_time_stamp, function_features);
  sa->recovery_time_stamp      = recovery_time_stamp;
  sa->function_features.first  = true;
  sa->function_features.second = function_features;
  std::size_t hash_node_id     = std::hash<pfcp::node_id_t>{}(node_id);

  associate_with_upf_profile(sa, node_id);

  if (!associations_graph.full()) {
    associations_graph.insert_into_graph(sa);
    // Display UPF Node profile
    sa->get_upf_node_profile().display();
    trigger_heartbeat_request_procedure(sa);
  } else {
    Logger::smf_app().info("What do we do if too many associations?");
  }
  return true;
}

//------------------------------------------------------------------------------
bool pfcp_associations::add_association(
    pfcp::node_id_t& node_id, pfcp::recovery_time_stamp_t& recovery_time_stamp,
    pfcp::up_function_features_s& function_features,
    pfcp::enterprise_specific_s& enterprise_specific,
    bool& restore_n4_sessions) {
  // TODO what is this enterprise_specfic, this is not used at all, even before
  // the refactor

  std::shared_ptr<pfcp_association> sa = check_association_on_add(
      node_id, recovery_time_stamp, restore_n4_sessions, true,
      function_features);
  if (sa) return true;

  restore_n4_sessions = false;
  sa                  = std::make_shared<pfcp_association>(
      node_id, recovery_time_stamp, function_features);
  sa->recovery_time_stamp      = recovery_time_stamp;
  sa->function_features.first  = true;
  sa->function_features.second = function_features;
  std::size_t hash_node_id     = std::hash<pfcp::node_id_t>{}(node_id);

  // TODO why don't we check the FQDN and associate with UPF profile here as
  // well?

  if (!associations_graph.full()) {
    associations_graph.insert_into_graph(sa);
    trigger_heartbeat_request_procedure(sa);
  } else {
    Logger::smf_app().info("What do we do if too many associations?");
  }
  return true;
}

//------------------------------------------------------------------------------
bool pfcp_associations::update_association(
    pfcp::node_id_t& node_id, pfcp::up_function_features_s& function_features) {
  std::shared_ptr<pfcp_association> sa =
      std::shared_ptr<pfcp_association>(nullptr);
  // TODO should we trigger sth in the graph as well or ongoing PDU sessions?
  if (get_association(node_id, sa)) {
    sa->function_features.first  = true;
    sa->function_features.second = function_features;
  } else {
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
bool pfcp_associations::get_association(
    const pfcp::node_id_t& node_id, std::shared_ptr<pfcp_association>& sa) {
  std::shared_ptr<pfcp_association> association = {};
  std::size_t hash_node_id                      = {};
  pfcp::node_id_t node_id_tmp                   = node_id;

  // Resolve FQDN/IP Addr if necessary
  fqdn::resolve(node_id_tmp);

  // We suppose that by default hash map is made with node_id_type FQDN
  if (node_id_tmp.node_id_type == pfcp::NODE_ID_TYPE_FQDN) {
    hash_node_id = std::hash<pfcp::node_id_t>{}(node_id_tmp);
    association  = associations_graph.get_association(hash_node_id);
    if (association) {
      sa = association;
      return true;
    }
    node_id_tmp.node_id_type = pfcp::NODE_ID_TYPE_IPV4_ADDRESS;
  } else if (node_id_tmp.node_id_type == pfcp::NODE_ID_TYPE_IPV4_ADDRESS) {
    hash_node_id = std::hash<pfcp::node_id_t>{}(node_id_tmp);
    association  = associations_graph.get_association(hash_node_id);
    if (association) {
      sa = association;
      return true;
    }
    node_id_tmp.node_id_type = pfcp::NODE_ID_TYPE_FQDN;
  }

  // We didn't found association, may be because hash map is made with different
  // node type
  hash_node_id = std::hash<pfcp::node_id_t>{}(node_id_tmp);
  association  = associations_graph.get_association(hash_node_id);
  if (association) {
    sa = association;
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
bool pfcp_associations::get_association(
    const pfcp::fseid_t& cp_fseid,
    std::shared_ptr<pfcp_association>& sa) const {
  std::shared_ptr<pfcp_association> association =
      associations_graph.get_association(cp_fseid);
  if (association) {
    sa = association;
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
void pfcp_associations::restore_n4_sessions(const pfcp::node_id_t& node_id) {
  std::shared_ptr<pfcp_association> sa = {};
  if (get_association(node_id, sa)) {
    sa->restore_n4_sessions();
  }
}

//------------------------------------------------------------------------------
void pfcp_associations::trigger_heartbeat_request_procedure(
    std::shared_ptr<pfcp_association>& s) {
  s->timer_heartbeat = itti_inst->timer_setup(
      PFCP_ASSOCIATION_HEARTBEAT_INTERVAL_SEC, 0, TASK_SMF_N4,
      TASK_SMF_N4_TRIGGER_HEARTBEAT_REQUEST, s->hash_node_id);
}

//------------------------------------------------------------------------------
void pfcp_associations::initiate_heartbeat_request(
    timer_id_t timer_id, uint64_t arg2_user) {
  size_t hash_node_id = (size_t) arg2_user;
  auto association    = associations_graph.get_association(hash_node_id);
  if (association) {
    Logger::smf_n4().info(
        "PFCP HEARTBEAT PROCEDURE hash %u starting", hash_node_id);
    association->num_retries_timer_heartbeat = 0;
    smf_n4_inst->send_heartbeat_request(association);
  }
}

//------------------------------------------------------------------------------
void pfcp_associations::timeout_heartbeat_request(
    timer_id_t timer_id, uint64_t arg2_user) {
  size_t hash_node_id = (size_t) arg2_user;
  auto association    = associations_graph.get_association(hash_node_id);
  if (association) {
    if (association->num_retries_timer_heartbeat <
        PFCP_ASSOCIATION_HEARTBEAT_MAX_RETRIES) {
      Logger::smf_n4().info(
          "PFCP HEARTBEAT PROCEDURE hash %u TIMED OUT (retrie %d)",
          hash_node_id, association->num_retries_timer_heartbeat);
      association->num_retries_timer_heartbeat++;
      smf_n4_inst->send_heartbeat_request(association);
    } else {
      Logger::smf_n4().warn(
          "PFCP HEARTBEAT PROCEDURE FAILED after %d retries, remove the "
          "association with this UPF",
          PFCP_ASSOCIATION_HEARTBEAT_MAX_RETRIES);
      // Related session contexts and PFCP associations become invalid and may
      // be deleted-> Send request to SMF App to remove all associated
      // sessions and notify AMF accordingly
      std::shared_ptr<itti_n4_node_failure> itti_msg =
          std::make_shared<itti_n4_node_failure>(TASK_SMF_N4, TASK_SMF_APP);
      itti_msg->node_id = association->node_id;
      int ret           = itti_inst->send_msg(itti_msg);
      if (RETURNok != ret) {
        Logger::smf_n4().error(
            "Could not send ITTI message %s to task TASK_SMF_APP",
            itti_msg->get_msg_name());
      }

      // Remove UPF from the associations
      remove_association(hash_node_id);
      associations_graph.print_graph();
    }
  }
}

//------------------------------------------------------------------------------
void pfcp_associations::timeout_release_request(
    timer_id_t timer_id, uint64_t arg2_user) {
  size_t hash_node_id = (size_t) arg2_user;
  auto association    = associations_graph.get_association(hash_node_id);
  if (association) {
    Logger::smf_n4().info("PFCP RELEASE REQUEST hash %u", hash_node_id);
    smf_n4_inst->send_release_request(association);
  }
}

//------------------------------------------------------------------------------
void pfcp_associations::handle_receive_heartbeat_response(
    const uint64_t trxn_id) {
  auto association = associations_graph.get_association_for_trxn_id(trxn_id);

  if (association) {
    itti_inst->timer_remove(association->timer_heartbeat);
    trigger_heartbeat_request_procedure(association);
    return;
  }
}

//------------------------------------------------------------------------------
std::shared_ptr<upf_graph> pfcp_associations::select_up_node(
    const int node_selection_criteria) {
  return associations_graph.select_upf_node(node_selection_criteria);
}

//------------------------------------------------------------------------------
std::shared_ptr<upf_graph> pfcp_associations::select_up_node(
    const snssai_t& snssai, const std::string& dnn) {
  return associations_graph.select_upf_node(snssai, dnn);
}

//------------------------------------------------------------------------------
std::shared_ptr<upf_graph> pfcp_associations::select_up_node(
    const SmPolicyDecision& decision, const snssai_t& snssai,
    const std::string& dnn) {
  return associations_graph.select_upf_nodes(decision, snssai, dnn);
}

//------------------------------------------------------------------------------
void pfcp_associations::notify_add_session(
    const pfcp::node_id_t& node_id, const pfcp::fseid_t& cp_fseid) {
  std::shared_ptr<pfcp_association> sa = {};
  if (get_association(node_id, sa)) {
    sa->notify_add_session(cp_fseid);
  }
}

//------------------------------------------------------------------------------
void pfcp_associations::notify_del_session(const pfcp::fseid_t& cp_fseid) {
  std::shared_ptr<pfcp_association> sa = {};
  if (get_association(cp_fseid, sa)) {
    sa->notify_del_session(cp_fseid);
  }
}

//------------------------------------------------------------------------------
bool pfcp_associations::add_peer_candidate_node(
    const pfcp::node_id_t& node_id) {
  for (std::vector<std::shared_ptr<pfcp_association>>::iterator it =
           pending_associations.begin();
       it < pending_associations.end(); ++it) {
    if ((*it)->node_id == node_id) {
      // TODO purge sessions of this node
      Logger::smf_app().info("TODO purge sessions of this node");
      // Protect the deletion
      std::lock_guard<std::mutex> lck(m_mutex);
      pending_associations.erase(it);
      break;
    }
  }

  std::shared_ptr<pfcp_association> s =
      std::make_shared<pfcp_association>(node_id);
  pending_associations.push_back(s);
  return true;
}

//------------------------------------------------------------------------------
bool pfcp_associations::add_peer_candidate_node(
    const pfcp::node_id_t& node_id, const upf_profile& profile) {
  for (std::vector<std::shared_ptr<pfcp_association>>::iterator it =
           pending_associations.begin();
       it < pending_associations.end(); ++it) {
    if ((*it)->node_id == node_id) {
      // TODO purge sessions of this node
      Logger::smf_app().info("TODO purge sessions of this node");
      pending_associations.erase(it);
      break;
    }
  }

  std::shared_ptr<pfcp_association> s =
      std::make_shared<pfcp_association>(node_id);
  s->set_upf_node_profile(profile);
  pending_associations.push_back(s);
  // Logger::smf_app().info("Added a pending association candidate");
  // s->display();
  return true;
}

//------------------------------------------------------------------------------
bool pfcp_associations::remove_association(
    const std::string& node_instance_id) {
  // TODO
  return true;
}

//------------------------------------------------------------------------------
bool pfcp_associations::remove_association(const int32_t& hash_node_id) {
  auto association = associations_graph.get_association(hash_node_id);
  return associations_graph.remove_association(association);
}

/******************************************************************************/
/***************************** UPF GRAPH
 * **************************************/
/******************************************************************************/

//------------------------------------------------------------------------------
void upf_graph::insert_into_graph(const std::shared_ptr<pfcp_association>& sa) {
  if (!sa->upf_profile_is_set) {
    Logger::smf_app().info(
        "Cannot connect UPF to other nodes in the graph as it has no "
        "profile, "
        "just add the node");
    Logger::smf_app().info("Assume that the UPF has a N3 and a N6 interface.");

    edge n3_edge        = {};
    n3_edge.type        = iface_type::N3;
    n3_edge.nw_instance = smf_cfg->get_nwi(sa->node_id, iface_type::N3);
    n3_edge.uplink      = false;
    edge n6_edge        = {};
    n6_edge.type        = iface_type::N6;
    n6_edge.nw_instance = smf_cfg->get_nwi(sa->node_id, iface_type::N6);
    n6_edge.uplink      = true;

    add_upf_graph_edge(sa, n3_edge);
    add_upf_graph_edge(sa, n6_edge);

  } else {
    std::vector<std::shared_ptr<pfcp_association>> all_upfs;
    std::vector<std::pair<edge, edge>> edges;

    std::vector<edge> n3_edges;
    std::vector<edge> n6_edges;
    // iterate through all interfaces and see if the FQDN/IPv4 addresses of
    // existing UPFs match
    std::unique_lock graph_lock(graph_mutex);

    // Find N9 interfaces
    for (const auto& it : adjacency_list) {
      edge src_dst;
      edge dst_src;
      bool found = sa->find_upf_edge(it.first, src_dst);
      if (found) {
        // now other direction
        found = it.first->find_upf_edge(sa, dst_src);
        if (found) {
          all_upfs.push_back(it.first);
          edges.emplace_back(src_dst, dst_src);

        } else {
          Logger::smf_app().warn(
              "Found edge from %s to %s, but not in the other direction. You "
              "have an error in your UPF configuration. This UPF is not "
              "added "
              "to the graph",
              sa->get_printable_name().c_str(),
              it.first->get_printable_name().c_str());
        }
      }
    }
    // unlock as here we need unique locks
    graph_lock.unlock();
    // Find N6 or N3 edges
    sa->find_n3_edge(n3_edges);
    sa->find_n6_edge(n6_edges);

    if (all_upfs.empty()) {
      Logger::smf_app().debug(
          "Could not find other edges for UPF, just add UPF as a node");
      add_upf_graph_node(sa);
    } else {
      for (int i = 0; i < all_upfs.size(); i++) {
        add_upf_graph_edge(sa, all_upfs[i], edges[i].first, edges[i].second);
      }
    }
    for (auto n3_edge : n3_edges) {
      add_upf_graph_edge(sa, n3_edge);
    }
    for (auto n6_edge : n6_edges) {
      add_upf_graph_edge(sa, n6_edge);
    }
  }
  print_graph();
}

// TODO can we get rid of this inefficient method?
//------------------------------------------------------------------------------
std::shared_ptr<pfcp_association> upf_graph::get_association(
    const std::size_t association_hash) const {
  std::shared_lock graph_lock(graph_mutex);

  for (const auto& it : adjacency_list) {
    if (it.first->hash_node_id == association_hash) {
      return it.first;
    }
  }
  return {};
}

// TODO can we get rid of this inefficient method?
//------------------------------------------------------------------------------
std::shared_ptr<pfcp_association> upf_graph::get_association(
    const pfcp::fseid_t& cp_fseid) const {
  std::shared_lock graph_lock(graph_mutex);

  for (const auto& it : adjacency_list) {
    if (it.first->has_session(cp_fseid)) {
      return it.first;
    }
  }
  return {};
}

// TODO can we get rid of this inefficient method?
//------------------------------------------------------------------------------
std::shared_ptr<pfcp_association> upf_graph::get_association_for_trxn_id(
    const uint64_t trxn_id) const {
  std::shared_lock graph_lock(graph_mutex);
  for (const auto& it : adjacency_list) {
    if (it.first->trxn_id_heartbeat == trxn_id) {
      return it.first;
    }
  }
  return {};
}

//------------------------------------------------------------------------------
bool upf_graph::remove_association(
    const std::shared_ptr<pfcp_association>& association) {
  std::unique_lock graph_lock(graph_mutex);

  auto it = adjacency_list.find(association);

  if (it != adjacency_list.end()) {
    std::shared_ptr<pfcp_association> to_delete_association = it->first;
    edge to_delete_edge;

    // go through all the other nodes and remove this association from
    // adjacency list
    for (auto other_it : adjacency_list) {
      auto edge_it = other_it.second.begin();

      while (edge_it != other_it.second.end()) {
        if (edge_it->association == it->first) {
          edge_it = other_it.second.erase(edge_it);
        } else {
          edge_it++;
        }
      }
      // TODO does this work?
    }
    adjacency_list.erase(association);
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------------------------
void upf_graph::add_upf_graph_edge(
    const std::shared_ptr<pfcp_association>& source, edge& edge_info_src_dst) {
  add_upf_graph_node(source);

  std::unique_lock lock_graph(graph_mutex);
  auto it_src = adjacency_list.find(source);
  if (it_src == adjacency_list.end()) {
    return;
  }
  bool exists = false;
  for (const auto& edge : it_src->second) {
    if (edge == edge_info_src_dst) {
      exists = true;
      break;
    }
  }

  if (!exists) {
    it_src->second.push_back((edge_info_src_dst));
    Logger::smf_app().debug(
        "Successfully added UPF graph edge for %s: %s",
        source->get_upf_node_profile().get_fqdn().c_str(),
        edge_info_src_dst.to_string().c_str());
  }
}

//------------------------------------------------------------------------------
void upf_graph::add_upf_graph_edge(
    const std::shared_ptr<pfcp_association>& source,
    const std::shared_ptr<pfcp_association>& dest, edge& edge_info_src_dst,
    edge& edge_info_dst_src) {
  edge_info_src_dst.association = dest;
  edge_info_dst_src.association = source;
  add_upf_graph_edge(source, edge_info_src_dst);
  add_upf_graph_edge(dest, edge_info_dst_src);
}

//------------------------------------------------------------------------------
void upf_graph::add_upf_graph_node(
    const std::shared_ptr<pfcp_association>& node) {
  std::unique_lock lock_graph(graph_mutex);

  auto it = adjacency_list.find(node);

  if (it == adjacency_list.end()) {
    std::vector<edge> lst = {};
    adjacency_list.insert(std::make_pair(node, lst));

    Logger::smf_app().debug(
        "Successfully added UPF node: %s, (%u)",
        node->get_upf_node_profile().get_fqdn().c_str(), node->hash_node_id);
  }
}

//------------------------------------------------------------------------------
void upf_graph::print_graph() {
  std::shared_lock lock_graph(graph_mutex);
  std::string output;

  if (adjacency_list.empty()) {
    Logger::smf_app().debug("UPF graph is empty");
    return;
  }

  for (const auto& it : adjacency_list) {
    output.append("* ").append(it.first->get_printable_name()).append(" --> ");
    for (const auto& edge : it.second) {
      output.append(edge.to_string()).append(", ");
    }
    output.append("\n");
  }

  Logger::smf_app().debug("UPF graph ");
  Logger::smf_app().debug("%s", output.c_str());
}

//------------------------------------------------------------------------------
void upf_graph::dfs_next_upf(
    std::vector<edge>& info_dl, std::vector<edge>& info_ul,
    std::shared_ptr<pfcp_association>& upf) {
  // we need unique lock as visited array and stack is written
  std::unique_lock lock_graph(graph_mutex);

  if (!stack_asynch.empty()) {
    std::shared_ptr<pfcp_association> association = stack_asynch.top();
    stack_asynch.pop();

    auto node_it = adjacency_list.find(association);
    if (node_it == adjacency_list.end()) {
      // TODO this scenario might happen when in the meantime one of the UPFs
      // became inavailable. here we should terminate the PDU session and all
      // the other already established sessions
      Logger::smf_app().error(
          "DFS Asynch: node ID does not exist in UPF graph, this should not "
          "happen");
      return;
    }

    // here we need to check if we have more than one unvisited N9_UL edge
    // if yes, we have a UL CL scenario, and we need to finish the other
    // branch first
    Logger::smf_app().debug(
        "DFS Asynch: Handle UPF %s",
        node_it->first->get_printable_name().c_str());
    bool unvisited_n9_node = false;
    // as we have no diamond shape, we only need to care about this in UL
    // direction
    if (uplink_asynch) {
      for (const auto& edge_it : node_it->second) {
        if (edge_it.association) {
          if (!visited_asynch[edge_it.association]) {
            if (edge_it.type == iface_type::N9 && edge_it.uplink) {
              unvisited_n9_node = true;
              Logger::smf_app().debug(
                  "UL CL scenario: We have a node with an unvisited N9_UL "
                  "node.");
              break;
            }
          }
        }
      }
    }
    // we removed the current UPF from stack, but did not set visited
    // so it is re-visited again from another branch
    // we also dont add the neighbors yet
    if (!unvisited_n9_node) {
      visited_asynch[association] = true;

      for (const auto& edge_it : node_it->second) {
        // first add all neighbors to the stack
        if (edge_it.association) {
          if (!visited_asynch[edge_it.association]) {
            stack_asynch.push(edge_it.association);
          }
        }
      }
    }
    upf = node_it->first;

    // set correct edge infos
    for (auto& edge_it : node_it->second) {
      // copy QOS Flow for the whole graph
      //  TODO currently only one flow is supported
      if (edge_it.qos_flows.empty()) {
        std::shared_ptr<smf_qos_flow> flow =
            std::make_shared<smf_qos_flow>(qos_flow_asynch);
        edge_it.qos_flows.emplace_back(flow);
      }
      // TODO thought for refactor: It would be much nicer if it would be the
      // same edge object so we dont have to do that

      // pointer is not null -> N9 interface
      if (edge_it.association) {
        // we add the TEID here for the edge in the other direction
        // direct access is safe as we know the edge exists
        auto edge_node = adjacency_list[edge_it.association];
        for (auto edge_edge : edge_node) {
          if (edge_edge.qos_flows.empty()) {
            edge_edge.qos_flows.emplace_back(
                std::make_shared<smf_qos_flow>(qos_flow_asynch));
          }

          if (edge_edge.association &&
              edge_edge.association == node_it->first) {
            if (edge_edge.type == iface_type::N9 && edge_edge.uplink) {
              // downlink direction
              edge_it.qos_flows[0]->dl_fteid = edge_edge.qos_flows[0]->dl_fteid;
            } else if (edge_edge.type == iface_type::N9) {
              edge_it.qos_flows[0]->ul_fteid = edge_edge.qos_flows[0]->ul_fteid;
            }
          }
        }
      }

      // set the correct edges to return
      if (edge_it.uplink) {
        info_ul.push_back(edge_it);
      } else {
        info_dl.push_back(edge_it);
      }
      current_upf_asynch      = upf;
      current_edges_dl_asynch = info_dl;
      current_edges_ul_asynch = info_ul;
    }
  }
}

//------------------------------------------------------------------------------
void upf_graph::dfs_current_upf(
    std::vector<edge>& info_dl, std::vector<edge>& info_ul,
    std::shared_ptr<pfcp_association>& upf) {
  std::shared_lock graph_lock(graph_mutex);
  upf     = current_upf_asynch;
  info_dl = current_edges_dl_asynch;
  info_ul = current_edges_ul_asynch;
}

//------------------------------------------------------------------------------
void upf_graph::start_asynch_dfs_procedure(
    bool uplink, smf_qos_flow& qos_flow) {
  std::unique_lock graph_lock(graph_mutex);
  if (!stack_asynch.empty()) {
    Logger::smf_app().error(
        "Started DFS procedure, but old stack is not empty. Failure");
  }
  // clear the stack and visited array
  stack_asynch    = {};
  visited_asynch  = {};
  qos_flow_asynch = qos_flow;
  uplink_asynch   = uplink;

  // uplink start at the exit nodes, downlink start at access nodes, do not
  // actually do DFS but put them on the stack
  for (auto& it : adjacency_list) {
    for (auto& edge : it.second) {
      if ((uplink && edge.type == iface_type::N6) ||
          (!uplink && edge.type == iface_type::N3)) {
        stack_asynch.push(it.first);
        break;
      }
    }
  }
}

//---------------------------------------------------------------------------------------------
edge upf_graph::get_access_edge() const {
  std::shared_lock graph_lock(graph_mutex);
  edge info;
  for (const auto& node_it : adjacency_list) {
    for (const auto& edge_it : node_it.second) {
      if (edge_it.type == iface_type::N3) {
        return edge_it;
      }
    }
  }
  Logger::smf_app().warn(
      "Try to get the access edge for this graph, but it does not exist");
  return info;
}

//---------------------------------------------------------------------------------------------
void upf_graph::update_edge_info(
    const std::shared_ptr<pfcp_association>& upf, const edge& info) {
  std::unique_lock graph_lock(graph_mutex);
  auto it = adjacency_list.find(upf);
  if (it == adjacency_list.end()) {
    Logger::smf_app().error("Want to update edge, but UPF does not exist!");
    return;
  }

  for (auto& edge_it : it->second) {
    if (edge_it == info) {
      edge_it = info;
      break;
    }
  }
}

//---------------------------------------------------------------------------------------------
std::shared_ptr<upf_graph> upf_graph::select_upf_node(
    const snssai_t& snssai, const std::string& dnn) {
  Logger::smf_app().info("Select UPF Node");
  std::shared_ptr<upf_graph> upf_graph_ptr = std::make_shared<upf_graph>();
  std::shared_lock graph_lock(graph_mutex);
  std::shared_ptr<pfcp_association> not_found = {};
  if (adjacency_list.empty()) {
    Logger::smf_app().warn("No UPF available");
  }
  // First, only consider UPFs with profile ID set
  for (const auto& it : adjacency_list) {
    std::shared_ptr<pfcp_association> current_upf = it.first;
    Logger::smf_app().debug("Current UPF info");
    current_upf->display();
    if (current_upf->is_upf_profile_set()) {
      upf_info_t upf_info           = {};
      std::vector<snssai_t> snssais = {};
      current_upf->get_upf_node_profile().get_upf_info(upf_info);
      bool has_access  = false;
      bool has_exit    = false;
      edge access_edge = {};
      edge exit_edge   = {};
      for (const auto& edge : it.second) {
        Logger::smf_app().debug("Verify Slice/DNN support");
        // verify that UPF belongs to the same slice and supports this dnn
        if (edge.serves_network(dnn, snssai)) {
          if (edge.type == iface_type::N3) {
            access_edge = edge;
            has_access  = true;
          } else if (edge.type == iface_type::N6) {
            exit_edge = edge;
            has_exit  = true;
          }
        }
      }
      if (has_access && has_exit) {
        Logger::smf_app().info(
            "Found UPF for this PDU session: %s",
            current_upf->get_printable_name().c_str());
        current_upf->get_upf_node_profile().display();
        upf_graph_ptr->add_upf_graph_node(current_upf);
        upf_graph_ptr->add_upf_graph_edge(current_upf, access_edge);
        upf_graph_ptr->add_upf_graph_edge(current_upf, exit_edge);
        return upf_graph_ptr;
      }
    }
  }
  // In case previous round did not produce anything, just return first UPF
  // without profile (and just assume it has N3/N6)
  for (const auto& it : adjacency_list) {
    if (!it.first->is_upf_profile_set()) {
      for (auto edge : it.second) {
        if (edge.type != iface_type::N9) {
          upf_graph_ptr->add_upf_graph_edge(it.first, edge);
        }
      }
    }
  }

  if (upf_graph_ptr->adjacency_list.empty()) {
    upf_graph_ptr.reset();
    Logger::smf_app().warn("UPF could not be selected for the PDU session");
  }

  return upf_graph_ptr;
}

//---------------------------------------------------------------------------------------------
std::shared_ptr<upf_graph> upf_graph::select_upf_node(
    const int node_selection_criteria) {
  std::shared_lock graph_lock(graph_mutex);
  std::shared_ptr<pfcp_association> association = {};
  std::shared_ptr<upf_graph> upf_graph_ptr      = std::make_shared<upf_graph>();

  Logger::smf_app().error("Called non-implemented UPF selection function");

  if (adjacency_list.empty()) {
    upf_graph_ptr.reset();
    return upf_graph_ptr;
  }

  for (const auto& it : adjacency_list) {
    association = it.first;
    // TODO
    switch (node_selection_criteria) {
      case NODE_SELECTION_CRITERIA_BEST_MAX_HEARBEAT_RTT:
      case NODE_SELECTION_CRITERIA_MIN_PFCP_SESSIONS:
      case NODE_SELECTION_CRITERIA_MIN_UP_TIME:
      case NODE_SELECTION_CRITERIA_MAX_AVAILABLE_BW:
      case NODE_SELECTION_CRITERIA_NONE:
      default:
        break;
    }
    // just add first node and then break
    upf_graph_ptr->add_upf_graph_node(association);
    for (auto edge : it.second) {
      if (edge.type != iface_type::N9) {
        upf_graph_ptr->add_upf_graph_edge(it.first, edge);
      }
    }
    return upf_graph_ptr;
  }
  return upf_graph_ptr;
}

//------------------------------------------------------------------------------
// TODO in the current implementation, UL CL needs to be the first node,
// otherwise it is not explored anymore when graph is merged
std::shared_ptr<upf_graph> upf_graph::select_upf_nodes(
    const SmPolicyDecision& policy_decision, const snssai_t& snssai,
    const std::string& dnn) {
  // TODO move this maybe
  std::unique_lock graph_lock(graph_mutex);

  if (!policy_decision.pccRulesIsSet() ||
      !policy_decision.traffContDecsIsSet()) {
    Logger::smf_app().warn(
        "Cannot build UPF graph for PDU session when pcc rules or traffic "
        "control description is missing");
  }

  std::map<std::string, PccRule> pcc_rules = policy_decision.getPccRules();
  std::map<std::string, TrafficControlData> traffic_conts =
      policy_decision.getTraffContDecs();
  std::unordered_set<uint32_t> precedences;

  // std::shared_ptr<upf_graph> correct_sub_graph_ptr;
  std::unordered_set<std::string> dnais_from_all_rules;
  std::shared_ptr<upf_graph> sub_graph_ptr = {};

  // run DFS for each PCC rule, get different graphs and merge them

  for (const auto& rule : pcc_rules) {
    std::unordered_set<std::string> dnais;
    pfcp::redirect_information_t redirect_information = {};
    if (!rule.second.getRefTcData().empty()) {
      // we just take the first traffic control, as defined in the standard
      // see Note 1 in table 5.6.2.6-1 in TS29.512
      std::string tc_data_id = rule.second.getRefTcData()[0];

      auto traffic_it = traffic_conts.find(tc_data_id);
      if (traffic_it != traffic_conts.end()) {
        TrafficControlData data = traffic_it->second;
        traffic_it->second.getTcId();
        if (traffic_it->second.routeToLocsIsSet()) {
          for (const auto& route : traffic_it->second.getRouteToLocs()) {
            dnais.insert(route.getDnai());
            dnais_from_all_rules.insert(route.getDnai());
          }
          if (traffic_it->second.redirectInfoIsSet()) {
            RedirectInformation redirectInfo =
                traffic_it->second.getRedirectInfo();
            if (redirectInfo.isRedirectEnabled()) {
              redirect_information.redirect_address_type = static_cast<uint8_t>(
                  redirectInfo.getRedirectAddressType().getEnumValue());
              redirect_information.redirect_server_address =
                  redirectInfo.getRedirectServerAddress();
              redirect_information.redirect_server_address_length =
                  redirect_information.redirect_server_address.size();
            }
          }
        } else {
          Logger::smf_app().warn("Route to location is not set in PCC rules");
        }
      }
    } else {
      continue;
    }
    std::string flow_description = rule.second.getFirstFlowDescription();
    if (flow_description.empty()) {
      Logger::smf_app().warn(
          "Flow Description is empty. Skip PCC rule %s", rule.first.c_str());
    }

    std::unordered_map<
        std::shared_ptr<pfcp_association>, bool,
        std::hash<std::shared_ptr<pfcp_association>>>
        visited;

    // here we start the DFS algorithm for all start nodes because we can
    // have disconnected graphs

    uint32_t precedence = rule.second.getPrecedence();
    if (auto it = precedences.find(precedence) != precedences.end()) {
      Logger::smf_app().warn(
          "UPF graph selection failed: The precedences in the PCC rule "
          "are not unique. Aborting selection.");
      return nullptr;
    }
    precedences.insert(precedence);

    for (const auto& node : adjacency_list) {
      if (!visited[node.first]) {
        bool has_n3 = false;
        for (const auto& edge : node.second) {
          if (edge.type == iface_type::N3) {
            has_n3 = true;
            break;
          }
        }

        if (has_n3) {
          // need to make a copy in case the algorithm adds nodes to the graph
          // and then the graph is wrong
          std::shared_ptr<upf_graph> sub_graph_copy_ptr;
          if (sub_graph_ptr) {
            sub_graph_copy_ptr = std::make_shared<upf_graph>(*sub_graph_ptr);
          } else {
            sub_graph_ptr = std::make_shared<upf_graph>();
          }
          set_dfs_selection_criteria(
              dnais, flow_description, precedence, snssai, dnn,
              redirect_information);

          create_subgraph_dfs(sub_graph_ptr, node.first, visited);
          // Verify the merged graph with all DNAIs so far
          sub_graph_ptr->set_dfs_selection_criteria(
              dnais_from_all_rules, flow_description, precedence, snssai, dnn,
              redirect_information);

          if (!sub_graph_ptr->verify()) {
            // in case copy is null, new subgraph_ptr is also null, and we
            // create a new upf graph
            sub_graph_ptr = sub_graph_copy_ptr;
          } else {
            // this PCC rule is covered
            break;
          }
        }
      }
    }
  }

  // Now we verify the merged graph
  if (sub_graph_ptr) {
    sub_graph_ptr->set_dfs_selection_criteria(
        dnais_from_all_rules, dfs_flow_description, dfs_precedence, snssai, dnn,
        dfs_redirect_information);
  }
  if (sub_graph_ptr && sub_graph_ptr->verify()) {
    Logger::smf_app().info("Dynamic UPF selection successful.");
    sub_graph_ptr->print_graph();

    // now fix the flow description in case of an UL CL
    for (auto node : sub_graph_ptr->adjacency_list) {
      // use default flow description when there is only one edge
      if (node.second.size() == 1) {
        node.second.front().flow_description = DEFAULT_FLOW_DESCRIPTION;
      }
    }
  } else {
    Logger::smf_app().info("Dynamic UPF selection failed");
    sub_graph_ptr.reset();
  }

  return sub_graph_ptr;
}

//---------------------------------------------------------------------------------------------
bool upf_graph::verify() {
  int access_count = 0;
  bool has_exit    = false;
  std::unordered_set<std::string> all_dnais_in_graph;
  for (const auto& node : adjacency_list) {
    for (const auto& edge : node.second) {
      if (edge.type == iface_type::N3) {
        access_count++;
      } else if (edge.type == iface_type::N6) {
        has_exit = true;
      }
      all_dnais_in_graph.insert(edge.used_dnai);
    }
  }

  if (access_count > 1) {
    Logger::smf_app().info(
        "UPF graph has more than one access node, this is not supported. "
        "Please check your PCC rules and UPF configuration");
    return false;
  }

  // special case, here we allow one DNAI less
  if (adjacency_list.size() == 1 &&
      all_dnais_in_graph.size() == dfs_all_dnais.size() - 1) {
    Logger::smf_app().info(
        "Found UPF graph that serves DNAIs %s",
        get_dnai_list(all_dnais_in_graph).c_str());
    return true;
  }

  if (all_dnais_in_graph.size() != dfs_all_dnais.size()) {
    Logger::smf_app().debug(
        "Found UPF graph that serves DNAIs %s, but not all DNAIs from rule "
        "are covered (%s)",
        get_dnai_list(all_dnais_in_graph).c_str(),
        get_dnai_list(dfs_all_dnais).c_str());
    print_graph();
    return false;
  }

  if (access_count == 1 && has_exit) {
    Logger::smf_app().info(
        "Found UPF graph that serves DNAIs %s",
        get_dnai_list(all_dnais_in_graph).c_str());
    return true;
  } else if (access_count == 0 && !has_exit) {
    Logger::smf_app().info(
        "UPF graph that serves all DNAIs %s cannot be used, because it has "
        "neither entry nor exit nodes",
        get_dnai_list(all_dnais_in_graph).c_str());
    print_graph();
  } else if (access_count != 1) {
    Logger::smf_app().info(
        "UPF graph that serves all DNAIs %s cannot be used, because it does "
        "not have an access node",
        get_dnai_list(all_dnais_in_graph).c_str());
    print_graph();
  } else if (!has_exit) {
    Logger::smf_app().info(
        "UPF graph that serves all DNAIs %s cannot be used, because it does "
        "not have an exit node",
        get_dnai_list(all_dnais_in_graph).c_str());
    print_graph();
  }
  return false;
}

//---------------------------------------------------------------------------------------------
std::string upf_graph::get_dnai_list(const std::unordered_set<string>& dnais) {
  std::string out = {};

  for (const auto& dnai : dnais) {
    out.append(dnai).append(", ");
  }
  if (dnais.size() > 1) {
    out.erase(out.size() - 2);
  }
  return out;
}

//---------------------------------------------------------------------------------------------
void upf_graph::set_dfs_selection_criteria(
    const std::unordered_set<std::string>& all_dnais,
    const std::string& flow_description, uint32_t precedence,
    const snssai_t& snssai, const std::string& dnn,
    const pfcp::redirect_information_t& redirect_information) {
  dfs_all_dnais            = all_dnais;
  dfs_flow_description     = flow_description;
  dfs_precedence           = precedence;
  dfs_snssai               = snssai;
  dfs_dnn                  = dnn;
  dfs_redirect_information = redirect_information;
}

//---------------------------------------------------------------------------------------------
void upf_graph::create_subgraph_dfs(
    std::shared_ptr<upf_graph>& sub_graph,
    const std::shared_ptr<pfcp_association>& start_node,
    std::unordered_map<
        std::shared_ptr<pfcp_association>, bool,
        std::hash<std::shared_ptr<pfcp_association>>>& visited) {
  std::stack<std::shared_ptr<pfcp_association>> stack;
  stack.push(start_node);

  while (!stack.empty()) {
    std::shared_ptr<pfcp_association> node = stack.top();
    stack.pop();
    visited[node] = true;

    auto node_it = adjacency_list.find(node);
    if (node_it == adjacency_list.end()) {
      Logger::smf_app().error(
          "DFS: node ID does not exist in UPF graph, this should not happen");
      continue;
    }

    if (!node_it->first->is_upf_profile_set()) {
      Logger::smf_app().debug(
          "UPF without profile information is not considered in DFS");
      continue;
    }

    // DFS: Go through all edges and check if the UPF serves one of the DNAIs
    // from the PCC rule
    for (auto edge_it : node_it->second) {
      std::string found_dnai;
      if (!edge_it.serves_network(
              dfs_dnn, dfs_snssai, dfs_all_dnais, found_dnai)) {
        continue;  // do not consider this edge, does not serve DNN or SNSSAI or
                   // any DNAI
      }
      edge_it.used_dnai = found_dnai;

      edge_it.flow_description = dfs_flow_description;

      edge_it.redirect_information = dfs_redirect_information;
      // TODO move this precedence to the QOS FLOW?
      // refactor the way qos_flow is handled in general
      edge_it.precedence = dfs_precedence;

      // N3 or N6 edge, just add
      if (!edge_it.association) {
        if (edge_it.type == iface_type::N3) {
          edge_it.uplink = false;
        } else if (edge_it.type == iface_type::N6) {
          edge_it.uplink = true;
        }
        sub_graph->add_upf_graph_edge(node_it->first, edge_it);
      } else if (!visited[edge_it.association]) {
        std::string edge_upf = edge_it.association->get_printable_name();
        edge src_dst         = edge_it;
        src_dst.uplink       = true;  // N9 uplink as we start at access
        edge dst_src;

        // for the other direction we need to find this element in the
        // graph and find the original node; direct access is safe as we know
        // this element exists in graph as we have double lists

        // Note: We could also remove this step as this node is evaluated
        // anyway but then we need to somehow track the visited
        //  This is O(#edges_per_upf) so quite small
        auto edge_node = adjacency_list[edge_it.association];
        for (const auto& edge_edge : edge_node) {
          if (edge_edge.association == node_it->first) {
            dst_src        = edge_edge;
            dst_src.uplink = false;
            std::string used_dnai;
            if (!edge_edge.serves_network(
                    dfs_dnn, dfs_snssai, dfs_all_dnais, used_dnai)) {
              Logger::smf_app().error(
                  "Back-Edge in DFS does not serve network. check your "
                  "configuration");
              break;
            }
            dst_src.used_dnai = used_dnai;
          }
        }
        sub_graph->add_upf_graph_edge(
            node_it->first, edge_it.association, src_dst, dst_src);
        stack.push(edge_it.association);
      }
    }
  }
}

//---------------------------------------------------------------------------------------------
bool upf_graph::full() const {
  std::shared_lock graph_lock(graph_mutex);

  return adjacency_list.size() >= PFCP_MAX_ASSOCIATIONS;
}

std::string upf_graph::to_string(const std::string& indent) const {
  std::shared_lock graph_lock(graph_mutex);

  for (const auto& node : adjacency_list) {
    for (const auto& edge : node.second) {
      if (edge.type == iface_type::N3) {
        return to_string_from_start_node(indent, node.first);
      }
    }
  }

  return indent + "Invalid graph";
}

std::string upf_graph::to_string_from_start_node(
    const std::string& indent,
    const std::shared_ptr<pfcp_association>& start) const {
  std::shared_lock graph_lock(graph_mutex);

  std::unordered_map<
      std::shared_ptr<pfcp_association>, bool,
      std::hash<std::shared_ptr<pfcp_association>>>
      visited;
  std::list<std::shared_ptr<pfcp_association>> queue;

  std::string output;

  auto node_it = adjacency_list.find(start);
  if (node_it == adjacency_list.end()) {
    return output.append(indent)
        .append("Node ")
        .append(start->get_printable_name())
        .append(" does not exist in UPF graph.");
  }

  visited[start] = true;
  queue.push_back(start);
  std::map<std::string, std::string> output_per_iface_type;

  while (!queue.empty()) {
    auto node_queue = queue.front();
    node_it         = adjacency_list.find(node_queue);
    if (node_it == adjacency_list.end()) continue;

    queue.pop_front();

    for (const auto& edge : node_it->second) {
      // skip N6 output
      if (edge.type == iface_type::N6) continue;

      std::string iface = pfcp_association::string_from_iface_type(edge.type);
      if (!edge.nw_instance.empty()) {
        iface.append(": ").append(edge.nw_instance);
      }

      for (const auto& flow : edge.qos_flows) {
        // N3
        if (!edge.association) {
          output_per_iface_type[iface].append(flow->toString(indent + "\t"));
        }
        // N9
        else if (!visited[edge.association]) {
          output_per_iface_type[iface].append(flow->toString(indent + "\t"));
          visited[edge.association] = true;
          queue.push_back(edge.association);
        }
      }
    }
  }

  for (const auto& o : output_per_iface_type) {
    output.append(indent).append(o.first).append(":\n");
    output.append(o.second);
  }

  return output;
}
