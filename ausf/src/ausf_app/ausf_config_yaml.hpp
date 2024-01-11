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
#include "ausf_config.hpp"

constexpr auto AUSF_CONFIG_INSTANCE_ID         = "instance_id";
constexpr auto AUSF_CONFIG_INSTANCE_ID_LABEL   = "Instance ID";
constexpr auto AUSF_CONFIG_PID_DIRECTORY       = "pid_directory";
constexpr auto AUSF_CONFIG_PID_DIRECTORY_LABEL = "PID Directory";
constexpr auto AUSF_CONFIG_AUSF_NAME           = "ausf_name";
constexpr auto AUSF_CONFIG_AUSF_NAME_LABEL     = "AUSF Name";

namespace oai::config {

class ausf : public nf {
 private:
  int_config_value m_instance_id;
  string_config_value m_pid_directory;
  string_config_value m_ausf_name;

 public:
  explicit ausf(
      const std::string& name, const std::string& host,
      const sbi_interface& sbi);

  void from_yaml(const YAML::Node& node) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  [[nodiscard]] const uint32_t get_instance_id() const;
  [[nodiscard]] const std::string get_pid_directory() const;
  [[nodiscard]] const std::string get_ausf_name() const;
};

class ausf_config_yaml : public config {
 public:
  explicit ausf_config_yaml(
      const std::string& config_path, bool log_stdout, bool log_rot_file);
  virtual ~ausf_config_yaml();

  void to_ausf_config(oai::config::ausf_config& cfg);
  void pre_process();
};
}  // namespace oai::config
