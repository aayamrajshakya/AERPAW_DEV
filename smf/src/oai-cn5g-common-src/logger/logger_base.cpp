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

/*! \file logger_base.cpp
\brief
\author Stefan Spettel
\company OpenAirInterface Software Alliance
\date 2022
\email: stefan.spettel@eurecom.fr
*/

#include "logger_base.hpp"
#include <spdlog/sinks/rotating_file_sink.h>
#include <fmt/printf.h>

#include <iostream>
#include <sstream>
#include <string>
#include <memory>

using namespace oai::logger;

printf_logger::printf_logger(
    const std::string& nf_name, const std::string& category, bool log_stdout,
    bool log_rot_file) {
  // static to use the same sinks for all loggers
  static std::vector<spdlog::sink_ptr> sinks;

  if (sinks.empty()) {
    if (log_stdout) {
      auto color_sink =
          std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
      // because we redefine the names, we also redefine the colors
      color_sink->set_color(spdlog::level::warn, color_sink->magenta);
      color_sink->set_color(spdlog::level::err, color_sink->yellow_bold);
      color_sink->set_color(spdlog::level::critical, color_sink->red_bold);

      sinks.push_back(color_sink);
    }
    if (log_rot_file) {
      // TODO would be nice to configure logfile path, and max size maybe?
      std::string filename = nf_name + ".log";
      // 5MB rotating file limit with max 3 files
      sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
          filename, 5 * 1024 * 1024, 3));
    }
  }

  std::stringstream log_format{};
  log_format << "[%Y-%m-%dT%H:%M:%S.%f] [" << nf_name << "] [%n] [%l] %v";

  logger = std::make_shared<spdlog::logger>(
      category, std::begin(sinks), std::end(sinks));

  // Out of the box the level is debug
  logger->set_level(spdlog::level::debug);
}

void printf_logger::set_level(spdlog::level::level_enum level) {
  logger->set_level(level);
}

std::unordered_map<std::string, printf_logger> logger_registry::logger_map;

void logger_registry::register_logger(
    const std::string& nf_name, const std::string& logger_name, bool log_stdout,
    bool log_rot_file) {
  // use the static mutex to lock this function
  // thread-safe since C++11
  static auto mutex = std::mutex();

  auto lock = std::unique_lock(mutex);

  auto logger = printf_logger(nf_name, logger_name, log_stdout, log_rot_file);

  logger_map.insert(std::make_pair<>(logger_name, logger));
}

const printf_logger& logger_registry::get_logger(const std::string& logger) {
  // const STL functions can be accessed by multiple threads simultaneously
  // no lock required
  const auto& it  = logger_map.find(logger);
  const auto& end = logger_map.end();

  if (it == end) {
    throw std::runtime_error(fmt::format("Logger {} does not exist", logger));
  }
  return it->second;
}

void logger_registry::set_level(spdlog::level::level_enum level) {
  for (auto element : logger_map) {
    element.second.set_level(level);
  }
}
