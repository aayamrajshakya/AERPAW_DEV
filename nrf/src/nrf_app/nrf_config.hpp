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

/*! \file nrf_config.hpp
 * \brief
 \author  Lionel GAUTHIER, Tien-Thinh NGUYEN
 \company Eurecom
 \date 2020
 \email: lionel.gauthier@eurecom.fr, tien-thinh.nguyen@eurecom.fr
 */

#pragma once

#include "config.hpp"
#include "nrf_config_types.hpp"

namespace oai::config::nrf {

class nrf_config : public oai::config::config {
 public:
  // Stefan: we should get rid of this instance things (see PCF)
  unsigned int instance = 0;
  explicit nrf_config(
      const std::string& config_path, bool log_stdout, bool log_rot_file)
      : config(config_path, NRF_CONFIG_NAME, log_stdout, log_rot_file) {
    m_used_config_values = {
        LOG_LEVEL_CONFIG_NAME, NF_LIST_CONFIG_NAME, NF_CONFIG_HTTP_NAME,
        NRF_CONFIG_NAME};
    m_used_sbi_values = {NRF_CONFIG_NAME};

    m_register_nrf_feature.unset_config();

    auto nrf = std::make_shared<nrf_config_type>(
        NRF_CONFIG_NAME, "oai-nrf",
        sbi_interface("SBI", "oai-nrf", 80, "v1", "eth0"));
    add_nf(NRF_CONFIG_NAME, nrf);
  };

  std::shared_ptr<nrf_config_type> nrf() const {
    return std::static_pointer_cast<nrf_config_type>(get_local());
  };
};
}  // namespace oai::config::nrf
