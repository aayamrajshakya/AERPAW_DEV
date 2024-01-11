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

#include "Helpers.h"
#include "smf_config.hpp"
#include "SMFConfigurationApi.h"

extern std::unique_ptr<oai::config::smf::smf_config> smf_cfg;

namespace oai::smf_server::api {

using namespace oai::model::common::helpers;

SMFConfigurationApi::SMFConfigurationApi(
    std::shared_ptr<Pistache::Rest::Router> rtr) {
  router = rtr;
}

void SMFConfigurationApi::init() {
  setupRoutes();
}

void SMFConfigurationApi::setupRoutes() {
  using namespace Pistache::Rest;

  Routes::Get(
      *router,
      base + smf_cfg->sbi_api_version + NSMF_CUSTOMIZED_API_CONFIGURATION_URL,
      Routes::bind(&SMFConfigurationApi::read_configuration_handler, this));

  Routes::Put(
      *router,
      base + smf_cfg->sbi_api_version + NSMF_CUSTOMIZED_API_CONFIGURATION_URL,
      Routes::bind(&SMFConfigurationApi::update_configuration_handler, this));

  // Default handler, called when a route is not found
  router->addCustomHandler(Routes::bind(
      &SMFConfigurationApi::configuration_api_default_handler, this));
}

void SMFConfigurationApi::read_configuration_handler(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    this->read_configuration(response);
  } catch (nlohmann::detail::exception& e) {
    // send a 400 error
    response.send(Pistache::Http::Code::Bad_Request, e.what());
    return;
  } catch (Pistache::Http::HttpError& e) {
    response.send(static_cast<Pistache::Http::Code>(e.code()), e.what());
    return;
  } catch (std::exception& e) {
    // send a 500 error
    response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
    return;
  }
}

void SMFConfigurationApi::update_configuration_handler(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    auto configuration_info = nlohmann::json::parse(request.body());
    this->update_configuration(configuration_info, response);
  } catch (nlohmann::detail::exception& e) {
    // send a 400 error
    response.send(Pistache::Http::Code::Bad_Request, e.what());
    return;
  } catch (Pistache::Http::HttpError& e) {
    response.send(static_cast<Pistache::Http::Code>(e.code()), e.what());
    return;
  } catch (std::exception& e) {
    // send a 500 error
    response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
    return;
  }
}

void SMFConfigurationApi::configuration_api_default_handler(
    const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response) {
  response.send(
      Pistache::Http::Code::Not_Found, "The requested method does not exist");
}

}  // namespace oai::smf_server::api
