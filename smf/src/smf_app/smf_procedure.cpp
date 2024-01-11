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

/*! \file smf_procedure.cpp
 \author  Lionel GAUTHIER, Tien-Thinh NGUYEN
 \company Eurecom
 \date 2019
 \email: lionel.gauthier@eurecom.fr, tien-thinh.nguyen@eurecom.fr
 */

#include "smf_procedure.hpp"

#include <algorithm>  // std::search

#include "3gpp_29.244.h"
#include "3gpp_29.500.h"
#include "3gpp_29.502.h"
#include "3gpp_conversions.hpp"
#include "common_defs.h"
#include "conversions.hpp"
#include "itti.hpp"
#include "itti_msg_n4_restore.hpp"
#include "logger.hpp"
#include "smf_app.hpp"
#include "smf_config.hpp"
#include "smf_context.hpp"
#include "smf_pfcp_association.hpp"
#include "ProblemDetails.h"
#include "3gpp_24.501.h"

using namespace pfcp;
using namespace smf;
using namespace std;

extern itti_mw* itti_inst;
extern smf::smf_app* smf_app_inst;
extern std::unique_ptr<oai::config::smf::smf_config> smf_cfg;

pfcp::ue_ip_address_t smf_session_procedure::pfcp_ue_ip_address(
    const edge& edge) {
  // only used in PDR,so when it is an downlink edge, we are in UL procedure
  pfcp::ue_ip_address_t ue_ip;
  if (edge.uplink) {
    ue_ip.sd = 1;
  } else {
    ue_ip.sd = 0;
  }
  if (sps->ipv4) {
    ue_ip.v4           = 1;
    ue_ip.ipv4_address = sps->ipv4_address;
  }
  // TODO malformed PFCP message, should be fixed in PFCP layer, but we dont
  // need it for now
  if (sps->ipv6) {
    ue_ip.v6 = 0;
    // ue_ip.ipv6_address = sps->ipv6_address;
  }
  return ue_ip;
}

//------------------------------------------------------------------------------
pfcp::fteid_t smf_session_procedure::pfcp_prepare_fteid(
    pfcp::fteid_t& fteid, const bool& ftup_supported) {
  pfcp::fteid_t local_fteid;
  if (!ftup_supported) {
    Logger::smf_app().info(
        "Generating N3-UL TEID since current UPF does not support TEID "
        "Creation");
    local_fteid.ch           = 0;
    local_fteid.v4           = 1;
    local_fteid.chid         = 0;
    local_fteid.ipv4_address = conv::fromString(smf_cfg->local_n3_addr);
    sps->generate_teid(local_fteid);
    fteid = local_fteid;
    Logger::smf_app().info(
        "    UL F-TEID 0x%" PRIx32 " allocated for N3 IPv4 Addr : %s",
        local_fteid.teid, conv::toString(local_fteid.ipv4_address).c_str());
  } else if (fteid.is_zero()) {
    local_fteid.ch   = 1;
    local_fteid.v4   = 1;
    local_fteid.chid = 1;
    // same choose ID, indicates that same TEID should be generated for
    // more than one PDR
    local_fteid.choose_id = 42;
  } else {
    local_fteid = fteid;
  }
  return local_fteid;
}

//------------------------------------------------------------------------------
pfcp::create_far smf_session_procedure::pfcp_create_far(
    edge& edge, const qfi_t& qfi) {
  // When we have a FAR and edge is uplink we know we are in an uplink procedure
  //  e.g. FAR from N3 to N6, N6 is uplink edge -> uplink procedure

  pfcp::create_far create_far                       = {};
  pfcp::apply_action_t apply_action                 = {};
  pfcp::forwarding_parameters forwarding_parameters = {};

  // forwarding_parameters IEs
  pfcp::destination_interface_t destination_interface = {};

  apply_action.forw = 1;  // forward the packets

  pfcp::far_id_t far_id;

  auto flow = edge.get_qos_flow(qfi);
  if (!flow) {
    Logger::smf_app().error("Could not find QOS flow for this QFI.");
    return create_far;
  }

  if (edge.uplink) {
    // ACCESS is for downlink, CORE for uplink
    destination_interface.interface_value = pfcp::INTERFACE_VALUE_CORE;
    if (flow->far_id_ul.second == 0) {
      sps->generate_far_id(flow->far_id_ul.second);
      flow->far_id_ul.first = true;
    }
    far_id = flow->far_id_ul.second;

  } else {
    destination_interface.interface_value = pfcp::INTERFACE_VALUE_ACCESS;
    if (flow->far_id_dl.second == 0) {
      sps->generate_far_id((flow->far_id_dl.second));
      flow->far_id_dl.first = true;
    }
    far_id = flow->far_id_dl.second;
    if (smf_cfg->enable_dl_pdr_in_pfcp_sess_estab) {
      apply_action.forw = 0;
      apply_action.drop = 1;
      create_far.set(flow->far_id_dl.second);
      create_far.set(apply_action);
      return create_far;
    }
  }

  forwarding_parameters.set(destination_interface);

  //-------------------
  // Network Instance
  //-------------------
  if (!edge.nw_instance.empty()) {
    pfcp::network_instance_t network_instance = {};
    network_instance.network_instance         = edge.nw_instance;
    forwarding_parameters.set(network_instance);
  }

  if (edge.uplink && edge.redirect_information.redirect_address_type ==
                         pfcp::redirect_address_type_e::URL)
    forwarding_parameters.set(edge.redirect_information);

  if (edge.type != iface_type::N6) {
    pfcp::outer_header_creation_t outer_header_creation = {};
    outer_header_creation.outer_header_creation_description =
        OUTER_HEADER_CREATION_GTPU_UDP_IPV4;

    if (edge.uplink) {
      outer_header_creation.teid = flow->ul_fteid.teid;
      outer_header_creation.ipv4_address.s_addr =
          flow->ul_fteid.ipv4_address.s_addr;
    } else {
      outer_header_creation.teid = flow->dl_fteid.teid;
      outer_header_creation.ipv4_address.s_addr =
          flow->dl_fteid.ipv4_address.s_addr;
    }
    forwarding_parameters.set(outer_header_creation);
  }

  create_far.set(far_id);
  create_far.set(apply_action);
  create_far.set(
      forwarding_parameters);  // should check since destination
                               // interface is directly set to FAR (as
                               // described in Table 5.8.2.11.6-1)
  return create_far;
}

void smf_session_procedure::synch_ul_dl_edges(
    const vector<edge>& dl_edges, const vector<edge>& ul_edges,
    const qfi_t& qfi, bool synch_pdr_for_uplink) {
  // O(n2), but always very small elements in arrays (<10)
  for (auto dle : dl_edges) {
    auto dle_flow = dle.get_qos_flow(qfi);
    if (!dle_flow) continue;

    for (auto ule : ul_edges) {
      auto ule_flow = ule.get_qos_flow(qfi);
      if (!ule_flow) continue;

      if (ule_flow->far_id_ul.first) {
        dle_flow->far_id_ul = ule_flow->far_id_ul;
      }
      if (dle_flow->far_id_dl.first) {
        ule_flow->far_id_dl = dle_flow->far_id_dl;
      }
      if (synch_pdr_for_uplink && dle_flow->pdr_id_dl.rule_id != 0) {
        ule_flow->pdr_id_dl = dle_flow->pdr_id_dl;
      }
      if (synch_pdr_for_uplink && dle_flow->pdr_id_ul.rule_id != 0) {
        ule_flow->pdr_id_ul = dle_flow->pdr_id_ul;
      }
      if (ule_flow->pdr_id_ul.rule_id != 0) {
        dle_flow->pdr_id_ul = ule_flow->pdr_id_ul;
      }
      if (ule_flow->pdr_id_dl.rule_id != 0) {
        dle_flow->pdr_id_dl = ule_flow->pdr_id_dl;
      }
      if (dle_flow->urr_id.urr_id != 0) {
        ule_flow->urr_id = dle_flow->urr_id;
      }
      if (!dle_flow->ul_fteid.is_zero()) {
        ule_flow->ul_fteid = dle_flow->ul_fteid;
      }
    }
  }
}

//------------------------------------------------------------------------------
pfcp::create_pdr smf_session_procedure::pfcp_create_pdr(
    edge& edge, const qfi_t& qfi,
    const pfcp::up_function_features_s up_features) {
  // When we have a PDR and edge is uplink we know we are in a downlink
  // procedure, e.g. PDR from N6 to N3 -> N6 is uplink edge, so downlink
  // procedure

  //-------------------
  // IE create_pdr (section 5.8.2.11.3@TS 23.501)
  //-------------------
  pfcp::create_pdr create_pdr   = {};
  pfcp::precedence_t precedence = {};
  pfcp::pdi pdi                 = {};  // packet detection information
  pfcp::outer_header_removal_t outer_header_removal = {};
  // pdi IEs
  pfcp::source_interface_t source_interface          = {};
  pfcp::fteid_t local_fteid                          = {};
  pfcp::sdf_filter_t sdf_filter                      = {};
  pfcp::application_id_t application_id              = {};
  pfcp::_3gpp_interface_type_t source_interface_type = {};

  // get the default QoS profile
  std::shared_ptr<session_management_subscription> ss = {};
  // Default QoS Flow - Non GBR, qfi = 5qi

  auto flow = edge.get_qos_flow(qfi);

  if (!flow) {
    Logger::smf_app().error("Could not find QOS flow for this QFI.");
    return create_pdr;
  }

  pfcp::pdr_id_t pdr_id;
  pfcp::far_id_t far_id;

  // Packet detection information (see Table 7.5.2.2-2: PDI IE within PFCP
  // Session Establishment Request, 3GPP TS 29.244 V16.0.0)  source interface
  if (edge.uplink) {
    source_interface.interface_value = pfcp::INTERFACE_VALUE_CORE;
    if (flow->pdr_id_dl.rule_id == 0) {
      sps->generate_pdr_id(flow->pdr_id_dl);
    }
    pdr_id = flow->pdr_id_dl;
    far_id = flow->far_id_dl.second;
  } else {
    source_interface.interface_value = pfcp::INTERFACE_VALUE_ACCESS;
    if (flow->pdr_id_ul.rule_id == 0) {
      sps->generate_pdr_id(flow->pdr_id_ul);
    }
    pdr_id = flow->pdr_id_ul;
    far_id = flow->far_id_ul.second;
  }

  Logger::smf_app().debug("Created PDR ID, rule ID %d", pdr_id.rule_id);
  pdi.set(source_interface);

  //-------------------
  // Network Instance for Forward Action
  //-------------------
  if (!edge.nw_instance.empty()) {
    pfcp::network_instance_t network_instance = {};
    network_instance.network_instance         = edge.nw_instance;
    pdi.set(network_instance);
  }
  if (edge.type != iface_type::N6) {
    // in UPLINK always choose ID
    if (edge.uplink) {
      local_fteid      = pfcp_prepare_fteid(flow->dl_fteid, up_features.ftup);
      local_fteid.chid = 0;
    } else {
      local_fteid      = pfcp_prepare_fteid(flow->ul_fteid, up_features.ftup);
      std::string ipv4 = conv::toString(edge.ip_addr);
      if (!ipv4.empty() && !up_features.ftup) {
        local_fteid.ipv4_address    = edge.ip_addr;
        flow->dl_fteid.ipv4_address = edge.ip_addr;
      }
    }
    pdi.set(local_fteid);
  }

  // UE IP address
  pdi.set(pfcp_ue_ip_address(edge));

  if (edge.type == iface_type::N3) {
    source_interface_type.interface_type_value = pfcp::_3GPP_INTERFACE_TYPE_N3;
  } else if (edge.type == iface_type::N9) {
    source_interface_type.interface_type_value = pfcp::_3GPP_INTERFACE_TYPE_N9;
  }
  // do not remove outer header in dl direction
  if (edge.type != iface_type::N6) {
    outer_header_removal.outer_header_removal_description =
        OUTER_HEADER_REMOVAL_GTPU_UDP_IPV4;
    create_pdr.set(outer_header_removal);
    pdi.set(flow->qfi);  // QFI - QoS Flow ID
  }
  // TODO: Traffic Endpoint ID
  // TODO: Application ID
  // TODO: Ethernet PDU Session Information
  // TODO: Ethernet Packet Filter
  // TODO: Framed Route Information
  // TODO: Framed-Routing
  // TODO: Framed-IPv6-Route

  if (!edge.uplink && !edge.flow_description.empty()) {
    sdf_filter.fd               = 1;
    sdf_filter.flow_description = edge.flow_description;
    pdi.set(sdf_filter);
  }

  pdi.set(source_interface_type);

  // Here we take the precedence directly from the PCC rules. It should be okay
  // because both values are integer, but we might need to provide another
  // mapping
  precedence.precedence = edge.precedence;

  create_pdr.set(pdr_id);
  create_pdr.set(precedence);
  create_pdr.set(pdi);

  create_pdr.set(far_id);

  if (smf_cfg->enable_ur) {
    create_pdr.set(flow->urr_id);
  }

  return create_pdr;
}
//------------------------------------------------------------------------------
pfcp::create_pdr smf_session_procedure::pfcp_create_pdr_dl(
    edge& edge, const qfi_t& qfi) {
  //-------------------
  // IE create_pdr (section 5.8.2.11.3@TS 23.501)
  //-------------------
  pfcp::create_pdr create_pdr   = {};
  pfcp::precedence_t precedence = {};
  pfcp::pdi pdi                 = {};  // packet detection information
  pfcp::outer_header_removal_t outer_header_removal = {};
  // pdi IEs
  pfcp::source_interface_t source_interface          = {};
  pfcp::sdf_filter_t sdf_filter                      = {};
  pfcp::application_id_t application_id              = {};
  pfcp::_3gpp_interface_type_t source_interface_type = {};
  // pfcp::pdr_id_t pdr_id;
  pfcp::far_id_t far_id;

  auto flow = edge.get_qos_flow(qfi);

  if (!flow) {
    Logger::smf_app().error("Could not find QOS flow for this QFI.");
    return create_pdr;
  }
  far_id = flow->far_id_dl.second;

  sps->generate_pdr_id(flow->pdr_id_dl);

  source_interface.interface_value = pfcp::INTERFACE_VALUE_CORE;
  pdi.set(source_interface);
  pfcp::ue_ip_address_t ue_ip_addr = pfcp_ue_ip_address(edge);
  ue_ip_addr.sd                    = 1;
  pdi.set(ue_ip_addr);

  source_interface_type.interface_type_value = pfcp::_3GPP_INTERFACE_TYPE_N3;
  pdi.set(source_interface_type);

  precedence.precedence = 0;

  create_pdr.set(flow->pdr_id_dl);
  create_pdr.set(precedence);
  create_pdr.set(pdi);
  create_pdr.set(far_id);

  return create_pdr;
}
//------------------------------------------------------------------------------
pfcp::create_urr smf_session_procedure::pfcp_create_urr(
    edge& edge, const qfi_t& qfi) {
  auto flow = edge.get_qos_flow(qfi);

  sps->generate_urr_id(flow->urr_id);
  pfcp::create_urr create_urr                   = {};
  pfcp::measurement_method_t measurement_method = {};
  pfcp::measurement_period_t measurement_Period = {};
  pfcp::reporting_triggers_t reporting_triggers = {};
  pfcp::volume_threshold_t volume_threshold     = {};
  pfcp::time_threshold_t time_threshold         = {};

  if (!flow) {
    Logger::smf_app().error("Could not find QOS flow for this QFI.");
    return create_urr;
  }

  // Hardcoded values for the moment
  measurement_method.volum              = 1;  // Volume based usage report
  measurement_method.durat              = 1;
  measurement_Period.measurement_period = 10;  // Every 10 Sec
  reporting_triggers.perio              = 1;   // Periodic usage report
  reporting_triggers.volth              = 1;
  reporting_triggers.timth              = 1;
  reporting_triggers.volqu              = 0;
  reporting_triggers.timqu              = 0;

  volume_threshold.dlvol           = 1;
  volume_threshold.ulvol           = 0;
  volume_threshold.tovol           = 0;
  volume_threshold.downlink_volume = 1000;

  time_threshold.time_threshold = 5;

  create_urr.set(flow->urr_id);
  create_urr.set(measurement_method);
  create_urr.set(measurement_Period);
  create_urr.set(reporting_triggers);
  create_urr.set(time_threshold);
  create_urr.set(volume_threshold);

  return create_urr;
}

//------------------------------------------------------------------------------
smf_procedure_code smf_session_procedure::get_current_upf(
    std::vector<edge>& dl_edges, std::vector<edge>& ul_edges,
    std::shared_ptr<pfcp_association>& current_upf) {
  std::shared_ptr<upf_graph> graph = sps->get_sessions_graph();
  if (!graph) {
    Logger::smf_app().warn("UPF graph does not exist. Abort PFCP procedure");
    return smf_procedure_code::ERROR;
  }

  graph->dfs_current_upf(dl_edges, ul_edges, current_upf);

  if (!current_upf || ul_edges.empty() || dl_edges.empty()) {
    Logger::smf_app().warn("UPF selection failed!");
    return smf_procedure_code::ERROR;
  }
  return smf_procedure_code::OK;
}

//------------------------------------------------------------------------------
smf_procedure_code smf_session_procedure::get_next_upf(
    std::vector<edge>& dl_edges, std::vector<edge>& ul_edges,
    std::shared_ptr<pfcp_association>& next_upf) {
  std::shared_ptr<upf_graph> graph = sps->get_sessions_graph();
  if (!graph) {
    Logger::smf_app().warn("UPF graph does not exist. Abort PFCP procedure");
    return smf_procedure_code::ERROR;
  }

  graph->dfs_next_upf(dl_edges, ul_edges, next_upf);
  if (!next_upf) {
    Logger::smf_app().debug("UPF graph in SMF finished");
    return smf_procedure_code::OK;
  }

  if (dl_edges.empty() || ul_edges.empty()) {
    Logger::smf_app().warn("UPF selection failed!");
    return smf_procedure_code::ERROR;
  }

  return smf_procedure_code::CONTINUE;
}

//------------------------------------------------------------------------------
int n4_session_restore_procedure::run() {
  if (pending_sessions.size()) {
    itti_n4_restore* itti_msg = nullptr;
    for (std::set<pfcp::fseid_t>::iterator it = pending_sessions.begin();
         it != pending_sessions.end(); ++it) {
      if (!itti_msg) {
        itti_msg = new itti_n4_restore(TASK_SMF_N4, TASK_SMF_APP);
      }
      itti_msg->sessions.insert(*it);
      if (itti_msg->sessions.size() >= 64) {
        std::shared_ptr<itti_n4_restore> i =
            std::shared_ptr<itti_n4_restore>(itti_msg);
        int ret = itti_inst->send_msg(i);
        if (RETURNok != ret) {
          Logger::smf_n4().error(
              "Could not send ITTI message %s to task TASK_SMF_APP",
              i->get_msg_name());
        }
        itti_msg = nullptr;
      }
    }
    if (itti_msg) {
      std::shared_ptr<itti_n4_restore> i =
          std::shared_ptr<itti_n4_restore>(itti_msg);
      int ret = itti_inst->send_msg(i);
      if (RETURNok != ret) {
        Logger::smf_n4().error(
            "Could not send ITTI message %s to task TASK_SMF_APP",
            i->get_msg_name());
        return RETURNerror;
      }
    }
  }
  return RETURNok;
}

//------------------------------------------------------------------------------
smf_procedure_code
session_create_sm_context_procedure::send_n4_session_establishment_request() {
  std::shared_ptr<pfcp_association> current_upf;
  std::vector<edge> dl_edges;
  std::vector<edge> ul_edges;
  smf_procedure_code res = get_current_upf(dl_edges, ul_edges, current_upf);
  if (res != smf_procedure_code::OK) {
    return res;
  }

  n4_triggered = std::make_shared<itti_n4_session_establishment_request>(
      TASK_SMF_APP, TASK_SMF_N4);
  n4_triggered->seid    = 0;
  n4_triggered->trxn_id = this->trxn_id;
  n4_triggered->r_endpoint =
      endpoint(current_upf->node_id.u1.ipv4_address, pfcp::default_port);

  //-------------------
  // IE node_id_t
  //-------------------
  pfcp::node_id_t node_id = {};
  smf_cfg->get_pfcp_node_id(node_id);
  n4_triggered->pfcp_ies.set(node_id);

  //-------------------
  // IE fseid_t
  //-------------------
  pfcp::fseid_t cp_fseid = {};
  smf_cfg->get_pfcp_fseid(cp_fseid);
  cp_fseid.seid      = sps->seid;
  n4_triggered->seid = sps->seid;
  n4_triggered->pfcp_ies.set(cp_fseid);

  edge dl_edge = dl_edges[0];

  //-------------------
  // IE CREATE_URR ( Usage Reporting Rules)
  //-------------------
  if (smf_cfg->enable_ur) {
    pfcp::create_urr create_urr = pfcp_create_urr(dl_edge, current_flow.qfi);
    n4_triggered->pfcp_ies.set(create_urr);

    // set URR ID also for other edge
    synch_ul_dl_edges(dl_edges, ul_edges, current_flow.qfi, false);
  }

  // Here, we only consider an UL CL, so we create PDRs based on how many
  // Uplink edges we have, DL edge is fixed to first one

  // In UL CL case, we have 2 PDRs, but they have the same choose ID -> same
  // TEID We also set the same URR ID for all edges
  for (auto ul_edge : ul_edges) {
    //-------------------
    // IE CREATE_FAR
    //-------------------
    pfcp::create_far create_far = pfcp_create_far(ul_edge, current_flow.qfi);
    // copy created FAR ID to DL edge for PDR
    synch_ul_dl_edges(dl_edges, ul_edges, current_flow.qfi, false);

    // copy values from UL edge, so we simulate two downlink edges for PFCP
    auto flow                = dl_edge.get_qos_flow(current_flow.qfi);
    flow->pdr_id_ul          = 0;
    dl_edge.flow_description = ul_edge.flow_description;
    dl_edge.precedence       = ul_edge.precedence;

    //-------------------
    // IE CREATE_PDR
    //-------------------
    pfcp::create_pdr create_pdr = pfcp_create_pdr(
        dl_edge, current_flow.qfi, current_upf->function_features.second);
    synch_ul_dl_edges(dl_edges, ul_edges, current_flow.qfi, false);

    // ADD IEs to message
    //-------------------
    n4_triggered->pfcp_ies.set(create_pdr);
    n4_triggered->pfcp_ies.set(create_far);

    if (smf_cfg->enable_dl_pdr_in_pfcp_sess_estab) {
      pfcp::create_far create_far_dl =
          pfcp_create_far(dl_edge, current_flow.qfi);
      pfcp::create_pdr create_pdr_dl =
          pfcp_create_pdr_dl(dl_edge, current_flow.qfi);
      n4_triggered->pfcp_ies.set(create_pdr_dl);
      n4_triggered->pfcp_ies.set(create_far_dl);
    }
    // Handle PDR and FAR for downlink if thid feature is enabled
    if (smf_cfg->enable_dl_pdr_in_pfcp_sess_estab) {
      Logger::smf_app().info("Adding DL PDR and FAR start");
    }
  }

  // TODO: verify whether N4 SessionID should be included in PDR and FAR
  // (Section 5.8.2.11@3GPP TS 23.501)

  Logger::smf_app().info(
      "Sending ITTI message %s to task TASK_SMF_N4",
      n4_triggered->get_msg_name());
  int ret = itti_inst->send_msg(n4_triggered);
  if (RETURNok != ret) {
    Logger::smf_app().error(
        "Could not send ITTI message %s to task TASK_SMF_N4",
        n4_triggered->get_msg_name());
    return smf_procedure_code::ERROR;
  }
  return smf_procedure_code::CONTINUE;
}

//------------------------------------------------------------------------------
smf_procedure_code session_create_sm_context_procedure::run(
    std::shared_ptr<itti_n11_create_sm_context_request> sm_context_req,
    std::shared_ptr<itti_n11_create_sm_context_response> sm_context_resp,
    std::shared_ptr<smf::smf_context> sc) {
  Logger::smf_app().info("Perform a procedure - Create SM Context Request");
  // TODO check if compatible with ongoing procedures if any
  snssai_t snssai                  = sm_context_req->req.get_snssai();
  std::string dnn                  = sm_context_req->req.get_dnn();
  pdu_session_id_t pdu_session_id  = sm_context_req->req.get_pdu_session_id();
  std::shared_ptr<upf_graph> graph = {};

  // Find PDU session
  std::shared_ptr<smf_context_ref> scf = {};
  if (smf_app_inst->is_scid_2_smf_context(sm_context_req->scid)) {
    scf = smf_app_inst->scid_2_smf_context(sm_context_req->scid);
    // scf.get()->upf_node_id = up_node_id;
    std::shared_ptr<smf_pdu_session> sp = {};
    if (!sc.get()->find_pdu_session(scf.get()->pdu_session_id, sp)) {
      Logger::smf_app().warn("PDU session context does not exist!");
      sm_context_resp->res.set_cause(
          PDU_SESSION_APPLICATION_ERROR_CONTEXT_NOT_FOUND);
      return smf_procedure_code::ERROR;
    }

    if (sp->policy_ptr) {
      graph = pfcp_associations::get_instance().select_up_node(
          sp->policy_ptr->decision, snssai, dnn);
      if (!graph) {
        Logger::smf_app().warn(
            "UPF selection based on PCC rules failed. Use any UPF.");
      }
    }
    if (!graph) {
      // No policies found or graph selection failed, use default UPF selection
      graph = pfcp_associations::get_instance().select_up_node(snssai, dnn);
    }
    // if still there is no graph, send an error
    if (!graph) {
      // TODO better error code?
      sm_context_resp->res.set_cause(
          PDU_SESSION_APPLICATION_ERROR_PEER_NOT_RESPONDING);
      return smf_procedure_code::ERROR;
    } else {
      sp->set_sessions_graph(graph);
    }
  } else {
    Logger::smf_app().warn(
        "SM Context associated with this id " SCID_FMT " does not exit!",
        sm_context_req->scid);
    // TODO:
  }

  //-------------------
  n11_trigger           = sm_context_req;
  n11_triggered_pending = sm_context_resp;
  uint64_t seid         = smf_app_inst->generate_seid();
  sps->set_seid(seid);
  // for finding procedure when receiving response
  smf_app_inst->set_seid_2_smf_context(seid, sc);

  // get the default QoS profile
  subscribed_default_qos_t default_qos                = {};
  std::shared_ptr<session_management_subscription> ss = {};
  sc->get_default_qos(
      sm_context_req->req.get_snssai(), sm_context_req->req.get_dnn(),
      default_qos);

  // Create default QoS (Non-GBR) and associate far id and pdr id to this flow
  smf_qos_flow flow   = {};
  flow.pdu_session_id = sm_context_req->req.get_pdu_session_id();
  // default QoS profile
  flow.qfi.qfi                    = default_qos._5qi;
  flow.qos_profile._5qi           = default_qos._5qi;
  flow.qos_profile.arp            = default_qos.arp;
  flow.qos_profile.priority_level = default_qos.priority_level;

  // assign default QoS rule for this flow
  QOSRulesIE qos_rule = {};
  sc->get_default_qos_rule(
      qos_rule, sm_context_req->req.get_pdu_session_type());
  uint8_t rule_id = {0};
  sps->generate_qos_rule_id(rule_id);
  qos_rule.qosruleidentifer = rule_id;
  qos_rule.qosflowidentifer = flow.qfi.qfi;
  sps->add_qos_rule(qos_rule);

  sps->add_qos_flow(flow);
  sps->set_default_qos_flow(flow.qfi);
  current_flow = flow;
  graph->start_asynch_dfs_procedure(true, flow);

  std::vector<edge> dl_edges;
  std::vector<edge> ul_edges;
  std::shared_ptr<pfcp_association> upf = {};
  // Get next UPF for the first N4 session establishment
  get_next_upf(dl_edges, ul_edges, upf);

  return send_n4_session_establishment_request();
}

//------------------------------------------------------------------------------
smf_procedure_code session_create_sm_context_procedure::handle_itti_msg(
    itti_n4_session_establishment_response& resp,
    std::shared_ptr<smf::smf_context> sc) {
  Logger::smf_app().info(
      "Handle N4 Session Establishment Response (PDU Session Id %d)",
      n11_trigger->req.get_pdu_session_id());

  pfcp::cause_t cause = {};
  resp.pfcp_ies.get(cause);
  if (cause.cause_value == pfcp::CAUSE_VALUE_REQUEST_ACCEPTED) {
    resp.pfcp_ies.get(sps->up_fseid);
    n11_triggered_pending->res.set_cause(
        static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
  } else {
    // TODO we should have a good cause mapping here
    n11_triggered_pending->res.set_cause(static_cast<uint8_t>(
        cause_value_5gsm_e::CAUSE_31_REQUEST_REJECTED_UNSPECIFIED));
    // TODO we need to abort all ongoing sessions
    return smf_procedure_code::ERROR;
  }

  std::shared_ptr<pfcp_association> current_upf = {};
  std::vector<edge> dl_edges;
  std::vector<edge> ul_edges;
  if (get_current_upf(dl_edges, ul_edges, current_upf) !=
      smf_procedure_code::OK) {
    return smf_procedure_code::ERROR;
  }

  std::shared_ptr<smf_qos_flow> default_qos_flow = {};
  if (smf_cfg->enable_dl_pdr_in_pfcp_sess_estab &&
      resp.pfcp_ies.created_pdrs.empty()) {
    pfcp::pdr_id_t pdr_id_tmp;
    // we use qos flow for 1st PDR for the moment
    // TODO: remove this hardcoding of qos flow
    pdr_id_tmp.rule_id = 1;
    auto flow          = dl_edges[0].get_qos_flow(pdr_id_tmp);
    if (flow) {
      default_qos_flow = flow;
    }
  }

  for (const auto& it : resp.pfcp_ies.created_pdrs) {
    pfcp::pdr_id_t pdr_id = {};
    pfcp::far_id_t far_id = {};
    if (it.get(pdr_id)) {
      // even in UL CL scenario, taking the TEID from one PDR is enough
      auto flow = dl_edges[0].get_qos_flow(pdr_id);
      if (flow) {
        // pfcp::fteid_t local_up_fteid = { };
        if (flow->pdr_id_ul == pdr_id && it.get(flow->ul_fteid)) {
          // Update Qos Flow
          // TODO can i safely remove that?
          // sps->add_qos_flow(flow);
          default_qos_flow = flow;
        }
      } else {
        // This may happen in UL CL, when we have 2 PDR IDs but only one DL
        // edge
        Logger::smf_app().debug(
            "Could not get QoS Flow for created_pdr %d", pdr_id.rule_id);
      }
    } else {
      Logger::smf_app().error(
          "Could not get pdr_id for created_pdr in %s",
          resp.pfcp_ies.get_msg_name());
    }
  }

  synch_ul_dl_edges(dl_edges, ul_edges, current_flow.qfi, false);

  // covers the case that UL CL is returned from algorithm, but not all TEIDs
  // have been set (not all paths explored yet)
  //  we go through until no UPF is left or until we find one to send N4 to
  bool search_upf                = true;
  bool send_n4                   = true;
  smf_procedure_code send_n4_res = smf_procedure_code::ERROR;
  while (search_upf) {
    std::vector<edge> next_dl_edges;
    std::vector<edge> next_ul_edges;
    std::shared_ptr<pfcp_association> next_upf = {};
    send_n4_res = get_next_upf(next_dl_edges, next_ul_edges, next_upf);
    if (send_n4_res != smf_procedure_code::CONTINUE) {
      search_upf = false;
      send_n4    = false;
    } else {
      Logger::smf_app().debug(
          "Try to send N4 to UPF %s", next_upf->get_printable_name().c_str());
      // update FTEID for forward tunnel info for this edge
      send_n4 = true;
      for (auto ul_edge : next_ul_edges) {
        auto flow = ul_edge.get_qos_flow(current_flow.qfi);
        if (!flow) {
          send_n4 = false;
        }
        if (ul_edge.type == iface_type::N9 && flow->ul_fteid.is_zero()) {
          Logger::smf_app().debug(
              "UPF %s has unvisited UL edges",
              next_upf->get_printable_name().c_str());
          send_n4 = false;
        }
      }
      // if we found UPF to send N4, we don't need to search UPF anymore
      search_upf = !send_n4;
    }
  }
  if (send_n4) {
    return send_n4_session_establishment_request();
  }

  // flow_updated info will be used to construct N1,N2 container
  qos_flow_context_updated flow_updated = {};
  QOSRulesIE qos_rule                   = {};

  flow_updated.set_cause(
      static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
  if (!default_qos_flow) {
    flow_updated.set_cause(static_cast<uint8_t>(
        cause_value_5gsm_e::CAUSE_31_REQUEST_REJECTED_UNSPECIFIED));
  } else {
    if (default_qos_flow->ul_fteid.is_zero()) {
      flow_updated.set_cause(static_cast<uint8_t>(
          cause_value_5gsm_e::CAUSE_31_REQUEST_REJECTED_UNSPECIFIED));
    } else {
      flow_updated.set_ul_fteid(default_qos_flow->ul_fteid);  // tunnel info
    }
    if (sps->get_default_qos_rule(qos_rule)) {
      flow_updated.add_qos_rule(qos_rule);
    }
    flow_updated.set_qfi(default_qos_flow->qfi);
    qos_profile_t profile = {};
    profile               = default_qos_flow->qos_profile;
    flow_updated.set_qos_profile(profile);
    flow_updated.set_priority_level(
        default_qos_flow->qos_profile.priority_level);
  }

  // TODO: Set RQA (optional)

  n11_triggered_pending->res.set_qos_flow_context(flow_updated);

  if (flow_updated.cause_value !=
      static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED)) {
    return smf_procedure_code::ERROR;
  }

  return smf_procedure_code::OK;
}

//------------------------------------------------------------------------------
smf_procedure_code
session_update_sm_context_procedure::send_n4_session_modification_request() {
  Logger::smf_app().debug("Send N4 Session Modification Request");

  std::shared_ptr<pfcp_association> current_upf = {};
  std::vector<edge> dl_edges{};
  std::vector<edge> ul_edges{};

  if (get_current_upf(dl_edges, ul_edges, current_upf) !=
      smf_procedure_code::OK) {
    return smf_procedure_code::ERROR;
  }

  n4_triggered = std::make_shared<itti_n4_session_modification_request>(
      TASK_SMF_APP, TASK_SMF_N4);
  n4_triggered->seid    = sps->up_fseid.seid;
  n4_triggered->trxn_id = this->trxn_id;
  n4_triggered->r_endpoint =
      endpoint(current_upf->node_id.u1.ipv4_address, pfcp::default_port);

  edge dl_edge = dl_edges[0];
  for (auto& ul_edge : ul_edges) {
    // we set PDR ID UL to 0, so we create new ones
    auto flow_dl = dl_edge.get_qos_flow(current_flow.qfi);
    auto ul_flow = ul_edge.get_qos_flow(current_flow.qfi);
    if (!flow_dl || !ul_flow) {
      Logger::smf_app().error("Could not get the QOS Flow for DL procedure");
      continue;
    }
    flow_dl->pdr_id_dl = 0;

    pfcp::create_far create_far = pfcp_create_far(dl_edge, current_flow.qfi);

    ul_flow->far_id_dl = create_far.far_id;

    pfcp::create_pdr create_pdr = pfcp_create_pdr(
        ul_edge, current_flow.qfi, current_upf->function_features.second);

    // Add IEs to message
    n4_triggered->pfcp_ies.set(create_far);
    n4_triggered->pfcp_ies.set(create_pdr);
  }
  synch_ul_dl_edges(dl_edges, ul_edges, current_flow.qfi, false);

  Logger::smf_app().info(
      "Sending ITTI message %s to task TASK_SMF_N4",
      n4_triggered->get_msg_name());
  int ret = itti_inst->send_msg(n4_triggered);
  if (RETURNok != ret) {
    Logger::smf_app().error(
        "Could not send ITTI message %s to task TASK_SMF_N4",
        n4_triggered->get_msg_name());
    return smf_procedure_code::ERROR;
  }
  return smf_procedure_code::CONTINUE;
}

//------------------------------------------------------------------------------
smf_procedure_code session_update_sm_context_procedure::run(
    std::shared_ptr<itti_n11_update_sm_context_request> sm_context_req,
    std::shared_ptr<itti_n11_update_sm_context_response> sm_context_resp,
    std::shared_ptr<smf::smf_context> sc) {
  // Handle SM update sm context request
  // The SMF initiates an N4 Session Modification procedure with the UPF. The
  // SMF provides AN Tunnel Info to the UPF as well as the corresponding
  // forwarding rules

  bool send_n4 = false;
  Logger::smf_app().info("Perform a procedure - Update SM Context Request");
  // TODO check if compatible with ongoing procedures if any
  // Get UPF node
  std::shared_ptr<smf_context_ref> scf = {};
  scid_t scid                          = {};
  try {
    scid = std::stoi(sm_context_req->scid);
  } catch (const std::exception& err) {
    Logger::smf_app().warn(
        "SM Context associated with this id %s does not exit!",
        sm_context_req->scid.c_str());
  }
  if (smf_app_inst->is_scid_2_smf_context(scid)) {
    scf = smf_app_inst->scid_2_smf_context(scid);
    // up_node_id = scf.get()->upf_node_id;
  } else {
    Logger::smf_app().warn(
        "SM Context associated with this id " SCID_FMT " does not exit!", scid);
    // TODO:
    return smf_procedure_code::ERROR;
  }

  std::shared_ptr<smf_pdu_session> sp = {};
  if (!sc.get()->find_pdu_session(scf.get()->pdu_session_id, sp)) {
    Logger::smf_app().warn("PDU session context does not exist!");
    return smf_procedure_code::ERROR;
  }

  std::shared_ptr<upf_graph> graph = sp->get_sessions_graph();

  if (!graph) {
    Logger::smf_app().warn("PDU session does not have a UPF association");
    return smf_procedure_code::ERROR;
  }

  //  TODO: UPF insertion in case of Handover

  // The flow is not needed anymore in DL direction, as we already have flows
  smf_qos_flow empty_flow{};
  graph->start_asynch_dfs_procedure(false, empty_flow);

  std::shared_ptr<pfcp_association> current_upf = {};
  std::vector<edge> dl_edges;
  std::vector<edge> ul_edges;

  if (get_next_upf(dl_edges, ul_edges, current_upf) !=
      smf_procedure_code::CONTINUE) {
    Logger::smf_app().error("DL Procedure Error: No UPF to select");
    return smf_procedure_code::ERROR;
  }

  //-------------------
  n11_trigger           = sm_context_req;
  n11_triggered_pending = sm_context_resp;
  uint64_t seid         = smf_app_inst->generate_seid();
  sps->set_seid(seid);

  n4_triggered = std::make_shared<itti_n4_session_modification_request>(
      TASK_SMF_APP, TASK_SMF_N4);
  n4_triggered->seid    = sps->up_fseid.seid;
  n4_triggered->trxn_id = this->trxn_id;
  n4_triggered->r_endpoint =
      endpoint(current_upf->node_id.u1.ipv4_address, pfcp::default_port);

  // QoS Flow to be modified
  pdu_session_update_sm_context_request sm_context_req_msg =
      sm_context_req->req;
  std::vector<pfcp::qfi_t> list_of_qfis_to_be_modified = {};
  sm_context_req_msg.get_qfis(list_of_qfis_to_be_modified);

  for (auto i : list_of_qfis_to_be_modified) {
    Logger::smf_app().debug("QoS Flow to be modified (QFI %d)", i.qfi);
    // TODO currently just support one flow
    current_flow.qfi = i;
  }

  Logger::smf_app().debug(
      "Session procedure type: %s",
      session_management_procedures_type_e2str
          .at(static_cast<int>(session_procedure_type))
          .c_str());

  switch (session_procedure_type) {
    case session_management_procedures_type_e::
        PDU_SESSION_ESTABLISHMENT_UE_REQUESTED:
    case session_management_procedures_type_e::
        PDU_SESSION_MODIFICATION_SMF_REQUESTED:
    case session_management_procedures_type_e::
        PDU_SESSION_MODIFICATION_AN_REQUESTED:
    case session_management_procedures_type_e::
        PDU_SESSION_MODIFICATION_UE_INITIATED_STEP2:
    case session_management_procedures_type_e::HO_PATH_SWITCH_REQ:
    case session_management_procedures_type_e::N2_HO_PREPARATION_PHASE_STEP2: {
      pfcp::fteid_t dl_fteid = {};
      sm_context_req_msg.get_dl_fteid(dl_fteid);  // eNB's fteid
      // we use the first dl_edge as we can only have one N3 interface
      for (const auto& qfi : list_of_qfis_to_be_modified) {
        auto flow = dl_edges[0].get_qos_flow(qfi);
        if (!flow) {  // no QoS flow found
          Logger::smf_app().error(
              "could not find any QoS flow with QFI %d", qfi.qfi);
          // Set cause to SYSTEM_FAILURE and send response
          qos_flow_context_updated qcu = {};
          qcu.set_cause(static_cast<uint8_t>(
              cause_value_5gsm_e::CAUSE_31_REQUEST_REJECTED_UNSPECIFIED));
          qcu.set_qfi(qfi);
          n11_triggered_pending->res.add_qos_flow_context_updated(qcu);
          continue;
        }

        Logger::smf_app().debug(
            "FAR ID DL first %d,  FAR DL ID second "
            "0x%" PRIx32 " ",
            flow->far_id_dl.first, flow->far_id_dl.second.far_id);

        if ((dl_fteid == flow->dl_fteid) and (not flow->released)) {
          Logger::smf_app().debug("QFI %d dl_fteid unchanged", qfi.qfi);
          qos_flow_context_updated qcu = {};
          qcu.set_cause(static_cast<uint8_t>(
              cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
          qcu.set_qfi(qfi);
          n11_triggered_pending->res.add_qos_flow_context_updated(qcu);
          continue;
          // TODO when does this happen?
        } else if ((flow->far_id_dl.first) && (flow->far_id_dl.second.far_id)) {
          // TODO also refactor update
          Logger::smf_app().debug(
              "Update FAR DL "
              "0x%" PRIx32 " ",
              flow->far_id_dl.second.far_id);
          // Update FAR
          pfcp::update_far update_far                                     = {};
          pfcp::apply_action_t apply_action                               = {};
          pfcp::outer_header_creation_t outer_header_creation             = {};
          pfcp::update_forwarding_parameters update_forwarding_parameters = {};
          pfcp::destination_interface_t destination_interface             = {};

          update_far.set(flow->far_id_dl.second);
          outer_header_creation.outer_header_creation_description =
              OUTER_HEADER_CREATION_GTPU_UDP_IPV4;
          outer_header_creation.teid = dl_fteid.teid;
          outer_header_creation.ipv4_address.s_addr =
              dl_fteid.ipv4_address.s_addr;
          update_forwarding_parameters.set(outer_header_creation);
          destination_interface.interface_value = pfcp::INTERFACE_VALUE_ACCESS;
          update_forwarding_parameters.set(destination_interface);
          update_far.set(update_forwarding_parameters);
          apply_action.forw = 1;  // forward the packets
          // apply_action.nocp = 1; //notify the CP function about the arrival
          // of a first DL packet
          update_far.set(apply_action);
          n4_triggered->pfcp_ies.set(update_far);

          send_n4               = true;
          flow->far_id_dl.first = true;
          flow->dl_fteid        = dl_fteid;

        } else {
          flow->dl_fteid = dl_fteid;
          Logger::smf_app().debug("Create FAR DL");
          // for each UL edge we need a FAR, because of UL CL
          edge dl_edge = dl_edges[0];
          for (auto& edge : ul_edges) {
            // we set PDR ID UL to 0, so we create new ones
            auto flow_dl       = dl_edge.get_qos_flow(flow->qfi);
            flow_dl->pdr_id_dl = 0;

            pfcp::create_far create_far = pfcp_create_far(dl_edge, flow->qfi);

            synch_ul_dl_edges(dl_edges, ul_edges, flow->qfi, false);
            // Add IEs to message
            n4_triggered->pfcp_ies.set(create_far);
          }

          send_n4 = true;
          Logger::smf_app().debug(
              "FAR DL ID "
              "0x%" PRIx32 " ",
              flow->far_id_dl.second.far_id);
        }
        // TODO refactor update

        // for each UL edge we need to update or create the PDR
        for (auto& ul_edge : ul_edges) {
          auto ul_flow = ul_edge.get_qos_flow(qfi);
          if (not ul_flow->pdr_id_dl.rule_id) {
            Logger::smf_app().debug("Create PDR DL");
            //-------------------
            // IE create_pdr
            //-------------------
            pfcp::create_pdr create_pdr = pfcp_create_pdr(
                ul_edge, ul_flow->qfi, current_upf->function_features.second);
            n4_triggered->pfcp_ies.set(create_pdr);
            synch_ul_dl_edges(dl_edges, ul_edges, ul_flow->qfi, false);
            Logger::smf_app().debug(
                "PDR DL ID "
                "0x%" PRIx16 " ",
                ul_flow->pdr_id_dl.rule_id);
          } else {
            uint16_t rule_id = ul_flow->pdr_id_dl.rule_id;

            Logger::smf_app().debug(
                "Update FAR, PDR DL Rule Id "
                "0x%" PRIx16 ", FAR ID 0x%" PRIx32 " ",
                rule_id, flow->far_id_dl.second.far_id);
            /*
            // Update FAR
            far_id.far_id = flow->far_id_ul.second.far_id;
            pfcp::update_far update_far = { };
            pfcp::apply_action_t apply_action = { };

            update_far.set(flow->far_id_ul.second);
            apply_action.forw = 1;
            update_far.set(apply_action);

            n4_triggered->pfcp_ies.set(update_far);

            send_n4 = true;

            flow->far_id_dl.first = true;
            */

            pfcp::update_pdr update_pdr                       = {};
            pfcp::precedence_t precedence                     = {};
            pfcp::pdi pdi                                     = {};
            pfcp::ue_ip_address_t ue_ip_address               = {};
            pfcp::source_interface_t source_interface         = {};
            pfcp::outer_header_removal_t outer_header_removal = {};

            if (sps->ipv4) {
              ue_ip_address.v4 = 1;
              // Bit 3 – S/D: TS 29.244 R16, 8.2.62 In the PDI IE, if this bit
              // is set to "0", this indicates a Source IP address; if this bit
              // is set to "1", this indicates a Destination IP address.
              ue_ip_address.sd                  = 1;
              ue_ip_address.ipv4_address.s_addr = sps->ipv4_address.s_addr;
            }
            if (sps->ipv6) {
              ue_ip_address.v6           = 1;
              ue_ip_address.ipv6_address = sps->ipv6_address;
            }
            precedence.precedence = ul_edge.precedence;

            source_interface.interface_value = pfcp::INTERFACE_VALUE_CORE;
            if (!ul_edge.nw_instance.empty()) {
              // mandatory for travelping
              pfcp::network_instance_t network_instance = {};
              network_instance.network_instance         = ul_edge.nw_instance;
              pdi.set(network_instance);
            }

            pdi.set(source_interface);
            pdi.set(ue_ip_address);

            if (smf_cfg->enable_ur) {
              pfcp::urr_id_t urr_Id = ul_flow->urr_id;
              update_pdr.set(urr_Id);
            }

            if (ul_edge.type != iface_type::N6) {
              outer_header_removal.outer_header_removal_description =
                  OUTER_HEADER_REMOVAL_GTPU_UDP_IPV4;
              update_pdr.set(outer_header_removal);
            }

            update_pdr.set(ul_flow->pdr_id_dl);
            update_pdr.set(precedence);
            update_pdr.set(pdi);
            update_pdr.set(ul_flow->far_id_dl.second);

            // Add IEs to message
            n4_triggered->pfcp_ies.set(update_pdr);

            Logger::smf_app().debug(
                "PDR DL ID  "
                "0x%" PRIx16 " updated",
                ul_flow->pdr_id_dl.rule_id);
          }
          send_n4 = true;
        }
        // after a release flows
        if (not flow->ul_fteid.is_zero()) {
        }

        if (not flow->dl_fteid.is_zero()) {
        }
        // may be modified
        // TODO can I safely remove that?
        // sps->add_qos_flow(flow);

        qos_flow_context_updated qcu = {};
        qcu.set_cause(static_cast<uint8_t>(
            cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
        qcu.set_qfi(qfi);
        n11_triggered_pending->res.add_qos_flow_context_updated(qcu);
      }
    } break;

    case session_management_procedures_type_e::
        SERVICE_REQUEST_UE_TRIGGERED_STEP2: {
      pfcp::fteid_t dl_fteid = {};
      sm_context_req_msg.get_dl_fteid(dl_fteid);  // eNB's f-teid
      edge dl_edge = dl_edges[0];

      // we use the first dl_edge as we can only have one N3 interface
      // Stefan: TODO currently we only support one QoS flow here
      // but also at other places, so we have to test and fix multiple QoS flows
      // everywhere
      for (const auto& qfi : list_of_qfis_to_be_modified) {
        auto flow      = dl_edge.get_qos_flow(qfi);
        flow->dl_fteid = dl_fteid;
      }
      send_n4_session_modification_request();
      for (const auto& qfi : list_of_qfis_to_be_modified) {
        synch_ul_dl_edges(dl_edges, ul_edges, qfi, true);
      }
      // as we use the function to send N4 and do not construct it, we can stop
      // at this point
      return smf_procedure_code::OK;
    } break;

    case session_management_procedures_type_e::
        SERVICE_REQUEST_UE_TRIGGERED_STEP1: {
      Logger::smf_app().debug("SERVICE_REQUEST_UE_TRIGGERED_STEP1");
      for (const auto& qfi : list_of_qfis_to_be_modified) {
        auto flow = dl_edges[0].get_qos_flow(qfi);
        if (!flow) {  // no QoS flow found
          Logger::smf_app().error(
              "Could not find any QoS flow with QFI %d", qfi.qfi);
          // TODO: Check the appropriate value of cause
          qos_flow_context_updated qcu = {};
          qcu.set_cause(static_cast<uint8_t>(
              cause_value_5gsm_e::CAUSE_31_REQUEST_REJECTED_UNSPECIFIED));
          qcu.set_qfi(qfi);
          n11_triggered_pending->res.add_qos_flow_context_updated(qcu);
          continue;
        }

        edge& dl_edge = dl_edges[0];
        // CREATE_FAR
        for (auto& ul_edge : ul_edges) {
          //-------------------
          // IE CREATE_FAR
          //-------------------
          pfcp::create_far create_far = pfcp_create_far(ul_edge, flow->qfi);
          // copy created FAR ID to DL edge for PDR
          synch_ul_dl_edges(dl_edges, ul_edges, flow->qfi, true);

          // Copy values from UL edge, so we simulate two downlink edges for
          // PFCP
          auto flow_dl               = dl_edge.get_qos_flow(flow->qfi);
          auto flow_ul               = ul_edge.get_qos_flow(flow->qfi);
          flow_dl->pdr_id_ul.rule_id = 0;
          dl_edge.flow_description   = ul_edge.flow_description;
          dl_edge.precedence += 1;
          ul_edge.precedence += 1;
          graph->update_edge_info(current_upf, ul_edge);

          // CREATE_PDR
          pfcp::create_pdr create_pdr = pfcp_create_pdr(
              dl_edge, flow->qfi, current_upf->function_features.second);
          synch_ul_dl_edges(dl_edges, ul_edges, flow->qfi, true);

          // ADD IEs to message
          //-------------------
          n4_triggered->pfcp_ies.set(create_pdr);
          n4_triggered->pfcp_ies.set(create_far);
        }
        graph->update_edge_info(current_upf, dl_edge);
        send_n4 = true;

        qos_flow_context_updated qcu = {};
        qcu.set_cause(static_cast<uint8_t>(
            cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
        qcu.set_qfi(qfi);

        n11_triggered_pending->res.add_qos_flow_context_updated(qcu);
      }
    } break;

    case session_management_procedures_type_e::
        PDU_SESSION_RELEASE_AN_INITIATED: {
      Logger::smf_app().debug("PDU_SESSION_RELEASE_AN_INITIATED");
      // we use the first dl_edge as we can only have one N3 interface
      for (const auto& qfi : list_of_qfis_to_be_modified) {
        auto flow = dl_edges[0].get_qos_flow(qfi);
        if (!flow) {  // no QoS flow found
          Logger::smf_app().error(
              "Could not find any QoS flow with QFI %d", qfi.qfi);
          // TODO: Check the appropriate value of cause
          qos_flow_context_updated qcu = {};
          qcu.set_cause(static_cast<uint8_t>(
              cause_value_5gsm_e::CAUSE_31_REQUEST_REJECTED_UNSPECIFIED));
          qcu.set_qfi(qfi);
          n11_triggered_pending->res.add_qos_flow_context_updated(qcu);
          continue;
        }

        pfcp::remove_pdr remove_pdr_dl = {};
        pfcp::remove_pdr remove_pdr_ul = {};
        pfcp::remove_far remove_far_dl = {};
        pfcp::remove_far remove_far_ul = {};

        remove_pdr_dl.set(flow->pdr_id_dl);
        remove_pdr_ul.set(flow->pdr_id_ul);
        remove_far_dl.far_id.first         = true;
        remove_far_dl.far_id.second.far_id = flow->far_id_dl.second.far_id;
        remove_far_ul.far_id.first         = true;
        remove_far_ul.far_id.second.far_id = flow->far_id_ul.second.far_id;

        // we can get this request when we have only UL, so we have to check for
        // 0
        if (remove_pdr_dl.pdr_id.second.rule_id != 0) {
          n4_triggered->pfcp_ies.set(remove_pdr_dl);
        }
        if (remove_pdr_ul.pdr_id.second.rule_id != 0) {
          n4_triggered->pfcp_ies.set(remove_pdr_ul);
        }
        if (remove_far_dl.far_id.second.far_id != 0) {
          n4_triggered->pfcp_ies.set(remove_far_dl);
        }
        if (remove_far_ul.far_id.second.far_id != 0) {
          n4_triggered->pfcp_ies.set(remove_far_ul);
        }

        Logger::smf_app().debug(
            "Remove FAR ID DL first %d,  FAR DL ID second "
            "0x%" PRIx32 " ",
            flow->far_id_dl.first, flow->far_id_dl.second.far_id);

        Logger::smf_app().debug(
            "Remove FAR, PDR DL Rule Id "
            "0x%" PRIx16 ", FAR ID 0x%" PRIx32 " ",
            flow->pdr_id_dl.rule_id, flow->far_id_dl.second.far_id);

        Logger::smf_app().debug(
            "Remove FAR ID UL first %d,  FAR UL ID second "
            "0x%" PRIx32 " ",
            flow->far_id_ul.first, flow->far_id_ul.second.far_id);

        Logger::smf_app().debug(
            "Remove FAR, PDR UL Rule Id "
            "0x%" PRIx16 ", FAR ID 0x%" PRIx32 " ",
            flow->pdr_id_ul.rule_id, flow->far_id_ul.second.far_id);

        send_n4 = true;
        flow->clear_session();

        // we also have to remove PDR / FAR IDs in UL direction
        for (auto& ul_edge : ul_edges) {
          auto ul_flow = ul_edge.get_qos_flow(qfi);
          ul_flow->clear_session();
        }

        qos_flow_context_updated qcu = {};
        qcu.set_cause(static_cast<uint8_t>(
            cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
        qcu.set_qfi(qfi);
        n11_triggered_pending->res.add_qos_flow_context_updated(qcu);
      }
    } break;

    default: {
      Logger::smf_app().error(
          "Update SM Context procedure: Unknown session management type %d",
          (int) session_procedure_type);
    }
  }

  if (send_n4) {
    Logger::smf_app().info(
        "Sending ITTI message %s to task TASK_SMF_N4",
        n4_triggered->get_msg_name());
    int ret = itti_inst->send_msg(n4_triggered);
    if (RETURNok != ret) {
      Logger::smf_app().error(
          "Could not send ITTI message %s to task TASK_SMF_N4",
          n4_triggered->get_msg_name());
      return smf_procedure_code::ERROR;
    }
  } else {
    Logger::smf_app().error(
        "Update SM Context procedure: There is no QoS flow to be modified");
    return smf_procedure_code::ERROR;
  }
  return smf_procedure_code::OK;
}

//------------------------------------------------------------------------------
smf_procedure_code session_update_sm_context_procedure::handle_itti_msg(
    itti_n4_session_modification_response& resp,
    std::shared_ptr<smf::smf_context> sc) {
  Logger::smf_app().info(
      "Handle N4 Session Modification Response (PDU Session Id %d)",
      n11_trigger.get()->req.get_pdu_session_id());

  pfcp::cause_t cause = {};
  resp.pfcp_ies.get(cause);

  n11_triggered_pending->res.set_cause(static_cast<uint8_t>(
      cause_value_5gsm_e::CAUSE_31_REQUEST_REJECTED_UNSPECIFIED));

  if (cause.cause_value != CAUSE_VALUE_REQUEST_ACCEPTED) {
    // TODO: Nsmf_PDUSession_SMContextStatusNotify
    /*  If the PDU Session establishment is not successful, the SMF informs
     the AMF by invoking Nsmf_PDUSession_SMContextStatusNotify (Release). The
     SMF also releases any N4 session(s) created, any PDU Session address if
     allocated (e.g. IP address) and releases the association with PCF, if
     any. see step 18, section 4.3.2.2.1@3GPP TS 23.502)
     */
    // TODO: should we return here with smf_procedure_code::ERROR;
  } else {
    n11_triggered_pending->res.set_cause(
        static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
  }

  // list of accepted QFI(s) and AN Tunnel Info corresponding to the PDU
  // Session
  std::vector<pfcp::qfi_t> list_of_qfis_to_be_modified = {};
  n11_trigger->req.get_qfis(list_of_qfis_to_be_modified);

  std::shared_ptr<pfcp_association> current_upf = {};
  std::vector<edge> dl_edges{};
  std::vector<edge> ul_edges{};

  if (get_current_upf(dl_edges, ul_edges, current_upf) ==
      smf_procedure_code::ERROR) {
    Logger::smf_app().error("SMF DL procedure: Could not get current UPF");
    // TODO is this enough as an error message? We have cause 31 but not
    // values
    return smf_procedure_code::ERROR;
  }

  bool continue_n4 = true;

  Logger::smf_app().debug(
      "Session procedure type: %s",
      session_management_procedures_type_e2str
          .at(static_cast<int>(session_procedure_type))
          .c_str());

  nlohmann::json json_data = {};
  std::map<uint8_t, qos_flow_context_updated> qos_flow_context_to_be_updateds =
      {};
  n11_triggered_pending->res.get_all_qos_flow_context_updateds(
      qos_flow_context_to_be_updateds);
  n11_triggered_pending->res.remove_all_qos_flow_context_updateds();
  for (const auto& it : qos_flow_context_to_be_updateds)
    Logger::smf_app().debug("QoS Flow context to be modified QFI %d", it.first);

  switch (session_procedure_type) {
    case session_management_procedures_type_e::
        PDU_SESSION_ESTABLISHMENT_UE_REQUESTED:
    case session_management_procedures_type_e::
        PDU_SESSION_MODIFICATION_SMF_REQUESTED:
    case session_management_procedures_type_e::
        PDU_SESSION_MODIFICATION_AN_REQUESTED:
    case session_management_procedures_type_e::
        SERVICE_REQUEST_UE_TRIGGERED_STEP2:
    case session_management_procedures_type_e::
        PDU_SESSION_MODIFICATION_UE_INITIATED_STEP2:
    case session_management_procedures_type_e::HO_PATH_SWITCH_REQ:
    case session_management_procedures_type_e::N2_HO_PREPARATION_PHASE_STEP2: {
      pfcp::fteid_t n3_dl_fteid = {};
      n11_trigger->req.get_dl_fteid(n3_dl_fteid);

      Logger::smf_app().debug(
          "AN F-TEID ID "
          "0x%" PRIx32 ", IP Addr %s",
          n3_dl_fteid.teid, conv::toString(n3_dl_fteid.ipv4_address).c_str());

      if (session_procedure_type == session_management_procedures_type_e::
                                        SERVICE_REQUEST_UE_TRIGGERED_STEP2) {
        Logger::smf_app().error("bla temp");
      }

      for (const auto& it_created_pdr : resp.pfcp_ies.created_pdrs) {
        pfcp::pdr_id_t pdr_id = {};
        if (it_created_pdr.get(pdr_id)) {
          for (auto& ul_edge : ul_edges) {
            auto flow = ul_edge.get_qos_flow(pdr_id);
            if (flow) {
              Logger::smf_app().debug("QoS Flow, QFI %d", flow->qfi.qfi);
              for (const auto& it : qos_flow_context_to_be_updateds) {
                if (!it_created_pdr.get(flow->dl_fteid)) {
                  Logger::smf_app().warn(
                      "Could not get DL FTEID from PDR in DL");
                }

                if (it_created_pdr.get(flow->ul_fteid)) {
                  Logger::smf_app().debug(
                      "Got local_up_fteid from created_pdr %s",
                      flow->ul_fteid.toString().c_str());
                } else {
                  // UPF doesn't include its fteid in the response
                  Logger::smf_app().debug(
                      "Could not get local_up_fteid from created_pdr");
                }

                flow->released = false;
                // TODO can i safely remove that
                // sps->add_qos_flow(flow);

                qos_flow_context_updated qcu = {};
                qcu.set_cause(static_cast<uint8_t>(
                    cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
                qcu.set_qfi(pfcp::qfi_t(it.first));
                qcu.set_ul_fteid(flow->ul_fteid);
                qcu.set_dl_fteid(flow->dl_fteid);
                qcu.set_qos_profile(flow->qos_profile);
                n11_triggered_pending->res.add_qos_flow_context_updated(qcu);
                // TODO: remove this QFI from the list (as well as in
                // n11_trigger->req)
                break;
              }
            }
          }
        } else {
          Logger::smf_app().error(
              "Could not get pdr_id for created_pdr in %s",
              resp.pfcp_ies.get_msg_name());
        }
      }

      if (cause.cause_value == CAUSE_VALUE_REQUEST_ACCEPTED) {
        // TODO failed rule id
        for (const auto& it_update_far : n4_triggered->pfcp_ies.update_fars) {
          // TODO Stefan: I think when there is an update FAR in DL, we dont
          // need to update other UPFs, is that correct?
          continue_n4 = false;

          pfcp::far_id_t far_id = {};
          if (it_update_far.get(far_id)) {
            for (auto& ul_edge : ul_edges) {
              auto flow = ul_edge.get_qos_flow(far_id);
              if (flow) {
                for (const auto& it : qos_flow_context_to_be_updateds) {
                  if (it.first == flow->qfi.qfi) {
                    flow->dl_fteid = n3_dl_fteid;

                    // TODO can i safely remove that?
                    // sps->add_qos_flow(flow);

                    qos_flow_context_updated qcu = {};
                    qcu.set_cause(static_cast<uint8_t>(
                        cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
                    qcu.set_qfi(pfcp::qfi_t(it.first));
                    qcu.set_ul_fteid(flow->ul_fteid);
                    qcu.set_dl_fteid(flow->dl_fteid);
                    qcu.set_qos_profile(flow->qos_profile);
                    n11_triggered_pending->res.add_qos_flow_context_updated(
                        qcu);
                    break;
                  }
                }
              } else {
                Logger::smf_app().error(
                    "Could not get QoS flow for far_id for update_far in %s",
                    resp.pfcp_ies.get_msg_name());
              }
            }
          } else {
            Logger::smf_app().error(
                "Could not get far_id for update_far in %s",
                resp.pfcp_ies.get_msg_name());
          }
        }
      } else {
        Logger::smf_app().info(
            "PDU Session Update SM Context, rejected by UPF");
        return smf_procedure_code::ERROR;
      }
    } break;

    case session_management_procedures_type_e::
        SERVICE_REQUEST_UE_TRIGGERED_STEP1: {
      Logger::smf_app().debug(
          "PDU Session Update SM Context, SERVICE_REQUEST_UE_TRIGGERED_STEP1");

      for (const auto& it_created_pdr : resp.pfcp_ies.created_pdrs) {
        pfcp::pdr_id_t pdr_id = {};
        if (it_created_pdr.get(pdr_id)) {
          for (auto& ul_edge : ul_edges) {
            auto flow = ul_edge.get_qos_flow(pdr_id);
            if (flow) {
              Logger::smf_app().debug("QoS Flow, QFI %d", flow->qfi.qfi);
              for (const auto& it : qos_flow_context_to_be_updateds) {
                if (!it_created_pdr.get(flow->dl_fteid)) {
                  Logger::smf_app().warn(
                      "Could not get DL F-TEID from PDR in DL");
                }

                if (it_created_pdr.get(flow->ul_fteid)) {
                  Logger::smf_app().debug(
                      "Got UL F-TEID from created_pdr %s",
                      flow->ul_fteid.toString().c_str());
                } else {
                  // UPF doesn't include its f-teid in the response
                  Logger::smf_app().debug(
                      "Could not get UL F-TEID from created_pdr");
                }

                flow->released = false;
                // TODO can i safely remove that
                // sps->add_qos_flow(flow);

                qos_flow_context_updated qcu = {};
                qcu.set_cause(static_cast<uint8_t>(
                    cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
                qcu.set_qfi(pfcp::qfi_t(it.first));
                qcu.set_ul_fteid(flow->ul_fteid);
                qcu.set_dl_fteid(flow->dl_fteid);
                qcu.set_qos_profile(flow->qos_profile);
                n11_triggered_pending->res.add_qos_flow_context_updated(qcu);
                // TODO: remove this QFI from the list (as well as in
                // n11_trigger->req)
                break;
              }
            }
          }
        } else {
          Logger::smf_app().error(
              "Could not get pdr_id for created_pdr in %s",
              resp.pfcp_ies.get_msg_name());
        }
      }

      if (cause.cause_value == CAUSE_VALUE_REQUEST_ACCEPTED) {
        // TODO failed rule id
        for (const auto& it_update_far : n4_triggered->pfcp_ies.update_fars) {
          // TODO Stefan: I think when there is an update FAR in DL, we dont
          // need to update other UPFs, is that correct?
          continue_n4 = false;

          pfcp::far_id_t far_id = {};
          if (it_update_far.get(far_id)) {
            for (auto& ul_edge : ul_edges) {
              auto flow = ul_edge.get_qos_flow(far_id);
              if (flow) {
                for (const auto& it : qos_flow_context_to_be_updateds) {
                  if (it.first == flow->qfi.qfi) {
                    // flow->dl_fteid = n3_dl_fteid;
                    qos_flow_context_updated qcu = {};
                    qcu.set_cause(static_cast<uint8_t>(
                        cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
                    qcu.set_qfi(pfcp::qfi_t(it.first));
                    qcu.set_ul_fteid(flow->ul_fteid);
                    qcu.set_dl_fteid(flow->dl_fteid);
                    qcu.set_qos_profile(flow->qos_profile);
                    n11_triggered_pending->res.add_qos_flow_context_updated(
                        qcu);
                    break;
                  }
                }
              } else {
                Logger::smf_app().error(
                    "Could not get QoS flow for far_id for update_far in %s",
                    resp.pfcp_ies.get_msg_name());
              }
            }
          } else {
            Logger::smf_app().error(
                "Could not get far_id for update_far in %s",
                resp.pfcp_ies.get_msg_name());
          }
        }
      } else {
        Logger::smf_app().info(
            "PDU Session Update SM Context, rejected by UPF");
        return smf_procedure_code::ERROR;
      }

    } break;

    case session_management_procedures_type_e::
        PDU_SESSION_RELEASE_AN_INITIATED: {
      Logger::smf_app().debug("PDU_SESSION_RELEASE_AN_INITIATED");

      for (const auto& it : qos_flow_context_to_be_updateds) {
        Logger::smf_app().debug(
            "QoS Flow context to be modified QFI %d", it.first);
        // sps->remove_qos_flow(it.second.qfi);
      }
      // Mark as deactivated
      sps->set_upCnx_state(upCnx_state_e::UPCNX_STATE_DEACTIVATED);

      json_data["upCnxState"] = "DEACTIVATED";
      n11_triggered_pending->res.set_json_data(json_data);
    } break;

    default: {
      Logger::smf_app().error(
          "Update SM Context procedure: Unknown session management type %d",
          (int) session_procedure_type);
    }
  }

  std::shared_ptr<pfcp_association> next_upf = {};
  std::vector<edge> next_dl_edges{};
  std::vector<edge> next_ul_edges{};

  if (continue_n4 && get_next_upf(next_dl_edges, next_ul_edges, next_upf) ==
                         smf_procedure_code::CONTINUE) {
    return send_n4_session_modification_request();
  }

  // n11_triggered_pending->res.set_cause(cause.cause_value);
  n11_triggered_pending->res.set_http_code(
      http_status_code_e::HTTP_STATUS_CODE_200_OK);

  return smf_procedure_code::OK;
}

//------------------------------------------------------------------------------
smf_procedure_code
session_release_sm_context_procedure::send_n4_session_deletion_request() {
  std::vector<edge> dl_edges;
  std::vector<edge> ul_edges;
  std::shared_ptr<pfcp_association> current_upf = {};

  if (get_current_upf(dl_edges, ul_edges, current_upf) ==
      smf_procedure_code::ERROR) {
    return smf_procedure_code::ERROR;
  }

  n4_triggered = std::make_shared<itti_n4_session_deletion_request>(
      TASK_SMF_APP, TASK_SMF_N4);
  n4_triggered->seid    = sps->up_fseid.seid;
  n4_triggered->trxn_id = this->trxn_id;
  n4_triggered->r_endpoint =
      endpoint(current_upf->node_id.u1.ipv4_address, pfcp::default_port);

  Logger::smf_app().info(
      "Sending ITTI message %s to task TASK_SMF_N4",
      n4_triggered->get_msg_name());
  int ret = itti_inst->send_msg(n4_triggered);
  if (RETURNok != ret) {
    Logger::smf_app().error(
        "Could not send ITTI message %s to task TASK_SMF_N4",
        n4_triggered->get_msg_name());
    return smf_procedure_code::ERROR;
  }
  return smf_procedure_code::CONTINUE;
}
//------------------------------------------------------------------------------
smf_procedure_code session_release_sm_context_procedure::run(
    std::shared_ptr<itti_n11_release_sm_context_request> sm_context_req,
    std::shared_ptr<itti_n11_release_sm_context_response> sm_context_res,
    std::shared_ptr<smf::smf_context> sc) {
  Logger::smf_app().info("Release SM Context Request");
  // TODO check if compatible with ongoing procedures if any
  pfcp::node_id_t up_node_id = {};
  // Get UPF node
  std::shared_ptr<smf_context_ref> scf = {};
  scid_t scid                          = {};
  try {
    scid = std::stoi(sm_context_req->scid);
  } catch (const std::exception& err) {
    Logger::smf_app().warn(
        "SM Context associated with this id %s does not exit!",
        sm_context_req->scid.c_str());
  }
  if (smf_app_inst->is_scid_2_smf_context(scid)) {
    scf = smf_app_inst->scid_2_smf_context(scid);
    // up_node_id = scf.get()->upf_node_id;
  } else {
    Logger::smf_app().warn(
        "SM Context associated with this id " SCID_FMT " does not exit!", scid);
    // TODO:
    return smf_procedure_code::ERROR;
  }

  std::shared_ptr<smf_pdu_session> sp = {};
  if (!sc.get()->find_pdu_session(scf.get()->pdu_session_id, sp)) {
    Logger::smf_app().warn("PDU session context does not exist!");
    return smf_procedure_code::ERROR;
  }

  std::shared_ptr<upf_graph> graph = sp->get_sessions_graph();

  if (!graph) {
    Logger::smf_app().warn("PDU session does not have a UPF association");
    return smf_procedure_code::ERROR;
  }
  // we start from the access nodes, because we have only ULCLs we don't have
  // the situation that one UPF is returned more than once
  smf_qos_flow empty_flow;
  graph->start_asynch_dfs_procedure(false, empty_flow);

  std::vector<edge> dl_edges;
  std::vector<edge> ul_edges;
  std::shared_ptr<pfcp_association> current_upf = {};
  if (get_next_upf(dl_edges, ul_edges, current_upf) ==
      smf_procedure_code::ERROR) {
    return smf_procedure_code::ERROR;
  }

  n11_trigger           = sm_context_req;
  n11_triggered_pending = sm_context_res;
  uint64_t seid         = smf_app_inst->generate_seid();
  sps->set_seid(seid);
  return send_n4_session_deletion_request();
}

//------------------------------------------------------------------------------
smf_procedure_code session_release_sm_context_procedure::handle_itti_msg(
    itti_n4_session_deletion_response& resp,
    std::shared_ptr<smf::smf_context> sc) {
  Logger::smf_app().info(
      "Handle itti_n4_session_deletion_response (Release SM Context "
      "Request): "
      "pdu-session-id %d",
      n11_trigger.get()->req.get_pdu_session_id());

  pfcp::cause_t cause = {};
  resp.pfcp_ies.get(cause);

  std::vector<edge> dl_edges;
  std::vector<edge> ul_edges;
  std::shared_ptr<pfcp_association> current_upf = {};
  bool continue_n4                              = false;
  if (get_next_upf(dl_edges, ul_edges, current_upf) ==
      smf_procedure_code::CONTINUE) {
    // If we have to continue, we ignore the PFCP error code, because we
    // should at least remove other UPF sessions
    return send_n4_session_deletion_request();
    continue_n4 = false;
  }

  if (cause.cause_value == CAUSE_VALUE_REQUEST_ACCEPTED) {
    n11_triggered_pending->res.set_cause(
        static_cast<uint8_t>(cause_value_5gsm_e::CAUSE_255_REQUEST_ACCEPTED));
    Logger::smf_app().info("PDU Session Release SM Context accepted by UPFs");
    return smf_procedure_code::OK;
  } else {
    n11_triggered_pending->res.set_cause(static_cast<uint8_t>(
        cause_value_5gsm_e::CAUSE_31_REQUEST_REJECTED_UNSPECIFIED));
    // We cannot return an error here, because we need to delete all the UPFs
    return smf_procedure_code::ERROR;
  }

  // TODO:
  /* If it is the last PDU Session the SMF is handling for the UE for the
   associated (DNN, S- NSSAI), the SMF unsubscribes from Session Management
   Subscription data changes notification with the UDM by means of the
   Nudm_SDM_Unsubscribe service operation. The SMF invokes the
   Nudm_UECM_Deregistration service operation so that the UDM removes the
   association it had stored between the SMF identity and the associated DNN
   and PDU Session Id
   */
}
