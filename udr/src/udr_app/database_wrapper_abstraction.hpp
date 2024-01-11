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

/*! \file databse_wrapper_abstraction.hpp
 \brief
 \author
 \company Eurecom
 \date 2022
 \email: contact@openairinterface.org
 */

#ifndef DATABASE_WRAPPER_ABSTRACTION_HPP
#define DATABASE_WRAPPER_ABSTRACTION_HPP

#include <optional>
#include <nlohmann/json.hpp>

#include "Amf3GppAccessRegistration.h"
#include "AuthEvent.h"
#include "AuthenticationSubscription.h"
#include "PatchItem.h"
#include "SdmSubscription.h"
#include "SmfRegistration.h"
#include "Snssai.h"
#include "logger.hpp"
#include "udr.h"

namespace oai::udr::app {

class database_wrapper_abstraction {
 public:
  database_wrapper_abstraction(){};
  virtual ~database_wrapper_abstraction(){};
  // virtual std::unique_ptr<database_wrapper_abstraction> clone() const = 0;

  /*
   * Initialize the DB
   * @param void
   * @return true if successful, otherwise return false
   */
  virtual bool initialize() = 0;

  /*
   * Establish the connection between UDR and the DB
   * @param [uint32_t] num_retries: Number of retires
   * @return true if successful, otherwise return false
   */
  virtual bool connect(uint32_t num_retries) = 0;

  /*
   * Close the connection established to the DB
   * @param void
   * @return true if successful, otherwise return false
   */
  virtual bool close_connection() = 0;

  /*
   * Start event connection handling procedure
   * @param [void]
   * @return void
   */
  virtual void start_event_connection_handling() = 0;

  /*
   * Trigger connection handling procedure (kind of NF Heartbeat)
   * @param [uint64_t] ms:
   * @return void
   */
  virtual void trigger_connection_handling_procedure(uint64_t ms) = 0;

  /*
   * Insert a new item to the DB for the Authentication Subscription
   * @param [const std::string&] id: UE Identity
   * @param [const oai::udr::model::AuthenticationSubscription&]
   * authentication_subscription: Authentication data
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool insert_authentication_subscription(
      const std::string& id,
      const oai::udr::model::AuthenticationSubscription&
          authentication_subscription,
      nlohmann::json& json_data) = 0;

  /*
   * Delete an item from the DB for the Authentication Subscription
   * @param [const std::string&] id: UE Identity
   * @return true if successful, otherwise return false
   */
  virtual bool delete_authentication_subscription(
      const std::string& id, nlohmann::json& json_data) = 0;

  /*
   * Query an item from the DB for the Authentication Subscription
   * @param [const std::string&] id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool query_authentication_subscription(
      const std::string& id, nlohmann::json& json_data) = 0;

  /*
   * Update an item from the DB for the Authentication Subscription
   * @param [const std::string&] id: UE Identity
   * @param [const std::vector<oai::udr::model::PatchItem>&] patchItem:
   * patchItem
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool update_authentication_subscription(
      const std::string& id,
      const std::vector<oai::model::common::PatchItem>& patchItem,
      nlohmann::json& json_data) = 0;

  /*
   *  Query an item from the DB for AccessandMobilitySubscriptionData
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string& ] serving_plmn_id: Serving PLMN ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool query_am_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) = 0;

  /*
   * Insert an item into DB for AMF3GPPAccessRegistration Context
   * @param [const std::string&] ue_id: UE Identity
   * @param [oai::udr::model::Amf3GppAccessRegistration&]
   * amf3GppAccessRegistration: Context to be stored
   * @return true if successful, otherwise return false
   */
  virtual bool create_amf_context_3gpp(
      const std::string& ue_id,
      oai::udr::model::Amf3GppAccessRegistration& amf3GppAccessRegistration,
      nlohmann::json& json_data) = 0;

  /*
   *  Query for an item from the DB for AMF3GPPAccessRegistration
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @param [long code] code: HTTP response code
   * @return true if successful, otherwise return false
   */
  virtual bool query_amf_context_3gpp(
      const std::string& ue_id, nlohmann::json& json_data) = 0;

  /*
   *  Insert a new item into the DB for AuthenticationStatus
   * @param [const std::string&] ue_id: UE Identity
   * @param [const AuthEvent&] authEvent: Authentication Status data
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool insert_authentication_status(
      const std::string& ue_id, const oai::udr::model::AuthEvent& authEvent,
      nlohmann::json& json_data) = 0;

  /*
   *  Delete an item from the DB for AuthenticationStatus
   * @param [const std::string&] ue_id: UE Identity
   * @return true if successful, otherwise return false
   */
  virtual bool delete_authentication_status(
      const std::string& ue_id, nlohmann::json& json_data) = 0;

  /*
   * Query for an item from the DB for AuthenticationStatus
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool query_authentication_status(
      const std::string& ue_id, nlohmann::json& json_data) = 0;

  /*
   * Query an item from the DB for SDMSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] subs_id: subscription ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool query_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data) = 0;

  /*
   * Delete an item from the DB for SDMSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] subs_id: subscription ID
   * @return true if successful, otherwise return false
   */
  virtual bool delete_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data) = 0;

  /*
   * Update an item from the DB for SDMSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] subs_id: subscription ID
   * @param [oai::udr::model::SdmSubscription&] sdmSubscription: Subscription
   * information
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool update_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      nlohmann::json& json_data) = 0;

  /*
   * Insert a new item into the DB for SDMSubscriptions
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @param [oai::udr::model::SdmSubscription&] sdmSubscription: Subscription
   * information
   * @return true if successful, otherwise return false
   */
  virtual bool create_sdm_subscriptions(
      const std::string& ue_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      nlohmann::json& json_data) = 0;

  /*
   * Query an item from the DB for SDMSubscriptions
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool query_sdm_subscriptions(
      const std::string& ue_id, nlohmann::json& json_data) = 0;

  /*
   * Query an item from the DB for SessionManagementSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] serving_plmn_id: Serving PLMN ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @param [const oai::udr::model::Snssai&] snssai: SNSSAI
   * @param [const std::string&] dnn: DNN
   * @return true if successful, otherwise return false
   */
  virtual bool query_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data,
      const std::optional<oai::model::common::Snssai>& snssai,
      const std::optional<std::string>& dnn) = 0;

  /*
   * Query all items from the DB for SessionManagementSubscription
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool query_sm_data(nlohmann::json& json_data) = 0;

  /*
   * Delete an item from the DB for SessionManagementSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] serving_plmn_id: Serving PLMN ID
   * @param [const oai::udr::model::Snssai&] snssai: SNSSAI
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool delete_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      const std::optional<oai::model::common::Snssai>& snssai) = 0;

  /*
   * Insert a new item into the DB for SessionManagementSubscription
   * @param [const std::string&] ue_id: User Id (Imsi/supi)
   * @param [const std::string&] serving_plmn_id: Serving PLMN
   * @param [const oai::udr::model::SessionManagementSubscriptionData&]
   * subscription_data: subscription
   * @param [nlohmann::json&] json_data: Data in Json format
   * @param [uint32_t&] resource_id: ID represent the resource location
   * @return true if successful, otherwise return false
   */
  virtual bool create_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      oai::udr::model::SessionManagementSubscriptionData& subscription_data,
      nlohmann::json& json_data, uint32_t& resource_id) = 0;

  /*
   * Update an item from the DB for SessionManagementSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] serving_plmn_id: Serving PLMN ID
   * @param [const oai::udr::model::SessionManagementSubscriptionData&]
   * SessionManagementSubscription: subscription
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool update_sm_data(
      const std::string& ueId, const std::string& servingPlmnId,
      oai::udr::model::SessionManagementSubscriptionData& subscriptionData,
      nlohmann::json& json_data, uint32_t& resource_id) = 0;

  /*
   * Insert an item into the DB for SMFRegistration
   * @param [const std::string&] ue_id: UE Identity
   * @param [const int32_t&] pdu_session_id: PDU Session ID
   * @param [const oai::udr::model::SmfRegistration&] smfRegistration: SMF
   * Registration data
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool insert_smf_context_non_3gpp(
      const std::string& ue_id, const int32_t& pdu_session_id,
      const oai::udr::model::SmfRegistration& smfRegistration,
      nlohmann::json& json_data) = 0;

  /*
   * Delete an item from the DB for SMFRegistration
   * @param [const std::string&] ue_id: UE Identity
   * @param [const int32_t&] pdu_session_id: PDU Session ID
   * @return true if successful, otherwise return false
   */
  virtual bool delete_smf_context(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data) = 0;

  /*
   * Query an item from the DB SMFRegistration
   * @param [const std::string&] ue_id: UE Identity
   * @param [const int32_t&] pdu_session_id: PDU Session ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool query_smf_registration(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data) = 0;

  /*
   * Query an item from the DB for SMFRegistrationsCollection
   * @param [const std::string&] ue_id: UE Identity
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool query_smf_reg_list(
      const std::string& ue_id, nlohmann::json& json_data) = 0;

  /*
   * Query an item from the DB for SMFSelectionSubscription
   * @param [const std::string&] ue_id: UE Identity
   * @param [const std::string&] serving_plmn_id: Serving PLMN ID
   * @param [nlohmann::json&] json_data: Data in Json format
   * @return true if successful, otherwise return false
   */
  virtual bool query_smf_select_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data) = 0;
};
}  // namespace oai::udr::app

#endif
