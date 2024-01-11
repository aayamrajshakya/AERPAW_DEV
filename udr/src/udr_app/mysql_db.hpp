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

/*! \file mysql_db.hpp
 \brief
 \author
 \company Eurecom
 \date 2022
 \email: contact@openairinterface.org
 */

#ifndef MYSQL_DB_HPP
#define MYSQL_DB_HPP

#include <mysql/mysql.h>
#include <shared_mutex>

#include "Amf3GppAccessRegistration.h"
#include "database_wrapper.hpp"
#include "udr_event.hpp"
#include "Snssai.h"

namespace oai::udr::app {

class mysql_db : public database_wrapper<mysql_db> {
 public:
  mysql_db(udr_event& ev);
  virtual ~mysql_db();

  bool initialize();
  bool connect(uint32_t num_retries);
  bool close_connection();

  /*
   * Set the DB connection status
   * @param [bool] status: status to be set
   * @return void
   */
  void set_db_connection_status(bool status);

  /*
   * Get the DB connection status
   * @param void
   * @return current connection status
   */
  bool get_db_connection_status() const;

  /*
   * Verify the DB connection status and try to establish the connection if
   * necessary
   * @param void
   * @return current connection status after trying
   */
  bool check_connection_status();

  /*
   * Start the procedure for event connection handling
   * @param void
   * @return void
   */
  void start_event_connection_handling();

  /*
   * Trigger the procedure for event connection handling when neccessary
   * @param uint64_t ms
   * @return void
   */
  void trigger_connection_handling_procedure(uint64_t ms);

  /*
   * Get a unique key from a NSSAI (SST, SD) used in MySQL
   * @param [const oai::udr::model::Snssai&] snssai: SNSSAI
   * @param [uint32_t&] key: generated key
   * @return true if success, otherwise return false
   */
  bool get_key_from_snssai(
      const oai::model::common::Snssai& snssai, uint32_t& key);

  /*
   * Get NSSAI (SST, SD) from the corresponding key
   * @param [oai::udr::model::Snssai&] snssai: SNSSAI
   * @param [const uint32_t&] key: key
   * @return void
   */
  void get_snssai_from_key(
      oai::model::common::Snssai& snssai, const uint32_t& key);

  bool insert_authentication_subscription(
      const std::string& id,
      const oai::udr::model::AuthenticationSubscription&
          authentication_subscription,
      nlohmann::json& json_data);

  bool delete_authentication_subscription(
      const std::string& id, nlohmann::json& json_data);

  bool query_authentication_subscription(
      const std::string& id, nlohmann::json& json_data);

  bool update_authentication_subscription(
      const std::string& id,
      const std::vector<oai::model::common::PatchItem>& patchItem,
      nlohmann::json& json_data);

  bool query_am_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data);

  bool create_amf_context_3gpp(
      const std::string& ue_id,
      oai::udr::model::Amf3GppAccessRegistration& amf3GppAccessRegistration,
      nlohmann::json& json_data);

  bool query_amf_context_3gpp(
      const std::string& ue_id, nlohmann::json& json_data);

  bool insert_authentication_status(
      const std::string& ue_id, const oai::udr::model::AuthEvent& authEvent,
      nlohmann::json& json_data);

  bool delete_authentication_status(
      const std::string& ue_id, nlohmann::json& json_data);

  bool query_authentication_status(
      const std::string& ue_id, nlohmann::json& json_data);

  bool query_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data);

  bool delete_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      nlohmann::json& json_data);

  bool update_sdm_subscription(
      const std::string& ue_id, const std::string& subs_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      nlohmann::json& json_data);

  bool create_sdm_subscriptions(
      const std::string& ue_id,
      oai::udr::model::SdmSubscription& sdmSubscription,
      nlohmann::json& json_data);

  bool query_sdm_subscriptions(
      const std::string& ue_id, nlohmann::json& json_data);

  bool create_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      oai::udr::model::SessionManagementSubscriptionData& sm_subscription,
      nlohmann::json& json_data, uint32_t& resource_id);

  bool update_sm_data(
      const std::string& ueId, const std::string& servingPlmnId,
      oai::udr::model::SessionManagementSubscriptionData& subscriptionData,
      nlohmann::json& json_data, uint32_t& resource_id);

  bool query_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data,
      const std::optional<oai::model::common::Snssai>& snssai,
      const std::optional<std::string>& dnn);

  bool query_sm_data(nlohmann::json& json_data);

  bool delete_sm_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      const std::optional<oai::model::common::Snssai>& snssai);

  bool insert_smf_context_non_3gpp(
      const std::string& ue_id, const int32_t& pdu_session_id,
      const oai::udr::model::SmfRegistration& smfRegistration,
      nlohmann::json& json_data);

  bool delete_smf_context(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data);

  bool query_smf_registration(
      const std::string& ue_id, const int32_t& pdu_session_id,
      nlohmann::json& json_data);

  bool query_smf_reg_list(const std::string& ue_id, nlohmann::json& json_data);

  bool query_smf_select_data(
      const std::string& ue_id, const std::string& serving_plmn_id,
      nlohmann::json& json_data);

 private:
  MYSQL mysql_connector;
  bs2::connection db_connection_event;
  udr_event& m_event_sub;
  bool is_db_connection_active;
  mutable std::shared_mutex m_db_connection_status;
};
}  // namespace oai::udr::app

#endif
