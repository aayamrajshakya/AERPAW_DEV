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

#include "nrf_config_types.hpp"

#include "logger.hpp"
#include "nrf_config.hpp"

using namespace oai::config::nrf;

//------------------------------------------------------------------------------
nrf_config_type::nrf_config_type(
    const std::string& name, const std::string& host, const sbi_interface& sbi)
    : nf(name, host, sbi) {
  m_config_name = "NRF Config";
  m_heartbeat   = int_config_value(
      NRF_CONFIG_HEARTBEAT_LABEL, 10);  // Default value: 10 seconds
  m_heartbeat.set_validation_interval(1, 65535);
  m_suspended_nf_interval = int_config_value(
      NRF_CONFIG_SUSPENDED_NF_INTERVAL_LABEL,
      300);  // Default value: 300 seconds (5 minutes)
  m_suspended_nf_interval.set_validation_interval(20, 65535);
}

//------------------------------------------------------------------------------
void nrf_config_type::from_yaml(const YAML::Node& node) {
  nf::from_yaml(node);

  // Load NRF specified parameter
  for (const auto& elem : node) {
    auto key = elem.first.as<std::string>();

    if (key == NRF_CONFIG_HEARTBEAT) {
      m_heartbeat.from_yaml(elem.second);
    }
    if (key == NRF_CONFIG_SUSPENDED_NF_INTERVAL) {
      m_suspended_nf_interval.from_yaml(elem.second);
    }
  }
}

//------------------------------------------------------------------------------
nlohmann::json nrf_config_type::to_json() {
  nlohmann::json json_data                 = {};
  json_data                                = nf::to_json();
  json_data[m_heartbeat.get_config_name()] = m_heartbeat.to_json();
  json_data[m_suspended_nf_interval.get_config_name()] =
      m_suspended_nf_interval.to_json();
  return json_data;
}

//------------------------------------------------------------------------------
bool nrf_config_type::from_json(const nlohmann::json& json_data) {
  try {
    nf::from_json(json_data);
    if (json_data.find(m_heartbeat.get_config_name()) != json_data.end()) {
      m_heartbeat.from_json(json_data[m_heartbeat.get_config_name()]);
    }
    if (json_data.find(m_suspended_nf_interval.get_config_name()) !=
        json_data.end()) {
      m_suspended_nf_interval.from_json(
          json_data[m_suspended_nf_interval.get_config_name()]);
    }
    return true;
  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

//------------------------------------------------------------------------------
std::string nrf_config_type::to_string(const std::string& indent) const {
  std::string out          = {};
  std::string inner_indent = indent + indent;
  unsigned int inner_width = get_inner_width(inner_indent.length());
  out.append(nf::to_string(indent));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, m_heartbeat.get_config_name(),
          inner_width, std::to_string(m_heartbeat.get_value()) + " (seconds)"));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM,
          m_suspended_nf_interval.get_config_name(), inner_width,
          std::to_string(m_suspended_nf_interval.get_value()) + " (seconds)"));

  return out;
}

//------------------------------------------------------------------------------
void nrf_config_type::validate() {
  nf::validate();
  m_heartbeat.validate();
  m_suspended_nf_interval.validate();
}

//------------------------------------------------------------------------------
uint16_t nrf_config_type::get_heartbeat() const {
  return m_heartbeat.get_value();
}

//------------------------------------------------------------------------------
uint16_t nrf_config_type::get_suspended_nf_interval() const {
  return m_suspended_nf_interval.get_value();
}
