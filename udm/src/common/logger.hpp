/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this
 *file except in compliance with the License. You may obtain a copy of the
 *License at
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

/*! \file logger.hpp
 \brief
 \date 2020
 \email: contact@openairinterface.org
 */

#pragma once

#include <cstdarg>
#include <stdexcept>
#include <vector>
#include "logger_base.hpp"

static const std::string CONFIG      = "config";
static const std::string SYSTEM      = "system";
static const std::string UDM_UEAU    = "udm_ueau";
static const std::string UDM_UECM    = "udm_uecm";
static const std::string UDM_EE      = "udm_ee";
static const std::string UDM_SDM     = "udm_sdm";
static const std::string UDM_NRF     = "udm_nrf";
static const std::string UDM_SVR_LOG = "udm_server";
static const std::string UDM_APP     = "udm_app";

class Logger {
 public:
  static void init(
      const std::string& name, const bool log_stdout, const bool log_rot_file) {
    oai::logger::logger_registry::register_logger(
        name, CONFIG, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, SYSTEM, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, UDM_UEAU, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, UDM_UECM, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, UDM_EE, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, UDM_SDM, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, UDM_NRF, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, UDM_SVR_LOG, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, UDM_APP, log_stdout, log_rot_file);
  }
  static void set_level(spdlog::level::level_enum level) {
    oai::logger::logger_registry::set_level(level);
  }
  static bool should_log(spdlog::level::level_enum level) {
    return oai::logger::logger_registry::should_log(level);
  }

  static const oai::logger::printf_logger& config() {
    return oai::logger::logger_registry::get_logger(CONFIG);
  }
  static const oai::logger::printf_logger& system() {
    return oai::logger::logger_registry::get_logger(SYSTEM);
  }
  static const oai::logger::printf_logger& udm_ueau() {
    return oai::logger::logger_registry::get_logger(UDM_UEAU);
  }
  static const oai::logger::printf_logger& udm_uecm() {
    return oai::logger::logger_registry::get_logger(UDM_UECM);
  }
  static const oai::logger::printf_logger& udm_ee() {
    return oai::logger::logger_registry::get_logger(UDM_EE);
  }
  static const oai::logger::printf_logger& udm_sdm() {
    return oai::logger::logger_registry::get_logger(UDM_SDM);
  }
  static const oai::logger::printf_logger& udm_nrf() {
    return oai::logger::logger_registry::get_logger(UDM_NRF);
  }
  static const oai::logger::printf_logger& udm_server() {
    return oai::logger::logger_registry::get_logger(UDM_SVR_LOG);
  }
  static const oai::logger::printf_logger& udm_app() {
    return oai::logger::logger_registry::get_logger(UDM_APP);
  }
};
