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
#include "logger_base.hpp"
#include "if.hpp"
#include "common_defs.h"

#include <fmt/format.h>
#include <algorithm>
#include <regex>
#include <string>

using namespace oai::config;

const std::string INNER_LIST_ELEM = "+";

bool config_type::matches_regex(
    const std::string& value, const std::string& regex) {
  std::regex re(regex);

  if (!std::regex_match(value, re)) {
    logger::logger_registry::get_logger(LOGGER_NAME)
        .error("%s does not follow regex specification: %s", value, regex);
    return false;
  }
  return true;
}

bool config_type::is_set() const {
  return m_set;
}

config_type_e config_type::get_config_type() const {
  return config_type_e::invalid;
}

bool sbi_interface::validate() {
  if (!matches_regex(m_url, URL_REGEX)) {
    return false;
  }
  m_set = true;
  return true;
}

sbi_interface::sbi_interface(YAML::Node const& node) {
  m_api_version = node["api_version"].as<std::string>();
  // this is easily adaptable to HTTPS, just add a flag, and we change the URL
  m_url.append("http://")
      .append(node["host"].as<std::string>())
      .append(":")
      .append(node["port"].as<std::string>());
}

std::string sbi_interface::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = COLUMN_WIDTH;
  if (indent.length() < COLUMN_WIDTH) {
    inner_width = COLUMN_WIDTH - indent.length();
  }

  out.append("SBI Interface\n");
  out.append(indent).append(
      fmt::format(BASE_FORMATTER, INNER_LIST_ELEM, "URL", inner_width, m_url));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, "API Version", inner_width,
      m_api_version));
  return out;
}

config_type_e sbi_interface::get_config_type() const {
  return config_type_e::sbi;
}

const std::string& sbi_interface::get_api_version() const {
  return m_api_version;
}

const std::string& sbi_interface::get_url() const {
  return m_url;
}

local_interface::local_interface(YAML::Node const& node) {
  m_port    = node["port"].as<uint16_t>();
  m_if_name = node["name"].as<std::string>();
}

bool local_interface::validate() {
  unsigned int _mtu{};
  in_addr _addr4{};
  in_addr _netmask{};
  if (get_inet_addr_infos_from_iface(m_if_name, _addr4, _netmask, _mtu) ==
      RETURNerror) {
    logger::logger_registry::get_logger(LOGGER_NAME)
        .error(
            "Error in reading configuration from network interface %s",
            m_if_name);
    return false;
  }
  m_mtu   = _mtu;
  m_addr4 = _addr4;

  m_set = true;
  return true;
}

std::string local_interface::to_string(const std::string& indent) const {
  std::string out;
  unsigned int inner_width = COLUMN_WIDTH;
  if (indent.length() < COLUMN_WIDTH) {
    inner_width = COLUMN_WIDTH - indent.length();
  }

  out.append("Local Interface\n");
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
      m_if_name));
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, "Port", inner_width, m_port));

  return out;
}

config_type_e local_interface::get_config_type() const {
  return config_type_e::local;
}

const std::string& local_interface::get_if_name() const {
  return m_if_name;
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
  return m_port;
}

local_sbi_interface::local_sbi_interface(YAML::Node const& node) {
  m_if_name      = node["name"].as<std::string>();
  m_port         = node["port_http"].as<uint16_t>();
  m_port_http2   = node["port_http2"].as<uint16_t>();
  m_api_version  = node["api_version"].as<std::string>();
  m_http_version = node["http_version"].as<uint8_t>();
}

bool local_sbi_interface::validate() {
  bool sbi_validate = validate_sbi_api_version(m_api_version);
  if (!sbi_validate) {
    return false;
  }

  if (m_http_version < 1 || m_http_version > 2) {
    logger::logger_registry::get_logger(LOGGER_NAME)
        .error("Wrong HTTP version: %u. Should be 1 or 2", m_http_version);
    return false;
  }

  return local_interface::validate();
}

std::string local_sbi_interface::to_string(const std::string& indent) const {
  unsigned int inner_width = 0;
  if (indent.length() < COLUMN_WIDTH) {
    inner_width = COLUMN_WIDTH - indent.length();
  }

  std::string out = local_interface::to_string(indent);
  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, "API Version", inner_width,
      m_api_version));

  out.append(indent).append(fmt::format(
      BASE_FORMATTER, INNER_LIST_ELEM, "HTTP Version", inner_width,
      m_http_version));

  return out;
}

const std::string& local_sbi_interface::get_api_version() const {
  return m_api_version;
}

uint8_t local_sbi_interface::http_version() const {
  return m_http_version;
}

uint16_t local_sbi_interface::get_port_http2() const {
  return m_port_http2;
}

bool network_interface::validate_sbi_api_version(const std::string& v) {
  auto it =
      std::find(allowed_api_versions.begin(), allowed_api_versions.end(), v);
  if (it == allowed_api_versions.end()) {
    logger::logger_registry::get_logger(LOGGER_NAME)
        .error("API version %s not valid!", v);
    return false;
  }
  return true;
}

string_config_value::string_config_value(YAML::Node const& node) {
  m_value = node.as<std::string>();
}

std::string string_config_value::to_string(const std::string&) const {
  std::string out;
  return out.append(m_value);
}

bool string_config_value::validate() {
  m_set = true;
  return true;
}

config_type_e string_config_value::get_config_type() const {
  return config_type_e::string;
}

const std::string& string_config_value::get_value() const {
  return m_value;
}

option_config_value::option_config_value(YAML::Node const& node) {
  m_value = node.as<bool>();
}

std::string option_config_value::to_string(const std::string&) const {
  std::string val = m_value ? "Yes" : "No";
  return val;
}

bool option_config_value::validate() {
  m_set = true;
  return true;
}

config_type_e option_config_value::get_config_type() const {
  return config_type_e::option;
}

bool option_config_value::get_value() const {
  return m_value;
}

uint8_config_value factory::get_uint8_config(uint8_t val) {
  uint8_config_value v;
  v.m_value = val;
  return v;
}

uint8_config_value::uint8_config_value(const YAML::Node& node) {
  m_value = node.as<uint8_t>();
}

std::string uint8_config_value::to_string(const std::string&) const {
  return std::to_string(m_value);
}

bool uint8_config_value::validate() {
  if (m_value < m_min_value || m_value > m_max_value) {
    logger::logger_registry::get_logger(LOGGER_NAME)
        .error(
            "Value should be in [%u, %u], but it is: %u", m_min_value,
            m_max_value, m_value);
    return false;
  }
  m_set = true;
  return true;
}

config_type_e uint8_config_value::get_config_type() const {
  return config_type_e::uint8;
}

uint8_t uint8_config_value::get_value() const {
  return m_value;
}

void uint8_config_value::set_validation_min_value(uint8_t val) {
  m_min_value = val;
}

void uint8_config_value::set_validation_max_value(uint8_t val) {
  m_max_value = val;
}

option_config_value factory::get_option_config(bool val) {
  option_config_value v;
  v.m_value = val;
  return v;
}

string_config_value factory::get_string_config(const std::string& val) {
  string_config_value v;
  v.m_value = val;
  return v;
}
