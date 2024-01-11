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

/*! \file databse_wrapper.hpp
 \brief
 \author
 \company Eurecom
 \date 2022
 \email: contact@openairinterface.org
 */

#ifndef DATABASE_WRAPPER_HPP
#define DATABASE_WRAPPER_HPP

#include <nlohmann/json.hpp>

#include "Snssai.h"
#include "database_wrapper_abstraction.hpp"
#include "logger.hpp"
#include "udr.h"

namespace oai::udr::app {

template<class DerivedT>
class database_wrapper : public database_wrapper_abstraction {
 public:
  database_wrapper(){};
  virtual ~database_wrapper(){};

  /*std::unique_ptr<database_wrapper_abstraction> clone() const override {
          return std::make_unique<DerivedT>(static_cast<DerivedT
     const&>(*this));
      }
  */

  bool initialize() override {
    Logger::udr_app().debug("Initialize from database_wrapper");
    auto derived = static_cast<DerivedT*>(this);
    return derived->initialize();
  }

  bool connect(uint32_t num_retries) {
    Logger::udr_app().debug(
        "Establish the connection to the DB (from database_wrapper)");
    auto derived = static_cast<DerivedT*>(this);
    return derived->connect(num_retries);
  }
  bool close_connection() override {
    Logger::udr_app().debug("Initialize from database_wrapper");
    auto derived = static_cast<DerivedT*>(this);
    return derived->close_connection();
  }

  void start_event_connection_handling() {
    auto derived = static_cast<DerivedT*>(this);
    return derived->start_event_connection_handling();
  }

  void trigger_connection_handling_procedure(uint64_t ms) {
    auto derived = static_cast<DerivedT*>(this);
    return derived->trigger_connection_handling_procedure(ms);
  }

  bool insert_authentication_subscription(
      const std::string& id,
      const oai::udr::model::AuthenticationSubscription&
          authentication_subscription,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->insert_authentication_subscription(
        id, authentication_subscription, json_data);
  }

  bool delete_authentication_subscription(
      const std::string& id, nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->delete_authentication_subscription(id, json_data);
  }

  bool query_authentication_subscription(
      const std::string& id, nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_authentication_subscription(id, json_data);
  }

  bool update_authentication_subscription(
      const std::string& id,
      const std::vector<oai::model::common::PatchItem>& patchItem,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->update_authentication_subscription(
        id, patchItem, json_data);
  }

  bool query_am_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_am_data(ue_id, serving_plmn_id, json_data);
  }

  bool create_amf_context_3gpp(
      const std::string& ue_id,
      oai::udr::model::Amf3GppAccessRegistration& amf3GppAccessRegistration,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->create_amf_context_3gpp(
        ue_id, amf3GppAccessRegistration, json_data);
  }

  bool query_amf_context_3gpp(
      const std::string& ue_id, nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_amf_context_3gpp(ue_id, json_data);
  }

  bool insert_authentication_status(
      const std::string& ue_id, const oai::udr::model::AuthEvent& authEvent,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->insert_authentication_status(ue_id, authEvent, json_data);
  }

  bool delete_authentication_status(
      const std::string& ue_id, nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->delete_authentication_status(ue_id, json_data);
  }

  bool query_authentication_status(
      const std::string& ue_id, nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_authentication_status(ue_id, json_data);
  }

  bool query_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_sdm_subscription(ue_id, subs_id, json_data);
  }

  bool delete_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->delete_sdm_subscription(ue_id, subs_id, json_data);
  }

  bool update_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->update_sdm_subscription(
        ue_id, subs_id, sdmSubscription, json_data);
  }

  bool create_sdm_subscriptions(
      const std::string& ue_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->create_sdm_subscriptions(ue_id, sdmSubscription, json_data);
  }

  bool query_sdm_subscriptions(
      const std::string& ue_id, nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_sdm_subscriptions(ue_id, json_data);
  }

  bool query_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data,
      const std::optional<oai::model::common::Snssai>& snssai,
      const std::optional<std::string>& dnn) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_sm_data(
        ue_id, serving_plmn_id, json_data, snssai, dnn);
  }

  bool query_sm_data(nlohmann::json& json_data) {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_sm_data(json_data);
  }

  bool delete_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      const std::optional<oai::model::common::Snssai>& snssai) {
    auto derived = static_cast<DerivedT*>(this);
    return derived->delete_sm_data(ue_id, serving_plmn_id, snssai);
  }

  bool create_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      oai::udr::model::SessionManagementSubscriptionData& sm_subscription,
      nlohmann::json& json_data, uint32_t& resource_id) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->create_sm_data(
        ue_id, serving_plmn_id, sm_subscription, json_data, resource_id);
  }

  bool update_sm_data(
      const std::string& ueId, const std::string& servingPlmnId,
      oai::udr::model::SessionManagementSubscriptionData& subscriptionData,
      nlohmann::json& json_data, uint32_t& resource_id) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->update_sm_data(
        ueId, servingPlmnId, subscriptionData, json_data, resource_id);
  }

  bool insert_smf_context_non_3gpp(
      const std::string& ue_id, const int32_t& pdu_session_id,
      const oai::udr::model::SmfRegistration& smfRegistration,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->insert_smf_context_non_3gpp(
        ue_id, pdu_session_id, smfRegistration, json_data);
  }

  bool delete_smf_context(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->delete_smf_context(ue_id, pdu_session_id, json_data);
  }

  bool query_smf_registration(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_smf_registration(ue_id, pdu_session_id, json_data);
  }

  bool query_smf_reg_list(
      const std::string& ue_id, nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_smf_reg_list(ue_id, json_data);
  }

  bool query_smf_select_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) override {
    auto derived = static_cast<DerivedT*>(this);
    return derived->query_smf_select_data(ue_id, serving_plmn_id, json_data);
  }

 protected:
};
}  // namespace oai::udr::app

#endif
