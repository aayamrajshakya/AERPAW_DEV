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

/*! \file config_yaml_file.cpp
 \brief
 \author  Stefan Spettel
 \company phine.tech
 \date 2022
 \email: stefan.spettel@phine.tech
*/

#include "config_yaml_file.hpp"
#include "logger_base.hpp"

using namespace oai::logger;

void oai::config::yaml_file::read_from_file(
    const std::string& file_path, oai::config::config_iface& config) {
  try {
    YAML::Node node = YAML::LoadFile(file_path);
    for (const auto& elem : node) {
      bool success;
      auto key = elem.first.as<std::string>();
      if (elem.second.IsScalar()) {
        success = convert_type<uint8_config_value>(key, elem.second, config);
        if (!success) {
          success = convert_type<option_config_value>(key, elem.second, config);
        }
        if (!success) {
          success = convert_type<string_config_value>(key, elem.second, config);
        }
      } else {
        success = convert_type<sbi_interface>(key, elem.second, config);
        if (!success) {
          success = convert_type<local_sbi_interface>(key, elem.second, config);
        }
        if (!success) {
          success = convert_type<local_interface>(key, elem.second, config);
        }
      }

      if (!success) {
        logger_registry::get_logger(LOGGER_NAME)
            .warn("Could not parse YAML element: %s", key);
      }
    }
  } catch (YAML::BadFile& ex) {
    logger_registry::get_logger(LOGGER_NAME)
        .error(
            "Could not read YAML configuration file, please ensure that it "
            "exists: %s",
            ex.what());
    throw std::runtime_error(ex.what());
  } catch (std::exception& ex) {
    logger_registry::get_logger(LOGGER_NAME)
        .error("Could not parse YAML configuration file: %s", ex.what());
    throw std::runtime_error(ex.what());
  }
}

template<class T>
bool oai::config::yaml_file::convert_type(
    const std::string& key, const YAML::Node& node, config_iface& config) {
  try {
    T conf_val{node};
    std::unique_ptr<config_type> conf = std::make_unique<T>(conf_val);
    config.set_configuration(key, std::move(conf));
    return true;

  } catch (std::exception&) {
    return false;
  }
}
