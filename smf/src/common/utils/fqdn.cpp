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

#include "fqdn.hpp"
#include "logger.hpp"
#include "string.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#define MAX_NB_RESOLVE_TRIES 4
#define TIME_BETWEEN_TRIES 2

bool fqdn::resolve(
    const std::string& host_name, std::string& address, uint32_t& port,
    uint8_t& addr_type, const std::string& protocol) {
  int tries = 0;
  // Logger::smf_app().debug("Resolving a DNS (name %s)", host_name.c_str());
  while (tries < MAX_NB_RESOLVE_TRIES) {
    try {
      boost::asio::io_context io_context = {};
      // Logger::smf_app().debug("Resolving DNS Try #%u", tries + 1);

      boost::asio::ip::tcp::resolver resolver{io_context};
      boost::asio::ip::tcp::resolver::results_type endpoints =
          resolver.resolve(host_name, protocol);

      addr_type = 0;  // IPv4 by default
      for (auto it = endpoints.cbegin(); it != endpoints.cend(); it++) {
        // get the first Endpoint
        boost::asio::ip::tcp::endpoint endpoint = *it;
        address = endpoint.address().to_string();
        port    = endpoint.port();
        Logger::smf_app().debug(
            "Resolved a DNS (name %s, protocol %s): Ip Addr %s, port %u",
            host_name.c_str(), protocol.c_str(), address.c_str(), port);
        if (endpoint.address().is_v4())
          addr_type = 0;
        else
          addr_type = 1;
        return true;
      }
    } catch (std::exception& e) {
      tries++;
      if (tries == MAX_NB_RESOLVE_TRIES) {
        throw std::runtime_error(
            "Cannot resolve a DNS name " + std::string(e.what()) + " after " +
            std::to_string(tries) + " tries");
        return false;
      }
      std::this_thread::sleep_for(std::chrono::seconds(TIME_BETWEEN_TRIES));
    }
  }

  return false;
}

bool fqdn::resolve(pfcp::node_id_t& node_id) {
  Logger::smf_app().debug("Resolving an FQDN/IP Addr for an UPF node");

  // Resolve IP addr from FQDN
  if (node_id.node_id_type == pfcp::NODE_ID_TYPE_FQDN) {
    // Don't need to do if IP addr already available
    if (node_id.u1.ipv4_address.s_addr != INADDR_ANY) return true;
    // Resolve if FQDN available
    if (!node_id.fqdn.empty()) {
      Logger::smf_app().debug("FQDN %s", node_id.fqdn.c_str());
      std::string ip_addr_str  = {};
      uint32_t port            = {0};
      uint8_t addr_type        = {0};
      struct in_addr ipv4_addr = {};

      if (!fqdn::resolve(node_id.fqdn, ip_addr_str, port, addr_type)) {
        Logger::smf_app().warn(
            "Resolve FQDN %s: cannot resolve the hostname!",
            node_id.fqdn.c_str());
        return false;
      }
      switch (addr_type) {
        case 0: {
          node_id.u1.ipv4_address.s_addr = conv::fromString(ip_addr_str).s_addr;
          Logger::smf_app().debug(
              "Resolve FQDN %s, IP Addr %s", node_id.fqdn.c_str(),
              ip_addr_str.c_str());
        } break;
        case 1: {
          // TODO
          Logger::smf_app().warn(
              "Resolve FQDN: %s. IPv6 Addr, this mode has not been "
              "supported yet!",
              node_id.fqdn.c_str());
          return false;
        } break;
        default:
          Logger::smf_app().warn("Unknown Address type");
          return false;
      }
    } else {
      return false;  // No FQDN available
    }
    // Resolve hostname from an IP Addr
  } else if (node_id.node_id_type == pfcp::NODE_ID_TYPE_IPV4_ADDRESS) {
    // Don't need to do reserve_resolve if FQDN is already available
    if (!node_id.fqdn.empty()) {
      return true;
    } else {
      std::string hostname = {};
      std::string ip_str   = conv::toString(node_id.u1.ipv4_address);
      if (!fqdn::reverse_resolve(ip_str, hostname)) {
        Logger::smf_app().warn(
            "Could not resolve hostname for IP address %s", ip_str.c_str());
        return false;
      } else {
        node_id.fqdn = hostname;
        Logger::smf_app().debug(
            "Resolve IP Addr %s, FQDN %s", ip_str.c_str(),
            node_id.fqdn.c_str());
      }
      return true;
    }
  } else {
    // Don't support IPv6 for the moment
    return false;
  }

  return true;
}

bool fqdn::reverse_resolve(const std::string& ip_addr, std::string& host_name) {
  Logger::smf_app().debug("Resolving an IP address (name %s)", ip_addr.c_str());
  int tries = 0;
  while (tries < MAX_NB_RESOLVE_TRIES) {
    try {
      boost::asio::io_context io_context = {};
      Logger::smf_app().debug("Reverse Resolving Try #%u", tries);

      boost::asio::ip::address addr =
          boost::asio::ip::address::from_string(ip_addr);
      boost::asio::ip::tcp::endpoint ep;
      ep.address(addr);

      boost::asio::ip::tcp::resolver resolver(io_context);
      boost::asio::ip::tcp::resolver::results_type endpoints =
          resolver.resolve(ep);

      for (const auto& endpoint : endpoints) {
        // get the first endpoint
        host_name = endpoint.host_name();
        return true;
      }
    } catch (std::exception& e) {
      tries++;
      if (tries == MAX_NB_RESOLVE_TRIES) {
        Logger::smf_app().warn(
            "Cannot resolve IP address %s: %s -- After %d tries",
            ip_addr.c_str(), e.what(), tries);
        return false;
      }
      std::this_thread::sleep_for(std::chrono::seconds(TIME_BETWEEN_TRIES));
    }
  }
  return false;
}
