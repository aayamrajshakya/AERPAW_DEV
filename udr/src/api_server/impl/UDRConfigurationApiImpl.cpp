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

#include <UDRConfigurationApiImpl.h>

#include "3gpp_29.500.h"
#include "logger.hpp"

namespace oai::udr::api {

using namespace oai::udr::model;

UDRConfigurationApiImpl::UDRConfigurationApiImpl(
    std::shared_ptr<Pistache::Rest::Router> rtr, udr_app* udr_app_inst)
    : UDRConfigurationApi(rtr), m_udr_app(udr_app_inst) {}

void UDRConfigurationApiImpl::read_configuration(
    Pistache::Http::ResponseWriter& response) {
  Logger::udr_server().debug("Receive UDRConfiguration, handling...");

  nlohmann::json response_data = {};
  Pistache::Http::Code code    = {};
  long http_code               = 0;

  m_udr_app->handle_read_configuration(response_data, http_code);

  code = static_cast<Pistache::Http::Code>(http_code);
  Logger::udr_server().debug("HTTP Response code %d.\n", (int) code);
  // content type
  response.headers().add<Pistache::Http::Header::ContentType>(
      Pistache::Http::Mime::MediaType("application/json"));
  response.send(code, response_data.dump().c_str());
}

void UDRConfigurationApiImpl::update_configuration(
    nlohmann::json& configuration_info,
    Pistache::Http::ResponseWriter& response) {
  Logger::udr_server().debug("Update UDRConfiguration, handling...");

  Pistache::Http::Code code = {};
  long http_code            = 0;

  m_udr_app->handle_update_configuration(configuration_info, http_code);

  code = static_cast<Pistache::Http::Code>(http_code);
  Logger::udr_server().debug("HTTP Response code %d.\n", (int) code);
  if ((code == Pistache::Http::Code::Ok) or
      (code == Pistache::Http::Code::Accepted) or
      (code == Pistache::Http::Code::Created)) {
    // content type
    response.headers().add<Pistache::Http::Header::ContentType>(
        Pistache::Http::Mime::MediaType("application/json"));
    response.send(code, configuration_info.dump().c_str());
  } else {
    response.send(code);
  }
}

}  // namespace oai::udr::api
