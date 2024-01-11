/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under
 *one or more contributor license agreements.  See the NOTICE file distributed
 *with this work for additional information regarding copyright ownership. The
 *OpenAirInterface Software Alliance licenses this file to You under the OAI
 *Public License, Version 1.1  (the "License"); you may not use this file except
 *in compliance with the License. You may obtain a copy of the License at
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

#include "udr_config.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <iostream>
#include <libconfig.h++>

#include "common_defs.h"
#include "fqdn.hpp"
#include "if.hpp"
#include "logger.hpp"
#include "string.hpp"

using namespace libconfig;

namespace oai::udr::config {

//------------------------------------------------------------------------------
udr_config::udr_config()
    : db_conf(), instance(), udr_name(), pid_dir(), nudr() {
  nudr_http2_port  = 8080;
  nudr.api_version = "v1";
  db_type          = DB_TYPE_MYSQL;
  use_http2        = false;
  register_nrf     = false;
  use_fqdn_dns     = false;
  log_level        = spdlog::level::debug;
}

//------------------------------------------------------------------------------
udr_config::~udr_config() {}

//------------------------------------------------------------------------------
int udr_config::load(const std ::string& config_file) {
  Logger::udr_app().debug(
      "\nLoad UDR system configuration file(%s)", config_file.c_str());

  Config cfg;
  unsigned char buf_in6_addr[sizeof(struct in6_addr)];

  // Read the file. If there is an error, report it and exit.
  try {
    cfg.readFile(config_file.c_str());
  } catch (const FileIOException& fioex) {
    Logger::udr_app().error(
        "I/O error while reading file %s - %s", config_file.c_str(),
        fioex.what());
    throw;
  } catch (const ParseException& pex) {
    Logger::udr_app().error(
        "Parse error at %s:%d - %s", pex.getFile(), pex.getLine(),
        pex.getError());
    throw;
  }
  const Setting& root = cfg.getRoot();
  try {
    const Setting& udr_cfg = root[UDR_CONFIG_STRING_UDR_CONFIG];
  } catch (const SettingNotFoundException& nfex) {
    Logger::udr_app().error("%s : %s", nfex.what(), nfex.getPath());
    return RETURNerror;
  }
  const Setting& udr_cfg = root[UDR_CONFIG_STRING_UDR_CONFIG];
  try {
    udr_cfg.lookupValue(UDR_CONFIG_STRING_INSTANCE_ID, instance);
  } catch (const SettingNotFoundException& nfex) {
    Logger::udr_app().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
  }
  try {
    udr_cfg.lookupValue(UDR_CONFIG_STRING_PID_DIRECTORY, pid_dir);
  } catch (const SettingNotFoundException& nfex) {
    Logger::udr_app().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
  }
  try {
    udr_cfg.lookupValue(UDR_CONFIG_STRING_UDR_NAME, udr_name);
  } catch (const SettingNotFoundException& nfex) {
    Logger::config().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
  }

  // Log Level
  try {
    std::string string_level;
    udr_cfg.lookupValue(UDR_CONFIG_STRING_LOG_LEVEL, string_level);
    log_level = spdlog::level::from_str(string_level);
  } catch (const SettingNotFoundException& nfex) {
    Logger::config().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
  }

  try {
    const Setting& new_if_cfg = udr_cfg[UDR_CONFIG_STRING_INTERFACES];
    const Setting& nudr_cfg   = new_if_cfg[UDR_CONFIG_STRING_INTERFACE_NUDR];
    load_interface(nudr_cfg, nudr);
    // HTTP2 port
    if (!(nudr_cfg.lookupValue(
            UDR_CONFIG_STRING_HTTP2_PORT, nudr_http2_port))) {
      Logger::udr_app().error(UDR_CONFIG_STRING_HTTP2_PORT "failed");
      throw(UDR_CONFIG_STRING_HTTP2_PORT "failed");
    }

  } catch (const SettingNotFoundException& nfex) {
    Logger::udr_app().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
    return RETURNerror;
  }

  // Support features
  try {
    const Setting& support_features =
        udr_cfg[UDR_CONFIG_STRING_SUPPORT_FEATURES];
    std::string opt = {};

    support_features.lookupValue(
        UDR_CONFIG_STRING_SUPPORT_FEATURES_USE_FQDN_DNS, opt);
    if (boost::iequals(opt, "yes")) {
      use_fqdn_dns = true;
    } else {
      use_fqdn_dns = false;
    }

    support_features.lookupValue(
        UDR_CONFIG_STRING_SUPPORTED_FEATURES_REGISTER_NRF, opt);
    if (boost::iequals(opt, "yes")) {
      register_nrf = true;
    } else {
      register_nrf = false;
    }

    support_features.lookupValue(
        UDM_CONFIG_STRING_SUPPORT_FEATURES_USE_HTTP2, opt);
    if (boost::iequals(opt, "yes")) {
      use_http2 = true;
    } else {
      use_http2 = false;
    }

    support_features.lookupValue(UDR_CONFIG_STRING_DATABASE_TYPE, opt);

    if (boost::iequals(opt, "cassandra")) {
      db_type = DB_TYPE_CASSANDRA;
    } else if (boost::iequals(opt, "mongodb")) {
      db_type = DB_TYPE_MONGO;
    } else {
      db_type = DB_TYPE_MYSQL;  // Default for now
    }

  } catch (const SettingNotFoundException& nfex) {
    Logger::udr_app().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
    return RETURNerror;
  }

  // NRF
  if (register_nrf) {
    try {
      std::string astring = {};

      const Setting& nrf_cfg       = udr_cfg[UDR_CONFIG_STRING_NRF];
      struct in_addr nrf_ipv4_addr = {};
      unsigned int nrf_port        = 0;
      std::string nrf_api_version  = {};

      if (!use_fqdn_dns) {
        nrf_cfg.lookupValue(UDR_CONFIG_STRING_NRF_IPV4_ADDRESS, astring);
        IPV4_STR_ADDR_TO_INADDR(
            util::trim(astring).c_str(), nrf_ipv4_addr,
            "BAD IPv4 ADDRESS FORMAT FOR NRF !");
        nrf_addr.ipv4_addr = nrf_ipv4_addr;
        if (!(nrf_cfg.lookupValue(UDR_CONFIG_STRING_NRF_PORT, nrf_port))) {
          Logger::udr_app().error(UDR_CONFIG_STRING_NRF_PORT "failed");
          throw(UDR_CONFIG_STRING_NRF_PORT "failed");
        }
        nrf_addr.port = nrf_port;
        nrf_addr.uri_root =
            util::trim(astring) + ":" + std::to_string(nrf_port);

        if (!(nrf_cfg.lookupValue(
                UDR_CONFIG_STRING_API_VERSION, nrf_api_version))) {
          Logger::udr_app().error(UDR_CONFIG_STRING_API_VERSION "failed");
          throw(UDR_CONFIG_STRING_API_VERSION "failed");
        }
        nrf_addr.api_version = nrf_api_version;

      } else {
        nrf_cfg.lookupValue(UDR_CONFIG_STRING_FQDN_DNS, astring);
        uint8_t addr_type   = {0};
        std::string address = {};
        fqdn::resolve(astring, address, nrf_port, addr_type);
        if (addr_type != 0) {  // IPv6
          // TODO:
          throw("DO NOT SUPPORT IPV6 ADDR FOR NRF!");
        } else {  // IPv4
          IPV4_STR_ADDR_TO_INADDR(
              util::trim(address).c_str(), nrf_ipv4_addr,
              "BAD IPv4 ADDRESS FORMAT FOR NRF !");
          nrf_addr.ipv4_addr = nrf_ipv4_addr;
          // We hardcode amf port from config for the moment
          if (!(nrf_cfg.lookupValue(UDR_CONFIG_STRING_NRF_PORT, nrf_port))) {
            Logger::udr_app().error(UDR_CONFIG_STRING_NRF_PORT "failed");
            throw(UDR_CONFIG_STRING_NRF_PORT "failed");
          }
          nrf_addr.port = nrf_port;
          nrf_addr.api_version =
              DEFAULT_SBI_API_VERSION;  // TODO: to get API version from DNS
          nrf_addr.fqdn = astring;
          nrf_addr.uri_root =
              util::trim(address) + ":" + std::to_string(nrf_port);
        }
      }
    } catch (const SettingNotFoundException& nfex) {
      Logger::udr_app().error("%s : %s", nfex.what(), nfex.getPath());
      return RETURNerror;
    }
  }

  try {
    const Setting& db_cfg = udr_cfg[UDR_CONFIG_STRING_DB];
    db_cfg.lookupValue(UDR_CONFIG_STRING_DB_SERVER, db_conf.server);
    db_cfg.lookupValue(UDR_CONFIG_STRING_DB_USER, db_conf.user);
    db_cfg.lookupValue(UDR_CONFIG_STRING_DB_PASS, db_conf.pass);
    db_cfg.lookupValue(UDR_CONFIG_STRING_DB_NAME, db_conf.db_name);
    db_cfg.lookupValue(
        UDR_CONFIG_STRING_DB_CONNECTION_TIMEOUT, db_conf.connection_timeout);
  } catch (const SettingNotFoundException& nfex) {
    Logger::udr_app().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
    return RETURNerror;
  }
  if (db_type == DB_TYPE_MYSQL) {
    db_conf.port = 3306;  // MySQL Default
  } else if (db_type == DB_TYPE_MONGO) {
    db_conf.port = 27017;  // MongoDB Default
  }

  return RETURNok;
}

//------------------------------------------------------------------------------
int udr_config::load_interface(
    const libconfig::Setting& if_cfg, interface_cfg_t& cfg) {
  if_cfg.lookupValue(UDR_CONFIG_STRING_INTERFACE_NAME, cfg.if_name);
  util::trim(cfg.if_name);
  if (not boost::iequals(cfg.if_name, "none")) {
    std::string address = {};
    if_cfg.lookupValue(UDR_CONFIG_STRING_IPV4_ADDRESS, address);
    util::trim(address);
    if (boost::iequals(address, "read")) {
      if (get_inet_addr_infos_from_iface(
              cfg.if_name, cfg.addr4, cfg.network4, cfg.mtu)) {
        Logger::udr_app().error(
            "Could not read %s network interface configuration", cfg.if_name);
        return RETURNerror;
      }
    } else {
      std::vector<std::string> words = {};
      boost::split(
          words, address, boost::is_any_of("/"), boost::token_compress_on);
      if (words.size() != 2) {
        Logger::udr_app().error(
            "Bad value " UDR_CONFIG_STRING_IPV4_ADDRESS " = %s in config file",
            address.c_str());
        return RETURNerror;
      }
      unsigned char buf_in_addr[sizeof(struct in6_addr)];  // you never know...
      if (inet_pton(AF_INET, util::trim(words.at(0)).c_str(), buf_in_addr) ==
          1) {
        memcpy(&cfg.addr4, buf_in_addr, sizeof(struct in_addr));
      } else {
        Logger::udr_app().error(
            "In conversion: Bad value " UDR_CONFIG_STRING_IPV4_ADDRESS
            " = %s in config file",
            util::trim(words.at(0)).c_str());
        return RETURNerror;
      }
      cfg.network4.s_addr = htons(
          ntohs(cfg.addr4.s_addr) &
          0xFFFFFFFF << (32 - std::stoi(util::trim(words.at(1)))));
    }
    if_cfg.lookupValue(UDR_CONFIG_STRING_PORT, cfg.port);

    // API VERSION
    if (!(if_cfg.lookupValue(UDR_CONFIG_STRING_API_VERSION, cfg.api_version))) {
      Logger::udr_app().error(UDR_CONFIG_STRING_API_VERSION "failed");
      throw(UDR_CONFIG_STRING_API_VERSION "failed");
    }
  }
  return RETURNok;
}

//------------------------------------------------------------------------------
void udr_config::display() {
  Logger::config().info(
      "==== OAI-CN5G %s v%s ====", PACKAGE_NAME, PACKAGE_VERSION);
  Logger::config().info(
      "======================    UDR   =====================");
  Logger::config().info("Configuration UDR:");
  Logger::config().info("- Instance ................: %d", instance);
  Logger::config().info("- PID dir .................: %s", pid_dir.c_str());
  Logger::config().info("- UDR Name ................: %s", udr_name.c_str());

  Logger::config().info("- Nudr:");
  Logger::config().info(
      "    Interface name ........: %s", nudr.if_name.c_str());
  Logger::config().info(
      "    IPv4 Addr .............: %s", inet_ntoa(nudr.addr4));
  Logger::config().info("    HTTP1 Port ............: %d", nudr.port);
  Logger::config().info("    HTTP2 port ............: %d", nudr_http2_port);
  Logger::config().info(
      "    API version ...........: %s", nudr.api_version.c_str());
  Logger::config().info("- Supported Features:");
  Logger::config().info(
      "    Register NRF ..........: %s", register_nrf ? "Yes" : "No");
  Logger::config().info(
      "    Use FQDN ..............: %s", use_fqdn_dns ? "Yes" : "No");
  Logger::config().info(
      "    Use HTTP2 .............: %s", use_http2 ? "Yes" : "No");
  Logger::config().info(
      "    Database ..............: %s", db_type_e2str[db_type].c_str());
  if (register_nrf) {
    Logger::config().info("- NRF:");
    Logger::config().info(
        "    URI root ...............: %s", nrf_addr.uri_root);
    Logger::config().info(
        "    IPv4 Addr .............: %s",
        inet_ntoa(*((struct in_addr*) &nrf_addr.ipv4_addr)));
    Logger::config().info("    Port ..................: %lu  ", nrf_addr.port);
    Logger::config().info(
        "    API version ...........: %s", nrf_addr.api_version.c_str());
    if (use_fqdn_dns)
      Logger::config().info(
          "    FQDN ..................: %s", nrf_addr.fqdn.c_str());
  }

  if (db_type == DB_TYPE_CASSANDRA) {
    Logger::config().info("- Cassandra:");
    Logger::config().info(
        "    Cassandra DB ..........: not "
        "supported!");
  } else if (db_type == DB_TYPE_UNKNOWN) {
    Logger::config().info("- Unknown DB:");
    Logger::config().info(
        "    ..........: not "
        "supported!");
  } else if (db_type == DB_TYPE_MYSQL or db_type == DB_TYPE_MONGO) {
    if (db_type == DB_TYPE_MYSQL) {
      Logger::config().info("- MySQL:");
    } else {
      Logger::config().info("- Mongo:");
    }
    Logger::config().info(
        "    Server Addr ...........: %s", db_conf.server.c_str());
    Logger::config().info("    Server Port ...........: %d", db_conf.port);
    Logger::config().info(
        "    Username ..............: %s", db_conf.user.c_str());
    Logger::config().info(
        "    Password ..............: %s", db_conf.pass.c_str());
    Logger::config().info(
        "    Database ..............: %s", db_conf.db_name.c_str());
    Logger::config().info(
        "    DB Timeout ............: %d (seconds)",
        db_conf.connection_timeout);
  }

  Logger::config().info(
      "- Log Level will be .......: %s",
      spdlog::level::to_string_view(log_level));
}

}  // namespace oai::udr::config
