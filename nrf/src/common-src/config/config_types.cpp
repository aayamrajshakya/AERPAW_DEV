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

/*! \file config_types.cpp
 \brief
 \author  Stefan Spettel
 \company phine.tech
 \date 2022
 \email: stefan.spettel@phine.tech
*/

#include "config_types.hpp"
#include "config.hpp"
#include "conversions.hpp"
#include "if.hpp"
#include "common_defs.h"

#include <fmt/format.h>
#include <string>

#include <regex>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace oai::config;

bool config_type::is_set() const {
  return m_set;
}

void config_type::set_config() {
  m_set = true;
}

void config_type::unset_config() {
  m_set = false;
}

const std::string& config_type::get_config_name() const {
  return m_config_name;
}

void config_type::set_config_name(const std::string& name) {
  m_config_name = name;
}

unsigned int config_type::get_inner_width(unsigned int indent_length) {
  unsigned int inner_width = COLUMN_WIDTH;
  if (indent_length < COLUMN_WIDTH) {
    inner_width = COLUMN_WIDTH - indent_length;
  }

  return inner_width;
}

in_addr config_type::safe_convert_ip(const std::string& ipv4_string) {
  in_addr ip = conv::fromString(ipv4_string);

  if (!ip.s_addr) {
    throw std::runtime_error(
        fmt::format(+"The IP address {} is not valid", ipv4_string));
  }
  return ip;
}

void config_type::get_ipv4_range(
    const in_addr& ipv4_domain, const uint8_t& ipv4_prefix, in_addr& start_ip,
    in_addr& end_ip) {
  uint32_t ipv4_subnet = ntohl(ipv4_domain.s_addr);

  uint32_t startIP, endIP;

  startIP = ipv4_subnet & ~(0xFFFFFFFF >> ipv4_prefix);
  endIP   = startIP | (0xFFFFFFFF >> ipv4_prefix);

  std::string start = std::to_string((startIP >> 24) & 0xFF) + "." +
                      std::to_string((startIP >> 16) & 0xFF) + "." +
                      std::to_string((startIP >> 8) & 0xFF) + "." +
                      std::to_string(startIP & 0xFF);

  std::string end = std::to_string((endIP >> 24) & 0xFF) + "." +
                    std::to_string((endIP >> 16) & 0xFF) + "." +
                    std::to_string((endIP >> 8) & 0xFF) + "." +
                    std::to_string(endIP & 0xFF);

  start_ip = conv::fromString(start);
  end_ip   = conv::fromString(end);

  start_ip.s_addr = start_ip.s_addr + be32toh(1);
  end_ip.s_addr   = end_ip.s_addr - be32toh(1);
}

in6_addr config_type::safe_convert_ip6(const std::string& ipv6_string) {
  in6_addr ip = conv::fromStringV6(ipv6_string);
  if (IN6_IS_ADDR_UNSPECIFIED(&ip)) {
    throw std::runtime_error(
        fmt::format(+"The IPv6 address {} is not valid", ipv6_string));
  }
  return ip;
}

std::string config_type::add_indent(const std::string& indent) {
  std::string base_indent = fmt::format("{:<{}}", "", INDENT_WIDTH);
  return base_indent + indent;
}

nlohmann::json config_type::to_json() {
  return {};
}

bool config_type::from_json(const nlohmann::json&) {
  return false;
}

string_config_value::string_config_value(
    const std::string& name, const std::string& value) {
  m_config_name = name;
  m_value       = value;
  m_set         = true;
}

void string_config_value::from_yaml(const YAML::Node& node) {
  m_value = node.as<std::string>();
  m_set   = true;
}

nlohmann::json string_config_value::to_json() {
  nlohmann::json json_data = {};
  json_data                = m_value;
  return json_data;
}

bool string_config_value::from_json(const nlohmann::json& json_data) {
  try {
    m_value = json_data.get<std::string>();
  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string string_config_value::to_string(const std::string&) const {
  std::string out;
  return out.append(m_value);
}

void string_config_value::validate() {
  if (!m_set) return;
  std::regex re(m_regex);
  if (!std::regex_match(m_value, re)) {
    throw std::runtime_error(fmt::format(
        "{} (value: {}) does not follow the regex specification: {}",
        m_config_name, m_value, m_regex));
  }
}

const std::string& string_config_value::get_value() const {
  return m_value;
}

void string_config_value::set_validation_regex(const std::string& regex) {
  m_regex = regex;
}

option_config_value::option_config_value(const std::string& name, bool value) {
  m_config_name = name;
  m_value       = value;
  m_set         = true;
}

void option_config_value::from_yaml(const YAML::Node& node) {
  m_value = node.as<bool>();
  m_set   = true;
}

nlohmann::json option_config_value::to_json() {
  nlohmann::json json_data = {};
  json_data                = m_value;
  return json_data;
}

bool option_config_value::from_json(const nlohmann::json& json_data) {
  try {
    m_value = json_data.get<bool>();

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string option_config_value::to_string(const std::string&) const {
  std::string val = m_value ? "Yes" : "No";
  return val;
}

bool option_config_value::get_value() const {
  return m_value;
}

int_config_value::int_config_value(const std::string& name, int value) {
  m_config_name = name;
  m_value       = value;
  m_set         = true;
}

void int_config_value::from_yaml(const YAML::Node& node) {
  m_value = node.as<int>();
  m_set   = true;
}

nlohmann::json int_config_value::to_json() {
  nlohmann::json json_data = {};
  json_data                = m_value;
  return json_data;
}

bool int_config_value::from_json(const nlohmann::json& json_data) {
  try {
    m_value = json_data.get<int>();

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string int_config_value::to_string(const std::string&) const {
  return std::to_string(m_value);
}

void int_config_value::validate() {
  if (!m_set) return;
  if (m_value < m_min_value || m_value > m_max_value) {
    throw std::runtime_error(fmt::format(
        "{} (value: {}) must be in interval [{},{}]", m_config_name, m_value,
        m_min_value, m_max_value));
  }
}

int int_config_value::get_value() const {
  return m_value;
}

void int_config_value::set_validation_interval(int min, int max) {
  m_min_value = min;
  m_max_value = max;
}

local_interface::local_interface(
    const std::string& name, const std::string& host, uint16_t port,
    const std::string& if_name) {
  m_host        = string_config_value("host", host);
  m_config_name = name;
  m_if_name     = string_config_value("interface_name", if_name);
  m_port        = int_config_value("port", port);
  m_port.set_validation_interval(PORT_MIN_VALUE, PORT_MAX_VALUE);
  m_host.set_validation_regex(HOST_VALIDATOR_REGEX);
  m_set                = true;
  m_is_local_interface = true;
}

void local_interface::from_yaml(const YAML::Node& node) {
  if (node["port"]) {
    m_port.from_yaml(node["port"]);
  }
  if (node["interface_name"]) {
    m_if_name.from_yaml(node["interface_name"]);
  }
  m_set                = true;
  m_is_local_interface = true;
}

nlohmann::json local_interface::to_json() {
  nlohmann::json json_data               = {};
  json_data[m_if_name.get_config_name()] = m_if_name.to_json();
  if (!m_is_local_interface) return json_data;
  json_data[m_port.get_config_name()] = m_port.to_json();
  json_data["mtu"]                    = m_mtu;
  json_data["addr4"]                  = conv::toString(m_addr4);
  return json_data;
}

bool local_interface::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_host.get_config_name()) != json_data.end()) {
      m_host.from_json(json_data[m_host.get_config_name()]);
    }
    if (json_data.find(m_port.get_config_name()) != json_data.end()) {
      m_port.from_json(json_data[m_port.get_config_name()]);
    }
    if (json_data.find(m_if_name.get_config_name()) != json_data.end()) {
      m_if_name.from_json(json_data[m_if_name.get_config_name()]);
    }
    if (json_data.find("mtu") != json_data.end()) {
      m_mtu = json_data["mtu"].get<int>();
    }
    // TODO: IP Addr

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string local_interface::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = get_inner_width(indent.length());

  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, "Port", inner_width,
      m_port.get_value()));

  if (!m_is_local_interface) return out;
  out.append(to_string_for_local(indent));

  return out;
}

std::string local_interface::to_string_for_local(
    const std::string& indent) const {
  unsigned int inner_width = get_inner_width(indent.length());
  std::string out;

  std::string ip4 = conv::toString(m_addr4);
  std::string ip6 = conv::toString(m_addr6);

  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, "IPv4 Address ", inner_width, ip4));
  if (ip6 != "::") {
    out.append(indent).append(fmt::format(
        BASE_FORMATTER, INNER_LIST_ELEM, "IPv6 Address", inner_width, ip6));
  }
  out.append(indent).append(
      fmt::format(BASE_FORMATTER, INNER_LIST_ELEM, "MTU", inner_width, m_mtu));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, "Interface name: ", inner_width,
      m_if_name.get_value()));
  return out;
}

void local_interface::validate() {
  if (!m_set) return;
  m_host.validate();
  m_port.validate();
  if (!m_is_local_interface) return;

  m_if_name.validate();

  unsigned int _mtu{};
  in_addr _addr4{};
  in_addr _netmask{};
  if (get_inet_addr_infos_from_iface(
          m_if_name.get_value(), _addr4, _netmask, _mtu) == RETURNerror) {
    throw std::runtime_error(fmt::format(
        "Error in reading network interface {}. Make sure it exists",
        m_if_name.get_value()));
  }
  m_mtu   = _mtu;
  m_addr4 = _addr4;
}

const std::string& local_interface::get_host() const {
  return m_host.get_value();
}

const std::string& local_interface::get_if_name() const {
  return m_if_name.get_value();
}

const in_addr& local_interface::get_addr4() const {
  return m_addr4;
}

const in6_addr& local_interface::get_addr6() const {
  return m_addr6;
}

unsigned int local_interface::get_mtu() const {
  return m_mtu;
}

uint16_t local_interface::get_port() const {
  return m_port.get_value();
}

void local_interface::set_is_local_interface(bool val) {
  m_is_local_interface = val;
}

bool local_interface::is_local_interface() const {
  return m_is_local_interface;
}

void local_interface::set_host(const std::string& host) {
  string_config_value host_val =
      string_config_value(m_host.get_config_name(), host);
  host_val.set_validation_regex(HOST_VALIDATOR_REGEX);
  m_host = host_val;
}

sbi_interface::sbi_interface(
    const std::string& name, const std::string& host, uint16_t port,
    const std::string& api_version, const std::string& interface_name)
    : local_interface(name, host, port, interface_name) {
  m_config_name = name;
  m_host        = string_config_value("host", host);
  m_api_version = string_config_value("api_version", api_version);

  m_host.set_validation_regex(HOST_VALIDATOR_REGEX);
  m_api_version.set_validation_regex(API_VERSION_REGEX);
  m_set = true;
  set_is_local_interface(false);
  set_url();
}

void sbi_interface::from_yaml(const YAML::Node& node) {
  local_interface::from_yaml(node);
  set_is_local_interface(false);

  if (node["api_version"]) {
    m_api_version.from_yaml(node["api_version"]);
  }
  set_url();
  m_set = true;
}

nlohmann::json sbi_interface::to_json() {
  nlohmann::json json_data = {};
  set_is_local_interface(false);
  json_data                                  = local_interface::to_json();
  json_data[m_api_version.get_config_name()] = m_api_version.to_json();
  json_data["url"]                           = m_url;
  return json_data;
}

bool sbi_interface::from_json(const nlohmann::json& json_data) {
  set_is_local_interface(false);
  try {
    local_interface::from_json(json_data);

    if (json_data.find(m_api_version.get_config_name()) != json_data.end()) {
      m_api_version.from_json(json_data[m_api_version.get_config_name()]);
    }

    if (json_data.find("url") != json_data.end()) {
      m_url = json_data["url"].get<std::string>();
    }

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string sbi_interface::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = get_inner_width(indent.length());

  out.append(indent).append(
      fmt::format(BASE_FORMATTER, INNER_LIST_ELEM, "URL", inner_width, m_url));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, "API Version", inner_width,
      m_api_version.get_value()));

  if (!is_local_interface()) return out;

  std::string ip4 = conv::toString(m_addr4);
  std::string ip6 = conv::toString(m_addr6);

  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, "IPv4 Address ", inner_width, ip4));
  if (ip6 != "::") {
    out.append(indent).append(fmt::format(
        BASE_FORMATTER, INNER_LIST_ELEM, "IPv6 Address", inner_width, ip6));
  }

  return out;
}

void sbi_interface::validate() {
  if (!m_set) return;
  local_interface::validate();
  m_api_version.validate();
}

const std::string& sbi_interface::get_api_version() const {
  return m_api_version.get_value();
}

const std::string& sbi_interface::get_url() const {
  return m_url;
}

void sbi_interface::set_url() {
  m_url = "";
  // this is easily adaptable to HTTPS, just add a flag, and we change the URL
  m_url.append("http://")
      .append(get_host())
      .append(":")
      .append(std::to_string(get_port()));
}

nf::nf(
    const std::string& name, const std::string& host,
    const sbi_interface& sbi) {
  m_config_name = name;
  m_host        = string_config_value("host", host);
  m_sbi         = sbi;
  m_set         = true;
  m_host.set_validation_regex(HOST_VALIDATOR_REGEX);
  set_url();
}

void nf::from_yaml(const YAML::Node& node) {
  if (node[NF_CONFIG_HOST_NAME]) {
    m_host.from_yaml(node[NF_CONFIG_HOST_NAME]);
  }
  if (node["sbi"]) {
    m_sbi.m_host = m_host;
    m_sbi.from_yaml(node["sbi"]);
  }
  m_set = true;
  set_url();
}

nlohmann::json nf::to_json() {
  nlohmann::json json_data           = {};
  json_data[m_sbi.get_config_name()] = m_sbi.to_json();
  return json_data;
}

bool nf::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_sbi.get_config_name()) != json_data.end()) {
      m_sbi.from_json(json_data[m_sbi.get_config_name()]);
    }

    if (json_data.find("url") != json_data.end()) {
      m_url = json_data["url"].get<std::string>();
    }

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string nf::to_string(const std::string& indent) const {
  std::string out;
  if (!is_set()) {
    return "";
  }
  std::string inner_indent = add_indent(indent);
  unsigned int inner_width = get_inner_width(inner_indent.length());

  out.append(indent).append(m_config_name).append(":\n");
  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, OUTER_LIST_ELEM, m_host.get_config_name(),
          inner_width, m_host.get_value()));

  if (m_sbi.is_set()) {
    out.append(inner_indent)
        .append(
            fmt::format("{} {}\n", OUTER_LIST_ELEM, m_sbi.get_config_name()));
    out.append(m_sbi.to_string(add_indent(inner_indent)));
  }

  return out;
}

void nf::validate() {
  if (!m_set) return;
  m_host.validate();
  m_sbi.validate();
}

const sbi_interface& nf::get_sbi() const {
  return m_sbi;
}

const std::string& nf::get_host() const {
  return m_host.get_value();
}

const std::string& nf::get_url() const {
  return m_url;
}

void nf::set_url() {
  m_url = m_sbi.get_url();
}

nf_features_config::nf_features_config(
    const std::string& name, const std::string& nf_name, bool value) {
  m_option_value = option_config_value("", value);
  m_config_name  = name;
  m_nf_name      = nf_name;
  m_set          = true;
}

nf_features_config::nf_features_config(
    const std::string& name, const std::string& nf_name,
    const std::string& value) {
  m_string_value = string_config_value("", value);
  m_config_name  = name;
  m_nf_name      = nf_name;
  m_set          = true;
}

void nf_features_config::from_yaml(const YAML::Node& node) {
  if (node["general"]) {
    set_value(node["general"]);
  }

  // first we handle the NF-specific configuration
  if (node[m_nf_name]) {
    set_value(node[m_nf_name]);
  }
  m_set = true;
}

nlohmann::json nf_features_config::to_json() {
  nlohmann::json json_data = {};
  if (m_option_value.is_set())
    json_data = m_option_value.to_json();
  else
    json_data = m_string_value.to_json();
  return json_data;
}

bool nf_features_config::from_json(const nlohmann::json&) {
  try {
    // TODO:
    /*   if (json_data.find(m_config_name) != json_data.end()) {
           m_option_value.from_json(json_data[m_config_name]);
       }
       */
  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

void nf_features_config::set_value(const YAML::Node& node) {
  try {
    m_option_value.from_yaml(node);
  } catch (YAML::Exception&) {
    m_string_value.from_yaml(node);
  }
}

std::string nf_features_config::to_string(const std::string& indent) const {
  if (!m_set) return "";
  std::string out;
  unsigned int inner_width = get_inner_width(indent.length());
  if (m_string_value.get_value().empty()) {
    out.append(indent).append(fmt::format(
        BASE_FORMATTER, OUTER_LIST_ELEM, m_config_name, inner_width,
        m_option_value.to_string("")));
  } else {
    out.append(indent).append(fmt::format(
        BASE_FORMATTER, OUTER_LIST_ELEM, m_config_name, inner_width,
        m_string_value.to_string("")));
  }
  return out;
}

void nf_features_config::validate() {
  if (!m_set) return;
  m_string_value.validate();
}

void nf_features_config::set_validation_regex(const std::string& regex) {
  m_string_value.set_validation_regex(regex);
}

bool nf_features_config::get_option() const {
  return m_option_value.get_value();
}

const std::string& nf_features_config::get_string() const {
  return m_string_value.get_value();
}

database_config::database_config(const std::string& name) {
  m_config_name   = name;
  m_set           = false;
  m_host          = string_config_value(NF_CONFIG_HOST_NAME, "mysql");
  m_database_type = string_config_value(DATABASE_CONFIG_DATABASE_TYPE, "mysql");
  m_port          = database_config::get_default_database_port(m_database_type);
  m_user          = string_config_value(DATABASE_CONFIG_USER, "root");
  m_pass          = string_config_value(DATABASE_CONFIG_PASSWORD, "linux");
  m_database_name =
      string_config_value(DATABASE_CONFIG_DATABASE_NAME, "oai_db");
  m_connection_timeout =
      int_config_value(DATABASE_CONFIG_CONNECTION_TIMEOUT, 300);

  m_database_type.set_validation_regex("mysql|cassandra|mongodb");
  m_port.set_validation_interval(0, 65535);
}

void database_config::from_yaml(const YAML::Node& node) {
  m_set = true;
  if (node[NF_CONFIG_HOST_NAME]) {
    m_host.from_yaml(node[NF_CONFIG_HOST_NAME]);
  }
  if (node[DATABASE_CONFIG_DATABASE_TYPE]) {
    m_database_type.from_yaml(node[DATABASE_CONFIG_DATABASE_TYPE]);
  }
  if (node[DATABASE_CONFIG_PORT]) {
    m_port.from_yaml(node[DATABASE_CONFIG_PORT]);
  } else {
    // SET DEFAULT DB PORTS
    m_port = database_config::get_default_database_port(m_database_type);
  }
  if (node[DATABASE_CONFIG_USER]) {
    m_user.from_yaml(node[DATABASE_CONFIG_USER]);
  }
  if (node[DATABASE_CONFIG_PASSWORD]) {
    m_pass.from_yaml(node[DATABASE_CONFIG_PASSWORD]);
  }
  if (node[DATABASE_CONFIG_DATABASE_NAME]) {
    m_database_name.from_yaml(node[DATABASE_CONFIG_DATABASE_NAME]);
  }
  if (node[DATABASE_CONFIG_CONNECTION_TIMEOUT]) {
    m_connection_timeout.from_yaml(node[DATABASE_CONFIG_CONNECTION_TIMEOUT]);
  }
}

nlohmann::json database_config::to_json() {
  nlohmann::json json_data                     = {};
  json_data[m_host.get_config_name()]          = m_host.to_json();
  json_data[m_port.get_config_name()]          = m_port.to_json();
  json_data[m_database_type.get_config_name()] = m_database_type.to_json();
  json_data[m_user.get_config_name()]          = m_user.to_json();
  json_data[m_pass.get_config_name()]          = m_pass.to_json();
  json_data[m_database_name.get_config_name()] = m_database_name.to_json();
  json_data[m_connection_timeout.get_config_name()] =
      m_connection_timeout.to_json();
  return json_data;
}

bool database_config::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_host.get_config_name()) != json_data.end()) {
      m_host.from_json(json_data[m_host.get_config_name()]);
    }
    if (json_data.find(m_database_type.get_config_name()) != json_data.end()) {
      m_database_type.from_json(json_data[m_database_type.get_config_name()]);
    }
    if (json_data.find(m_port.get_config_name()) != json_data.end()) {
      m_port.from_json(json_data[m_port.get_config_name()]);
    } else {
      // SET DEFAULT DB PORTS
      m_port = database_config::get_default_database_port(m_database_type);
    }
    if (json_data.find(m_user.get_config_name()) != json_data.end()) {
      m_user.from_json(json_data[m_user.get_config_name()]);
    }
    if (json_data.find(m_pass.get_config_name()) != json_data.end()) {
      m_pass.from_json(json_data[m_pass.get_config_name()]);
    }
    if (json_data.find(m_database_name.get_config_name()) != json_data.end()) {
      m_database_name.from_json(json_data[m_database_name.get_config_name()]);
    }
    if (json_data.find(m_connection_timeout.get_config_name()) !=
        json_data.end()) {
      m_connection_timeout.from_json(
          json_data[m_connection_timeout.get_config_name()]);
    }
  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

int_config_value database_config::get_default_database_port(
    const string_config_value& database_type) const {
  if (database_type.get_value() == "mysql") {
    return int_config_value(DATABASE_CONFIG_PORT, 3306);
  }
  if (database_type.get_value() == "mongodb") {
    return int_config_value(DATABASE_CONFIG_PORT, 27017);
  }
  if (database_type.get_value() == "casandra") {
    return int_config_value(DATABASE_CONFIG_PORT, 9042);
  }
  return int_config_value(DATABASE_CONFIG_PORT, 0);
}

std::string database_config::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = get_inner_width(indent.length());
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, NF_CONFIG_HOST_NAME_LABEL, inner_width,
      m_host.get_value()));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, DATABASE_CONFIG_PORT_LABEL, inner_width,
      m_port.get_value()));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, DATABASE_CONFIG_DATABASE_TYPE_LABEL,
      inner_width, m_database_type.get_value()));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, DATABASE_CONFIG_USER_LABEL, inner_width,
      m_user.get_value()));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, DATABASE_CONFIG_PASSWORD_LABEL,
      inner_width, m_pass.get_value()));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, DATABASE_CONFIG_DATABASE_NAME_LABEL,
      inner_width, m_database_name.get_value()));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, DATABASE_CONFIG_CONNECTION_TIMEOUT_LABEL,
      inner_width, m_connection_timeout.get_value()));
  return out;
}

void database_config::validate() {
  m_database_type.validate();
}

const std::string& database_config::get_host() const {
  return m_host.get_value();
}

int database_config::get_port() const {
  return m_port.get_value();
}

const std::string& database_config::get_user() const {
  return m_user.get_value();
}

const std::string& database_config::get_pass() const {
  return m_pass.get_value();
}

const std::string& database_config::get_database_name() const {
  return m_database_name.get_value();
}

const std::string& database_config::get_database_type() const {
  return m_database_type.get_value();
}

int database_config::get_connection_timeout() const {
  return m_connection_timeout.get_value();
}

ue_dns::ue_dns(
    const std::string& primary_dns_v4, const std::string& secondary_dns_v4,
    const std::string& primary_dns_v6, const std::string& secondary_dns_v6) {
  m_config_name    = "ue_dns";
  m_primary_dns_v4 = string_config_value("primary_dns_ipv4", primary_dns_v4);
  m_primary_dns_v6 = string_config_value("primary_dns_ipv6", primary_dns_v6);
  m_secondary_dns_v6 =
      string_config_value("secondary_dns_ipv6", secondary_dns_v6);
  m_secondary_dns_v4 =
      string_config_value("secondary_dns_ipv4", secondary_dns_v4);

  m_primary_dns_v4.set_validation_regex(IPV4_ADDRESS_VALIDATOR_REGEX);
  m_primary_dns_v6.set_validation_regex(IPV6_ADDRESS_VALIDATOR_REGEX);
  m_secondary_dns_v4.set_validation_regex(IPV4_ADDRESS_VALIDATOR_REGEX);
  m_secondary_dns_v6.set_validation_regex(IPV6_ADDRESS_VALIDATOR_REGEX);

  // only Primary IPv4 is mandatory
  if (primary_dns_v6.empty()) {
    m_primary_dns_v6.unset_config();
  }
  if (secondary_dns_v6.empty()) {
    m_secondary_dns_v6.unset_config();
  }
  if (secondary_dns_v4.empty()) {
    m_secondary_dns_v4.unset_config();
  }
}

void ue_dns::from_yaml(const YAML::Node& node) {
  if (node["primary_ipv4"]) {
    m_primary_dns_v4.from_yaml(node["primary_ipv4"]);
  }
  if (node["primary_ipv6"]) {
    m_primary_dns_v6.from_yaml(node["primary_ipv6"]);
  }
  if (node["secondary_ipv4"]) {
    m_secondary_dns_v4.from_yaml(node["secondary_ipv4"]);
  }
  if (node["secondary_ipv6"]) {
    m_secondary_dns_v6.from_yaml(node["secondary_ipv6"]);
  }
}

nlohmann::json ue_dns::to_json() {
  nlohmann::json json_data                      = {};
  json_data[m_primary_dns_v4.get_config_name()] = m_primary_dns_v4.to_json();
  json_data[m_secondary_dns_v4.get_config_name()] =
      m_secondary_dns_v4.to_json();
  json_data[m_primary_dns_v6.get_config_name()] = m_primary_dns_v6.to_json();
  json_data[m_secondary_dns_v6.get_config_name()] =
      m_secondary_dns_v6.to_json();
  return json_data;
}

bool ue_dns::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_primary_dns_v4.get_config_name()) != json_data.end()) {
      m_primary_dns_v4.from_json(json_data[m_primary_dns_v4.get_config_name()]);
    }

    if (json_data.find(m_secondary_dns_v4.get_config_name()) !=
        json_data.end()) {
      m_secondary_dns_v4.from_json(
          json_data[m_secondary_dns_v4.get_config_name()]);
    }
    if (json_data.find(m_primary_dns_v6.get_config_name()) != json_data.end()) {
      m_primary_dns_v6.from_json(json_data[m_primary_dns_v6.get_config_name()]);
    }
    if (json_data.find(m_secondary_dns_v6.get_config_name()) !=
        json_data.end()) {
      m_secondary_dns_v6.from_json(
          json_data[m_secondary_dns_v6.get_config_name()]);
    }

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string ue_dns::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = get_inner_width(indent.length());

  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, m_primary_dns_v4.get_config_name(),
      inner_width, m_primary_dns_v4.to_string("")));

  if (m_primary_dns_v6.is_set()) {
    out.append(indent).append(fmt::format(
        BASE_FORMATTER, OUTER_LIST_ELEM, m_primary_dns_v6.get_config_name(),
        inner_width, m_primary_dns_v6.to_string("")));
  }

  if (m_secondary_dns_v4.is_set()) {
    out.append(indent).append(fmt::format(
        BASE_FORMATTER, OUTER_LIST_ELEM, m_secondary_dns_v4.get_config_name(),
        inner_width, m_secondary_dns_v4.to_string("")));
  }

  if (m_secondary_dns_v6.is_set()) {
    out.append(indent).append(fmt::format(
        BASE_FORMATTER, OUTER_LIST_ELEM, m_secondary_dns_v6.get_config_name(),
        inner_width, m_secondary_dns_v6.to_string("")));
  }

  return out;
}

void ue_dns::validate() {
  m_primary_dns_v4.validate();
  m_secondary_dns_v4.validate();
  m_primary_dns_v6.validate();
  m_secondary_dns_v6.validate();

  m_primary_dns_v4_ip = safe_convert_ip(m_primary_dns_v4.get_value());
  if (m_secondary_dns_v4.is_set()) {
    m_secondary_dns_v4_ip = safe_convert_ip(m_secondary_dns_v4.get_value());
  }
  if (m_primary_dns_v6.is_set()) {
    m_primary_dns_v6_ip = safe_convert_ip6(m_primary_dns_v6.get_value());
  }
  if (m_secondary_dns_v6.is_set()) {
    m_secondary_dns_v6_ip = safe_convert_ip6(m_secondary_dns_v6.get_value());
  }
}

const in_addr& ue_dns::get_primary_dns_v4() const {
  return m_primary_dns_v4_ip;
}

const in_addr& ue_dns::get_secondary_dns_v4() const {
  return m_secondary_dns_v4_ip;
}

const in6_addr& ue_dns::get_primary_dns_v6() const {
  return m_primary_dns_v6_ip;
}

const in6_addr& ue_dns::get_secondary_dns_v6() const {
  return m_secondary_dns_v6_ip;
}

dnn_config::dnn_config(
    const std::string& dnn, const std::string& pdu_type,
    const std::string& ipv4_subnet, const std::string& ipv6_prefix)
    : m_ue_dns("8.8.8.8", "1.1.1.1", "", "") {
  m_config_name      = "DNN";
  m_dnn              = string_config_value("DNN", dnn);
  m_pdu_session_type = string_config_value("PDU session type", pdu_type);
  m_ipv4_subnet      = string_config_value("IPv4 subnet", ipv4_subnet);
  m_ipv6_prefix      = string_config_value("IPv6 prefix", ipv6_prefix);
  // we unset it here, so that we can check if we can overwrite it from the
  // default UE DNS config
  m_ue_dns.unset_config();

  // here we have to cut the end $ and start ^ from the regex for it to work
  std::string start_ipv4 = IPV4_ADDRESS_VALIDATOR_REGEX.substr(
      0, IPV4_ADDRESS_VALIDATOR_REGEX.length() - 1);

  m_pdu_session_type.set_validation_regex(PDU_SESSION_TYPE_REGEX);
  m_ipv4_subnet.set_validation_regex(start_ipv4 + "/[0-9]{1,2}");
  m_ipv6_prefix.set_validation_regex(IPV6_ADDRESS_VALIDATOR_REGEX);
  if (ipv6_prefix.empty()) {
    m_ipv6_prefix.unset_config();
  }
  m_set = true;
}

void dnn_config::from_yaml(const YAML::Node& node) {
  if (node["dnn"]) {
    m_dnn.from_yaml(node["dnn"]);
  }
  if (node["pdu_session_type"]) {
    m_pdu_session_type.from_yaml(node["pdu_session_type"]);
  }
  if (node["ipv4_subnet"]) {
    m_ipv4_subnet.from_yaml(node["ipv4_subnet"]);
  }
  if (node["ipv6_prefix"]) {
    m_ipv6_prefix.from_yaml(node["ipv6_prefix"]);
  }
  if (node["ue_dns"]) {
    m_ue_dns.from_yaml(node["ue_dns"]);
    m_ue_dns.set_config();
  }
}

nlohmann::json dnn_config::to_json() {
  nlohmann::json json_data           = {};
  json_data[m_dnn.get_config_name()] = m_dnn.to_json();
  json_data[m_pdu_session_type.get_config_name()] =
      m_pdu_session_type.to_json();
  json_data[m_ipv4_subnet.get_config_name()] = m_ipv4_subnet.to_json();
  json_data[m_ipv6_prefix.get_config_name()] = m_ipv6_prefix.to_json();

  json_data["ipv4_pool_start_ip"] = conv::toString(m_ipv4_pool_start_ip);
  json_data["ipv4_pool_end_ip"]   = conv::toString(m_ipv4_pool_end_ip);
  json_data["ipv6_prefix_ip"]     = conv::toString(m_ipv6_prefix_ip);
  json_data["ipv6_prefix_length"] = m_ipv6_prefix_length;
  return json_data;
}

bool dnn_config::from_json(const nlohmann::json& json_data) {
  try {
    if (json_data.find(m_dnn.get_config_name()) != json_data.end()) {
      m_dnn.from_json(json_data[m_dnn.get_config_name()]);
    }
    if (json_data.find(m_pdu_session_type.get_config_name()) !=
        json_data.end()) {
      m_pdu_session_type.from_json(
          json_data[m_pdu_session_type.get_config_name()]);
    }
    if (json_data.find(m_ipv4_subnet.get_config_name()) != json_data.end()) {
      m_ipv4_subnet.from_json(json_data[m_ipv4_subnet.get_config_name()]);
    }

    // TODO: trim
    m_ipv4_pool_start_ip =
        safe_convert_ip(json_data["ipv4_pool_start_ip"].get<std::string>());
    m_ipv4_pool_end_ip =
        safe_convert_ip(json_data["m_ipv4_pool_end_ip"].get<std::string>());
    // TODO:
    // m_ipv6_prefix_ip
    // m_ipv6_prefix_length

  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

[[nodiscard]] std::string dnn_config::to_string(
    const std::string& indent) const {
  std::string out;

  std::string inner_indent = add_indent(indent);
  unsigned int inner_width = get_inner_width(inner_indent.length());

  out.append(fmt::format("{} {}:\n", OUTER_LIST_ELEM, m_dnn.get_config_name()));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, INNER_LIST_ELEM, m_dnn.get_config_name(), inner_width,
          m_dnn.to_string("")));

  out.append(inner_indent)
      .append(fmt::format(
          BASE_FORMATTER, INNER_LIST_ELEM, m_pdu_session_type.get_config_name(),
          inner_width, m_pdu_session_type.to_string("")));
  if (m_pdu_session_type_generated == PDU_SESSION_TYPE_E_IPV4V6 ||
      m_pdu_session_type_generated == PDU_SESSION_TYPE_E_IPV6) {
    out.append(inner_indent)
        .append(fmt::format(
            BASE_FORMATTER, INNER_LIST_ELEM, m_ipv6_prefix.get_config_name(),
            inner_width, m_ipv6_prefix.to_string("")));
  }
  if ((m_pdu_session_type_generated == PDU_SESSION_TYPE_E_IPV4V6 ||
       m_pdu_session_type_generated == PDU_SESSION_TYPE_E_IPV4) &&
      !m_ipv4_subnet.to_string("").empty()) {
    out.append(inner_indent)
        .append(fmt::format(
            BASE_FORMATTER, INNER_LIST_ELEM, m_ipv4_subnet.get_config_name(),
            inner_width, m_ipv4_subnet.to_string("")));
  }
  out.append(inner_indent)
      .append(fmt::format("{} {}:\n", INNER_LIST_ELEM, "DNS Settings"));
  std::string inner_indent2 = add_indent(inner_indent);

  out.append(m_ue_dns.to_string(inner_indent2));

  return out;
}

void dnn_config::validate() {
  m_pdu_session_type.validate();
  m_ipv4_subnet.validate();
  m_ipv6_prefix.validate();
  m_ue_dns.validate();

  m_pdu_session_type_generated =
      pdu_session_type_t(m_pdu_session_type.get_value());

  if (m_pdu_session_type_generated ==
          pdu_session_type_e::PDU_SESSION_TYPE_E_IPV4 ||
      m_pdu_session_type_generated ==
          pdu_session_type_e::PDU_SESSION_TYPE_E_IPV4V6) {
    std::vector<std::string> ips;
    std::vector<std::string> ipsub;

    boost::split(
        ipsub, m_ipv4_subnet.get_value(), boost::is_any_of("/"),
        boost::token_compress_on);

    if (ipsub.size() != 2) {
      throw std::runtime_error(fmt::format(
          "The IP address subnet {} is not valid", m_ipv4_subnet.get_value()));
    }

    boost::trim_left(ipsub[0]);
    boost::trim_right(ipsub[0]);

    m_ipv4_subnet_ip = safe_convert_ip(ipsub[0]);
    m_ipv4_prefix    = std::stoi(ipsub[1], nullptr, 0);
    get_ipv4_range(
        m_ipv4_subnet_ip, m_ipv4_prefix, m_ipv4_pool_start_ip,
        m_ipv4_pool_end_ip);
  }
  if (m_pdu_session_type_generated ==
          pdu_session_type_e::PDU_SESSION_TYPE_E_IPV6 ||
      m_pdu_session_type_generated ==
          pdu_session_type_e::PDU_SESSION_TYPE_E_IPV4V6) {
    std::vector<std::string> ip6s;
    boost::split(
        ip6s, m_ipv6_prefix.get_value(), boost::is_any_of("/"),
        boost::token_compress_on);

    if (ip6s.size() != 2) {
      // TODO IPv6 is no handled correctly in PPA, which is why we print a
      // warning here instead of an exception. We need to verify IPv6 and IPv4V6
      // end to end

      // throw std::runtime_error(fmt::format(
      //    "The IPv6 prefix / length {} is not valid",
      //    m_ipv6_prefix.get_value()));

      logger::logger_registry::get_logger(LOGGER_NAME)
          .warn(
              "The IPv6 prefix / length %s is not valid",
              m_ipv6_prefix.get_value());
    } else {
      try {
        m_ipv6_prefix_ip     = safe_convert_ip6(ip6s[0]);
        m_ipv6_prefix_length = std::stoi(ip6s[1]);
      } catch (std::runtime_error& e) {
        logger::logger_registry::get_logger(LOGGER_NAME).warn(e.what());
      }
    }
  }
}

[[nodiscard]] const in_addr& dnn_config::get_ipv4_pool_start() const {
  return m_ipv4_pool_start_ip;
}

[[nodiscard]] const in_addr& dnn_config::get_ipv4_pool_end() const {
  return m_ipv4_pool_end_ip;
}

[[nodiscard]] const in_addr& dnn_config::get_ipv4_subnet() const {
  return m_ipv4_subnet_ip;
}

[[nodiscard]] const int& dnn_config::get_ipv4_subnet_prefix() const {
  return m_ipv4_prefix;
}

[[nodiscard]] const in6_addr& dnn_config::get_ipv6_prefix() const {
  return m_ipv6_prefix_ip;
}

[[nodiscard]] uint8_t dnn_config::get_ipv6_prefix_length() const {
  return m_ipv6_prefix_length;
}

[[nodiscard]] const pdu_session_type_t& dnn_config::get_pdu_session_type()
    const {
  return m_pdu_session_type_generated;
}

const std::string& dnn_config::get_dnn() const {
  return m_dnn.get_value();
}

const ue_dns& dnn_config::get_ue_dns() const {
  return m_ue_dns;
}

void dnn_config::set_ue_dns(const ue_dns& dns) {
  m_ue_dns = dns;
  m_ue_dns.set_config();
}

nf_http_version::nf_http_version() {
  m_set     = false;
  m_version = string_config_value(NF_CONFIG_HTTP_NAME, "1.1");
  m_version.set_validation_regex("1|1.1|2|3");
  m_config_name = NF_CONFIG_HTTP_NAME;
}

void nf_http_version::from_yaml(const YAML::Node& node) {
  m_set = true;
  m_version.from_yaml(node);
}

nlohmann::json nf_http_version::to_json() {
  nlohmann::json json_data = {};
  json_data                = m_version.to_json();
  return json_data;
}

bool nf_http_version::from_json(const nlohmann::json& json_data) {
  try {
    m_version.from_json(json_data);
  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string nf_http_version::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = get_inner_width(indent.length());
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, m_version.get_config_name(), inner_width,
      m_version.get_value()));
  return out;
}

void nf_http_version::validate() {
  if (!m_set) return;
  m_version.validate();
}

const std::string& nf_http_version::get_http_version() const {
  return m_version.get_value();
}

curl_timeout::curl_timeout() {
  m_set          = false;
  m_curl_timeout = int_config_value(
      NF_CONFIG_CURL_TIMEOUT, NF_CONFIG_CURL_TIMEOUT_DEFAULT_VALUE);
  m_curl_timeout.set_validation_interval(
      NF_CONFIG_CURL_TIMEOUT_MIN_VALUE, NF_CONFIG_CURL_TIMEOUT_MAX_VALUE);
  m_config_name = NF_CONFIG_CURL_TIMEOUT;
}

void curl_timeout::from_yaml(const YAML::Node& node) {
  m_set = true;
  m_curl_timeout.from_yaml(node);
}

nlohmann::json curl_timeout::to_json() {
  nlohmann::json json_data = {};
  json_data                = m_curl_timeout.to_json();
  return json_data;
}

bool curl_timeout::from_json(const nlohmann::json& json_data) {
  try {
    m_curl_timeout.from_json(json_data);
  } catch (nlohmann::detail::exception& e) {
    // TODO:
  } catch (std::exception& e) {
    // TODO:
  }
  return false;
}

std::string curl_timeout::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = get_inner_width(indent.length());
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, OUTER_LIST_ELEM, NF_CONFIG_CURL_TIMEOUT_LABEL,
      inner_width, std::to_string(m_curl_timeout.get_value()) + " (ms)"));
  return out;
}

void curl_timeout::validate() {
  if (!m_set) return;
  m_curl_timeout.validate();
}

const uint32_t curl_timeout::get() const {
  return m_curl_timeout.get_value();
}
