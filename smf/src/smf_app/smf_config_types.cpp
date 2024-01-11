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

/*! \file smf_config_types.cpp
 * \brief
 \author Stefan Spettel
 \company phine.tech
 \date 2023
 \email: stefan.spettel@phine.tech
 */

#include "smf_config_types.hpp"
#include "smf_config.hpp"
#include "conv.hpp"

using namespace oai::config::smf;
using namespace oai::config;
using namespace oai::model::common;
using namespace oai::model::nrf;

smf_support_features::smf_support_features(
    bool local_subscription_info, bool local_pcc_rules) {
  set_config_name("supported_features");
  m_local_subscription_infos = option_config_value(
      "use_local_subscription_info", local_subscription_info);
  m_local_pcc_rules =
      option_config_value("use_local_pcc_rules", local_pcc_rules);
  m_external_ausf = option_config_value("use_external_ausf", false);
  m_external_udm  = option_config_value("use_external_udm", false);
  m_external_nssf = option_config_value("use_external_nssf", false);
}

// TODO we should move this to common and use it together with AMF and other NFs
smf_support_features::smf_support_features(
    bool external_ausf, bool external_udm, bool external_nssf) {
  set_config_name("supported_features");
  m_external_ausf = option_config_value("use_external_ausf", external_ausf);
  m_external_udm  = option_config_value("use_external_udm", external_udm);
  m_external_nssf = option_config_value("use_external_nssf", external_nssf);
}

void smf_support_features::from_yaml(const YAML::Node& node) {
  if (node[USE_LOCAL_PCC_RULES_CONFIG_VALUE]) {
    m_local_pcc_rules.from_yaml(node[USE_LOCAL_PCC_RULES_CONFIG_VALUE]);
  }
  if (node[USE_LOCAL_SUBSCRIPTION_INFOS_CONFIG_VALUE]) {
    m_local_subscription_infos.from_yaml(
        node[USE_LOCAL_SUBSCRIPTION_INFOS_CONFIG_VALUE]);
  }
  if (node[USE_EXTERNAL_AUSF_CONFIG_VALUE]) {
    m_external_ausf.from_yaml(node[USE_EXTERNAL_AUSF_CONFIG_VALUE]);
  }
  if (node[USE_EXTERNAL_UDM_CONFIG_VALUE]) {
    m_external_udm.from_yaml(node[USE_EXTERNAL_UDM_CONFIG_VALUE]);
  }
  if (node[USE_EXTERNAL_NSSF_CONFIG_VALUE]) {
    m_external_nssf.from_yaml(node[USE_EXTERNAL_AUSF_CONFIG_VALUE]);
  }
}

nlohmann::json smf_support_features::to_json() {
  nlohmann::json json_data = {};
  json_data[m_local_pcc_rules.get_config_name()] =
      m_local_pcc_rules.get_value();
  json_data[m_local_subscription_infos.get_config_name()] =
      m_local_subscription_infos.get_value();
  json_data[m_external_ausf.get_config_name()] = m_external_ausf.get_value();
  json_data[m_external_udm.get_config_name()]  = m_external_udm.get_value();
  json_data[m_external_nssf.get_config_name()] = m_external_nssf.get_value();
  return json_data;
}

bool smf_support_features::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_local_pcc_rules.get_config_name()) !=
        json_data.end()) {
      m_local_pcc_rules.from_json(
          json_data[m_local_pcc_rules.get_config_name()]);
    }
    if (json_data.find(m_local_subscription_infos.get_config_name()) !=
        json_data.end()) {
      m_local_subscription_infos.from_json(
          json_data[m_local_subscription_infos.get_config_name()]);
    }
    if (json_data.find(m_external_ausf.get_config_name()) != json_data.end()) {
      m_external_ausf.from_json(json_data[m_external_ausf.get_config_name()]);
    }
    if (json_data.find(m_external_udm.get_config_name()) != json_data.end()) {
      m_external_udm.from_json(json_data[m_external_udm.get_config_name()]);
    }
    if (json_data.find(m_external_nssf.get_config_name()) != json_data.end()) {
      m_external_nssf.from_json(json_data[m_external_nssf.get_config_name()]);
    }

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string smf_support_features::to_string(const std::string& indent) const {
  std::string out;
  std::string inner_indent = indent + indent;
  unsigned int inner_width = get_inner_width(inner_indent.length());
  out.append(indent).append(m_config_name).append(":\n");

  if (!m_local_subscription_infos.get_config_name().empty()) {
    out.append(inner_indent)
        .append(fmt::format(
            BASE_FORMATTER, INNER_LIST_ELEM,
            m_local_subscription_infos.get_config_name(), inner_width,
            m_local_subscription_infos.to_string("")));
  }

  if (!m_local_pcc_rules.get_config_name().empty()) {
    out.append(inner_indent)
        .append(fmt::format(
            BASE_FORMATTER, INNER_LIST_ELEM,
            m_local_pcc_rules.get_config_name(), inner_width,
            m_local_pcc_rules.to_string("")));
  }

  if (!m_external_ausf.get_config_name().empty()) {
    out.append(inner_indent)
        .append(fmt::format(
            BASE_FORMATTER, INNER_LIST_ELEM, m_external_ausf.get_config_name(),
            inner_width, m_external_ausf.to_string("")));
  }

  if (!m_external_udm.get_config_name().empty()) {
    out.append(inner_indent)
        .append(fmt::format(
            BASE_FORMATTER, INNER_LIST_ELEM, m_external_udm.get_config_name(),
            inner_width, m_external_udm.to_string("")));
  }

  if (!m_external_nssf.get_config_name().empty()) {
    out.append(inner_indent)
        .append(fmt::format(
            BASE_FORMATTER, INNER_LIST_ELEM, m_external_nssf.get_config_name(),
            inner_width, m_external_nssf.to_string("")));
  }

  return out;
}

bool smf_support_features::use_local_subscription_info() const {
  return m_local_subscription_infos.get_value();
}

bool smf_support_features::use_local_pcc_rules() const {
  return m_local_pcc_rules.get_value();
}

bool smf_support_features::use_external_ausf() const {
  return m_external_ausf.get_value();
}

bool smf_support_features::use_external_udm() const {
  return m_external_udm.get_value();
}

bool smf_support_features::use_external_nssf() const {
  return m_external_nssf.get_value();
}

upf::upf(
    const std::string& host, int port, bool enable_usage_reporting,
    bool enable_dl_pdr_in_session_establishment,
    const std::string& local_n3_ip) {
  m_host = string_config_value("host", host);
  m_port = int_config_value("port", port);
  m_usage_reporting =
      option_config_value("enable_usage_reporting", enable_usage_reporting);
  m_dl_pdr_in_session_establishment = option_config_value(
      "enable_dl_pdr_in_session_establishment",
      enable_dl_pdr_in_session_establishment);
  m_local_n3_ipv4 = string_config_value("local_n3_ipv4", local_n3_ip);

  m_host.set_validation_regex(HOST_VALIDATOR_REGEX);
  m_port.set_validation_interval(PORT_MIN_VALUE, PORT_MAX_VALUE);
  m_local_n3_ipv4.set_validation_regex(IPV4_ADDRESS_VALIDATOR_REGEX);
  if (local_n3_ip.empty()) {
    m_local_n3_ipv4.unset_config();
  }
  InterfaceUpfInfoItem item_n3;
  InterfaceUpfInfoItem item_n6;
  UPInterfaceType type_n3;
  UPInterfaceType type_n6;
  type_n3.setEnumValue(UPInterfaceType_anyOf::eUPInterfaceType_anyOf::N3);
  type_n6.setEnumValue(UPInterfaceType_anyOf::eUPInterfaceType_anyOf::N6);
  item_n3.setNetworkInstance("access.oai.org");
  item_n3.setInterfaceType(type_n3);

  item_n6.setNetworkInstance("core.oai.org");
  item_n6.setInterfaceType(type_n6);

  m_upf_info.setInterfaceUpfInfoList(
      std::vector<InterfaceUpfInfoItem>{item_n3, item_n6});
  m_set = true;
}

void upf::from_yaml(const YAML::Node& node) {
  if (node["host"]) {
    m_host.from_yaml(node["host"]);
  }
  if (node["port"]) {
    m_port.from_yaml(node["port"]);
  }
  if (node["config"]) {
    if (node["config"]["enable_usage_reporting"]) {
      m_usage_reporting.from_yaml(node["config"]["enable_usage_reporting"]);
    }
    if (node["config"]["enable_dl_pdr_in_pfcp_session_establishment"]) {
      m_dl_pdr_in_session_establishment.from_yaml(
          node["config"]["enable_dl_pdr_in_pfcp_session_establishment"]);
    }
    if (node["config"]["n3_local_ipv4"]) {
      m_local_n3_ipv4.from_yaml(node["config"]["n3_local_ipv4"]);
    }
  }
  if (node["upf_info"]) {
    nlohmann::json j =
        oai::utils::conversions::yaml_to_json(node["upf_info"], false);
    nlohmann::from_json(j, m_upf_info);
  }
}

nlohmann::json upf::to_json() {
  nlohmann::json json_data            = {};
  json_data[m_host.get_config_name()] = m_host.get_value();
  json_data[m_port.get_config_name()] = m_port.get_value();
  json_data["config"][m_usage_reporting.get_config_name()] =
      m_usage_reporting.get_value();
  json_data["config"][m_dl_pdr_in_session_establishment.get_config_name()] =
      m_dl_pdr_in_session_establishment.get_value();
  json_data["config"][m_local_n3_ipv4.get_config_name()] =
      m_local_n3_ipv4.to_json();
  nlohmann::to_json(json_data["upf_info"], m_upf_info);
  return json_data;
}

bool upf::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_host.get_config_name()) != json_data.end()) {
      m_host.from_json(json_data[m_host.get_config_name()]);
    }

    if (json_data.find(m_port.get_config_name()) != json_data.end()) {
      m_port.from_json(json_data[m_port.get_config_name()]);
    }

    if (json_data.find("config") != json_data.end()) {
      if (json_data["config"].find(m_usage_reporting.get_config_name()) !=
          json_data.end()) {
        m_usage_reporting.from_json(
            json_data["config"][m_usage_reporting.get_config_name()]);
      }
      if (json_data["config"].find(
              m_dl_pdr_in_session_establishment.get_config_name()) !=
          json_data.end()) {
        m_dl_pdr_in_session_establishment.from_json(
            json_data["config"]
                     [m_dl_pdr_in_session_establishment.get_config_name()]);
      }
      if (json_data["config"].find(m_local_n3_ipv4.get_config_name()) !=
          json_data.end()) {
        m_local_n3_ipv4.from_json(
            json_data["config"][m_local_n3_ipv4.get_config_name()]);
      }
      nlohmann::from_json(json_data["upf_info"], m_upf_info);
    }

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string upf::to_string(const std::string& indent) const {
  std::string out;
  std::string inner_indent = add_indent(indent);
  unsigned int inner_width = get_inner_width(inner_indent.length());
  out.append(indent).append(
      fmt::format("{} {}\n", INNER_LIST_ELEM, m_host.get_value()));

  std::string fmt_value = get_value_formatter(3);
  out.append(
      fmt::format(fmt_value, m_host.get_config_name(), m_host.to_string("")));
  out.append(
      fmt::format(fmt_value, m_port.get_config_name(), m_port.to_string("")));
  out.append(fmt::format(
      fmt_value, m_usage_reporting.get_config_name(),
      m_usage_reporting.to_string("")));
  out.append(fmt::format(
      fmt_value, m_dl_pdr_in_session_establishment.get_config_name(),
      m_dl_pdr_in_session_establishment.to_string("")));
  if (m_local_n3_ipv4.is_set()) {
    out.append(fmt::format(
        fmt_value, m_local_n3_ipv4.get_config_name(),
        m_local_n3_ipv4.to_string("")));
  }
  if (m_upf_info_is_set) {
    out.append(m_upf_info.to_string(3));
  }
  return out;
}

void upf::validate() {
  if (!m_set) return;
  m_host.validate();
  m_port.validate();
  m_local_n3_ipv4.validate();
  if (m_upf_info_is_set) {
    m_upf_info.validate(false);
  }
}

const std::string& upf::get_host() const {
  return m_host.get_value();
}

uint16_t upf::get_port() const {
  return m_port.get_value();
}

bool upf::enable_usage_reporting() const {
  return m_usage_reporting.get_value();
}

bool upf::enable_dl_pdr_in_session_establishment() const {
  return m_dl_pdr_in_session_establishment.get_value();
}

const std::string& upf::get_local_n3_ip() const {
  return m_local_n3_ipv4.get_value();
}

const oai::model::nrf::UpfInfo& upf::get_upf_info() const {
  return m_upf_info;
}

void upf::enable_upf_info(bool val) {
  m_upf_info_is_set = val;
}

ims_config::ims_config(
    const std::string& pcscf_ip_v4, const std::string& pcscf_ip_v6) {
  m_pcscf_v4 = string_config_value("p-cscf_ipv4", pcscf_ip_v4);
  m_pcscf_v6 = string_config_value("p-cscf_ipv6", pcscf_ip_v6);

  m_pcscf_v4.set_validation_regex(IPV4_ADDRESS_VALIDATOR_REGEX);
  m_pcscf_v6.set_validation_regex(IPV6_ADDRESS_VALIDATOR_REGEX);
  if (pcscf_ip_v6.empty()) {
    m_pcscf_v6.unset_config();
  }
}

void ims_config::from_yaml(const YAML::Node& node) {
  if (node["pcscf_ipv4"]) {
    m_pcscf_v4.from_yaml(node["pcscf_ipv4"]);
  }
  if (node["pcscf_ipv6"]) {
    m_pcscf_v6.from_yaml(node["pcscf_ipv6"]);
  }
}

nlohmann::json ims_config::to_json() {
  nlohmann::json json_data                = {};
  json_data[m_pcscf_v4.get_config_name()] = m_pcscf_v4.get_value();
  json_data[m_pcscf_v6.get_config_name()] = m_pcscf_v6.get_value();
  return json_data;
}

bool ims_config::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_pcscf_v4.get_config_name()) != json_data.end()) {
      m_pcscf_v4.from_json(json_data[m_pcscf_v4.get_config_name()]);
    }

    if (json_data.find(m_pcscf_v6.get_config_name()) != json_data.end()) {
      m_pcscf_v6.from_json(json_data[m_pcscf_v6.get_config_name()]);
    }

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string ims_config::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = get_inner_width(indent.length());

  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, m_pcscf_v4.get_config_name(),
      inner_width, m_pcscf_v4.to_string("")));
  if (m_pcscf_v6.is_set()) {
    out.append(indent).append(fmt::format(
        BASE_FORMATTER, OUTER_LIST_ELEM, m_pcscf_v6.get_config_name(),
        inner_width, m_pcscf_v6.to_string("")));
  }
  return out;
}

void ims_config::validate() {
  m_pcscf_v4.validate();
  m_pcscf_v6.validate();

  if (m_pcscf_v4.is_set()) {
    m_pcscf_v4_ip = safe_convert_ip(m_pcscf_v4.get_value());
  } else if (m_pcscf_v6.is_set()) {
    m_pcscf_v6_ip = safe_convert_ip6(m_pcscf_v6.get_value());
  } else {
    throw std::runtime_error(
        fmt::format("You need to specify either a valid IPv4 or IPv6 address"));
  }
}

const in_addr& ims_config::get_pcscf_v4() const {
  return m_pcscf_v4_ip;
}

const in6_addr& ims_config::get_pcscf_v6() const {
  return m_pcscf_v6_ip;
}

smf_config_type::smf_config_type(
    const std::string& name, const std::string& host, const sbi_interface& sbi,
    const local_interface& n4)
    : nf(name, host, sbi),
      m_ims_config("127.0.0.1", ""),
      m_support_feature(false, true),
      m_ue_dns("8.8.8.8", "1.1.1.1", "", "") {
  m_config_name = "SMF Config";
  m_ue_mtu      = int_config_value("ue_mtu", 1500);
  m_ue_mtu.set_validation_interval(1, 65535);
  m_upfs.push_back(DEFAULT_UPF);
  m_n4 = n4;
}

void smf_config_type::from_yaml(const YAML::Node& node) {
  nf::from_yaml(node);
  if (node["ue_mtu"]) {
    m_ue_mtu.from_yaml(node["ue_mtu"]);
  }
  if (node["support_features"]) {
    m_support_feature.from_yaml(node["support_features"]);
  }
  if (node["ue_dns"]) {
    m_ue_dns.from_yaml(node["ue_dns"]);
  }
  if (node["ims"]) {
    m_ims_config.from_yaml(node["ims"]);
  }
  if (node["n4"]) {
    m_n4.set_host(get_host());
    m_n4.from_yaml(node["n4"]);
  }
  if (node["upfs"]) {
    // any default UPF is deleted if people configure UPFs
    m_upfs.clear();
    for (const auto& yaml_upf : node["upfs"]) {
      // TODO should we have a default host here?
      upf u = upf("", 8805, false, false, "");
      u.from_yaml(yaml_upf);
      m_upfs.push_back(u);
    }
  }
  if (node["smf_info"]) {
    // any default SNSSAI info list is deleted if people configure a profile
    m_smf_info.getSNssaiSmfInfoList().clear();
    nlohmann::json j =
        oai::utils::conversions::yaml_to_json(node["smf_info"], false);
    nlohmann::from_json(j, m_smf_info);
  }
  if (node["local_subscription_infos"]) {
    // any default subscription is deleted if people configure it
    m_subscription_infos.clear();
    for (const auto& yaml_sub : node["local_subscription_infos"]) {
      subscription_info_config subcfg(
          DEFAULT_DNN, DEFAULT_SSC_MODE, DEFAULT_QOS, DEFAULT_S_AMBR,
          DEFAULT_SNSSAI);
      subcfg.from_yaml(yaml_sub);
      m_subscription_infos.push_back(subcfg);
    }
  }
}

nlohmann::json smf_config_type::to_json() {
  nlohmann::json json_data = {};

  json_data                                      = nf::to_json();
  json_data[m_support_feature.get_config_name()] = m_support_feature.to_json();
  json_data[m_ue_dns.get_config_name()]          = m_ue_dns.to_json();
  json_data["upfs"]                              = nlohmann::json::array();
  for (auto u : m_upfs) {
    json_data["upfs"].push_back(u.to_json());
  }
  json_data["local_subscription_infos"] = nlohmann::json::array();
  for (auto s : m_subscription_infos) {
    json_data["local_subscription_infos"].push_back(s.to_json());
  }
  nlohmann::to_json(json_data["smf_info"], m_smf_info);
  return json_data;
}

bool smf_config_type::from_json(const nlohmann::json& json_data) {
  try {
    nf::from_json(json_data);
    if (json_data.find(m_support_feature.get_config_name()) !=
        json_data.end()) {
      m_support_feature.from_json(
          json_data[m_support_feature.get_config_name()]);
    }
    if (json_data.find(m_ue_dns.get_config_name()) != json_data.end()) {
      m_ue_dns.from_json(json_data[m_ue_dns.get_config_name()]);
    }
    if (json_data.find("smf_info") != json_data.end()) {
      nlohmann::from_json(json_data["smf_info"], m_smf_info);
    }
    // TODO: UPF
    // TODO: local_subscription_infos
    return true;
  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string smf_config_type::to_string(const std::string& indent) const {
  std::string out = nf::to_string("");

  unsigned int inner_width = get_inner_width(indent.length());
  out.append(indent).append(
      fmt::format("{} {}\n", OUTER_LIST_ELEM, m_n4.get_config_name()));
  out.append(m_n4.to_string(add_indent(indent)));

  out.append(m_support_feature.to_string(indent));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, m_ue_mtu.get_config_name(), inner_width,
      m_ue_mtu.to_string("")));
  out.append(m_ims_config.to_string(indent));
  std::string inner_indent = indent + indent;
  if (!m_upfs.empty()) {
    out.append(indent).append("UPF List:\n");
    for (const auto& upf : m_upfs) {
      out.append(upf.to_string(inner_indent));
    }
  }
  if (!m_subscription_infos.empty()) {
    out.append(indent).append("Local Subscription Infos:\n");
    for (const auto& sub : m_subscription_infos) {
      out.append(sub.to_string(inner_indent));
    }
  }
  out.append(m_smf_info.to_string(1));

  return out;
}

void smf_config_type::validate() {
  nf::validate();
  m_ue_dns.validate();
  m_ue_mtu.validate();
  m_n4.validate();
  for (auto& upf : m_upfs) {
    upf.validate();
  }
  m_ims_config.validate();
  for (auto& sub : m_subscription_infos) {
    sub.validate();
  }
  m_smf_info.validate();
}

const smf_support_features& smf_config_type::get_smf_support_features() const {
  return m_support_feature;
}

const ue_dns& smf_config_type::get_ue_dns() const {
  return m_ue_dns;
}

const ims_config& smf_config_type::get_ims_config() const {
  return m_ims_config;
}

uint16_t smf_config_type::get_ue_mtu() const {
  return m_ue_mtu.get_value();
}

const std::vector<upf>& smf_config_type::get_upfs() const {
  return m_upfs;
}

std::vector<subscription_info_config>&
smf_config_type::get_subscription_info() {
  return m_subscription_infos;
}

const local_interface& smf_config_type::get_n4() const {
  return m_n4;
}

const SmfInfo& smf_config_type::get_smf_info() {
  return m_smf_info;
}

subscription_info_config::subscription_info_config(
    const std::string& dnn, uint8_t ssc_mode,
    const subscribed_default_qos_t& qos, const session_ambr_t& session_ambr,
    const Snssai& snssai)
    : m_qos_profile(qos, session_ambr), m_snssai(snssai) {
  m_dnn         = string_config_value("dnn", dnn);
  m_ssc_mode    = int_config_value("ssc_mode", ssc_mode);
  m_config_name = "local_subscription_info";

  // TODO do we need a validation regex for DNN?
  m_ssc_mode.set_validation_interval(1, 3);
}

void subscription_info_config::from_yaml(const YAML::Node& node) {
  if (node["dnn"]) {
    m_dnn.from_yaml(node["dnn"]);
  }
  if (node["ssc_mode"]) {
    m_ssc_mode.from_yaml(node["ssc_mode"]);
  }
  if (node["single_nssai"]) {
    nlohmann::json j =
        oai::utils::conversions::yaml_to_json(node["single_nssai"], false);
    nlohmann::from_json(j, m_snssai);
  }
  if (node["qos_profile"]) {
    m_qos_profile.from_yaml(node["qos_profile"]);
  }
}

nlohmann::json subscription_info_config::to_json() {
  nlohmann::json json_data                = {};
  json_data[m_dnn.get_config_name()]      = m_dnn.to_json();
  json_data[m_ssc_mode.get_config_name()] = m_ssc_mode.to_json();
  nlohmann::to_json(json_data["snssai"], m_snssai);
  json_data[m_qos_profile.get_config_name()] = m_qos_profile.to_json();
  return json_data;
}

bool subscription_info_config::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_dnn.get_config_name()) != json_data.end()) {
      m_dnn.from_json(json_data[m_dnn.get_config_name()]);
    }

    if (json_data.find(m_ssc_mode.get_config_name()) != json_data.end()) {
      m_ssc_mode.from_json(json_data[m_ssc_mode.get_config_name()]);
    }
    if (json_data.find("snssai") != json_data.end()) {
      nlohmann::from_json(json_data["snssai"], m_snssai);
    }
    if (json_data.find(m_qos_profile.get_config_name()) != json_data.end()) {
      m_qos_profile.from_json(json_data[m_qos_profile.get_config_name()]);
    }

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string subscription_info_config::to_string(
    const std::string& indent) const {
  std::string out;
  out.append(indent).append(
      fmt::format("{} {}\n", OUTER_LIST_ELEM, m_config_name));
  std::string inner_indent = add_indent(indent);
  unsigned int inner_width = get_inner_width(inner_indent.length());

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, INNER_LIST_ELEM, m_dnn.get_config_name(), inner_width,
          m_dnn.to_string("")));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, INNER_LIST_ELEM, m_ssc_mode.get_config_name(),
          inner_width, m_ssc_mode.to_string("")));

  out.append(m_snssai.to_string(3));

  out.append(m_qos_profile.to_string(inner_indent));

  return out;
}

void subscription_info_config::validate() {
  m_dnn.validate();
  m_ssc_mode.validate();
  m_snssai.validate();
  m_qos_profile.validate();
}

const std::string& subscription_info_config::get_dnn() const {
  return m_dnn.get_value();
}

uint8_t subscription_info_config::get_ssc_mode() const {
  return m_ssc_mode.get_value();
}

const subscribed_default_qos_t& subscription_info_config::get_default_qos()
    const {
  return m_qos_profile.get_default_qos();
}

const session_ambr_t& subscription_info_config::get_session_ambr() const {
  return m_qos_profile.get_session_ambr();
}

const Snssai& subscription_info_config::get_single_nssai() const {
  return m_snssai;
}

qos_profile_config_value::qos_profile_config_value(
    const subscribed_default_qos_t& qos, const session_ambr_t& ambr) {
  m_qos          = qos;
  m_ambr         = ambr;
  m_5qi          = int_config_value("5qi", m_qos._5qi);
  m_priority     = int_config_value("priority", m_qos.priority_level);
  m_arp_priority = int_config_value("arp_priority", m_qos.arp.priority_level);
  m_arp_preempt_capability =
      string_config_value("arp_preempt_capability", m_qos.arp.preempt_cap);
  m_arp_preempt_vulnerability =
      string_config_value("arp_preempt_vulnerability", m_qos.arp.preempt_vuln);
  m_session_ambr_dl = string_config_value("session_ambr_dl", m_ambr.downlink);
  m_session_ambr_ul = string_config_value("session_ambr_ul", m_ambr.uplink);

  m_5qi.set_validation_interval(1, 254);
  m_priority.set_validation_interval(1, 127);
  m_arp_priority.set_validation_interval(1, 15);

  m_config_name = "qos_profile";

  // TODO ARP Preempt and session AMBR String regex validators
}

void qos_profile_config_value::from_yaml(const YAML::Node& node) {
  if (node["5qi"]) {
    m_5qi.from_yaml(node["5qi"]);
    m_qos._5qi = m_5qi.get_value();
  }
  if (node["priority"]) {
    m_priority.from_yaml(node["priority"]);
    m_qos.priority_level = m_priority.get_value();
  }
  if (node["arp_priority"]) {
    m_arp_priority.from_yaml(node["arp_priority"]);
    m_qos.arp.priority_level = m_arp_priority.get_value();
  }
  if (node["arp_preempt_capability"]) {
    m_arp_preempt_capability.from_yaml(node["arp_preempt_capability"]);
    m_qos.arp.preempt_cap = m_arp_preempt_capability.get_value();
  }
  if (node["arp_preempt_vulnerability"]) {
    m_arp_preempt_vulnerability.from_yaml(node["arp_preempt_vulnerability"]);
    m_qos.arp.preempt_vuln = m_arp_preempt_vulnerability.get_value();
  }
  if (node["session_ambr_ul"]) {
    m_session_ambr_ul.from_yaml(node["session_ambr_ul"]);
    m_ambr.uplink = m_session_ambr_ul.get_value();
  }
  if (node["session_ambr_dl"]) {
    m_session_ambr_dl.from_yaml(node["session_ambr_dl"]);
    m_ambr.downlink = m_session_ambr_dl.get_value();
  }
}

nlohmann::json qos_profile_config_value::to_json() {
  nlohmann::json json_data                = {};
  json_data[m_5qi.get_config_name()]      = m_5qi.to_json();
  json_data[m_priority.get_config_name()] = m_priority.to_json();

  json_data[m_arp_priority.get_config_name()] = m_arp_priority.to_json();
  json_data[m_arp_preempt_capability.get_config_name()] =
      m_arp_preempt_capability.to_json();
  json_data[m_arp_preempt_vulnerability.get_config_name()] =
      m_arp_preempt_vulnerability.to_json();
  json_data[m_session_ambr_ul.get_config_name()] = m_session_ambr_ul.to_json();
  json_data[m_session_ambr_dl.get_config_name()] = m_session_ambr_dl.to_json();
  return json_data;
}

bool qos_profile_config_value::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_5qi.get_config_name()) != json_data.end()) {
      m_5qi.from_json(json_data[m_5qi.get_config_name()]);
    }

    if (json_data.find(m_priority.get_config_name()) != json_data.end()) {
      m_priority.from_json(json_data[m_priority.get_config_name()]);
    }
    if (json_data.find(m_arp_priority.get_config_name()) != json_data.end()) {
      m_arp_priority.from_json(json_data[m_arp_priority.get_config_name()]);
    }
    if (json_data.find(m_arp_preempt_capability.get_config_name()) !=
        json_data.end()) {
      m_arp_preempt_capability.from_json(
          json_data[m_arp_preempt_capability.get_config_name()]);
    }
    if (json_data.find(m_arp_preempt_vulnerability.get_config_name()) !=
        json_data.end()) {
      m_arp_preempt_vulnerability.from_json(
          json_data[m_arp_preempt_vulnerability.get_config_name()]);
    }
    if (json_data.find(m_session_ambr_ul.get_config_name()) !=
        json_data.end()) {
      m_session_ambr_ul.from_json(
          json_data[m_session_ambr_ul.get_config_name()]);
    }
    if (json_data.find(m_session_ambr_dl.get_config_name()) !=
        json_data.end()) {
      m_session_ambr_dl.from_json(
          json_data[m_session_ambr_dl.get_config_name()]);
    }

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string qos_profile_config_value::to_string(
    const std::string& indent) const {
  std::string out;

  out.append(indent).append(
      fmt::format("{} {}:\n", INNER_LIST_ELEM, m_config_name));
  std::string inner_indent = add_indent(indent);

  unsigned int inner_width = get_inner_width(inner_indent.length());

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, m_5qi.get_config_name(), inner_width,
          m_5qi.to_string("")));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, m_priority.get_config_name(),
          inner_width, m_priority.to_string("")));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, m_arp_priority.get_config_name(),
          inner_width, m_arp_priority.to_string("")));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM,
          m_arp_preempt_vulnerability.get_config_name(), inner_width,
          m_arp_preempt_vulnerability.to_string("")));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM,
          m_arp_preempt_capability.get_config_name(), inner_width,
          m_arp_preempt_capability.to_string("")));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, m_session_ambr_dl.get_config_name(),
          inner_width, m_session_ambr_dl.to_string("")));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, m_session_ambr_ul.get_config_name(),
          inner_width, m_session_ambr_ul.to_string("")));

  return out;
}

void qos_profile_config_value::validate() {
  m_5qi.validate();
  m_priority.validate();
  m_arp_priority.validate();
  m_arp_preempt_vulnerability.validate();
  m_arp_preempt_capability.validate();
  m_session_ambr_ul.validate();
  m_session_ambr_dl.validate();
}

const subscribed_default_qos_t& qos_profile_config_value::get_default_qos()
    const {
  return m_qos;
}

const session_ambr_t& qos_profile_config_value::get_session_ambr() const {
  return m_ambr;
}
