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

#pragma once

#include "config.hpp"

constexpr auto NRF_CONFIG_HEARTBEAT                   = "heartbeat";
constexpr auto NRF_CONFIG_HEARTBEAT_LABEL             = "Heartbeat";
constexpr auto NRF_CONFIG_SUSPENDED_NF_INTERVAL       = "suspended_nf_interval";
constexpr auto NRF_CONFIG_SUSPENDED_NF_INTERVAL_LABEL = "Suspended NF Interval";

namespace oai::config::nrf {

class nrf_config_type : public oai::config::nf {
  friend class nrf_config;

 private:
  int_config_value m_heartbeat;
  int_config_value m_suspended_nf_interval;

 public:
  explicit nrf_config_type(
      const std::string& name, const std::string& host,
      const sbi_interface& sbi);
  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;

  void validate() override;

  [[nodiscard]] uint16_t get_heartbeat() const;
  void set_heartbeat(uint16_t);

  [[nodiscard]] uint16_t get_suspended_nf_interval() const;
  void set_suspended_nf_interval(uint16_t);
};

}  // namespace oai::config::nrf
