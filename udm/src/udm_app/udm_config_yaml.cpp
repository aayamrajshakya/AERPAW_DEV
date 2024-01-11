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

#include "udm_config_yaml.hpp"

#include "conversions.hpp"
#include "logger.hpp"
#include <boost/algorithm/string.hpp>

namespace oai::config {

//------------------------------------------------------------------------------
udm::udm(
    const std::string& name, const std::string& host, const sbi_interface& sbi)
    : nf(name, host, sbi) {}

void udm::from_yaml(const YAML::Node& node) {
  nf::from_yaml(node);

  // Load UDM specified parameter
  for (const auto& elem : node) {
    auto key = elem.first.as<std::string>();

    if (key == UDM_CONFIG_INSTANCE_ID) {
      m_instance_id.from_yaml(elem.second);
    }

    if (key == UDM_CONFIG_PID_DIRECTORY) {
      m_pid_directory.from_yaml(elem.second);
    }

    if (key == UDM_CONFIG_UDM_NAME) {
      m_udm_name.from_yaml(elem.second);
    }
  }
}

//------------------------------------------------------------------------------
std::string udm::to_string(const std::string& indent) const {
  std::string out;
  std::string inner_indent = indent + indent;
  unsigned int inner_width = get_inner_width(inner_indent.length());

  out.append(indent).append(nf::to_string(indent));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, UDM_CONFIG_INSTANCE_ID_LABEL,
          inner_width, m_instance_id.get_value()));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, UDM_CONFIG_PID_DIRECTORY_LABEL,
          inner_width, m_pid_directory.get_value()));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, UDM_CONFIG_UDM_NAME_LABEL,
          inner_width, m_udm_name.get_value()));

  return out;
}

//------------------------------------------------------------------------------
const uint32_t udm::get_instance_id() const {
  return m_instance_id.get_value();
}
//------------------------------------------------------------------------------
const std::string udm::get_pid_directory() const {
  return m_pid_directory.get_value();
}
//------------------------------------------------------------------------------
const std::string udm::get_udm_name() const {
  return m_udm_name.get_value();
}

//------------------------------------------------------------------------------
udm_config_yaml::udm_config_yaml(
    const std::string& config_path, bool log_stdout, bool log_rot_file)
    : oai::config::config(
          config_path, oai::config::UDM_CONFIG_NAME, log_stdout, log_rot_file) {
  m_used_sbi_values = {
      oai::config::UDM_CONFIG_NAME, oai::config::UDR_CONFIG_NAME,
      oai::config::NRF_CONFIG_NAME};
  m_used_config_values = {
      oai::config::LOG_LEVEL_CONFIG_NAME, oai::config::REGISTER_NF_CONFIG_NAME,
      oai::config::NF_CONFIG_HTTP_NAME,   oai::config::NF_CONFIG_CURL_TIMEOUT,
      oai::config::NF_LIST_CONFIG_NAME,   oai::config::UDM_CONFIG_NAME};

  // TODO with NF_Type and switch
  // TODO: Still we need to add default NFs even we don't use this in all_in_one
  // use case
  auto m_udm = std::make_shared<udm>(
      "UDM", "oai-udm", sbi_interface("SBI", "oai-udm", 80, "v1", "eth0"));
  add_nf(oai::config::UDM_CONFIG_NAME, m_udm);

  auto m_udr = std::make_shared<nf>(
      "UDR", "oai-udr", sbi_interface("SBI", "oai-udr", 80, "v1", "eth0"));
  add_nf(oai::config::UDR_CONFIG_NAME, m_udr);

  auto m_nrf = std::make_shared<nf>(
      "NRF", "oai-nrf", sbi_interface("SBI", "oai-nrf", 80, "v1", "eth0"));
  add_nf(oai::config::NRF_CONFIG_NAME, m_nrf);

  update_used_nfs();
}

//------------------------------------------------------------------------------
udm_config_yaml::~udm_config_yaml() {}

void udm_config_yaml::pre_process() {
  // Process configuration information to display only the appropriate
  // information
  // TODO: discover UDR via NRF
  std::shared_ptr<nf> udr = get_nf(oai::config::UDR_CONFIG_NAME);
  udr->set_config();
}

//------------------------------------------------------------------------------
void udm_config_yaml::to_udm_config(oai::udm::config::udm_config& cfg) {
  std::shared_ptr<udm> udm_local = std::static_pointer_cast<udm>(get_local());
  cfg.instance                   = udm_local->get_instance_id();
  cfg.pid_dir                    = udm_local->get_pid_directory();
  cfg.udm_name                   = udm_local->get_udm_name();
  cfg.log_level                  = spdlog::level::from_str(log_level());
  cfg.register_nrf               = register_nrf();
  cfg.curl_timeout               = get_curl_timeout();

  cfg.use_fqdn_dns = false;  // TODO: to be removed
  if (get_http_version() == 2) cfg.use_http2 = true;

  cfg.sbi.api_version = local().get_sbi().get_api_version();
  cfg.sbi_http2_port  = local().get_sbi().get_port();
  cfg.sbi.port        = local().get_sbi().get_port();
  cfg.sbi.addr4       = local().get_sbi().get_addr4();
  cfg.sbi.if_name     = local().get_sbi().get_if_name();

  if (get_nf(oai::config::NRF_CONFIG_NAME)) {
    cfg.nrf_addr.api_version =
        get_nf(oai::config::NRF_CONFIG_NAME)->get_sbi().get_api_version();
    cfg.nrf_addr.uri_root = get_nf(oai::config::NRF_CONFIG_NAME)->get_url();
  }
  if (get_nf(oai::config::UDR_CONFIG_NAME)) {
    cfg.udr_addr.api_version =
        get_nf(oai::config::UDR_CONFIG_NAME)->get_sbi().get_api_version();
    cfg.udr_addr.uri_root = get_nf(oai::config::UDR_CONFIG_NAME)->get_url();
  }
}
}  // namespace oai::config
