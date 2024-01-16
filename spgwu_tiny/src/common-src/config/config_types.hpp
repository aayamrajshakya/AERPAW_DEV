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

/*! \file config_types.hpp
 \brief
 \author  Stefan Spettel
 \company phine.tech
 \date 2022
 \email: stefan.spettel@phine.tech
*/

#pragma once

#include "string"
#include <netinet/in.h>
#include <vector>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace oai::config {

const std::vector<std::string> allowed_api_versions{"v1", "v2"};

const std::string URL_REGEX = "http://.*:[0-9]*";

// Only used for pretty-printing
enum class config_type_e { string, option, sbi, local, invalid, uint8 };

class config_type {
  friend class yaml_file_iface;

 public:
  /**
   * Returns a string representation of the config. The indent is prepended at
   * each line
   * @param indent to be prepended
   * @return string representation
   */
  [[nodiscard]] virtual std::string to_string(
      const std::string& indent) const = 0;

  /**
   * Validates the configuration and marks the configuration as set if
   * successful
   * @return true if validation successful, false otherwise
   */
  [[nodiscard]] virtual bool validate() = 0;

  /**
   * Gets the config type of this config
   * @return config_type_e
   */
  [[nodiscard]] virtual config_type_e get_config_type() const = 0;

  /**
   * Checks if the configuration is set. Configuration is not set if it has not
   * been validated.
   * @return true if set, false otherwise
   */
  [[nodiscard]] virtual bool is_set() const;

  /**
   * Helper function to match a regex
   * @param value to match again
   * @param regex that is used for the matching
   * @return true if matched, false otherwise
   */
  static bool matches_regex(const std::string& value, const std::string& regex);

  virtual ~config_type() = default;

 protected:
  bool m_set = false;
};

class string_config_value : public config_type {
  friend class factory;

 private:
  std::string m_value;
  std::string m_regex;

 public:
  explicit string_config_value(YAML::Node const&);
  string_config_value() = default;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  [[nodiscard]] bool validate() override;
  [[nodiscard]] config_type_e get_config_type() const override;

  [[nodiscard]] const std::string& get_value() const;
};

class option_config_value : public config_type {
  friend class factory;

 private:
  bool m_value = false;

 public:
  explicit option_config_value(YAML::Node const&);
  option_config_value() = default;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  [[nodiscard]] bool validate() override;
  [[nodiscard]] config_type_e get_config_type() const override;

  [[nodiscard]] bool get_value() const;
};

class uint8_config_value : public config_type {
  friend class factory;

 private:
  uint8_t m_value     = 0;
  uint8_t m_min_value = 0;
  uint8_t m_max_value = UINT8_MAX;

 public:
  explicit uint8_config_value(YAML::Node const&);
  uint8_config_value() = default;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  [[nodiscard]] bool validate() override;
  [[nodiscard]] config_type_e get_config_type() const override;

  [[nodiscard]] uint8_t get_value() const;
  void set_validation_min_value(uint8_t val);
  void set_validation_max_value(uint8_t val);
};

class network_interface : public config_type {
 public:
  static bool validate_sbi_api_version(const std::string& v);
};

class sbi_interface : public network_interface {
 private:
  std::string m_api_version;
  std::string m_url;

 public:
  explicit sbi_interface(YAML::Node const&);
  sbi_interface() = default;

  [[nodiscard]] bool validate() override;
  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  [[nodiscard]] config_type_e get_config_type() const override;

  [[nodiscard]] const std::string& get_api_version() const;
  [[nodiscard]] const std::string& get_url() const;
};

class local_interface : public network_interface {
  friend class local_sbi_interface;

 private:
  std::string m_if_name{};
  in_addr m_addr4{};
  in6_addr m_addr6{};
  unsigned int m_mtu{};
  uint16_t m_port{};

 public:
  explicit local_interface(YAML::Node const&);
  local_interface() = default;

  [[nodiscard]] bool validate() override;
  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  [[nodiscard]] config_type_e get_config_type() const override;

  [[nodiscard]] const std::string& get_if_name() const;
  [[nodiscard]] const in_addr& get_addr4() const;
  [[nodiscard]] const in6_addr& get_addr6() const;
  [[nodiscard]] unsigned int get_mtu() const;
  [[nodiscard]] uint16_t get_port() const;
};

class local_sbi_interface : public local_interface {
 private:
  std::string m_api_version{};
  uint8_t m_http_version = 1;
  uint16_t m_port_http2{};

 public:
  explicit local_sbi_interface(YAML::Node const&);
  local_sbi_interface() = default;

  [[nodiscard]] bool validate() override;
  [[nodiscard]] std::string to_string(const std::string& indent) const override;

  [[nodiscard]] const std::string& get_api_version() const;
  [[nodiscard]] uint8_t http_version() const;
  [[nodiscard]] uint16_t get_port_http2() const;
};

class factory {
 public:
  static option_config_value get_option_config(bool val);

  static string_config_value get_string_config(const std::string& val);

  static uint8_config_value get_uint8_config(uint8_t val);
};

}  // namespace oai::config
