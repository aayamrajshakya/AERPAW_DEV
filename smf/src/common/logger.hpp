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

/*! \file logger.hpp
\brief
\author Stefan Spettel
\company OpenAirInterface Software Alliance
\date 2022
\email: stefan.spettel@eurecom.fr
*/

#pragma once

#include "logger_base.hpp"

static const std::string SMF_APP        = "smf_app";
static const std::string SMF_SBI        = "smf_sbi";
static const std::string SYSTEM         = "system ";
static const std::string UDP            = "udp    ";
static const std::string PFCP           = "pfcp   ";
static const std::string SMF_N4         = "smf_n4 ";
static const std::string SMF_N1         = "smf_n1 ";
static const std::string SMF_N2         = "smf_n2 ";
static const std::string SMF_N7         = "smf_n7 ";
static const std::string SMF_API_SERVER = "smf_api";
static const std::string ITTI           = "itti   ";
static const std::string ASYNC          = "async  ";

class Logger {
 public:
  static void init(
      const std::string& name, bool log_stdout, bool log_rot_file) {
    oai::logger::logger_registry::register_logger(
        name, SMF_APP, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, SMF_SBI, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, SYSTEM, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, UDP, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, SMF_N1, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, SMF_N2, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, SMF_N4, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, SMF_N7, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, SMF_API_SERVER, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, PFCP, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, ITTI, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, ASYNC, log_stdout, log_rot_file);
  }
  static void set_level(spdlog::level::level_enum level) {
    oai::logger::logger_registry::set_level(level);
  }
  static bool should_log(spdlog::level::level_enum level) {
    return oai::logger::logger_registry::should_log(level);
  }

  static const oai::logger::printf_logger& smf_app() {
    return oai::logger::logger_registry::get_logger(SMF_APP);
  }

  static const oai::logger::printf_logger& smf_sbi() {
    return oai::logger::logger_registry::get_logger(SMF_SBI);
  }

  static const oai::logger::printf_logger& system() {
    return oai::logger::logger_registry::get_logger(SYSTEM);
  }

  static const oai::logger::printf_logger& udp() {
    return oai::logger::logger_registry::get_logger(UDP);
  }

  static const oai::logger::printf_logger& smf_n1() {
    return oai::logger::logger_registry::get_logger(SMF_N1);
  }

  static const oai::logger::printf_logger& smf_n2() {
    return oai::logger::logger_registry::get_logger(SMF_N2);
  }
  static const oai::logger::printf_logger& smf_n4() {
    return oai::logger::logger_registry::get_logger(SMF_N4);
  }

  static const oai::logger::printf_logger& smf_n7() {
    return oai::logger::logger_registry::get_logger(SMF_N7);
  }

  static const oai::logger::printf_logger& smf_api_server() {
    return oai::logger::logger_registry::get_logger(SMF_API_SERVER);
  }

  static const oai::logger::printf_logger& pfcp() {
    return oai::logger::logger_registry::get_logger(PFCP);
  }

  static const oai::logger::printf_logger& itti() {
    return oai::logger::logger_registry::get_logger(ITTI);
  }

  static const oai::logger::printf_logger& async_cmd() {
    return oai::logger::logger_registry::get_logger(ASYNC);
  }
};
