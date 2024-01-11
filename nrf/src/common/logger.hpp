/*
 * Copyright (c) 2017 Sprint
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstdarg>
#include <stdexcept>
#include <vector>
#include "logger_base.hpp"

static const std::string ASYNC_CMD = "async_cmd";
static const std::string ITTI      = "itti";
static const std::string NRF_APP   = "nrf_app";
static const std::string SYSTEM    = "system";
static const std::string NRF_SBI   = "nrf_sbi";

class Logger {
 public:
  static void init(
      const std::string& name, const bool log_stdout, const bool log_rot_file) {
    oai::logger::logger_registry::register_logger(
        name, ASYNC_CMD, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, ITTI, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, NRF_APP, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, SYSTEM, log_stdout, log_rot_file);
    oai::logger::logger_registry::register_logger(
        name, NRF_SBI, log_stdout, log_rot_file);
  }
  static void set_level(spdlog::level::level_enum level) {
    oai::logger::logger_registry::set_level(level);
  }
  static bool should_log(spdlog::level::level_enum level) {
    return oai::logger::logger_registry::should_log(level);
  }
  static const oai::logger::printf_logger& async_cmd() {
    return oai::logger::logger_registry::get_logger(ASYNC_CMD);
  }
  static const oai::logger::printf_logger& itti() {
    return oai::logger::logger_registry::get_logger(ITTI);
  }
  static const oai::logger::printf_logger& nrf_app() {
    return oai::logger::logger_registry::get_logger(NRF_APP);
  }
  static const oai::logger::printf_logger& system() {
    return oai::logger::logger_registry::get_logger(SYSTEM);
  }
  static const oai::logger::printf_logger& nrf_sbi() {
    return oai::logger::logger_registry::get_logger(NRF_SBI);
  }
};
