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

/*! \file config_from_file.hpp
 \brief
 \author  Stefan Spettel
 \company phine.tech
 \date 2022
 \email: stefan.spettel@phine.tech
*/

#pragma once

#include "config.hpp"
#include "config_types.hpp"

#include <string>
#include <yaml-cpp/yaml.h>
#include <iostream>

namespace oai::config {

class yaml_file_iface {
  /**
   * Reads configuration from YAML file and loads it into the configuration
   * @throw std::runtime_error when file does not exist or could not be parsed
   * @param filepath filepath to load configuration from
   * @param config config object to load into
   */
  virtual void read_from_file(
      const std::string& file_path, config_iface& config) = 0;
};

class yaml_file : public yaml_file_iface {
 public:
  explicit yaml_file() = default;

  void read_from_file(
      const std::string& file_path, config_iface& config) override;

 private:
  template<class T>
  static bool convert_type(
      const std::string& conf, const YAML::Node& node, config_iface& config);
};

}  // namespace oai::config
