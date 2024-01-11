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

/*! \file udr_http2-server.h
 \brief
 \author  Tien-Thinh NGUYEN
 \company Eurecom
 \date 2021
 \email: tien-thinh.nguyen@eurecom.fr
 */

#ifndef FILE_UDR_HTTP2_SERVER_SEEN
#define FILE_UDR_HTTP2_SERVER_SEEN

#include <nghttp2/asio_http2_server.h>

#include "Amf3GppAccessRegistration.h"
#include "AuthEvent.h"
#include "PatchItem.h"
#include "SdmSubscription.h"
#include "SmfRegistration.h"
#include "udr_app.hpp"

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

class udr_http2_server {
 public:
  udr_http2_server(
      std::string addr, uint32_t port, oai::udr::app::udr_app* udr_app_inst)
      : m_address(addr), m_port(port), server(), m_udr_app(udr_app_inst) {}
  void start();
  void init(size_t thr) {}

  void query_am_data_handler(
      const std::string& ue_id, const std::string& serving_plmn_id,
      const response& response);

  void create_amf_context_3gpp_handler(
      const std::string& ue_id,
      oai::udr::model::Amf3GppAccessRegistration& amf3GppAccessRegistration,
      const response& response);

  void query_amf_context_3gpp_handler(
      const std::string& ue_id, const response& response);

  void create_authentication_status_handler(
      const std::string& ue_id, const oai::udr::model::AuthEvent& authEvent,
      const response& response);

  void delete_authentication_status_handler(
      const std::string& ue_id, const response& response);

  void query_authentication_status_handler(
      const std::string& ue_id, const response& response);

  void modify_authentication_subscription_handler(
      const std::string& ue_id,
      const std::vector<oai::model::common::PatchItem>& patchItem,
      const response& response);

  void read_authentication_subscription_handler(
      const std::string& ue_id, const response& response);

  void create_authentication_subscription_handler(
      const std::string& ue_id,
      const oai::udr::model::AuthenticationSubscription&
          authentication_subscription,
      const response& response);

  void delete_authentication_subscription_handler(
      const std::string& ue_id, const response& response);

  void query_sdm_subscription_handler(
      const std::string& ue_id, const std::string& subs_id,
      const response& response);

  void remove_sdm_subscription_handler(
      const std::string& ue_id, const std::string& subs_id,
      const response& response);

  void modify_sdm_subscription_handler(
      const std::string& ue_id, const std::string& subs_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      const response& response);

  void update_sdm_subscription_handler(
      const std::string& ue_id, const std::string& subs_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      const response& response);

  void create_sdm_subscriptions_handler(
      const std::string& ue_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      const response& response);

  void query_sdm_subscriptions_handler(
      const std::string& ue_id, const response& response);

  void query_sm_data_handler(
      const std::string& ue_id, const std::string& serving_plmn_id,
      const response& response,
      std::optional<oai::model::common::Snssai>& snssai,
      std::optional<std::string>& dnn);

  void query_sm_data_handler(const response& response);

  void create_sm_data_handler(
      const std::string& ue_id, const std::string& serving_plmn_id,
      oai::udr::model::SessionManagementSubscriptionData& subscription_data,
      const response& response);

  void update_sm_data_handler(
      const std::string& ue_id, const std::string& serving_plmn_id,
      oai::udr::model::SessionManagementSubscriptionData& subscription_data,
      const response& response);

  void delete_sm_data_handler(
      const std::string& ue_id, const std::string& serving_plmn_id,
      std::optional<oai::model::common::Snssai>& snssai,
      const response& response);

  void create_smf_context_non_3gpp_handler(
      const std::string& ue_id, const int32_t& pdu_session_id,
      const oai::udr::model::SmfRegistration& smfRegistration,
      const response& response);

  void delete_smf_context_handler(
      const std::string& ue_id, const int32_t& pdu_session_id,
      const response& response);

  void query_smf_registration_handler(
      const std::string& ue_id, const int32_t& pdu_session_id,
      const response& response);

  void query_smf_reg_list_handler(
      const std::string& ue_id, const response& response);

  void query_smf_select_data_handler(
      const std::string& ue_id, const std::string& serving_plmn_id,
      const response& response);

  void read_configuration_handler(const response& response);

  void update_configuration_handler(
      nlohmann::json& configuration_info, const response& response);

  void stop();

 private:
  std::string m_address;
  uint32_t m_port;
  http2 server;
  oai::udr::app::udr_app* m_udr_app;
};

#endif
