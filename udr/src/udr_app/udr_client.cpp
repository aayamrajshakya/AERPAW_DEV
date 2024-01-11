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

/*! \file udr_client.cpp
 \brief
 \author
 \company Eurecom
 \date 2020
 \email: contact@openairinterface.org
 */

#include "udr_client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "3gpp_29.500.h"
#include "logger.hpp"
#include "udr.h"

using namespace oai::udr::app;
using namespace oai::udr::config;

extern udr_config udr_cfg;

//------------------------------------------------------------------------------
// To read content of the response from NF
static std::size_t callback(
    const char* in, std::size_t size, std::size_t num, std::string* out) {
  const std::size_t totalBytes(size * num);
  out->clear();
  out->append(in, totalBytes);
  return totalBytes;
}

//------------------------------------------------------------------------------
udr_client::udr_client() {}

//------------------------------------------------------------------------------
udr_client::~udr_client() {
  Logger::udr_app().debug("Delete UDR Client instance...");
}

//------------------------------------------------------------------------------
bool udr_client::curl_http_client(
    std::string remote_uri, std::string method, std::string msg_body,
    std::string& response, long& response_code) {
  Logger::udr_app().info("Send HTTP message with body %s", msg_body.c_str());
  Logger::udr_app().info("Server URI %s", remote_uri.c_str());

  bool result = false;

  uint32_t str_len = msg_body.length();
  char* body_data  = (char*) malloc(str_len + 1);
  memset(body_data, 0, str_len + 1);
  memcpy((void*) body_data, (void*) msg_body.c_str(), str_len);

  curl_global_init(CURL_GLOBAL_ALL);
  CURL* curl = curl_easy_init();

  uint8_t http_version = 1;
  if (udr_cfg.use_http2) http_version = 2;

  bool is_response_ok = false;

  if (curl) {
    CURLcode res               = {};
    struct curl_slist* headers = nullptr;
    if ((method.compare("POST") == 0) or (method.compare("PUT") == 0) or
        (method.compare("PATCH") == 0)) {
      std::string content_type = "Content-Type: application/json";
      headers = curl_slist_append(headers, content_type.c_str());
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    curl_easy_setopt(curl, CURLOPT_URL, remote_uri.c_str());
    if (method.compare("POST") == 0)
      curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
    else if (method.compare("PUT") == 0)
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    else if (method.compare("DELETE") == 0)
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    else if (method.compare("PATCH") == 0)
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    else
      curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, NF_CURL_TIMEOUT_MS);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);
    curl_easy_setopt(curl, CURLOPT_INTERFACE, udr_cfg.nudr.if_name.c_str());

    if (http_version == 2) {
      if (Logger::should_log(spdlog::level::debug))
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
      // we use a self-signed test server, skip verification during debugging
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      curl_easy_setopt(
          curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
    }

    // Response information
    std::unique_ptr<std::string> httpData(new std::string());
    std::unique_ptr<std::string> httpHeaderData(new std::string());

    // Hook up data handling function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, httpHeaderData.get());

    if ((method.compare("POST") == 0) or (method.compare("PUT") == 0) or
        (method.compare("PATCH") == 0)) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, msg_body.length());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_data);
    }

    int num_retries = 0;
    while (num_retries < MAX_CURL_RETRY) {
      num_retries++;
      res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        // Sleep between two consecutive retries
        usleep(TIME_INTERVAL_CURL_RETRY * pow(2, num_retries - 1));
        Logger::udr_app().debug("Curl retry %d ...", num_retries);
        continue;
      } else {
        break;
      }
    }

    if (res != CURLE_OK) {
      Logger::udr_app().debug(
          "Still could not reach the destination after %d retries",
          MAX_CURL_RETRY);
    } else {
      result = true;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      Logger::udr_app().debug(
          "Get response with HTTP code (%d)", response_code);

      if (response_code == HTTP_STATUS_CODE_200_OK or
          response_code == HTTP_STATUS_CODE_201_CREATED or
          response_code == HTTP_STATUS_CODE_204_NO_CONTENT) {
        // TODO
        is_response_ok = true;
      }

      // Process the response
      response = *httpData.get();
      if (!response.empty())
        Logger::udr_app().info(
            "Get response with Json data: %s", response.c_str());
      nlohmann::json response_data = {};
      //    std::string cause = {};
      if (!is_response_ok) {
        try {
          response_data = nlohmann::json::parse(response);
          // cause = response_data["error"]["cause"];
        } catch (nlohmann::json::exception& e) {
          Logger::udr_app().info(
              "Could not get Json content from the response");
          // Set the default Cause
          // response_data["error"]["cause"] = "504 Gateway Timeout";
          // cause = response_data["error"]["cause"];
        }

        // Logger::udr_app().warn("Curl Request failed");
        // Logger::udr_app().info("Cause value: %s", cause.c_str());
        // TODO:
      }
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  } else {
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  if (body_data) {
    free(body_data);
    body_data = nullptr;
  }
  return result;
}
