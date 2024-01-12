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

/*! \file config.cpp
 \brief
 \author  Stefan Spettel
 \company phine.tech
 \date 2022
 \email: stefan.spettel@phine.tech
*/

#include "config.hpp"
#include <fmt/format.h>
#include <stdexcept>
#include <sstream>

using namespace oai::config;

void config::set_configuration(
    const std::string& name, std::unique_ptr<config_type> val) {
  std::unique_lock lock(m_config_mutex);
  m_config[name] = std::move(val);
}

void config::set_configuration_mandatory(const std::string& name) {
  std::unique_lock lock(m_config_mutex);
  m_mandatory_keys.push_back(name);
}

bool config::validate() const {
  std::shared_lock lock(m_config_mutex);
  bool success = true;
  // check if all keys are present
  for (const auto& key : m_mandatory_keys) {
    const auto it = m_config.find(key);
    if (it == m_config.end()) {
      logger::logger_registry::get_logger(LOGGER_NAME)
          .error(
              "Mandatory configuration %s does not exist in configuration",
              key);
      success = false;
    }
  }

  for (const auto& conf : m_config) {
    if (!conf.second->validate()) {
      logger::logger_registry::get_logger(LOGGER_NAME)
          .error("Validation of %s not successful", conf.first);
      success = false;
    }
  }

  return success;
}

std::string config::to_string() const {
  std::shared_lock lock(m_config_mutex);

  std::string base_conf_out;
  std::string support_features_out;
  std::string sbi_out;
  std::string local_iface_out;
  std::string others_out;

  for (const auto& conf : m_config) {
    std::string val = fmt::format(
        BASE_FORMATTER, "-", conf.first, COLUMN_WIDTH,
        conf.second->to_string("  "));

    switch (conf.second->get_config_type()) {
      case config_type_e::local:
        local_iface_out.append(val);
        break;
      case config_type_e::string:
      case config_type_e::uint8:
        base_conf_out.append(val);
        break;
      case config_type_e::option:
        support_features_out.append(val);
        break;
      case config_type_e::sbi:
        sbi_out.append(val);
        break;
      case config_type_e::invalid:
        logger::logger_registry::get_logger(LOGGER_NAME)
            .error(
                "General error in configuration. Invalid type of: %s",
                conf.first);
        others_out.append(val);
        break;
      default:
        logger::logger_registry::get_logger(LOGGER_NAME)
            .warn(
                "%s unhandled value %d", __PRETTY_FUNCTION__,
                static_cast<int>(conf.second->get_config_type()));
    }
  }

  std::string out;
  if (!base_conf_out.empty()) {
    out.append("Basic Configuration:\n").append(base_conf_out);
  }
  if (!support_features_out.empty()) {
    out.append("Support features:\n").append(support_features_out);
  }
  if (!local_iface_out.empty()) {
    out.append("Local Interfaces:\n").append(local_iface_out);
  }
  if (!sbi_out.empty()) {
    out.append("SBI Interfaces:\n").append(sbi_out);
  }
  if (!others_out.empty()) {
    out.append("Other configuration:\n").append(others_out);
  }

  return out;
}

const std::string& config::get_base_conf_val(const std::string& name) const {
  return get<string_config_value>(name).get_value();
}

bool config::get_support_feature(const std::string& name) const {
  return get<option_config_value>(name).get_value();
}

const sbi_interface& config::get_sbi_interface(const std::string& name) const {
  return get<sbi_interface>(name);
}

const local_interface& config::get_local_interface(
    const std::string& name) const {
  return get<local_interface>(name);
}

const local_sbi_interface& config::get_local_sbi_interface(
    const std::string& name) const {
  return get<local_sbi_interface>(name);
}

uint8_t config::get_uint8_conf_val(const std::string& name) const {
  return get_uint8_conf(name).get_value();
}

const uint8_config_value& config::get_uint8_conf(
    const std::string& name) const {
  return get<uint8_config_value>(name);
}

template<typename T>
const T& config::get(const std::string& name) const {
  static_assert(
      std::is_base_of<config_type, T>::value,
      "Templated config type not derived from config_type");

  std::shared_lock lock(m_config_mutex);
  const auto it = m_config.find(name);
  if (it == m_config.end()) {
    throw std::invalid_argument(
        fmt::format("Configuration {} does not exist", name));
  }
  // may throw a bad_cast exception, we catch it to conform with interface
  try {
    return dynamic_cast<T&>(*(it->second));
  } catch (std::bad_cast& e) {
    throw std::invalid_argument(
        fmt::format("Configuration {} has wrong type: {}", name, e.what()));
  }
}

void config::display() const {
  std::stringstream ss(to_string());
  std::string line;

  logger::logger_registry::get_logger(LOGGER_NAME)
      .info("==== OPENAIRINTERFACE %s v%s ====", m_nf_name, PACKAGE_VERSION);

  while (std::getline(ss, line)) {
    logger::logger_registry::get_logger(LOGGER_NAME).info(line);
  }
}
