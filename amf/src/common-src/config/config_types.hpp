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
 \date 2023
 \email: stefan.spettel@phine.tech
*/

#pragma once

#include "string"
#include "pdu_session_type.hpp"
#include <netinet/in.h>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

namespace oai::config {
const std::string INNER_LIST_ELEM = "+";
const std::string OUTER_LIST_ELEM = "-";
const std::string EMPTY_LIST_ELEM = " ";

class config_type {
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
   * Validates the configuration
   * @throws std::runtime_error if validation is not successful
   */
  virtual void validate(){};

  /**
   * Overwrites the values from a YAML node
   * @param node YAML node, non-existing values are not evaluated, but types
   * must be correct
   * @throws YAML::Exception if YAML conversion failed
   */
  virtual void from_yaml(const YAML::Node& node) = 0;

  /**
   * Convert to JSON format
   * @return void
   */
  virtual nlohmann::json to_json();
  /**
   * Get value from JSON
   * @return void
   */
  virtual bool from_json(const nlohmann::json& json_data);

  /**
   * Checks if the configuration is set. Configuration is not set if it has not
   * been validated.
   * @return true if set, false otherwise
   */
  [[nodiscard]] virtual bool is_set() const;

  /**
   * Sets the configuration
   */
  virtual void set_config();

  /**
   * Unsets the configuration
   */
  virtual void unset_config();

  /**
   * Returns the name of this configuration in a user-friendly representation
   * @return
   */
  [[nodiscard]] virtual const std::string& get_config_name() const;

  /**
   * Sets the configuration name
   *  @param const std::string&: Name of the config_type
   */
  virtual void set_config_name(const std::string& name);

  virtual ~config_type() = default;

 protected:
  bool m_set = false;
  std::string m_config_name;

  static unsigned int get_inner_width(unsigned int indent_length);

  static std::string add_indent(const std::string& indent);

  static in_addr safe_convert_ip(const std::string& ipv4_string);

  static void get_ipv4_range(
      const in_addr& ipv4_subnet, const uint8_t& ipv4_prefix, in_addr& start_ip,
      in_addr& end_ip);

  static in6_addr safe_convert_ip6(const std::string& ipv6_string);
};

class string_config_value : public config_type {
 private:
  std::string m_value;
  std::string m_regex = ".*";

 public:
  explicit string_config_value(
      const std::string& name, const std::string& value);
  string_config_value() = default;

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  void validate() override;

  [[nodiscard]] const std::string& get_value() const;
  void set_validation_regex(const std::string& regex);
};

class option_config_value : public config_type {
 private:
  bool m_value = false;

 public:
  explicit option_config_value(const std::string& name, bool value);
  option_config_value() = default;

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;

  [[nodiscard]] bool get_value() const;
};

class int_config_value : public config_type {
 private:
  int m_value     = 0;
  int m_min_value = 0;
  int m_max_value = INT32_MAX;

 public:
  explicit int_config_value(const std::string& name, int value);
  int_config_value() = default;

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  void validate() override;

  [[nodiscard]] int get_value() const;
  void set_validation_interval(int min, int max);
};

class local_interface : public config_type {
  friend class nf;

 protected:
  string_config_value m_host{};
  int_config_value m_port{};
  [[nodiscard]] std::string to_string_for_local(
      const std::string& indent) const;

  // these values are read from the m_if_name
  in_addr m_addr4{};
  in6_addr m_addr6{};

 private:
  string_config_value m_if_name{};
  unsigned int m_mtu{};
  bool m_is_local_interface = false;

 public:
  explicit local_interface(
      const std::string& name, const std::string& host, uint16_t port,
      const std::string& if_name);

  local_interface() = default;

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;
  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  void validate() override;

  [[nodiscard]] const std::string& get_host() const;
  [[nodiscard]] const std::string& get_if_name() const;
  [[nodiscard]] const in_addr& get_addr4() const;
  [[nodiscard]] const in6_addr& get_addr6() const;
  [[nodiscard]] unsigned int get_mtu() const;
  [[nodiscard]] uint16_t get_port() const;

  void set_is_local_interface(bool val);
  [[nodiscard]] bool is_local_interface() const;
  void set_host(const std::string& host);
};

class sbi_interface : public local_interface {
  friend class nf;

 private:
  string_config_value m_api_version;
  std::string m_url;

  void set_url();

 public:
  explicit sbi_interface(
      const std::string& name, const std::string& host, uint16_t port,
      const std::string& api_version, const std::string& interface_name);

  sbi_interface() = default;

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;
  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  void validate() override;

  [[nodiscard]] const std::string& get_api_version() const;
  [[nodiscard]] const std::string& get_url() const;
};

enum class interface_type_e { n1, n4 };

class nf : public config_type {
  friend class config;

 private:
  sbi_interface m_sbi;
  string_config_value m_host;

  std::string m_url;  // Moved from SBI interface
  void set_url();     // Moved from SBI interface

 public:
  explicit nf(
      const std::string& name, const std::string& host,
      const sbi_interface& sbi);
  explicit nf() = default;

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  void validate() override;
  [[nodiscard]] const sbi_interface& get_sbi() const;
  [[nodiscard]] const std::string& get_host() const;
  [[nodiscard]] const std::string& get_url() const;
};

class nf_features_config : public config_type {
 private:
  // TODO: either have string or option
  string_config_value m_string_value{};
  option_config_value m_option_value{};
  std::string m_nf_name;

  void set_value(const YAML::Node& node);

 public:
  explicit nf_features_config(
      const std::string& name, const std::string& nf_name, bool value);

  explicit nf_features_config(
      const std::string& name, const std::string& nf_name,
      const std::string& value);

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  void validate() override;
  void set_validation_regex(const std::string& regex);
  [[nodiscard]] bool get_option() const;
  [[nodiscard]] const std::string& get_string() const;
};

class database_config : public config_type {
 private:
  string_config_value m_host;
  int_config_value m_port;
  string_config_value m_user;
  string_config_value m_pass;
  string_config_value m_database_name;
  string_config_value m_database_type;
  int_config_value m_connection_timeout;

 protected:
  [[nodiscard]] int_config_value get_default_database_port(
      const string_config_value& database_type) const;

 public:
  explicit database_config(const std::string& name);

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;
  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  void validate() override;

  [[nodiscard]] const std::string& get_host() const;
  [[nodiscard]] int get_port() const;
  [[nodiscard]] const std::string& get_user() const;
  [[nodiscard]] const std::string& get_pass() const;
  [[nodiscard]] const std::string& get_database_name() const;
  [[nodiscard]] const std::string& get_database_type() const;
  [[nodiscard]] int get_connection_timeout() const;
};

class ue_dns : public config_type {
 private:
  string_config_value m_primary_dns_v4;
  string_config_value m_secondary_dns_v4;
  string_config_value m_primary_dns_v6;
  string_config_value m_secondary_dns_v6;

  // generated values
  in_addr m_primary_dns_v4_ip{};
  in_addr m_secondary_dns_v4_ip{};
  in6_addr m_primary_dns_v6_ip{};
  in6_addr m_secondary_dns_v6_ip{};

 public:
  explicit ue_dns(
      const std::string& primary_dns_v4, const std::string& secondary_dns_v4,
      const std::string& primary_dns_v6, const std::string& secondary_dns_v6);

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;

  void validate() override;

  [[nodiscard]] const in_addr& get_primary_dns_v4() const;
  [[nodiscard]] const in_addr& get_secondary_dns_v4() const;
  [[nodiscard]] const in6_addr& get_primary_dns_v6() const;
  [[nodiscard]] const in6_addr& get_secondary_dns_v6() const;
};

// TODO we should just use the DnnConfiguration data structure, but that
// requires a lot of changes in the using classes
class dnn_config : public config_type {
 private:
  string_config_value m_dnn;
  string_config_value m_pdu_session_type;
  string_config_value m_ipv4_subnet;
  string_config_value m_ipv6_prefix;
  ue_dns m_ue_dns;

  // generated
  int m_ipv4_prefix{};
  in_addr m_ipv4_subnet_ip{};

  in_addr m_ipv4_pool_start_ip{};
  in_addr m_ipv4_pool_end_ip{};
  in6_addr m_ipv6_prefix_ip{};

 private:
  uint8_t m_ipv6_prefix_length{};
  pdu_session_type_t m_pdu_session_type_generated;

 public:
  explicit dnn_config(
      const std::string& dnn, const std::string& pdu_type,
      const std::string& ipv4_subnet, const std::string& ipv6_prefix);

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;

  void validate() override;

  [[nodiscard]] const in_addr& get_ipv4_pool_start() const;
  [[nodiscard]] const in_addr& get_ipv4_pool_end() const;
  [[nodiscard]] const in_addr& get_ipv4_subnet() const;
  [[nodiscard]] const in_addr& get_ipv4_subnet_mask() const;
  [[nodiscard]] const int& get_ipv4_subnet_prefix() const;
  [[nodiscard]] const in6_addr& get_ipv6_prefix() const;
  [[nodiscard]] uint8_t get_ipv6_prefix_length() const;
  [[nodiscard]] const pdu_session_type_t& get_pdu_session_type() const;
  [[nodiscard]] const std::string& get_dnn() const;
  [[nodiscard]] const ue_dns& get_ue_dns() const;
  void set_ue_dns(const ue_dns& dns);
};

class nf_http_version : public config_type {
 private:
  string_config_value m_version{};

 public:
  explicit nf_http_version();

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  void validate() override;
  [[nodiscard]] const std::string& get_http_version() const;
};

class curl_timeout : public config_type {
 private:
  int_config_value m_curl_timeout{};

 public:
  explicit curl_timeout();

  void from_yaml(const YAML::Node& node) override;
  nlohmann::json to_json() override;
  bool from_json(const nlohmann::json& json_data) override;

  [[nodiscard]] std::string to_string(const std::string& indent) const override;
  void validate() override;
  [[nodiscard]] const uint32_t get() const;
};

}  // namespace oai::config
