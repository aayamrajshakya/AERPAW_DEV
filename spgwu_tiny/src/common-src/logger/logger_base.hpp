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

/*! \file logger_base.hpp
\brief
\author Stefan Spettel
\company OpenAirInterface Software Alliance
\date 2022
\email: stefan.spettel@eurecom.fr
*/

#pragma once

#include <cstdarg>
#include <stdexcept>
#include <vector>
#include <memory>
#include <unordered_map>

// Used by SPDLOG to use external FMT library
#define SPDLOG_FMT_EXTERNAL

// this way we redefine: warn->start, error->warn, critical->error
#define SPDLOG_LEVEL_NAMES                                                     \
  {"trace", "debug", "info", "start", "warning", "error", "off"};

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/common.h>
#include <fmt/printf.h>

namespace oai::logger {
/**
 * Wrapper class for spdlog, stores reference to spdlog log object and allows
 * safe printf-style formatting
 */
class printf_logger {
 public:
  printf_logger(
      const std::string& nf_name, const std::string& name, bool log_stdout,
      bool log_rot_file);

  void set_level(spdlog::level::level_enum level);
  bool should_log(spdlog::level::level_enum level) {
    return logger->should_log(level);
  }

  template<typename... T>
  void trace(const std::string& fmt, const T&... args) const {
    log_printf(spdlog::level::trace, fmt, args...);
  }

  template<typename... T>
  void trace(const char* fmt, const T&... args) const {
    log_printf(spdlog::level::trace, fmt, args...);
  }

  template<typename... T>
  void debug(const std::string& fmt, const T&... args) const {
    log_printf(spdlog::level::debug, fmt, args...);
  }

  template<typename... T>
  void debug(const char* fmt, const T&... args) const {
    log_printf(spdlog::level::debug, fmt, args...);
  }

  template<typename... T>
  void info(const std::string& fmt, const T&... args) const {
    log_printf(spdlog::level::info, fmt, args...);
  }

  template<typename... T>
  void info(const char* fmt, const T&... args) const {
    log_printf(spdlog::level::info, fmt, args...);
  }

  template<typename... T>
  void startup(const std::string& fmt, const T&... args) const {
    log_printf(spdlog::level::warn, fmt, args...);
  }

  template<typename... T>
  void startup(const char* fmt, const T&... args) const {
    log_printf(spdlog::level::warn, fmt, args...);
  }

  template<typename... T>
  void warn(const std::string& fmt, const T&... args) const {
    log_printf(spdlog::level::err, fmt, args...);
  }

  template<typename... T>
  void warn(const char* fmt, const T&... args) const {
    log_printf(spdlog::level::err, fmt, args...);
  }

  template<typename... T>
  void error(const std::string& fmt, const T&... args) const {
    log_printf(spdlog::level::critical, fmt, args...);
  }

  template<typename... T>
  void error(const char* fmt, const T&... args) const {
    log_printf(spdlog::level::critical, fmt, args...);
  }

 private:
  std::shared_ptr<spdlog::logger> logger;

  template<typename... T>
  void log_printf(
      const spdlog::level::level_enum& lvl, const std::string& fmt,
      const T&... args) const {
    // to prevent "expensive" string formatting
    if (!logger->should_log(lvl)) {
      return;
    }

    try {
      std::string format = fmt::sprintf(fmt, args...);
      logger->log(lvl, "{}", format);
    } catch (fmt::format_error& err) {
      // It would be better to not catch here, but keep it here for now
      // to ensure that we don't break when we replace the logger
      logger->error("Format error in format string {}: {}", fmt, err.what());
    }
  }
};

class logger_registry {
 public:
  static void register_logger(
      const std::string& nf_name, const std::string& logger_name,
      bool log_stdout, bool log_rot_file);

  static const printf_logger& get_logger(const std::string& logger);
  static void set_level(spdlog::level::level_enum level);
  static bool should_log(spdlog::level::level_enum level) {
    if (logger_map.empty()) {
      return false;
    }
    const auto& it = logger_map.begin();
    return it->second.should_log(level);
  }

 private:
  static std::unordered_map<std::string, printf_logger> logger_map;
};

}  // namespace oai::logger
