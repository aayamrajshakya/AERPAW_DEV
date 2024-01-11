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

#ifndef SMFConfigurationApi_H_
#define SMFConfigurationApi_H_

#include <pistache/http.h>
#include <pistache/http_headers.h>
#include <pistache/optional.h>
#include <pistache/router.h>

#include <nlohmann/json.hpp>

#include "smf.h"

namespace oai::smf_server::api {

class SMFConfigurationApi {
 public:
  SMFConfigurationApi(std::shared_ptr<Pistache::Rest::Router>);
  virtual ~SMFConfigurationApi() {}
  void init();

  const std::string base = NSMF_CUSTOMIZED_API_BASE;

 private:
  void setupRoutes();

  void read_configuration_handler(
      const Pistache::Rest::Request& request,
      Pistache::Http::ResponseWriter response);
  void update_configuration_handler(
      const Pistache::Rest::Request& request,
      Pistache::Http::ResponseWriter response);
  void configuration_api_default_handler(
      const Pistache::Rest::Request& request,
      Pistache::Http::ResponseWriter response);

  std::shared_ptr<Pistache::Rest::Router> router;

  virtual void read_configuration(Pistache::Http::ResponseWriter& response) = 0;
  virtual void update_configuration(
      nlohmann::json& configuration_info,
      Pistache::Http::ResponseWriter& response) = 0;
};

}  // namespace oai::smf_server::api

#endif
