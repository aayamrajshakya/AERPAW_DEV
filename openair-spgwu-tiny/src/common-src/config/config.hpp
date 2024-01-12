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

/*! \file config.hpp
 \brief
 \author  Stefan Spettel
 \company phine.tech
 \date 2022
 \email: stefan.spettel@phine.tech
*/

#pragma once

#include "config_types.hpp"
#include "logger_base.hpp"

#include <string>
#include <map>
#include <memory>
#include <shared_mutex>

namespace oai::config {

const int COLUMN_WIDTH           = 30;
const std::string BASE_FORMATTER = " {} {:.<{}}: {}\n";
const std::string LOGGER_NAME    = "config ";

class config_iface {
 public:
  /**
   * Sets a string configuration, adds if not existing or overrides if exists
   * Takes ownership of configuration, becomes null after call to this function
   * @param name name of the configuration
   * @param val value of the configuration
   */
  virtual void set_configuration(
      const std::string& name, std::unique_ptr<config_type> val) = 0;

  /**
   * Sets a configuration of any type with name to be mandatory, used for
   * validation
   * @param name name of the configuration
   */
  virtual void set_configuration_mandatory(const std::string& name) = 0;

  /**
   * Validates the configuration:
   *  - All configurations set as mandatory must be present
   *  - All present configurations must pass their type-specific validation
   * @return True if validation passed, false otherwise
   */
  [[nodiscard]] virtual bool validate() const = 0;

  /**
   * Returns a string representation of the configuration
   * @return string representation
   */
  [[nodiscard]] virtual std::string to_string() const = 0;

  // The get method is not defined here, as we cannot have virtual template
  // members Annoying that I cannot have a virtual template method just called
  // "get"
  /**
   * Gets a string base configuration
   * @throws std::invalid_argument when name does not exist in configuration
   * @param name of the configuration
   * @return value
   */
  [[nodiscard]] virtual const std::string& get_base_conf_val(
      const std::string& name) const = 0;

  /**
   * Gets a boolean configuration
   * @throws std::invalid_argument when name does not exist in configuration
   * @param name of the configuration
   * @return value
   */
  [[nodiscard]] virtual bool get_support_feature(
      const std::string& name) const = 0;

  /**
   * Gets a SBI interface configuration
   * @throws std::invalid_argument when name does not exist in configuration
   * @param name of the configuration
   * @return value
   */
  [[nodiscard]] virtual const sbi_interface& get_sbi_interface(
      const std::string& name) const = 0;

  /**
   * Gets a local SBI interface configuration
   * @throws std::invalid_argument when name does not exist in configuration
   * @param name of the configuration
   * @return value
   */
  [[nodiscard]] virtual const local_sbi_interface& get_local_sbi_interface(
      const std::string& name) const = 0;

  /**
   * Gets a local interface configuration
   * @throws std::invalid_argument when name does not exist in configuration
   * @param name of the configuration
   * @return value
   */
  [[nodiscard]] virtual const local_interface& get_local_interface(
      const std::string& name) const = 0;

  /**
   * Gets a uint8 configuration value
   * @throws std::invalid_argument when name does not exist in configuration
   * @param name of the configuration
   * @return value
   */
  [[nodiscard]] virtual uint8_t get_uint8_conf_val(
      const std::string& name) const = 0;

  /**
   * Gets a uint8 configuration
   * @throws std::invalid_argument when name does not exist in configuration
   * @param name of the configuration
   * @return value
   */
  [[nodiscard]] virtual const uint8_config_value& get_uint8_conf(
      const std::string& name) const = 0;

  /**
   * Displays the to_string method to the config logger
   */
  virtual void display() const = 0;

  virtual ~config_iface() = default;
};

class config : public config_iface {
 public:
  explicit config(
      const std::string& nf_name, bool log_stdout, bool log_rot_file) {
    logger::logger_registry::register_logger(
        nf_name, LOGGER_NAME, log_stdout, log_rot_file);
    m_nf_name = nf_name;
  }

  void set_configuration(
      const std::string& name, std::unique_ptr<config_type> val) override;

  void set_configuration_mandatory(const std::string& name) override;

  [[nodiscard]] bool validate() const override;

  [[nodiscard]] std::string to_string() const override;

  /**
   * Gets configuration of type T, must be derived from conf_type
   * @tparam T sub_type of conf_type
   * @param name of the configuration
   * @throws std::invalid_argument when name does not exist in configuration
   * @return configuration of type T
   */
  template<typename T>
  [[nodiscard]] const T& get(const std::string& name) const;

  [[nodiscard]] const std::string& get_base_conf_val(
      const std::string& name) const override;

  [[nodiscard]] bool get_support_feature(
      const std::string& name) const override;

  [[nodiscard]] const sbi_interface& get_sbi_interface(
      const std::string& name) const override;

  [[nodiscard]] const local_interface& get_local_interface(
      const std::string& name) const override;

  [[nodiscard]] const local_sbi_interface& get_local_sbi_interface(
      const std::string& name) const override;

  [[nodiscard]] uint8_t get_uint8_conf_val(
      const std::string& name) const override;

  [[nodiscard]] const uint8_config_value& get_uint8_conf(
      const std::string& name) const override;

  void display() const override;

 private:
  std::map<std::string, std::unique_ptr<config_type>> m_config;

  std::vector<std::string> m_mandatory_keys;

  mutable std::shared_mutex m_config_mutex;

  std::string m_nf_name;
};

}  // namespace oai::config