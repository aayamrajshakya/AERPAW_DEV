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

/*! \file udr_app.cpp
 \brief
 \author
 \company Eurecom
 \date 2020
 \email: contact@openairinterface.org
 */

#include "udr_app.hpp"

#include "3gpp_29.500.h"
#include "AccessAndMobilitySubscriptionData.h"
#include "AuthenticationSubscription.h"
#include "cassandra_db.hpp"
#include "logger.hpp"
#include "mysql_db.hpp"
#include "mongo_db.hpp"
#include "udr_config.hpp"
#include "udr_config_yaml.hpp"
#include "udr_nrf.hpp"

using namespace oai::udr::app;
using namespace oai::udr::model;
using namespace oai::model::common;
using namespace oai::udr::config;

extern udr_app* udr_app_inst;
extern udr_config udr_cfg;
extern std::unique_ptr<oai::config::udr_config_yaml> udr_cfg_yaml;

//------------------------------------------------------------------------------
udr_app::udr_app(const std::string& config_file, udr_event& ev)
    : event_sub(ev) {
  Logger::udr_app().startup("Starting...");

  // Use the appropriate DB connector to initialize the connection to the DB
  if (udr_cfg.db_type == DB_TYPE_CASSANDRA) {
    db_connector = std::make_shared<cassandra_db>();
  } else if (udr_cfg.db_type == DB_TYPE_MYSQL) {
    db_connector = std::make_shared<mysql_db>(ev);
  } else {
    db_connector = std::make_shared<mongo_db>(ev);
  }

  if (!db_connector->initialize()) {
    Logger::udr_app().error("Error when initializing a connection with DB");
    return;
  }

  if (!db_connector->connect(MAX_FIRST_CONNECTION_RETRY)) {
    Logger::udr_app().warn("Could not establish the connection to the DB");
  }

  Logger::udr_app().startup("Started");
}

//------------------------------------------------------------------------------
udr_app::~udr_app() {
  Logger::udr_app().debug("Delete UDR APP instance...");
  // Close DB connection
  db_connector->close_connection();
}

//------------------------------------------------------------------------------
void udr_app::handle_query_am_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve the access and mobility subscription data of an UE "
      "(ID %s)",
      ue_id.c_str(), ue_id.c_str());

  if (db_connector->query_am_data(ue_id, serving_plmn_id, response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s] AccessAndMobilitySubscriptionData Data: %s", ue_id.c_str(),
        response_data.dump().c_str());
    // TODO: headers
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_create_amf_context_3gpp(
    const std::string& ue_id,
    Amf3GppAccessRegistration& amf3GppAccessRegistration,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Store the AMF context data of an UE (ID %s) using 3GPP "
      "Access in the UDR",
      ue_id.c_str(), ue_id.c_str());

  if (db_connector->create_amf_context_3gpp(
          ue_id, amf3GppAccessRegistration, response_data)) {
    code = HTTP_STATUS_CODE_201_CREATED;
    Logger::udr_app().info(
        "[UE Id %s] Amf3GppAccessRegistration Data: %s", ue_id.c_str(),
        response_data.dump().c_str());
    // TODO: Location
    // TODO: CODE 204
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_query_amf_context_3gpp(
    const std::string& ue_id, nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve the AMF context data of an UE using 3GPP Access",
      ue_id.c_str());

  if (db_connector->query_amf_context_3gpp(ue_id, response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s] Amf3GppAccessRegistration Data: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_create_authentication_status(
    const std::string& ue_id, const AuthEvent& authEvent,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Store the Authentication Status data of an UE",
      ue_id.c_str());

  if (db_connector->insert_authentication_status(
          ue_id, authEvent, response_data)) {
    code = HTTP_STATUS_CODE_204_NO_CONTENT;
    Logger::udr_app().info(
        "[UE Id %s] Successful stored the Authentication Status data",
        ue_id.c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_delete_authentication_status(
    const std::string& ue_id, nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Remove the Authentication Status data of an UE",
      ue_id.c_str());

  if (db_connector->delete_authentication_status(ue_id, response_data)) {
    code = HTTP_STATUS_CODE_204_NO_CONTENT;
    Logger::udr_app().info(
        "[UE Id %s] Successful removed the Authentication Status data",
        ue_id.c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_query_authentication_status(
    const std::string& ue_id, nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve the Authentication Status data of an UE",
      ue_id.c_str());

  if (db_connector->query_authentication_status(ue_id, response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s] AuthEvent Data: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_create_authentication_data(
    const std::string& ue_id,
    const AuthenticationSubscription& authentication_subscription,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Create an Authentication Subscription data of an UE",
      ue_id.c_str());

  if (db_connector->insert_authentication_subscription(
          ue_id, authentication_subscription, response_data)) {
    code = HTTP_STATUS_CODE_201_CREATED;
    Logger::udr_app().info(
        "[UE Id %s] AuthenticationSubscription: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_delete_authentication_data(
    const std::string& ue_id, nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Delete an Authentication Subscription data of an UE",
      ue_id.c_str());

  if (db_connector->delete_authentication_subscription(ue_id, response_data)) {
    code = HTTP_STATUS_CODE_204_NO_CONTENT;
    Logger::udr_app().info(
        "[UE Id %s] Successful removed the Authentication Subscription data",
        ue_id.c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_modify_authentication_subscription(
    const std::string& ue_id, const std::vector<PatchItem>& patchItem,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Modify the Authentication Subscription data of an UE",
      ue_id.c_str());

  if (db_connector->update_authentication_subscription(
          ue_id, patchItem, response_data)) {
    code = HTTP_STATUS_CODE_204_NO_CONTENT;
    Logger::udr_app().info(
        "[UE Id %s] Successful modified the Authentication subscription data",
        ue_id.c_str());
    // TODO: Code 200

  } else {
    code = HTTP_STATUS_CODE_403_FORBIDDEN;
    // TODO: problem details
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_read_authentication_subscription(
    const std::string& ue_id, nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve the Authentication Subscription data of an UE",
      ue_id.c_str());

  if (db_connector->query_authentication_subscription(ue_id, response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s] AuthenticationSubscription: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_404_NOT_FOUND;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_query_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve an individual SdmSubscription identified by subsId",
      ue_id.c_str());

  if (db_connector->query_sdm_subscription(ue_id, subs_id, response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s]SdmSubscriptions: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_404_NOT_FOUND;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_remove_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info("[UE Id %s] Delete a SdmSubscriptions", ue_id.c_str());

  if (db_connector->delete_sdm_subscription(ue_id, subs_id, response_data)) {
    code = HTTP_STATUS_CODE_204_NO_CONTENT;
    Logger::udr_app().info(
        "[UE Id %s] Successful removed a SdmSubscriptions", ue_id.c_str());
  } else {
    code = HTTP_STATUS_CODE_404_NOT_FOUND;  // TODO
  }
  return;

  Logger::udr_app().debug(
      "[UE Id %s] SdmSubscription DELETE - successful", ue_id.c_str());
}

//------------------------------------------------------------------------------
void udr_app::handle_update_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    SdmSubscription& sdmSubscription, nlohmann::json& response_data,
    long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Update an individual SDM Subscription of an UE",
      ue_id.c_str());

  if (db_connector->update_sdm_subscription(
          ue_id, subs_id, sdmSubscription, response_data)) {
    code = HTTP_STATUS_CODE_204_NO_CONTENT;
    Logger::udr_app().info(
        "[UE Id %s] Successful updated a SDMSubscription", ue_id.c_str());
  } else {
    code = HTTP_STATUS_CODE_404_NOT_FOUND;
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_create_sdm_subscriptions(
    const std::string& ue_id, SdmSubscription& sdmSubscription,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Create an individual SDM subscription", ue_id.c_str());

  if (db_connector->create_sdm_subscriptions(
          ue_id, sdmSubscription, response_data)) {
    code = HTTP_STATUS_CODE_201_CREATED;
    Logger::udr_app().info(
        "[UE Id %s] SdmSubscription: %s", ue_id.c_str(),
        response_data.dump().c_str());
    // TODO: Location
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_query_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve the SDM subscriptions of an UE", ue_id.c_str());

  if (db_connector->query_sdm_subscriptions(ue_id, response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s] SdmSubscriptions: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_query_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& response_data, long& code,
    const std::optional<Snssai>& snssai,
    const std::optional<std::string>& dnn) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve the Session Management Subscription Data of an UE",
      ue_id.c_str());

  if (db_connector->query_sm_data(
          ue_id, serving_plmn_id, response_data, snssai, dnn)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s] SessionManagementSubscriptionData: %s", ue_id.c_str(),
        response_data.dump().c_str());
    // TODO: Headers
  } else if (response_data.is_null()) {
    code = HTTP_STATUS_CODE_404_NOT_FOUND;
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_query_sm_data(nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "Retrieve the Session Management subscription data of all UEs");

  if (db_connector->query_sm_data(response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "SessionManagementSubscriptionData: %s", response_data.dump().c_str());
    // TODO: Headers
  } else if (response_data.is_null()) {
    code = HTTP_STATUS_CODE_404_NOT_FOUND;
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_create_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    SessionManagementSubscriptionData& subscription_data,
    nlohmann::json& response_data, long& code, uint32_t& resource_id) {
  Logger::udr_app().info(
      "[UE Id %s] Create a Session Management Subscription Data of an UE",
      ue_id.c_str());

  if (db_connector->create_sm_data(
          ue_id, serving_plmn_id, subscription_data, response_data,
          resource_id)) {
    code = HTTP_STATUS_CODE_201_CREATED;
    Logger::udr_app().info(
        "[UE Id %s] SessionManagementSubscription: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    if (response_data.find("error") != response_data.end()) {
      std::string e = response_data["error"];
      if (e.find("exists")) {
        code = HTTP_STATUS_CODE_400_BAD_REQUEST;
        return;
      }
    }
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_update_sm_data(
    const std::string& ueId, const std::string& servingPlmnId,
    SessionManagementSubscriptionData& subscriptionData,
    nlohmann::json& response_data, long& code, uint32_t& resource_id) {
  Logger::udr_app().info(
      "Update a Session Management subscription data of a UE");

  if (db_connector->update_sm_data(
          ueId, servingPlmnId, subscriptionData, response_data, resource_id)) {
    if (resource_id > 0) {
      code = HTTP_STATUS_CODE_201_CREATED;
    } else {
      code = HTTP_STATUS_CODE_204_NO_CONTENT;
    }
    Logger::udr_app().info(
        "SessionManagementSubscription: %s", response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_delete_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    const std::optional<Snssai>& snssai, nlohmann::json& response_data,
    long& code) {
  Logger::udr_app().info(
      "[UE Id %s]  Delete a Session Management subscription data of a UE",
      ue_id.c_str());

  if (db_connector->delete_sm_data(ue_id, serving_plmn_id, snssai)) {
    code = HTTP_STATUS_CODE_204_NO_CONTENT;
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_create_smf_context_non_3gpp(
    const std::string& ue_id, const int32_t& pdu_session_id,
    const SmfRegistration& smfRegistration, nlohmann::json& response_data,
    long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Create an individual SMF Registration data of an UE",
      ue_id.c_str());

  if (db_connector->insert_smf_context_non_3gpp(
          ue_id, pdu_session_id, smfRegistration, response_data)) {
    code = HTTP_STATUS_CODE_201_CREATED;
    Logger::udr_app().info(
        "[UE Id %s] SmfRegistration: %s", ue_id.c_str(),
        response_data.dump().c_str());
    // TODO: Location
    // TODO: Code 204
  } else {
    code = HTTP_STATUS_CODE_404_NOT_FOUND;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_delete_smf_context(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Remove an individual SMF context data of an UE",
      ue_id.c_str());

  if (db_connector->delete_smf_context(ue_id, pdu_session_id, response_data)) {
    code = HTTP_STATUS_CODE_204_NO_CONTENT;
    Logger::udr_app().info(
        "[UE Id %s] Successful deleted SMF context data", ue_id.c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_query_smf_registration(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve the individual SMF registration of an UE",
      ue_id.c_str());

  if (db_connector->query_smf_registration(
          ue_id, pdu_session_id, response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s] SmfRegistration: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_query_smf_reg_list(
    const std::string& ue_id, nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve the SMF registration list of an UE", ue_id.c_str());

  if (db_connector->query_smf_reg_list(ue_id, response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s] SmfRegList: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
void udr_app::handle_query_smf_select_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& response_data, long& code) {
  Logger::udr_app().info(
      "[UE Id %s] Retrieve the SMF selection subscription data of an UE",
      ue_id.c_str());

  if (db_connector->query_smf_select_data(
          ue_id, serving_plmn_id, response_data)) {
    code = HTTP_STATUS_CODE_200_OK;
    Logger::udr_app().info(
        "[UE Id %s] SmfSelectionSubscriptionData: %s", ue_id.c_str(),
        response_data.dump().c_str());
  } else {
    code = HTTP_STATUS_CODE_500_INTERNAL_SERVER_ERROR;  // TODO
  }
  return;
}

//------------------------------------------------------------------------------
bool udr_app::handle_read_configuration(
    nlohmann::json& config_info, long& code) {
  Logger::udr_app().info("Handle a request to get UDR Configuration");

  // Process the request and trigger the response from UDR Server
  udr_cfg_yaml->to_json(config_info);
  Logger::udr_app().debug(
      "UDR configuration:\n %s", config_info.dump().c_str());
  code = static_cast<uint32_t>(http_status_code_e::HTTP_STATUS_CODE_200_OK);
  return true;
  return true;
}

//------------------------------------------------------------------------------
bool udr_app::handle_update_configuration(
    nlohmann::json& config_info, long& code) {
  Logger::udr_app().info("Handle a request to update UDR configuration");

  // TODO: remove this part to enable this functionality
  code = static_cast<uint32_t>(
      http_status_code_e::HTTP_STATUS_CODE_501_NOT_IMPLEMENTED);
  return false;

  // Process the request and trigger the response from UDR Server
  if (udr_cfg_yaml->from_json(config_info)) {
    udr_cfg_yaml->to_json(config_info);
    Logger::udr_app().debug(
        "UDR configuration:\n %s", config_info.dump().c_str());
    code = static_cast<uint32_t>(http_status_code_e::HTTP_STATUS_CODE_200_OK);
    return true;
  } else {
    code = static_cast<uint32_t>(
        http_status_code_e::HTTP_STATUS_CODE_400_BAD_REQUEST);
    // TODO set problem_details
    return false;
  }
  return true;
  return true;
}
