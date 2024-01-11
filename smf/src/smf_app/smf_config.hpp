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

/*! \file smf_config.hpp
 * \brief
 \author  Lionel GAUTHIER, Tien-Thinh NGUYEN, Stefan Spettel
 \company Eurecom, phine.tech
 \date 2023
 \email: lionel.gauthier@eurecom.fr, tien-thinh.nguyen@eurecom.fr,
 stefan.spettel@phine.tech
 */

#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <mutex>
#include <vector>
#include "thread_sched.hpp"

#include "3gpp_29.244.h"
#include "pfcp.hpp"
#include "smf.h"
#include "smf_profile.hpp"
#include "config.hpp"
#include "logger_base.hpp"
#include "smf_config_types.hpp"
#include "if.hpp"
#include "Snssai.h"

namespace oai::config::smf {

const std::string USE_LOCAL_PCC_RULES_CONFIG_VALUE = "use_local_pcc_rules";
const std::string USE_LOCAL_SUBSCRIPTION_INFOS_CONFIG_VALUE =
    "use_local_subscription_info";
const std::string USE_EXTERNAL_AUSF_CONFIG_VALUE = "use_external_ausf";
const std::string USE_EXTERNAL_UDM_CONFIG_VALUE  = "use_external_udm";
const std::string USE_EXTERNAL_NSSF_CONFIG_VALUE = "use_external_nssf";

const oai::model::common::Snssai DEFAULT_SNSSAI{1};
const session_ambr_t DEFAULT_S_AMBR{"1000Mbps", "1000Mbps"};
const std::string DEFAULT_DNN  = "default";
const uint8_t DEFAULT_SSC_MODE = 1;
const subscribed_default_qos_t DEFAULT_QOS{
    9,
    {1, "NOT_PREEMPT", "NOT_PREEMPTABLE"},
    1};
const upf DEFAULT_UPF{"oai-upf", 8805, false, false, ""};

typedef struct interface_cfg_s {
  std::string if_name;
  struct in_addr addr4;
  struct in6_addr addr6;
  unsigned int mtu;
  unsigned int port;
  util::thread_sched_params thread_rd_sched_params;
  nlohmann::json to_json() const {
    nlohmann::json json_data = {};
    json_data["if_name"]     = this->if_name;
    json_data["addr4"]       = inet_ntoa(this->addr4);
    char str_addr6[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &this->addr6, str_addr6, sizeof(str_addr6));
    json_data["addr6"] = str_addr6;
    json_data["mtu"]   = this->mtu;
    json_data["port"]  = this->port;
    // TODO: thread_rd_sched_params
    return json_data;
  }

  void from_json(nlohmann::json& json_data) {
    this->if_name         = json_data["if_name"].get<std::string>();
    std::string addr4_str = {};
    addr4_str             = json_data["addr4"].get<std::string>();

    if (boost::iequals(addr4_str, "read")) {
      struct in_addr net_mask_addr4 {};
      if (get_inet_addr_infos_from_iface(
              this->if_name, this->addr4, net_mask_addr4, this->mtu)) {
        Logger::smf_app().error(
            "Could not read %s network interface configuration", this->if_name);
        return;
      }
    } else {
      IPV4_STR_ADDR_TO_INADDR(
          util::trim(addr4_str).c_str(), this->addr4,
          "BAD IPv4 ADDRESS FORMAT FOR INTERFACE !");
      // TODO: addr6
      this->mtu  = json_data["mtu"].get<int>();
      this->port = json_data["port"].get<int>();
    }
    // TODO: thread_rd_sched_params
  }
} interface_cfg_t;

typedef struct itti_cfg_s {
  util::thread_sched_params itti_timer_sched_params;
  util::thread_sched_params n4_sched_params;
  util::thread_sched_params smf_app_sched_params;
  util::thread_sched_params async_cmd_sched_params;
} itti_cfg_t;

typedef struct dnn_s {
  std::string dnn;
  std::string dnn_label;
  struct in_addr ue_pool_range_low;
  struct in_addr ue_pool_range_high;
  struct in6_addr paa_pool6_prefix;
  uint8_t paa_pool6_prefix_len;
  pdu_session_type_t pdu_session_type;

  nlohmann::json to_json() const {
    nlohmann::json json_data        = {};
    json_data["dnn"]                = this->dnn;
    json_data["ue_pool_range_low"]  = inet_ntoa(this->ue_pool_range_low);
    json_data["ue_pool_range_high"] = inet_ntoa(this->ue_pool_range_high);
    char str_addr6[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &this->paa_pool6_prefix, str_addr6, sizeof(str_addr6));
    json_data["paa_pool6_prefix"]     = str_addr6;
    json_data["paa_pool6_prefix_len"] = this->paa_pool6_prefix_len;
    json_data["pdu_session_type"]     = this->pdu_session_type.to_string();
    return json_data;
  }

  void from_json(nlohmann::json& json_data) {
    this->dnn = json_data["dnn"].get<std::string>();

    std::string ue_pool_range_low_str = {};
    ue_pool_range_low_str = json_data["ue_pool_range_low"].get<std::string>();
    IPV4_STR_ADDR_TO_INADDR(
        util::trim(ue_pool_range_low_str).c_str(), this->ue_pool_range_low,
        "BAD IPv4 ADDRESS FORMAT FOR INTERFACE !");

    std::string ue_pool_range_high_str = {};
    ue_pool_range_high_str = json_data["ue_pool_range_high"].get<std::string>();
    IPV4_STR_ADDR_TO_INADDR(
        util::trim(ue_pool_range_high_str).c_str(), this->ue_pool_range_high,
        "BAD IPv4 ADDRESS FORMAT FOR INTERFACE !");

    // TODO: pool_id_iv6
    pdu_session_type.from_json(json_data["pdu_session_type"]);
  }
} dnn_t;

class smf_config : public config {
 private:
  // TODO only temporary, to avoid changing all the references to the config in
  // all the calling classes
  void to_smf_config();

  // TODO only temporary, we should not resolve on startup in the config
  static in_addr resolve_nf(const std::string& host);

  void update_used_nfs() override;

 public:
  spdlog::level::level_enum log_level;
  unsigned int instance = 0;
  interface_cfg_t n4;
  interface_cfg_t sbi;
  unsigned int sbi_http2_port;
  std::string sbi_api_version;
  itti_cfg_t itti;

  std::map<std::string, dnn_t> dnns;

  bool force_push_pco;

  bool register_nrf;
  bool discover_upf;
  bool discover_pcf;
  bool use_local_subscription_info;
  bool use_local_pcc_rules;
  unsigned int http_version;
  bool enable_ur;
  bool enable_dl_pdr_in_pfcp_sess_estab;
  std::string local_n3_addr;

  std::vector<pfcp::node_id_t> upfs;

  // Network instance
  // bool network_instance_configuration;
  struct upf_nwi_list_s {
    pfcp::node_id_t upf_id;
    std::string domain_access;
    std::string domain_core;
    //      std::string domain_sgi_lan;

    nlohmann::json to_json() const {
      nlohmann::json json_data   = {};
      json_data["upf_id"]        = upf_id.toString();
      json_data["domain_access"] = domain_access;
      json_data["domain_core"]   = domain_core;

      return json_data;
    }
  };
  typedef struct upf_nwi_list_s upf_nwi_list_t;

  std::vector<upf_nwi_list_t> upf_nwi_list;

  smf_config(const std::string& configPath, bool logStdout, bool logRotFile);

  int get_pfcp_node_id(pfcp::node_id_t& node_id);
  int get_pfcp_fseid(pfcp::fseid_t& fseid);
  bool is_dotted_dnn_handled(
      const std::string& dnn, const pdu_session_type_t& pdn_session_type);
  std::string get_default_dnn();

  /**
   * Returns network instance of iface_type typ. If not found, empty string is
   * returned
   * @param node_id IP address or FQDN to match against configuration
   * @return NWI or empty string
   */
  std::string get_nwi(
      const pfcp::node_id_t& node_id, const ::smf::iface_type& type) const;

  /**
   * Returns configured UPF based on node_id.
   * Compares UPF host config value with node_id FQDN and IPv4 address in this
   * order
   * @param node_id PFCP node id, FQDN, IPv4 address must be set
   * @throws std::invalid_argument  in case UPF is not to be found or type is
   * IPv6
   * @return upf
   */
  const oai::config::smf::upf& get_upf(const pfcp::node_id_t& node_id) const;

  /**
   * Returns SMF configuration pointer which stores SMF-specific configuration
   * @return SMF configuration
   */
  std::shared_ptr<smf_config_type> smf() const;

  /**
   * Returns UE DNS from the DNN, returns default DNS if DNN is not found
   * @param dnn
   * @return
   */
  const ue_dns& get_dns_from_dnn(const std::string& dnn);

  bool init() override;

  /*
   * Represent SMF's config as json object
   * @param [nlohmann::json &] json_data: Json data
   * @return void
   */
  void to_json(nlohmann::json& json_data);

  /*
   * Update SMF's config from Json
   * @param [nlohmann::json &] json_data: Updated configuration in json format
   * @return true if success otherwise return false
   */
  bool from_json(nlohmann::json& json_data);
};

}  // namespace oai::config::smf
