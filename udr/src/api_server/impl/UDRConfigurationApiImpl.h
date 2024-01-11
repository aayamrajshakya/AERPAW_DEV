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

#ifndef UDR_CONFIGURATION_API_IMPL_H_
#define UDR_CONFIGURATION_API_IMPL_H_

#include "udr_app.hpp"
#include <UDRConfigurationApi.h>
#include <pistache/http.h>
#include <pistache/optional.h>

namespace oai::udr::api {

using namespace oai::udr::model;
using namespace oai::udr::app;

class UDRConfigurationApiImpl : public oai::udr::api::UDRConfigurationApi {
 private:
  udr_app* m_udr_app;

 public:
  UDRConfigurationApiImpl(
      std::shared_ptr<Pistache::Rest::Router>, udr_app* udr_app_inst);
  ~UDRConfigurationApiImpl() {}

  void read_configuration(Pistache::Http::ResponseWriter& response);

  void update_configuration(
      nlohmann::json& configuration_info,
      Pistache::Http::ResponseWriter& response);
};

}  // namespace oai::udr::api

#endif
