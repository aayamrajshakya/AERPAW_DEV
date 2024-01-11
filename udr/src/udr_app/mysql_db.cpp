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

/*! \file mysql_db.cpp
 \brief
 \author
 \company Eurecom
 \date 2022
 \email: contact@openairinterface.org
 */

#include "mysql_db.hpp"

#include <chrono>
#include <thread>

#include "AccessAndMobilitySubscriptionData.h"
#include "AuthenticationSubscription.h"
#include "ProblemDetails.h"
#include "SdmSubscription.h"
#include "logger.hpp"
#include "udr_config.hpp"
#include "PatchOperation_anyOf.h"

#include <boost/algorithm/string.hpp>

using namespace oai::udr::app;
using namespace oai::udr::model;
using namespace oai::model::common;
using namespace oai::udr::config;
using namespace boost::placeholders;

extern udr_config udr_cfg;

//------------------------------------------------------------------------------
mysql_db::mysql_db(udr_event& ev)
    : database_wrapper<mysql_db>(), m_event_sub(ev), m_db_connection_status() {
  is_db_connection_active = false;
  start_event_connection_handling();
}

//------------------------------------------------------------------------------
mysql_db::~mysql_db() {
  if (db_connection_event.connected()) db_connection_event.disconnect();
  close_connection();
}

//------------------------------------------------------------------------------
bool mysql_db::initialize() {
  Logger::udr_db().debug("Initializing MySQL DB ...");
  if (!mysql_init(&mysql_connector)) {
    Logger::udr_db().error("Cannot initialize MySQL");
    throw std::runtime_error("Cannot initialize MySQL");
  }
  Logger::udr_db().debug("Done!");
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::connect(uint32_t num_retries) {
  Logger::udr_db().debug("Connecting to MySQL DB");

  int i = 0;
  while (i < num_retries) {
    // TODO: use mysql_real_connect_nonblocking (only from MySQL 8.0.16)
    if (!mysql_real_connect(
            &mysql_connector, udr_cfg.db_conf.server.c_str(),
            udr_cfg.db_conf.user.c_str(), udr_cfg.db_conf.pass.c_str(),
            udr_cfg.db_conf.db_name.c_str(), udr_cfg.db_conf.port, 0, 0)) {
      Logger::udr_db().error(
          "An error occurred when connecting to MySQL DB (%s), retry ...",
          mysql_error(&mysql_connector));
      i++;
      set_db_connection_status(false);
      // throw std::runtime_error("Cannot connect to MySQL DB");
    } else {
      Logger::udr_db().info("Connected to MySQL DB");
      set_db_connection_status(true);
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  if (i == num_retries) {
    return false;
    // throw std::runtime_error("Cannot connect to MySQL DB");
  }
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::close_connection() {
  Logger::udr_db().debug("Close the connection with MySQL DB");
  mysql_close(&mysql_connector);
  set_db_connection_status(false);
  return true;
}

//------------------------------------------------------------------------------
void mysql_db::set_db_connection_status(bool status) {
  std::unique_lock lock(m_db_connection_status);
  is_db_connection_active = status;
}

//------------------------------------------------------------------------------
bool mysql_db::get_db_connection_status() const {
  std::shared_lock lock(m_db_connection_status);
  return is_db_connection_active;
}

//---------------------------------------------------------------------------------------------
void mysql_db::start_event_connection_handling() {
  // get current time
  uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

  struct itimerspec its;
  its.it_value.tv_sec  = udr_cfg.db_conf.connection_timeout;  // seconds
  its.it_value.tv_nsec = 0;  // 100 * 1000 * 1000; //100ms
  const uint64_t interval =
      its.it_value.tv_sec * 1000 +
      its.it_value.tv_nsec / 1000000;  // convert sec, nsec to msec

  db_connection_event = m_event_sub.subscribe_task_nf_heartbeat(
      boost::bind(&mysql_db::trigger_connection_handling_procedure, this, _1),
      interval, ms + interval);
}
//---------------------------------------------------------------------------------------------
void mysql_db::trigger_connection_handling_procedure(uint64_t ms) {
  _unused(ms);
  std::time_t current_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  Logger::udr_db().debug(
      "DB Connection handling, current time: %s", std::ctime(&current_time));

  if (mysql_ping(&mysql_connector)) {
    set_db_connection_status(false);
    Logger::udr_app().warn(
        "The connection to the DB is not active, reset the connection and try "
        "again ...");
    // If couldn't connect to the DB
    // Reset the connection and try again
    close_connection();
    initialize();
    if (!connect(MAX_CONNECTION_RETRY))
      Logger::udr_app().warn("Could not establish the connection to the DB");
  } else {
    return;
  }
}

//------------------------------------------------------------------------------
bool mysql_db::check_connection_status() {
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to the MySQL is currently inactive");
    // Try to re-establish the connection
    trigger_connection_handling_procedure(0);
  }
  return get_db_connection_status();
}

//------------------------------------------------------------------------------
bool mysql_db::get_key_from_snssai(
    const oai::model::common::Snssai& snssai, uint32_t& key) {
  uint8_t sst        = 0;
  uint32_t sd        = 0;
  sst                = snssai.getSst() & 0x000000ff;
  std::string sd_str = {};
  sd_str             = snssai.getSd();

  if (!sd_str.empty()) {
    uint8_t base = 10;
    try {
      if (sd_str.size() > 2) {
        if (boost::iequals(sd_str.substr(0, 2), "0x")) {
          base = 16;
        }
      }
      sd = std::stoul(sd_str, nullptr, base);
    } catch (const std::exception& e) {
      Logger::udr_db().error(
          "Error when converting from string to int for S-NSSAI SD, error: %s",
          e.what());
      return false;
    }
  }
  // Get 3 lower bytes only
  sd  = sd & 0x00ffffff;
  key = (sd << 8 | sst);
  return true;
}

//------------------------------------------------------------------------------
void mysql_db::get_snssai_from_key(
    oai::model::common::Snssai& snssai, const uint32_t& key) {
  uint8_t sst = 0;
  uint32_t sd = 0;
  sst         = key & 0x000000ff;
  sd          = (key >> 8) & 0x00ffffff;
  snssai.setSst(sst);
  snssai.setSd(std::to_string(sd));
}

//------------------------------------------------------------------------------
bool mysql_db::insert_authentication_subscription(
    const std::string& id, const AuthenticationSubscription& auth_subscription,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res          = nullptr;
  MYSQL_ROW row           = {};
  nlohmann::json json_tmp = {};

  std::string query =
      "SELECT * FROM AuthenticationSubscription WHERE ueid='" + id + "'";
  Logger::udr_db().info(
      "[UE Id %s] MySQL Query: %s", id.c_str(), query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size()) != 0) {
    Logger::udr_db().error(
        "[UE Id %s] Failed when executing mysql_real_query with SQL Query: %s",
        id.c_str(), query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", id.c_str(),
        query.c_str());
    return false;
  }

  std::string where_condition = {};
  row                         = mysql_fetch_row(res);

  if (row != nullptr) {
    Logger::udr_db().debug(
        "[UE Id %s] AuthenticationSubscription existed, update with new "
        "values!",
        id.c_str());
    // Update accordingly
    mysql_free_result(res);
    query = "UPDATE AuthenticationSubscription SET authenticationMethod='" +
            auth_subscription.getAuthenticationMethod() + "'";
    where_condition = " WHERE ueid='" + id + "'";
  } else {
    // Insert/create new record
    mysql_free_result(res);
    query = "INSERT INTO AuthenticationSubscription SET ueid='" + id + "'" +
            ",authenticationMethod='" +
            auth_subscription.getAuthenticationMethod() + "'";
  }

  query +=
      (auth_subscription.encPermanentKeyIsSet() ?
           ",encPermanentKey='" + auth_subscription.getEncPermanentKey() + "'" :
           "") +
      (auth_subscription.protectionParameterIdIsSet() ?
           ",protectionParameterId='" +
               auth_subscription.getProtectionParameterId() + "'" :
           "") +
      (auth_subscription.authenticationManagementFieldIsSet() ?
           ",authenticationManagementField='" +
               auth_subscription.getAuthenticationManagementField() + "'" :
           "") +
      (auth_subscription.algorithmIdIsSet() ?
           ",algorithmId='" + auth_subscription.getAlgorithmId() + "'" :
           "") +
      (auth_subscription.encOpcKeyIsSet() ?
           ",encOpcKey='" + auth_subscription.getEncOpcKey() + "'" :
           "") +
      (auth_subscription.encTopcKeyIsSet() ?
           ",encTopcKey='" + auth_subscription.getEncTopcKey() + "'" :
           "") +
      (auth_subscription.n5gcAuthMethodIsSet() ?
           ",n5gcAuthMethod='" + auth_subscription.getN5gcAuthMethod() + "'" :
           "") +
      (auth_subscription.supiIsSet() ?
           ",supi='" + auth_subscription.getSupi() + "'" :
           "");

  if (auth_subscription.sequenceNumberIsSet()) {
    to_json(json_tmp, auth_subscription.getSequenceNumber());
    query += ",sequenceNumber='" + json_tmp.dump() + "'";
  }

  query += where_condition;
  Logger::udr_db().info(
      "[UE Id %s] MySQL Query: %s", id.c_str(), query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", id.c_str(),
        query.c_str());
    return false;
  }

  to_json(json_data, auth_subscription);

  Logger::udr_db().debug(
      "[UE Id %s] AuthenticationSubscription: %s", id.c_str(),
      json_data.dump().c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::delete_authentication_subscription(
    const std::string& id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  const std::string query =
      "DELETE FROM AuthenticationSubscription WHERE ueid='" + id + "'";

  Logger::udr_db().debug(
      "[UE Id %s] MySQL Query %s: ", id.c_str(), query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", id.c_str(),
        query.c_str());
    return false;
  }

  Logger::udr_db().debug(
      "[UE Id %s] Deleted AuthenticationSubscription (with UE ID %s) "
      "successfully",
      id.c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_authentication_subscription(
    const std::string& id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  Logger::udr_db().info(
      "[UE Id %s] Query Authentication Subscription", id.c_str());
  MYSQL_RES* res     = nullptr;
  MYSQL_ROW row      = {};
  MYSQL_FIELD* field = nullptr;
  nlohmann::json j   = {};
  bool result        = false;

  AuthenticationSubscription authentication_subscription = {};
  const std::string query =
      "SELECT * FROM AuthenticationSubscription WHERE ueid='" + id + "'";
  Logger::udr_db().info(
      "[UE Id %s] MySQL Query: %s", id.c_str(), query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size()) != 0) {
    Logger::udr_db().error(
        "[UE Id %s] Failed when executing mysql_real_query with SQL Query: %s",
        id.c_str(), query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);

  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", id.c_str(),
        query.c_str());
    return false;
  }

  row = mysql_fetch_row(res);

  if (row != nullptr) {
    for (int i = 0; (field = mysql_fetch_field(res)); i++) {
      Logger::udr_db().debug(
          "[UE Id %s] Row [%d]: %s ", id.c_str(), i, field->name);
      if (boost::iequals("authenticationMethod", field->name)) {
        authentication_subscription.setAuthenticationMethod(row[i]);
      } else if (
          boost::iequals("encPermanentKey", field->name) && row[i] != nullptr) {
        authentication_subscription.setEncPermanentKey(row[i]);
      } else if (
          boost::iequals("protectionParameterId", field->name) &&
          row[i] != nullptr) {
        authentication_subscription.setProtectionParameterId(row[i]);
      } else if (
          boost::iequals("sequenceNumber", field->name) && row[i] != nullptr) {
        SequenceNumber sequencenumber = {};
        nlohmann::json::parse(row[i]).get_to(sequencenumber);
        authentication_subscription.setSequenceNumber(sequencenumber);
      } else if (
          boost::iequals("authenticationManagementField", field->name) &&
          row[i] != nullptr) {
        authentication_subscription.setAuthenticationManagementField(row[i]);
      } else if (
          boost::iequals("algorithmId", field->name) && row[i] != nullptr) {
        authentication_subscription.setAlgorithmId(row[i]);
      } else if (
          boost::iequals("encOpcKey", field->name) && row[i] != nullptr) {
        authentication_subscription.setEncOpcKey(row[i]);
      } else if (
          boost::iequals("encTopcKey", field->name) && row[i] != nullptr) {
        authentication_subscription.setEncTopcKey(row[i]);
      } else if (
          boost::iequals("vectorGenerationInHss", field->name) &&
          row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          authentication_subscription.setVectorGenerationInHss(true);
        else
          authentication_subscription.setVectorGenerationInHss(false);
      } else if (
          boost::iequals("n5gcAuthMethod", field->name) && row[i] != nullptr) {
        authentication_subscription.setN5gcAuthMethod(row[i]);
      } else if (
          boost::iequals("rgAuthenticationInd", field->name) &&
          row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          authentication_subscription.setRgAuthenticationInd(true);
        else
          authentication_subscription.setRgAuthenticationInd(false);
      } else if (boost::iequals("supi", field->name) && row[i] != nullptr) {
        authentication_subscription.setSupi(row[i]);
      }
    }

    to_json(json_data, authentication_subscription);
    result = true;
  } else {
    Logger::udr_db().error(
        "[UE Id %s] AuthenticationSubscription no data！ SQL Query: %s",
        id.c_str(), query.c_str());
    result = false;
  }

  mysql_free_result(res);
  return result;
}

//------------------------------------------------------------------------------
bool mysql_db::update_authentication_subscription(
    const std::string& ue_id, const std::vector<PatchItem>& patchItem,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res = nullptr;
  MYSQL_ROW row  = {};
  const std::string select_Authenticationsubscription =
      "SELECT * from AuthenticationSubscription WHERE ueid='" + ue_id + "'";

  Logger::udr_db().info(
      "[UE Id %s] MySQL Query: %s", ue_id.c_str(),
      select_Authenticationsubscription.c_str());

  std::string query    = {};
  nlohmann::json tmp_j = {};

  for (int i = 0; i < patchItem.size(); i++) {
    if ((patchItem[i].getOp().getEnumValue() ==
         PatchOperation_anyOf::ePatchOperation_anyOf::REPLACE) &&
        patchItem[i].valueIsSet()) {
      patchItem[i].getValue();
      SequenceNumber sequencenumber;
      nlohmann::json::parse(patchItem[i].getValue().c_str())
          .get_to(sequencenumber);

      if (mysql_real_query(
              &mysql_connector, select_Authenticationsubscription.c_str(),
              (unsigned long) select_Authenticationsubscription.size())) {
        Logger::udr_db().error(
            "[UE Id %s] mysql_real_query failure！SQL Query: %s", ue_id.c_str(),
            select_Authenticationsubscription.c_str());
        return false;
      }

      res = mysql_store_result(&mysql_connector);
      if (res == nullptr) {
        Logger::udr_db().error(
            "[UE Id %s] mysql_store_result failure！SQL Query: %s",
            ue_id.c_str(), select_Authenticationsubscription.c_str());
        return false;
      }
      if (mysql_num_rows(res)) {
        nlohmann::json sequencenumber_j;
        query = "UPDATE AuthenticationSubscription SET sequenceNumber='";

        to_json(sequencenumber_j, sequencenumber);
        query += sequencenumber_j.dump() + "'";
        query += " WHERE ueid='" + ue_id + "'";
      } else {
        Logger::udr_db().error(
            "[UE Id %s] AuthenticationSubscription no data！ SQL Query %s",
            ue_id.c_str(), select_Authenticationsubscription.c_str());
      }

      Logger::udr_db().info(
          "[UE Id %s] MySQL Update command %s", ue_id.c_str(), query.c_str());
      mysql_free_result(res);

      if (mysql_real_query(
              &mysql_connector, query.c_str(), (unsigned long) query.size()) !=
          0) {
        Logger::udr_db().error(
            "[UE Id %s]  Update mysql failure！ SQL command: %s", ue_id.c_str(),
            query.c_str());
        // TODO: Problem details
        return false;
      }
    }

    to_json(tmp_j, patchItem[i]);
    json_data += tmp_j;
  }

  Logger::udr_db().info(
      "[UE Id %s] AuthenticationSubscription PATCH: %s", ue_id.c_str(),
      json_data.dump().c_str());

  //	  code          = HTTP_STATUS_CODE_204_NO_CONTENT;
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_am_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res     = nullptr;
  MYSQL_ROW row      = {};
  MYSQL_FIELD* field = nullptr;

  AccessAndMobilitySubscriptionData subscription_data = {};
  Logger::udr_db().debug("[UE Id %s] Handle Query AM Data", ue_id.c_str());

  // TODO: Define query template in a header file
  const std::string query =
      "SELECT * from AccessAndMobilitySubscriptionData WHERE ueid='" + ue_id +
      "' AND servingPlmnid='" + serving_plmn_id + "'";

  Logger::udr_db().debug(
      "[UE Id %s] SQL Query: %s", ue_id.c_str(), query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！", ue_id.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！", ue_id.c_str());
    return false;
  }

  row = mysql_fetch_row(res);

  if (row != nullptr) {
    for (int i = 0; (field = mysql_fetch_field(res)); i++) {
      try {
        if (boost::iequals("supportedFeatures", field->name) &&
            row[i] != nullptr) {
          subscription_data.setSupportedFeatures(row[i]);
        } else if (boost::iequals("gpsis", field->name) && row[i] != nullptr) {
          std::vector<std ::string> gpsis;
          nlohmann::json::parse(row[i]).get_to(gpsis);
          subscription_data.setGpsis(gpsis);
        } else if (
            boost::iequals("internalGroupIds", field->name) &&
            row[i] != nullptr) {
          std::vector<std ::string> internalgroupids;
          nlohmann::json::parse(row[i]).get_to(internalgroupids);
          subscription_data.setInternalGroupIds(internalgroupids);
        } else if (
            boost::iequals("sharedVnGroupDataIds", field->name) &&
            row[i] != nullptr) {
          std::map<std ::string, std::string> shared_vn_group_data_ids;
          nlohmann::json::parse(row[i]).get_to(shared_vn_group_data_ids);
          subscription_data.setSharedVnGroupDataIds(shared_vn_group_data_ids);
        } else if (
            boost::iequals("subscribedUeAmbr", field->name) &&
            row[i] != nullptr) {
          AmbrRm subscribedueambr;
          nlohmann::json::parse(row[i]).get_to(subscribedueambr);
          subscription_data.setSubscribedUeAmbr(subscribedueambr);
        } else if (boost::iequals("nssai", field->name) && row[i] != nullptr) {
          Nssai nssai = {};
          nlohmann::json::parse(row[i]).get_to(nssai);
          subscription_data.setNssai(nssai);
        } else if (
            boost::iequals("ratRestrictions", field->name) &&
            row[i] != nullptr) {
          std ::vector<RatType> ratrestrictions;
          nlohmann::json::parse(row[i]).get_to(ratrestrictions);
          subscription_data.setRatRestrictions(ratrestrictions);
        } else if (
            boost::iequals("forbiddenAreas", field->name) &&
            row[i] != nullptr) {
          std ::vector<Area> forbiddenareas;
          nlohmann::json::parse(row[i]).get_to(forbiddenareas);
          subscription_data.setForbiddenAreas(forbiddenareas);
        } else if (
            boost::iequals("serviceAreaRestriction", field->name) &&
            row[i] != nullptr) {
          ServiceAreaRestriction servicearearestriction;
          nlohmann::json::parse(row[i]).get_to(servicearearestriction);
          subscription_data.setServiceAreaRestriction(servicearearestriction);
        } else if (
            boost::iequals("coreNetworkTypeRestrictions", field->name) &&
            row[i] != nullptr) {
          std ::vector<CoreNetworkType> corenetworktyperestrictions;
          nlohmann::json::parse(row[i]).get_to(corenetworktyperestrictions);
          subscription_data.setCoreNetworkTypeRestrictions(
              corenetworktyperestrictions);
        } else if (
            boost::iequals("rfspIndex", field->name) && row[i] != nullptr) {
          int32_t a = std::stoi(row[i]);
          subscription_data.setRfspIndex(a);
        } else if (
            boost::iequals("subsRegTimer", field->name) && row[i] != nullptr) {
          int32_t a = std::stoi(row[i]);
          subscription_data.setSubsRegTimer(a);
        } else if (
            boost::iequals("ueUsageType", field->name) && row[i] != nullptr) {
          int32_t a = std::stoi(row[i]);
          subscription_data.setUeUsageType(a);
        } else if (
            boost::iequals("mpsPriority", field->name) && row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            subscription_data.setMpsPriority(true);
          else
            subscription_data.setMpsPriority(false);
        } else if (
            boost::iequals("mcsPriority", field->name) && row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            subscription_data.setMcsPriority(true);
          else
            subscription_data.setMcsPriority(false);
        } else if (
            boost::iequals("activeTime", field->name) && row[i] != nullptr) {
          int32_t a = std::stoi(row[i]);
          subscription_data.setActiveTime(a);
        } else if (
            boost::iequals("sorInfo", field->name) && row[i] != nullptr) {
          SorInfo sorinfo;
          nlohmann::json::parse(row[i]).get_to(sorinfo);
          subscription_data.setSorInfo(sorinfo);
        } else if (
            boost::iequals("sorInfoExpectInd", field->name) &&
            row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            subscription_data.setSorInfoExpectInd(true);
          else
            subscription_data.setSorInfoExpectInd(false);
        } else if (
            boost::iequals("sorafRetrieval", field->name) &&
            row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            subscription_data.setSorafRetrieval(true);
          else
            subscription_data.setSorafRetrieval(false);
        } else if (
            boost::iequals("sorUpdateIndicatorList", field->name) &&
            row[i] != nullptr) {
          std ::vector<SorUpdateIndicator> sorupdateindicatorlist;
          nlohmann::json::parse(row[i]).get_to(sorupdateindicatorlist);
          subscription_data.setSorUpdateIndicatorList(sorupdateindicatorlist);
        } else if (
            boost::iequals("upuInfo", field->name) && row[i] != nullptr) {
          UpuInfo upuinfo;
          nlohmann::json::parse(row[i]).get_to(upuinfo);
          subscription_data.setUpuInfo(upuinfo);
        } else if (
            boost::iequals("micoAllowed", field->name) && row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            subscription_data.setMicoAllowed(true);
          else
            subscription_data.setMicoAllowed(false);
        } else if (
            boost::iequals("sharedAmDataIds", field->name) &&
            row[i] != nullptr) {
          std ::vector<std ::string> sharedamdataids;
          nlohmann::json::parse(row[i]).get_to(sharedamdataids);
          subscription_data.setSharedAmDataIds(sharedamdataids);
        } else if (
            boost::iequals("odbPacketServices", field->name) &&
            row[i] != nullptr) {
          OdbPacketServices odbpacketservices;
          nlohmann::json::parse(row[i]).get_to(odbpacketservices);
          subscription_data.setOdbPacketServices(odbpacketservices);
        } else if (
            boost::iequals("serviceGapTime", field->name) &&
            row[i] != nullptr) {
          int32_t a = std::stoi(row[i]);
          subscription_data.setServiceGapTime(a);
        } else if (
            boost::iequals("mdtUserConsent", field->name) &&
            row[i] != nullptr) {
          MdtUserConsent mdtuserconsent;
          nlohmann::json::parse(row[i]).get_to(mdtuserconsent);
          subscription_data.setMdtUserConsent(mdtuserconsent);
        } else if (
            boost::iequals("mdtConfiguration", field->name) &&
            row[i] != nullptr) {
          MdtConfiguration mdtconfiguration;
          nlohmann::json::parse(row[i]).get_to(mdtconfiguration);
          subscription_data.setMdtConfiguration(mdtconfiguration);
        } else if (
            boost::iequals("traceData", field->name) && row[i] != nullptr) {
          TraceData tracedata;
          nlohmann::json::parse(row[i]).get_to(tracedata);
          subscription_data.setTraceData(tracedata);
        } else if (
            boost::iequals("cagData", field->name) && row[i] != nullptr) {
          CagData cagdata;
          nlohmann::json::parse(row[i]).get_to(cagdata);
          subscription_data.setCagData(cagdata);
        } else if (boost::iequals("stnSr", field->name) && row[i] != nullptr) {
          subscription_data.setStnSr(row[i]);
        } else if (
            boost::iequals("cMsisdn", field->name) && row[i] != nullptr) {
          subscription_data.setCMsisdn(row[i]);
        } else if (
            boost::iequals("nbIoTUePriority", field->name) &&
            row[i] != nullptr) {
          int32_t a = std::stoi(row[i]);
          subscription_data.setNbIoTUePriority(a);
        } else if (
            boost::iequals("nssaiInclusionAllowed", field->name) &&
            row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            subscription_data.setNssaiInclusionAllowed(true);
          else
            subscription_data.setNssaiInclusionAllowed(false);
        } else if (
            boost::iequals("rgWirelineCharacteristics", field->name) &&
            row[i] != nullptr) {
          subscription_data.setRgWirelineCharacteristics(row[i]);
        } else if (
            boost::iequals("ecRestrictionDataWb", field->name) &&
            row[i] != nullptr) {
          EcRestrictionDataWb ecrestrictiondatawb;
          nlohmann::json::parse(row[i]).get_to(ecrestrictiondatawb);
          subscription_data.setEcRestrictionDataWb(ecrestrictiondatawb);
        } else if (
            boost::iequals("ecRestrictionDataNb", field->name) &&
            row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            subscription_data.setEcRestrictionDataNb(true);
          else
            subscription_data.setEcRestrictionDataNb(false);
        } else if (
            boost::iequals("expectedUeBehaviourList", field->name) &&
            row[i] != nullptr) {
          ExpectedUeBehaviourData expecteduebehaviourlist;
          nlohmann::json::parse(row[i]).get_to(expecteduebehaviourlist);
          subscription_data.setExpectedUeBehaviourList(expecteduebehaviourlist);
        } else if (
            boost::iequals("primaryRatRestrictions", field->name) &&
            row[i] != nullptr) {
          std ::vector<RatType> primaryratrestrictions;
          nlohmann::json::parse(row[i]).get_to(primaryratrestrictions);
          subscription_data.setPrimaryRatRestrictions(primaryratrestrictions);
        } else if (
            boost::iequals("secondaryRatRestrictions", field->name) &&
            row[i] != nullptr) {
          std ::vector<RatType> secondaryratrestrictions;
          nlohmann::json::parse(row[i]).get_to(secondaryratrestrictions);
          subscription_data.setSecondaryRatRestrictions(
              secondaryratrestrictions);
        } else if (
            boost::iequals("edrxParametersList", field->name) &&
            row[i] != nullptr) {
          std ::vector<EdrxParameters> edrxparameterslist;
          nlohmann::json::parse(row[i]).get_to(edrxparameterslist);
          subscription_data.setEdrxParametersList(edrxparameterslist);
        } else if (
            boost::iequals("ptwParametersList", field->name) &&
            row[i] != nullptr) {
          std ::vector<PtwParameters> ptwparameterslist;
          nlohmann::json::parse(row[i]).get_to(ptwparameterslist);
          subscription_data.setPtwParametersList(ptwparameterslist);
        } else if (
            boost::iequals("iabOperationAllowed", field->name) &&
            row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            subscription_data.setIabOperationAllowed(true);
          else
            subscription_data.setIabOperationAllowed(false);
        } else if (
            boost::iequals("wirelineForbiddenAreas", field->name) &&
            row[i] != nullptr) {
          std ::vector<WirelineArea> wirelineforbiddenareas;
          nlohmann::json::parse(row[i]).get_to(wirelineforbiddenareas);
          subscription_data.setWirelineForbiddenAreas(wirelineforbiddenareas);
        } else if (
            boost::iequals("wirelineServiceAreaRestriction", field->name) &&
            row[i] != nullptr) {
          WirelineServiceAreaRestriction wirelineservicearearestriction;
          nlohmann::json::parse(row[i]).get_to(wirelineservicearearestriction);
          subscription_data.setWirelineServiceAreaRestriction(
              wirelineservicearearestriction);
        }
      } catch (std::exception e) {
        Logger::udr_db().error(
            "[UE Id %s] Cannot set values for Subscription Data: %s",
            ue_id.c_str(), e.what());
        return false;
      }
    }

    to_json(json_data, subscription_data);
    Logger::udr_db().debug(
        "[UE Id %s] AccessAndMobilitySubscriptionData GET (JSON): %s",
        ue_id.c_str(), json_data.dump().c_str());
  } else {
    Logger::udr_db().error(
        "[UE Id %s] No data available for AccessAndMobilitySubscriptionData!",
        ue_id.c_str());
    return false;
  }

  mysql_free_result(res);
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::create_amf_context_3gpp(
    const std::string& ue_id,
    Amf3GppAccessRegistration& amf3GppAccessRegistration,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res = nullptr;
  MYSQL_ROW row  = {};

  const std::string select_AMF3GPPAccessRegistration =
      "SELECT * FROM Amf3GppAccessRegistration WHERE ueid='" + ue_id + "'";
  std::string query = {};

  nlohmann::json j = {};

  if (mysql_real_query(
          &mysql_connector, select_AMF3GPPAccessRegistration.c_str(),
          (unsigned long) select_AMF3GPPAccessRegistration.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        select_AMF3GPPAccessRegistration.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", ue_id.c_str(),
        select_AMF3GPPAccessRegistration.c_str());
    return false;
  }

  if (mysql_num_rows(res)) {
    query =
        "UPDATE Amf3GppAccessRegistration SET amfInstanceId='" +
        amf3GppAccessRegistration.getAmfInstanceId() + "'" +
        (amf3GppAccessRegistration.supportedFeaturesIsSet() ?
             ",supportedFeatures='" +
                 amf3GppAccessRegistration.getSupportedFeatures() + "'" :
             "") +
        (amf3GppAccessRegistration.purgeFlagIsSet() ?
             (amf3GppAccessRegistration.isPurgeFlag() ? ",purgeFlag=1" :
                                                        ",purgeFlag=0") :
             "") +
        (amf3GppAccessRegistration.peiIsSet() ?
             ",pei='" + amf3GppAccessRegistration.getPei() + "'" :
             "") +
        ",deregCallbackUri='" +
        amf3GppAccessRegistration.getDeregCallbackUri() + "'" +
        (amf3GppAccessRegistration.pcscfRestorationCallbackUriIsSet() ?
             ",pcscfRestorationCallbackUri='" +
                 amf3GppAccessRegistration.getPcscfRestorationCallbackUri() +
                 "'" :
             "") +
        (amf3GppAccessRegistration.initialRegistrationIndIsSet() ?
             (amf3GppAccessRegistration.isInitialRegistrationInd() ?
                  ",initialRegistrationInd=1" :
                  ",initialRegistrationInd=0") :
             "") +
        (amf3GppAccessRegistration.drFlagIsSet() ?
             (amf3GppAccessRegistration.isDrFlag() ? ",drFlag=1" :
                                                     ",drFlag=0") :
             "") +
        (amf3GppAccessRegistration.urrpIndicatorIsSet() ?
             (amf3GppAccessRegistration.isUrrpIndicator() ?
                  ",urrpIndicator=1" :
                  ",urrpIndicator=0") :
             "") +
        (amf3GppAccessRegistration.amfEeSubscriptionIdIsSet() ?
             ",amfEeSubscriptionId='" +
                 amf3GppAccessRegistration.getAmfEeSubscriptionId() + "'" :
             "") +
        (amf3GppAccessRegistration.ueSrvccCapabilityIsSet() ?
             (amf3GppAccessRegistration.isUeSrvccCapability() ?
                  ",ueSrvccCapability=1" :
                  ",ueSrvccCapability=0") :
             "") +
        (amf3GppAccessRegistration.registrationTimeIsSet() ?
             ",registrationTime='" +
                 amf3GppAccessRegistration.getRegistrationTime() + "'" :
             "") +
        (amf3GppAccessRegistration.noEeSubscriptionIndIsSet() ?
             (amf3GppAccessRegistration.isNoEeSubscriptionInd() ?
                  ",noEeSubscriptionInd=1" :
                  ",noEeSubscriptionInd=0") :
             "");

    if (amf3GppAccessRegistration.imsVoPsIsSet()) {
      to_json(j, amf3GppAccessRegistration.getImsVoPs());
      query += ",imsVoPs='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.amfServiceNameDeregIsSet()) {
      to_json(j, amf3GppAccessRegistration.getAmfServiceNameDereg());
      query += ",amfServiceNameDereg='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.amfServiceNamePcscfRestIsSet()) {
      to_json(j, amf3GppAccessRegistration.getAmfServiceNamePcscfRest());
      query += ",amfServiceNamePcscfRest='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.backupAmfInfoIsSet()) {
      nlohmann::json tmp;
      j.clear();
      std::vector<BackupAmfInfo> backupamfinfo =
          amf3GppAccessRegistration.getBackupAmfInfo();
      for (int i = 0; i < backupamfinfo.size(); i++) {
        to_json(tmp, backupamfinfo[i]);
        j += tmp;
      }
      query += ",backupAmfInfo='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.epsInterworkingInfoIsSet()) {
      to_json(j, amf3GppAccessRegistration.getEpsInterworkingInfo());
      query += ",epsInterworkingInfo='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.vgmlcAddressIsSet()) {
      to_json(j, amf3GppAccessRegistration.getVgmlcAddress());
      query += ",vgmlcAddress='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.contextInfoIsSet()) {
      to_json(j, amf3GppAccessRegistration.getContextInfo());
      query += ",contextInfo='" + j.dump() + "'";
    }

    to_json(j, amf3GppAccessRegistration.getGuami());
    query += ",guami='" + j.dump() + "'";
    to_json(j, amf3GppAccessRegistration.getRatType());
    query += ",ratType='" + j.dump() + "'";
    query += " WHERE ueid='" + ue_id + "'";
  } else {
    query =
        "INSERT INTO Amf3GppAccessRegistration SET ueid='" + ue_id + "'" +
        ",amfInstanceId='" + amf3GppAccessRegistration.getAmfInstanceId() +
        "'" +
        (amf3GppAccessRegistration.supportedFeaturesIsSet() ?
             ",supportedFeatures='" +
                 amf3GppAccessRegistration.getSupportedFeatures() + "'" :
             "") +
        (amf3GppAccessRegistration.purgeFlagIsSet() ?
             (amf3GppAccessRegistration.isPurgeFlag() ? ",purgeFlag=1" :
                                                        ",purgeFlag=0") :
             "") +
        (amf3GppAccessRegistration.peiIsSet() ?
             ",pei='" + amf3GppAccessRegistration.getPei() + "'" :
             "") +
        ",deregCallbackUri='" +
        amf3GppAccessRegistration.getDeregCallbackUri() + "'" +
        (amf3GppAccessRegistration.pcscfRestorationCallbackUriIsSet() ?
             ",pcscfRestorationCallbackUri='" +
                 amf3GppAccessRegistration.getPcscfRestorationCallbackUri() +
                 "'" :
             "") +
        (amf3GppAccessRegistration.initialRegistrationIndIsSet() ?
             (amf3GppAccessRegistration.isInitialRegistrationInd() ?
                  ",initialRegistrationInd=1" :
                  ",initialRegistrationInd=0") :
             "") +
        (amf3GppAccessRegistration.drFlagIsSet() ?
             (amf3GppAccessRegistration.isDrFlag() ? ",drFlag=1" :
                                                     ",drFlag=0") :
             "") +
        (amf3GppAccessRegistration.urrpIndicatorIsSet() ?
             (amf3GppAccessRegistration.isUrrpIndicator() ?
                  ",urrpIndicator=1" :
                  ",urrpIndicator=0") :
             "") +
        (amf3GppAccessRegistration.amfEeSubscriptionIdIsSet() ?
             ",amfEeSubscriptionId='" +
                 amf3GppAccessRegistration.getAmfEeSubscriptionId() + "'" :
             "") +
        (amf3GppAccessRegistration.ueSrvccCapabilityIsSet() ?
             (amf3GppAccessRegistration.isUeSrvccCapability() ?
                  ",ueSrvccCapability=1" :
                  ",ueSrvccCapability=0") :
             "") +
        (amf3GppAccessRegistration.registrationTimeIsSet() ?
             ",registrationTime='" +
                 amf3GppAccessRegistration.getRegistrationTime() + "'" :
             "") +
        (amf3GppAccessRegistration.noEeSubscriptionIndIsSet() ?
             (amf3GppAccessRegistration.isNoEeSubscriptionInd() ?
                  ",noEeSubscriptionInd=1" :
                  ",noEeSubscriptionInd=0") :
             "");

    if (amf3GppAccessRegistration.imsVoPsIsSet()) {
      to_json(j, amf3GppAccessRegistration.getImsVoPs());
      query += ",imsVoPs='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.amfServiceNameDeregIsSet()) {
      to_json(j, amf3GppAccessRegistration.getAmfServiceNameDereg());
      query += ",amfServiceNameDereg='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.amfServiceNamePcscfRestIsSet()) {
      to_json(j, amf3GppAccessRegistration.getAmfServiceNamePcscfRest());
      query += ",amfServiceNamePcscfRest='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.backupAmfInfoIsSet()) {
      nlohmann::json tmp = {};
      j.clear();
      std::vector<BackupAmfInfo> backupamfinfo =
          amf3GppAccessRegistration.getBackupAmfInfo();
      for (int i = 0; i < backupamfinfo.size(); i++) {
        to_json(tmp, backupamfinfo[i]);
        j += tmp;
      }
      query += ",backupAmfInfo='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.epsInterworkingInfoIsSet()) {
      to_json(j, amf3GppAccessRegistration.getEpsInterworkingInfo());
      query += ",epsInterworkingInfo='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.vgmlcAddressIsSet()) {
      to_json(j, amf3GppAccessRegistration.getVgmlcAddress());
      query += ",vgmlcAddress='" + j.dump() + "'";
    }
    if (amf3GppAccessRegistration.contextInfoIsSet()) {
      to_json(j, amf3GppAccessRegistration.getContextInfo());
      query += ",contextInfo='" + j.dump() + "'";
    }

    to_json(j, amf3GppAccessRegistration.getGuami());
    query += ",guami='" + j.dump() + "'";
    to_json(j, amf3GppAccessRegistration.getRatType());
    query += ",ratType='" + j.dump() + "'";
  }

  mysql_free_result(res);
  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s]  mysql_real_query failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  to_json(json_data, amf3GppAccessRegistration);

  Logger::udr_db().debug(
      "[UE Id %s] Amf3GppAccessRegistration PUT: %s", ue_id.c_str(),
      json_data.dump().c_str());

  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_amf_context_3gpp(
    const std::string& ue_id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res                                      = nullptr;
  MYSQL_ROW row                                       = {};
  MYSQL_FIELD* field                                  = nullptr;
  Amf3GppAccessRegistration amf3gppaccessregistration = {};
  const std::string query =
      "SELECT * FROM Amf3GppAccessRegistration WHERE ueid='" + ue_id + "'";

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  row = mysql_fetch_row(res);

  if (row != nullptr) {
    for (int i = 0; (field = mysql_fetch_field(res)); i++) {
      if (boost::iequals("amfInstanceId", field->name)) {
        amf3gppaccessregistration.setAmfInstanceId(row[i]);
      } else if (
          boost::iequals("supportedFeatures", field->name) &&
          row[i] != nullptr) {
        amf3gppaccessregistration.setSupportedFeatures(row[i]);
      } else if (
          boost::iequals("purgeFlag", field->name) && row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          amf3gppaccessregistration.setPurgeFlag(true);
        else
          amf3gppaccessregistration.setPurgeFlag(false);
      } else if (boost::iequals("pei", field->name) && row[i] != nullptr) {
        amf3gppaccessregistration.setPei(row[i]);
      } else if (boost::iequals("imsVoPs", field->name) && row[i] != nullptr) {
        ImsVoPs imsvops;
        nlohmann::json::parse(row[i]).get_to(imsvops);
        amf3gppaccessregistration.setImsVoPs(imsvops);
      } else if (boost::iequals("deregCallbackUri", field->name)) {
        amf3gppaccessregistration.setDeregCallbackUri(row[i]);
      } else if (
          boost::iequals("amfServiceNameDereg", field->name) &&
          row[i] != nullptr) {
        oai::model::nrf::ServiceName amfservicenamedereg;
        nlohmann::json::parse(row[i]).get_to(amfservicenamedereg);
        amf3gppaccessregistration.setAmfServiceNameDereg(amfservicenamedereg);
      } else if (
          boost::iequals("pcscfRestorationCallbackUri", field->name) &&
          row[i] != nullptr) {
        amf3gppaccessregistration.setPcscfRestorationCallbackUri(row[i]);
      } else if (
          boost::iequals("amfServiceNamePcscfRest", field->name) &&
          row[i] != nullptr) {
        oai::model::nrf::ServiceName amfservicenamepcscfrest;
        nlohmann::json::parse(row[i]).get_to(amfservicenamepcscfrest);
        amf3gppaccessregistration.setAmfServiceNamePcscfRest(
            amfservicenamepcscfrest);
      } else if (
          boost::iequals("initialRegistrationInd", field->name) &&
          row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          amf3gppaccessregistration.setInitialRegistrationInd(true);
        else
          amf3gppaccessregistration.setInitialRegistrationInd(false);
      } else if (boost::iequals("guami", field->name)) {
        Guami guami;
        nlohmann::json::parse(row[i]).get_to(guami);
        amf3gppaccessregistration.setGuami(guami);
      } else if (
          boost::iequals("backupAmfInfo", field->name) && row[i] != nullptr) {
        std ::vector<BackupAmfInfo> backupamfinfo;
        nlohmann::json::parse(row[i]).get_to(backupamfinfo);
        amf3gppaccessregistration.setBackupAmfInfo(backupamfinfo);
      } else if (boost::iequals("drFlag", field->name) && row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          amf3gppaccessregistration.setDrFlag(true);
        else
          amf3gppaccessregistration.setDrFlag(false);
      } else if (boost::iequals("ratType", field->name)) {
        RatType rattype;
        nlohmann::json::parse(row[i]).get_to(rattype);
        amf3gppaccessregistration.setRatType(rattype);
      } else if (
          boost::iequals("urrpIndicator", field->name) && row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          amf3gppaccessregistration.setUrrpIndicator(true);
        else
          amf3gppaccessregistration.setUrrpIndicator(false);
      } else if (
          boost::iequals("amfEeSubscriptionId", field->name) &&
          row[i] != nullptr) {
        amf3gppaccessregistration.setAmfEeSubscriptionId(row[i]);
      } else if (
          boost::iequals("epsInterworkingInfo", field->name) &&
          row[i] != nullptr) {
        EpsInterworkingInfo epsinterworkinginfo;
        nlohmann::json::parse(row[i]).get_to(epsinterworkinginfo);
        amf3gppaccessregistration.setEpsInterworkingInfo(epsinterworkinginfo);
      } else if (
          boost::iequals("ueSrvccCapability", field->name) &&
          row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          amf3gppaccessregistration.setUeSrvccCapability(true);
        else
          amf3gppaccessregistration.setUeSrvccCapability(false);
      } else if (
          boost::iequals("registrationTime", field->name) &&
          row[i] != nullptr) {
        amf3gppaccessregistration.setRegistrationTime(row[i]);
      } else if (
          boost::iequals("vgmlcAddress", field->name) && row[i] != nullptr) {
        VgmlcAddress vgmlcaddress;
        nlohmann::json::parse(row[i]).get_to(vgmlcaddress);
        amf3gppaccessregistration.setVgmlcAddress(vgmlcaddress);
      } else if (
          boost::iequals("contextInfo", field->name) && row[i] != nullptr) {
        ContextInfo contextinfo;
        nlohmann::json::parse(row[i]).get_to(contextinfo);
        amf3gppaccessregistration.setContextInfo(contextinfo);
      } else if (
          boost::iequals("noEeSubscriptionInd", field->name) &&
          row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          amf3gppaccessregistration.setNoEeSubscriptionInd(true);
        else
          amf3gppaccessregistration.setNoEeSubscriptionInd(false);
      }
    }
    to_json(json_data, amf3gppaccessregistration);

    Logger::udr_db().debug(
        "[UE Id %s] Amf3GppAccessRegistration GET %s", ue_id.c_str(),
        json_data.dump().c_str());
  } else {
    Logger::udr_db().error(
        "[UE Id %s] Amf3GppAccessRegistration no data！ SQL Query %s",
        ue_id.c_str(), query.c_str());
  }

  mysql_free_result(res);
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::insert_authentication_status(
    const std::string& ue_id, const AuthEvent& authEvent,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res = nullptr;
  MYSQL_ROW row  = {};

  const std::string select_AuthenticationStatus =
      "SELECT * FROM AuthenticationStatus WHERE ueid='" + ue_id + "'";
  std::string query = {};

  Logger::udr_db().info(
      "[UE Id %s] MySQL query: %s", ue_id.c_str(),
      select_AuthenticationStatus.c_str());

  if (mysql_real_query(
          &mysql_connector, select_AuthenticationStatus.c_str(),
          (unsigned long) select_AuthenticationStatus.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", ue_id.c_str(),
        select_AuthenticationStatus.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query %s", ue_id.c_str(),
        select_AuthenticationStatus.c_str());
    return false;
  }

  if (mysql_num_rows(res)) {
    query = "UPDATE AuthenticationStatus SET nfInstanceId='" +
            authEvent.getNfInstanceId() + "'" +
            ",success=" + (authEvent.isSuccess() ? "1" : "0") + ",timeStamp='" +
            authEvent.getTimeStamp() + "'" + ",authType='" +
            authEvent.getAuthType() + "'" + ",servingNetworkName='" +
            authEvent.getServingNetworkName() + "'" +
            (authEvent.authRemovalIndIsSet() ?
                 (authEvent.isAuthRemovalInd() ? ",authRemovalInd=1" :
                                                 ",authRemovalInd=0") :
                 "");
    //        to_json(j,authEvent.getAuthType());
    //        query += ",authType='"+j.dump()+"'";
    query += " WHERE ueid='" + ue_id + "'";
  } else {
    query = "INSERT INTO AuthenticationStatus SET ueid='" + ue_id + "'" +
            ",nfInstanceId='" + authEvent.getNfInstanceId() + "'" +
            ",success=" + (authEvent.isSuccess() ? "1" : "0") + ",timeStamp='" +
            authEvent.getTimeStamp() + "'" + ",authType='" +
            authEvent.getAuthType() + "'" + ",servingNetworkName='" +
            authEvent.getServingNetworkName() + "'" +
            (authEvent.authRemovalIndIsSet() ?
                 (authEvent.isAuthRemovalInd() ? ",authRemovalInd=1" :
                                                 ",authRemovalInd=0") :
                 "");
    //        to_json(j,authEvent.getAuthType());
    //        query += ",authType='"+j.dump()+"'";
  }

  Logger::udr_db().info(
      "[UE Id %s] MySQL query: %s", ue_id.c_str(), query.c_str());

  mysql_free_result(res);
  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql create failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  nlohmann::json tmp = {};
  to_json(tmp, authEvent);
  Logger::udr_db().info(
      "[UE Id %s] AuthenticationStatus PUT: %s", ue_id.c_str(),
      tmp.dump().c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::delete_authentication_status(
    const std::string& ue_id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  const std::string query =
      "DELETE FROM AuthenticationStatus WHERE ueid='" + ue_id + "'";

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  Logger::udr_db().debug(
      "[UE Id %s] AuthenticationStatus DELETE - successful", ue_id.c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::query_authentication_status(
    const std::string& ue_id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res                 = nullptr;
  MYSQL_ROW row                  = {};
  MYSQL_FIELD* field             = nullptr;
  AuthEvent authenticationstatus = {};
  const std::string query =
      "SELECT * FROM AuthenticationStatus WHERE ueid='" + ue_id + "'";

  Logger::udr_db().info(
      "[UE Id %s] MySQL query: %s", ue_id.c_str(), query.c_str());
  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！", ue_id.c_str());
    return false;
  }

  row = mysql_fetch_row(res);
  if (row != nullptr) {
    for (int i = 0; (field = mysql_fetch_field(res)); i++) {
      if (boost::iequals("nfInstanceId", field->name)) {
        authenticationstatus.setNfInstanceId(row[i]);
      } else if (boost::iequals("success", field->name)) {
        if (strcmp(row[i], "0"))
          authenticationstatus.setSuccess(true);
        else
          authenticationstatus.setSuccess(false);
      } else if (boost::iequals("timeStamp", field->name)) {
        authenticationstatus.setTimeStamp(row[i]);
      } else if (boost::iequals("authType", field->name)) {
        //                AuthType authtype;
        //                nlohmann::json::parse(row[i]).get_to(authtype);
        authenticationstatus.setAuthType(row[i]);
      } else if (boost::iequals("servingNetworkName", field->name)) {
        authenticationstatus.setServingNetworkName(row[i]);
      } else if (
          boost::iequals("authRemovalInd", field->name) && row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          authenticationstatus.setAuthRemovalInd(true);
        else
          authenticationstatus.setAuthRemovalInd(false);
      }
    }

    to_json(json_data, authenticationstatus);
    Logger::udr_db().info(
        "[UE Id %s] AuthenticationStatus GET: %s", ue_id.c_str(),
        json_data.dump().c_str());
  } else {
    Logger::udr_db().error(
        "[UE Id %s] AuthenticationStatus no data！ SQL Query %s", ue_id.c_str(),
        query.c_str());
  }

  mysql_free_result(res);
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::query_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res                   = nullptr;
  MYSQL_ROW row                    = {};
  MYSQL_FIELD* field               = nullptr;
  SdmSubscription SdmSubscriptions = {};
  const std::string query = "SELECT * FROM SdmSubscriptions WHERE ueid='" +
                            ue_id + "' AND subsId=" + subs_id;

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  row = mysql_fetch_row(res);
  if (row != nullptr) {
    for (int i = 0; (field = mysql_fetch_field(res)); i++) {
      if (boost::iequals("nfInstanceId", field->name)) {
        SdmSubscriptions.setNfInstanceId(row[i]);
      } else if (
          boost::iequals("implicitUnsubscribe", field->name) &&
          row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          SdmSubscriptions.setImplicitUnsubscribe(true);
        else
          SdmSubscriptions.setImplicitUnsubscribe(false);
      } else if (boost::iequals("expires", field->name) && row[i] != nullptr) {
        SdmSubscriptions.setExpires(row[i]);
      } else if (boost::iequals("callbackReference", field->name)) {
        SdmSubscriptions.setCallbackReference(row[i]);
      } else if (
          boost::iequals("amfServiceName", field->name) && row[i] != nullptr) {
        oai::model::nrf::ServiceName amfservicename;
        nlohmann::json::parse(row[i]).get_to(amfservicename);
        SdmSubscriptions.setAmfServiceName(amfservicename);
      } else if (boost::iequals("monitoredResourceUris", field->name)) {
        std::vector<std::string> monitoredresourceuris;
        nlohmann::json::parse(row[i]).get_to(monitoredresourceuris);
        SdmSubscriptions.setMonitoredResourceUris(monitoredresourceuris);
      } else if (
          boost::iequals("singleNssai", field->name) && row[i] != nullptr) {
        Snssai singlenssai;
        nlohmann::json::parse(row[i]).get_to(singlenssai);
        SdmSubscriptions.setSingleNssai(singlenssai);
      } else if (boost::iequals("dnn", field->name) && row[i] != nullptr) {
        SdmSubscriptions.setDnn(row[i]);
      } else if (
          boost::iequals("subscriptionId", field->name) && row[i] != nullptr) {
        SdmSubscriptions.setSubscriptionId(row[i]);
      } else if (boost::iequals("plmnId", field->name) && row[i] != nullptr) {
        PlmnId plmnid;
        nlohmann::json::parse(row[i]).get_to(plmnid);
        SdmSubscriptions.setPlmnId(plmnid);
      } else if (
          boost::iequals("immediateReport", field->name) && row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          SdmSubscriptions.setImmediateReport(true);
        else
          SdmSubscriptions.setImmediateReport(false);
      } else if (boost::iequals("report", field->name) && row[i] != nullptr) {
        SubscriptionDataSets report;
        nlohmann::json::parse(row[i]).get_to(report);
        SdmSubscriptions.setReport(report);
      } else if (
          boost::iequals("supportedFeatures", field->name) &&
          row[i] != nullptr) {
        SdmSubscriptions.setSupportedFeatures(row[i]);
      } else if (
          boost::iequals("contextInfo", field->name) && row[i] != nullptr) {
        ContextInfo contextinfo;
        nlohmann::json::parse(row[i]).get_to(contextinfo);
        SdmSubscriptions.setContextInfo(contextinfo);
      }
    }
    to_json(json_data, SdmSubscriptions);
    Logger::udr_db().debug(
        "[UE Id %s] SdmSubscription GET: %s", ue_id.c_str(),
        json_data.dump().c_str());
  } else {
    Logger::udr_db().error(
        "[UE Id %s] SdmSubscription no data！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
  }

  mysql_free_result(res);
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::mysql_db::delete_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res                = nullptr;
  ProblemDetails problemdetails = {};

  const std::string select_query =
      "SELECT * FROM SdmSubscriptions WHERE ueid='" + ue_id +
      "' AND subsId=" + subs_id;

  const std::string query = "DELETE FROM SdmSubscriptions WHERE ueid='" +
                            ue_id + "' AND subsId=" + subs_id;

  if (mysql_real_query(
          &mysql_connector, select_query.c_str(),
          (unsigned long) select_query.size())) {
    problemdetails.setCause("USER_NOT_FOUND");
    to_json(json_data, problemdetails);
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    problemdetails.setCause("USER_NOT_FOUND");
    to_json(json_data, problemdetails);
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  if (!mysql_num_rows(res)) {
    problemdetails.setCause("DATA_NOT_FOUND");
    to_json(json_data, problemdetails);
    return false;
  }
  mysql_free_result(res);

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    problemdetails.setCause("USER_NOT_FOUND");
    to_json(json_data, problemdetails);
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::update_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    SdmSubscription& sdmSubscription, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res = nullptr;
  MYSQL_ROW row  = {};

  const std::string select_query =
      "SELECT * FROM SdmSubscriptions WHERE ueid='" + ue_id +
      "' AND subsId=" + subs_id;
  std::string query             = {};
  nlohmann::json j              = {};
  ProblemDetails problemdetails = {};

  if (mysql_real_query(
          &mysql_connector, select_query.c_str(),
          (unsigned long) select_query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  if (mysql_num_rows(res)) {
    nlohmann::json MonitoredResourceUris_json(
        sdmSubscription.getMonitoredResourceUris());

    query =
        "UPDATE SdmSubscriptions SET nfInstanceId='" +
        sdmSubscription.getNfInstanceId() + "'" +
        (sdmSubscription.implicitUnsubscribeIsSet() ?
             (sdmSubscription.isImplicitUnsubscribe() ?
                  ",implicitUnsubscribe=1" :
                  ",implicitUnsubscribe=0") :
             "") +
        (sdmSubscription.expiresIsSet() ?
             ",expires='" + sdmSubscription.getExpires() + "'" :
             "") +
        ",callbackReference='" + sdmSubscription.getCallbackReference() + "'" +
        (sdmSubscription.dnnIsSet() ?
             ",dnn='" + sdmSubscription.getDnn() + "'" :
             "") +
        (sdmSubscription.subscriptionIdIsSet() ?
             ",subscriptionId='" + sdmSubscription.getSubscriptionId() + "'" :
             "") +
        (sdmSubscription.immediateReportIsSet() ?
             (sdmSubscription.isImmediateReport() ? ",immediateReport=1" :
                                                    ",immediateReport=0") :
             "") +
        (sdmSubscription.supportedFeaturesIsSet() ?
             ",supportedFeatures='" + sdmSubscription.getSupportedFeatures() +
                 "'" :
             "");

    if (sdmSubscription.amfServiceNameIsSet()) {
      to_json(j, sdmSubscription.getAmfServiceName());
      query += ",amfServiceName='" + j.dump() + "'";
    }
    if (sdmSubscription.singleNssaiIsSet()) {
      to_json(j, sdmSubscription.getSingleNssai());
      query += ",singleNssai='" + j.dump() + "'";
    }
    if (sdmSubscription.plmnIdIsSet()) {
      to_json(j, sdmSubscription.getPlmnId());
      query += ",plmnId='" + j.dump() + "'";
    }
    if (sdmSubscription.reportIsSet()) {
      to_json(j, sdmSubscription.getReport());
      query += ",report='" + j.dump() + "'";
    }
    if (sdmSubscription.contextInfoIsSet()) {
      to_json(j, sdmSubscription.getContextInfo());
      query += ",contextInfo='" + j.dump() + "'";
    }

    query +=
        ",monitoredResourceUris='" + MonitoredResourceUris_json.dump() + "'";

    query += " WHERE ueid='" + ue_id + "' AND subsId=" + subs_id;
  } else {
    to_json(json_data, problemdetails);
    mysql_free_result(res);
    return false;
  }

  mysql_free_result(res);
  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  nlohmann::json json_tmp = {};
  to_json(json_tmp, sdmSubscription);

  Logger::udr_db().debug(
      "[UE Id %s] SdmSubscription PUT: %s", ue_id.c_str(),
      json_tmp.dump().c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::create_sdm_subscriptions(
    const std::string& ue_id, SdmSubscription& sdmSubscription,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res   = nullptr;
  MYSQL_ROW row    = {};
  nlohmann::json j = {};
  int32_t subs_id  = 0;
  int32_t count    = 0;
  std::string query =
      "SELECT subsId FROM SdmSubscriptions WHERE ueid='" + ue_id + "'";

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  while (row = mysql_fetch_row(res)) {
    count++;
    if (strcmp(row[0], std::to_string(count).c_str())) {
      subs_id = count;
      break;
    }
  }
  mysql_free_result(res);

  query =
      "INSERT INTO SdmSubscriptions SET ueid='" + ue_id + "'" +
      ",nfInstanceId='" + sdmSubscription.getNfInstanceId() + "'" +
      (sdmSubscription.implicitUnsubscribeIsSet() ?
           (sdmSubscription.isImplicitUnsubscribe() ?
                ",implicitUnsubscribe=1" :
                ",implicitUnsubscribe=0") :
           "") +
      (sdmSubscription.expiresIsSet() ?
           ",expires='" + sdmSubscription.getExpires() + "'" :
           "") +
      ",callbackReference='" + sdmSubscription.getCallbackReference() + "'" +
      (sdmSubscription.dnnIsSet() ? ",dnn='" + sdmSubscription.getDnn() + "'" :
                                    "") +
      (sdmSubscription.subscriptionIdIsSet() ?
           ",subscriptionId='" + sdmSubscription.getSubscriptionId() + "'" :
           "") +
      (sdmSubscription.immediateReportIsSet() ?
           (sdmSubscription.isImmediateReport() ? ",immediateReport=1" :
                                                  ",immediateReport=0") :
           "") +
      (sdmSubscription.supportedFeaturesIsSet() ?
           ",supportedFeatures='" + sdmSubscription.getSupportedFeatures() +
               "'" :
           "");

  if (sdmSubscription.amfServiceNameIsSet()) {
    to_json(j, sdmSubscription.getAmfServiceName());
    query += ",amfServiceName='" + j.dump() + "'";
  }
  if (sdmSubscription.singleNssaiIsSet()) {
    to_json(j, sdmSubscription.getSingleNssai());
    query += ",singleNssai='" + j.dump() + "'";
  }
  if (sdmSubscription.plmnIdIsSet()) {
    to_json(j, sdmSubscription.getPlmnId());
    query += ",plmnId='" + j.dump() + "'";
  }
  if (sdmSubscription.reportIsSet()) {
    to_json(j, sdmSubscription.getReport());
    query += ",report='" + j.dump() + "'";
  }
  if (sdmSubscription.contextInfoIsSet()) {
    to_json(j, sdmSubscription.getContextInfo());
    query += ",contextInfo='" + j.dump() + "'";
  }

  nlohmann::json MonitoredResourceUris_json(
      sdmSubscription.getMonitoredResourceUris());
  query += ",monitoredResourceUris='" + MonitoredResourceUris_json.dump() + "'";

  if (subs_id && (subs_id == count)) {
    query += ",subsId=" + std::to_string(subs_id);
  }

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  to_json(j, sdmSubscription);
  json_data = j;

  Logger::udr_db().debug(
      "[UE Id %s] SdmSubscriptions POST: %s", ue_id.c_str(),
      json_data.dump().c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res     = nullptr;
  MYSQL_ROW row      = {};
  MYSQL_FIELD* field = nullptr;
  std::vector<std::string> fields;
  nlohmann::json j   = {};
  nlohmann::json tmp = {};

  const std::string query =
      "SELECT * FROM SdmSubscriptions WHERE ueid='" + ue_id + "'";

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  while ((field = mysql_fetch_field(res))) {
    fields.push_back(field->name);
  }

  j.clear();

  while ((row = mysql_fetch_row(res))) {
    SdmSubscription sdmsubscriptions = {};
    tmp.clear();

    for (int i = 0; i < fields.size(); i++) {
      if (boost::iequals("nfInstanceId", fields[i].c_str())) {
        sdmsubscriptions.setNfInstanceId(row[i]);
      } else if (
          boost::iequals("implicitUnsubscribe", fields[i].c_str()) &&
          row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          sdmsubscriptions.setImplicitUnsubscribe(true);
        else
          sdmsubscriptions.setImplicitUnsubscribe(false);
      } else if (
          boost::iequals("expires", fields[i].c_str()) && row[i] != nullptr) {
        sdmsubscriptions.setExpires(row[i]);
      } else if (boost::iequals("callbackReference", fields[i].c_str())) {
        sdmsubscriptions.setCallbackReference(row[i]);
      } else if (
          boost::iequals("amfServiceName", fields[i].c_str()) &&
          row[i] != nullptr) {
        oai::model::nrf::ServiceName amfservicename;
        nlohmann::json::parse(row[i]).get_to(amfservicename);
        sdmsubscriptions.setAmfServiceName(amfservicename);
      } else if (boost::iequals("monitoredResourceUris", fields[i].c_str())) {
        std::vector<std::string> monitoredresourceuris;
        nlohmann::json::parse(row[i]).get_to(monitoredresourceuris);
        sdmsubscriptions.setMonitoredResourceUris(monitoredresourceuris);
      } else if (
          boost::iequals("singleNssai", fields[i].c_str()) &&
          row[i] != nullptr) {
        Snssai singlenssai;
        nlohmann::json::parse(row[i]).get_to(singlenssai);
        sdmsubscriptions.setSingleNssai(singlenssai);
      } else if (
          boost::iequals("dnn", fields[i].c_str()) && row[i] != nullptr) {
        sdmsubscriptions.setDnn(row[i]);
      } else if (
          boost::iequals("subscriptionId", fields[i].c_str()) &&
          row[i] != nullptr) {
        sdmsubscriptions.setSubscriptionId(row[i]);
      } else if (
          boost::iequals("plmnId", fields[i].c_str()) && row[i] != nullptr) {
        PlmnId plmnid;
        nlohmann::json::parse(row[i]).get_to(plmnid);
        sdmsubscriptions.setPlmnId(plmnid);
      } else if (
          boost::iequals("immediateReport", fields[i].c_str()) &&
          row[i] != nullptr) {
        if (strcmp(row[i], "0"))
          sdmsubscriptions.setImmediateReport(true);
        else
          sdmsubscriptions.setImmediateReport(false);
      } else if (
          boost::iequals("report", fields[i].c_str()) && row[i] != nullptr) {
        SubscriptionDataSets report;
        nlohmann::json::parse(row[i]).get_to(report);
        sdmsubscriptions.setReport(report);
      } else if (
          boost::iequals("supportedFeatures", fields[i].c_str()) &&
          row[i] != nullptr) {
        sdmsubscriptions.setSupportedFeatures(row[i]);
      } else if (
          boost::iequals("contextInfo", fields[i].c_str()) &&
          row[i] != nullptr) {
        ContextInfo contextinfo;
        nlohmann::json::parse(row[i]).get_to(contextinfo);
        sdmsubscriptions.setContextInfo(contextinfo);
      }
    }
    to_json(tmp, sdmsubscriptions);
    j += tmp;
  }

  mysql_free_result(res);
  json_data = j;

  Logger::udr_db().debug(
      "[UE Id %s] SdmSubscriptions GET: %s", ue_id.c_str(),
      json_data.dump().c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::create_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    SessionManagementSubscriptionData& sm_subscription,
    nlohmann::json& json_data, uint32_t& resource_id) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res          = nullptr;
  MYSQL_ROW row           = {};
  nlohmann::json json_tmp = {};

  Snssai single_nssai = sm_subscription.getSingleNssai();

  std::string nssai_query = " AND JSON_EXTRACT(singleNssai, \"$.sst\")=" +
                            std::to_string(single_nssai.getSst());

  if (!single_nssai.getSd().empty()) {
    nssai_query += " AND JSON_EXTRACT(singleNssai, \"$.sd\")='" +
                   single_nssai.getSd() + "'";
  }

  std::string query =
      "SELECT * FROM SessionManagementSubscriptionData WHERE ueid='" + ue_id +
      "'" + "AND servingPlmnid='" + serving_plmn_id + "'" + nssai_query;

  Logger::udr_db().info(
      "[UE Id %s] MySQL Query: %s", ue_id.c_str(), query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size()) != 0) {
    Logger::udr_db().error(
        "[UE Id %s] Failed when executing mysql_real_query with SQL Query: %s",
        ue_id.c_str(), query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  row = mysql_fetch_row(res);
  if (row != nullptr) {  // Existed
    Logger::udr_db().error(
        "[UE Id %s] SessionManagementSubscriptionData existed!", ue_id.c_str());
    json_data["error"] = "resource already exists";
    return false;
  }
  mysql_free_result(res);

  // Insert a new row into DB
  query = "INSERT INTO SessionManagementSubscriptionData SET ueid='" + ue_id +
          "'" + ",servingPlmnid='" + serving_plmn_id + "'" +
          (sm_subscription.sharedDnnConfigurationsIdIsSet() ?
               ",sharedDnnConfigurationsId='" +
                   sm_subscription.getSharedDnnConfigurationsId() + "'" :
               "") +
          (sm_subscription.sharedTraceDataIdIsSet() ?
               ",sharedTraceDataId='" + sm_subscription.getSharedTraceDataId() +
                   "'" :
               "") +
          (sm_subscription.r3gppChargingCharacteristicsIsSet() ?
               ",3gppChargingCharacteristics='" +
                   sm_subscription.getR3gppChargingCharacteristics() + "'" :
               "");

  to_json(json_tmp, sm_subscription.getSingleNssai());
  query += ",singleNssai='" + json_tmp.dump() + "'";

  if (sm_subscription.dnnConfigurationsIsSet()) {
    json_tmp = sm_subscription.getDnnConfigurations();
    query += ",dnnConfigurations='" + json_tmp.dump() + "'";
  }

  if (sm_subscription.internalGroupIdsIsSet()) {
    nlohmann::json j;
    std::vector<std::string> internalGroupIds =
        sm_subscription.getInternalGroupIds();

    for (int i = 0; i < internalGroupIds.size(); i++) {
      json_tmp.push_back(internalGroupIds[i]);
    }
    query += ",internalGroupIds='" + json_tmp.dump() + "'";
  }

  if (sm_subscription.sharedVnGroupDataIdsIsSet()) {
    json_tmp = sm_subscription.getSharedVnGroupDataIds();
    query += ",sharedVnGroupDataIds='" + json_tmp.dump() + "'";
  }

  if (sm_subscription.odbPacketServicesIsSet()) {
    to_json(json_tmp, sm_subscription.getOdbPacketServices());
    query += ",odbPacketServices='" + json_tmp.dump() + "'";
  }

  if (sm_subscription.traceDataIsSet()) {
    to_json(json_tmp, sm_subscription.getTraceData());
    query += ",traceData='" + json_tmp.dump() + "'";
  }

  if (sm_subscription.expectedUeBehavioursListIsSet()) {
    json_tmp = sm_subscription.getExpectedUeBehavioursList();
    query += ",expectedUeBehavioursList='" + json_tmp.dump() + "'";
  }

  if (sm_subscription.suggestedPacketNumDlListIsSet()) {
    json_tmp = sm_subscription.getSuggestedPacketNumDlList();
    query += ",suggestedPacketNumDlList='" + json_tmp.dump() + "'";
  }

  Logger::udr_db().info(
      "[UE Id %s] MySQL Query: %s", ue_id.c_str(), query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  // Get SubscriptionId (used as part of the created resource's URI)
  // TODO: use LAST_INSERT_ID()
  // resource_id = mysql_insert_id(&mysql_connector) && 0x00000000ffffffff;

  std::string query_sub_id =
      "SELECT subscriptionId FROM SessionManagementSubscriptionData WHERE "
      "ueid='" +
      ue_id + "'" + "AND servingPlmnid='" + serving_plmn_id + "'" + nssai_query;

  Logger::udr_db().info(
      "[UE Id %s] MySQL Query: %s", ue_id.c_str(), query.c_str());
  if (mysql_real_query(
          &mysql_connector, query_sub_id.c_str(),
          (unsigned long) query_sub_id.size()) != 0) {
    Logger::udr_db().error(
        "[UE Id %s] Failed when executing mysql_real_query with SQL Query: %s",
        ue_id.c_str(), query_sub_id.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);

  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", ue_id.c_str(),
        query_sub_id.c_str());
    return false;
  }

  row = mysql_fetch_row(res);

  if (row != nullptr and row[0] != nullptr) {
    try {
      resource_id = std::stoi(row[0]);
    } catch (const std::exception& err) {
      Logger::udr_db().error(
          "[UE Id %s] Couldn't get SubscriptionId", ue_id.c_str());
      return false;
    }
    Logger::udr_db().debug(
        "[UE Id %s] SubscriptionId: %u", ue_id.c_str(), resource_id);
  }

  to_json(json_data, sm_subscription);

  Logger::udr_db().debug(
      "[UE Id %s] Inserted SessionManagementSubscription: %s", ue_id.c_str(),
      json_data.dump().c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::update_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    SessionManagementSubscriptionData& subscription_data,
    nlohmann::json& json_data, uint32_t& resource_id) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res                = nullptr;
  MYSQL_ROW row                 = {};
  std::string query             = {};
  ProblemDetails problemdetails = {};
  nlohmann::json j              = {};

  Snssai single_nssai = subscription_data.getSingleNssai();

  std::string nssai_query = " AND JSON_EXTRACT(singleNssai, \"$.sst\")=" +
                            std::to_string(single_nssai.getSst());

  if (!single_nssai.getSd().empty()) {
    nssai_query += " AND JSON_EXTRACT(singleNssai, \"$.sd\")='" +
                   single_nssai.getSd() + "'";
  }

  query = "SELECT * FROM SessionManagementSubscriptionData WHERE ueid='" +
          ue_id + "'" + "AND servingPlmnid='" + serving_plmn_id + "'" +
          nssai_query;

  Logger::udr_db().info(
      "[UE Id %s] MySQL Query: %s", ue_id.c_str(), query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size()) != 0) {
    Logger::udr_db().error(
        "[UE Id %s] Failed when executing mysql_real_query with SQL Query: %s",
        ue_id.c_str(), query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }
  row = mysql_fetch_row(res);
  if (row != nullptr) {  // if the row is existed, then update the UE's info
    Logger::udr_db().debug(
        "[UE Id %s] SessionManagementSubscriptionData existed!", ue_id.c_str());
    query =
        "UPDATE SessionManagementSubscriptionData "
        "SET " +
        (subscription_data.sharedDnnConfigurationsIdIsSet() ?
             "'sharedDnnConfigurationsId='" +
                 subscription_data.getSharedDnnConfigurationsId() + "'" :
             "") +
        (subscription_data.sharedTraceDataIdIsSet() ?
             ",sharedTraceDataId='" + subscription_data.getSharedTraceDataId() +
                 "'" :
             "") +
        (subscription_data.r3gppChargingCharacteristicsIsSet() ?
             ",3gppChargingCharacteristics='" +
                 subscription_data.getR3gppChargingCharacteristics() + "'" :
             "");

    if (subscription_data.dnnConfigurationsIsSet()) {
      j = subscription_data.getDnnConfigurations();
      query += "dnnConfigurations='" + j.dump() + "'";
    }
    if (subscription_data.internalGroupIdsIsSet()) {
      j = subscription_data.getInternalGroupIds();
      query += ",internalGroupIds='" + j.dump() + "'";
    }
    if (subscription_data.sharedVnGroupDataIdsIsSet()) {
      j = subscription_data.getSharedVnGroupDataIds();
      query += ",sharedVnGroupDataIds='" + j.dump() + "'";
    }
    if (subscription_data.odbPacketServicesIsSet()) {
      j = subscription_data.getOdbPacketServices();
      query += ",odbPacketServices='" + j.dump() + "'";
    }
    if (subscription_data.traceDataIsSet()) {
      j = subscription_data.getTraceData();
      query += ",traceData='" + j.dump() + "'";
    }
    if (subscription_data.expectedUeBehavioursListIsSet()) {
      j = subscription_data.getExpectedUeBehavioursList();
      query += ",expectedUeBehavioursList='" + j.dump() + "'";
    }
    if (subscription_data.suggestedPacketNumDlListIsSet()) {
      j = subscription_data.getSuggestedPacketNumDlList();
      query += ",suggestedPacketNumDlList='" + j.dump() + "'";
    }

    query += " WHERE ueId='" + ue_id + "' AND servingPlmnId='" +
             serving_plmn_id + "'" + nssai_query;

    if (mysql_real_query(
            &mysql_connector, query.c_str(), (unsigned long) query.size())) {
      Logger::udr_db().error(
          "mysql_real_query failure！ SQL Query: %s", query.c_str());
      return false;
    }

    to_json(json_data, subscription_data);
    Logger::udr_db().debug(
        "SessionManagementSubscriptionData PUT: %s", json_data.dump().c_str());
    return true;
  } else {  // Create a new row
    return create_sm_data(
        ue_id, serving_plmn_id, subscription_data, json_data, resource_id);
  }
}

//------------------------------------------------------------------------------
bool mysql_db::query_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data, const std::optional<Snssai>& snssai,
    const std::optional<std::string>& dnn) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res     = nullptr;
  MYSQL_ROW row      = {};
  MYSQL_FIELD* field = nullptr;
  nlohmann::json j   = {};

  std::string query =
      "SELECT * FROM SessionManagementSubscriptionData WHERE ueid='" + ue_id +
      "' AND servingPlmnid='" + serving_plmn_id + "' ";
  std::string option_str = {};

  if (snssai.has_value()) {
    option_str += " AND JSON_EXTRACT(singleNssai, \"$.sst\")=" +
                  std::to_string(snssai.value().getSst());

    if (!snssai.value().getSd().empty()) {
      option_str += " AND JSON_EXTRACT(singleNssai, \"$.sd\")='" +
                    snssai.value().getSd() + "'";
    }
  }

  if (dnn.has_value()) {
    option_str += " AND JSON_EXTRACT(dnnConfigurations, \'$.\"" + dnn.value() +
                  "\"\') IS NOT NULL";
  }

  query += option_str;
  Logger::udr_db().debug(
      "[UE Id %s] MySQL query: %s", ue_id.c_str(), query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure, SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure, SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  std::vector<std::string> fields;

  while ((field = mysql_fetch_field(res))) {
    fields.push_back(field->name);
  }
  if (fields.size() == 0) {
    Logger::udr_db().debug(
        "[UE Id %s] SessionManagementSubscriptionData no data found, SQL "
        "query: %s",
        ue_id.c_str(), query.c_str());
  }

  while ((row = mysql_fetch_row(res))) {
    nlohmann::json json_tmp                                                = {};
    SessionManagementSubscriptionData session_management_subscription_data = {};
    for (int i = 0; i < fields.size(); i++) {
      Logger::udr_db().debug(
          "[UE Id %s] SessionManagementSubscriptionData, Field name: %s",
          ue_id.c_str(), fields[i].c_str());
      if (boost::iequals("singleNssai", fields[i]) && row[i] != nullptr) {
        Snssai single_nssai = {};
        nlohmann::json::parse(row[i]).get_to(single_nssai);
        session_management_subscription_data.setSingleNssai(single_nssai);
      } else if (
          boost::iequals("dnnConfigurations", fields[i]) && row[i] != nullptr) {
        std ::map<std ::string, DnnConfiguration> dnn_configurations;
        nlohmann::json::parse(row[i]).get_to(dnn_configurations);
        session_management_subscription_data.setDnnConfigurations(
            dnn_configurations);
        Logger::udr_db().debug(
            "[UE Id %s] DNN configurations (row %d): %s", ue_id.c_str(), i,
            row[i]);
        for (auto d : dnn_configurations) {
          nlohmann::json temp = {};
          to_json(temp, d.second);
          Logger::udr_db().debug(
              "[UE Id %s] DNN configurations: %s", ue_id.c_str(),
              temp.dump().c_str());
        }
      } else if (
          boost::iequals("internalGroupIds", fields[i]) && row[i] != nullptr) {
        std ::vector<std ::string> internal_group_ids;
        nlohmann::json::parse(row[i]).get_to(internal_group_ids);
        session_management_subscription_data.setInternalGroupIds(
            internal_group_ids);
      } else if (
          boost::iequals("sharedVnGroupDataIds", fields[i]) &&
          row[i] != nullptr) {
        std ::map<std ::string, std ::string> shared_vn_group_data_ids;
        nlohmann::json::parse(row[i]).get_to(shared_vn_group_data_ids);
        session_management_subscription_data.setSharedVnGroupDataIds(
            shared_vn_group_data_ids);
      } else if (
          boost::iequals("sharedDnnConfigurationsId", fields[i]) &&
          row[i] != nullptr) {
        session_management_subscription_data.setSharedDnnConfigurationsId(
            row[i]);
      } else if (
          boost::iequals("odbPacketServices", fields[i]) && row[i] != nullptr) {
        OdbPacketServices odbpacketservices;
        nlohmann::json::parse(row[i]).get_to(odbpacketservices);
        session_management_subscription_data.setOdbPacketServices(
            odbpacketservices);
      } else if (boost::iequals("traceData", fields[i]) && row[i] != nullptr) {
        TraceData tracedata;
        nlohmann::json::parse(row[i]).get_to(tracedata);
        session_management_subscription_data.setTraceData(tracedata);
      } else if (
          boost::iequals("sharedTraceDataId", fields[i]) && row[i] != nullptr) {
        session_management_subscription_data.setSharedTraceDataId(row[i]);
      } else if (
          boost::iequals("expectedUeBehavioursList", fields[i]) &&
          row[i] != nullptr) {
        std ::map<std ::string, ExpectedUeBehaviourData>
            expecteduebehaviourslist;
        nlohmann::json::parse(row[i]).get_to(expecteduebehaviourslist);
        session_management_subscription_data.setExpectedUeBehavioursList(
            expecteduebehaviourslist);
      } else if (
          boost::iequals("suggestedPacketNumDlList", fields[i]) &&
          row[i] != nullptr) {
        std ::map<std ::string, SuggestedPacketNumDl> suggestedpacketnumdllist;
        nlohmann::json::parse(row[i]).get_to(suggestedpacketnumdllist);
        session_management_subscription_data.setSuggestedPacketNumDlList(
            suggestedpacketnumdllist);
      } else if (
          boost::iequals("3gppChargingCharacteristics", fields[i]) &&
          row[i] != nullptr) {
        session_management_subscription_data.setR3gppChargingCharacteristics(
            row[i]);
      }
    }
    to_json(json_tmp, session_management_subscription_data);
    json_data += json_tmp;
    Logger::udr_db().debug(
        "[UE Id %s] SessionManagementSubscriptionData: %s", ue_id.c_str(),
        json_data.dump().c_str());
  }
  mysql_free_result(res);

  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_sm_data(nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res     = nullptr;
  MYSQL_ROW row      = {};
  MYSQL_FIELD* field = nullptr;
  std::vector<std::string> fields;
  nlohmann::json ue_data = {};
  nlohmann::json j       = {};
  nlohmann::json tmp     = {};
  std::string query      = "SELECT * FROM SessionManagementSubscriptionData";

  Logger::udr_db().debug("MySQL query: %s", query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "mysql_real_query failure, SQL Query: %s", query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == NULL) {
    Logger::udr_db().error(
        "mysql_store_result failure, SQL Query: %s", query.c_str());
    return false;
  }

  while ((field = mysql_fetch_field(res))) {
    fields.push_back(field->name);
  }

  if (fields.size() == 0) {
    Logger::udr_db().debug(
        "SessionManagementSubscriptionData no data found, SQL "
        "query: %s",
        query.c_str());
  }

  while ((row = mysql_fetch_row(res))) {
    nlohmann::json json_tmp                                                = {};
    SessionManagementSubscriptionData session_management_subscription_data = {};
    std::string ue_id                                                      = {};
    for (int i = 0; i < fields.size(); i++) {
      Logger::udr_db().debug(
          "SessionManagementSubscriptionData, Field name: %s",
          fields[i].c_str());
      if (boost::iequals("ueid", fields[i]) && row[i] != nullptr) {
        // session_management_subscription_data.setUeId(row[i]);
        ue_id = row[i];
      } else if (
          boost::iequals("singleNssai", fields[i]) && row[i] != nullptr) {
        Snssai single_nssai = {};
        nlohmann::json::parse(row[i]).get_to(single_nssai);
        session_management_subscription_data.setSingleNssai(single_nssai);
      } else if (
          boost::iequals("dnnConfigurations", fields[i]) && row[i] != nullptr) {
        std ::map<std ::string, DnnConfiguration> dnn_configurations;
        nlohmann::json::parse(row[i]).get_to(dnn_configurations);
        session_management_subscription_data.setDnnConfigurations(
            dnn_configurations);
        Logger::udr_db().debug(
            "[UE Id %s] DNN configurations (row %d): %s", ue_id.c_str(), i,
            row[i]);
        for (auto d : dnn_configurations) {
          nlohmann::json temp = {};
          to_json(temp, d.second);
          Logger::udr_db().debug(
              "[UE Id %s] DNN configurations: %s", ue_id.c_str(),
              temp.dump().c_str());
        }
      } else if (
          boost::iequals("internalGroupIds", fields[i]) && row[i] != nullptr) {
        std ::vector<std ::string> internal_group_ids;
        nlohmann::json::parse(row[i]).get_to(internal_group_ids);
        session_management_subscription_data.setInternalGroupIds(
            internal_group_ids);
      } else if (
          boost::iequals("sharedVnGroupDataIds", fields[i]) &&
          row[i] != nullptr) {
        std ::map<std ::string, std ::string> shared_vn_group_data_ids;
        nlohmann::json::parse(row[i]).get_to(shared_vn_group_data_ids);
        session_management_subscription_data.setSharedVnGroupDataIds(
            shared_vn_group_data_ids);
      } else if (
          boost::iequals("sharedDnnConfigurationsId", fields[i]) &&
          row[i] != nullptr) {
        session_management_subscription_data.setSharedDnnConfigurationsId(
            row[i]);
      } else if (
          boost::iequals("odbPacketServices", fields[i]) && row[i] != nullptr) {
        OdbPacketServices odbpacketservices;
        nlohmann::json::parse(row[i]).get_to(odbpacketservices);
        session_management_subscription_data.setOdbPacketServices(
            odbpacketservices);
      } else if (boost::iequals("traceData", fields[i]) && row[i] != nullptr) {
        TraceData tracedata;
        nlohmann::json::parse(row[i]).get_to(tracedata);
        session_management_subscription_data.setTraceData(tracedata);
      } else if (
          boost::iequals("sharedTraceDataId", fields[i]) && row[i] != nullptr) {
        session_management_subscription_data.setSharedTraceDataId(row[i]);
      } else if (
          boost::iequals("expectedUeBehavioursList", fields[i]) &&
          row[i] != nullptr) {
        std ::map<std ::string, ExpectedUeBehaviourData>
            expecteduebehaviourslist;
        nlohmann::json::parse(row[i]).get_to(expecteduebehaviourslist);
        session_management_subscription_data.setExpectedUeBehavioursList(
            expecteduebehaviourslist);
      } else if (
          boost::iequals("suggestedPacketNumDlList", fields[i]) &&
          row[i] != nullptr) {
        std ::map<std ::string, SuggestedPacketNumDl> suggestedpacketnumdllist;
        nlohmann::json::parse(row[i]).get_to(suggestedpacketnumdllist);
        session_management_subscription_data.setSuggestedPacketNumDlList(
            suggestedpacketnumdllist);
      } else if (
          boost::iequals("3gppChargingCharacteristics", fields[i]) &&
          row[i] != nullptr) {
        session_management_subscription_data.setR3gppChargingCharacteristics(
            row[i]);
      }
    }
    to_json(json_tmp, session_management_subscription_data);
    json_tmp["ueid"] = ue_id;
    json_data += json_tmp;
    Logger::udr_db().debug(
        "SessionManagementSubscriptionData: %s", json_data.dump().c_str());
  }

  mysql_free_result(res);

  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::delete_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    const std::optional<Snssai>& snssai) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  std::string option_str = {};

  if (snssai.has_value()) {
    option_str += " AND JSON_EXTRACT(singleNssai, \"$.sst\")=" +
                  std::to_string(snssai.value().getSst());

    if (!snssai.value().getSd().empty()) {
      option_str += " AND JSON_EXTRACT(singleNssai, \"$.sd\")='" +
                    snssai.value().getSd() + "'";
    }
  }

  std::string query =
      "DELETE FROM SessionManagementSubscriptionData WHERE ueid='" + ue_id +
      "' AND servingPlmnid='" + serving_plmn_id + "'" + option_str;

  Logger::udr_db().debug("MySQL query: %s", query.c_str());

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "mysql_real_query failure！ SQL Query: %s", query.c_str());
    return false;
  }

  Logger::udr_db().debug(
      "[UE Id %s]  SessionManagementSubscriptionData DELETE - successful",
      ue_id.c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::insert_smf_context_non_3gpp(
    const std::string& ue_id, const int32_t& pdu_session_id,
    const SmfRegistration& smfRegistration, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res    = nullptr;
  MYSQL_ROW row     = {};
  std::string query = {};
  nlohmann::json j  = {};

  const std::string select_SmfRegistration =
      "SELECT * FROM SmfRegistrations WHERE ueid='" + ue_id +
      "' AND subpduSessionId=" + std::to_string(pdu_session_id);

  if (mysql_real_query(
          &mysql_connector, select_SmfRegistration.c_str(),
          (unsigned long) select_SmfRegistration.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        select_SmfRegistration.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！ SQL Query: %s", ue_id.c_str(),
        select_SmfRegistration.c_str());
    return false;
  }

  if (mysql_num_rows(res)) {
    query =
        "UPDATE SmfRegistrations SET smfInstanceId='" +
        smfRegistration.getSmfInstanceId() + "'" +
        ",pduSessionId=" + std::to_string(smfRegistration.getPduSessionId()) +
        (smfRegistration.smfSetIdIsSet() ?
             ",smfSetId='" + smfRegistration.getSmfSetId() + "'" :
             "") +
        (smfRegistration.supportedFeaturesIsSet() ?
             ",supportedFeatures='" + smfRegistration.getSupportedFeatures() +
                 "'" :
             "") +
        (smfRegistration.dnnIsSet() ?
             ",dnn='" + smfRegistration.getDnn() + "'" :
             "") +
        (smfRegistration.emergencyServicesIsSet() ?
             (smfRegistration.isEmergencyServices() ? ",emergencyServices=1" :
                                                      ",emergencyServices=0") :
             "") +
        (smfRegistration.pcscfRestorationCallbackUriIsSet() ?
             ",pcscfRestorationCallbackUri='" +
                 smfRegistration.getPcscfRestorationCallbackUri() + "'" :
             "") +
        (smfRegistration.pgwFqdnIsSet() ?
             ",pgwFqdn='" + smfRegistration.getPgwFqdn() + "'" :
             "") +
        (smfRegistration.epdgIndIsSet() ?
             (smfRegistration.isEpdgInd() ? ",epdgInd=1" : ",epdgInd=0") :
             "") +
        (smfRegistration.deregCallbackUriIsSet() ?
             ",deregCallbackUri='" + smfRegistration.getDeregCallbackUri() +
                 "'" :
             "") +
        (smfRegistration.registrationTimeIsSet() ?
             ",registrationTime='" + smfRegistration.getRegistrationTime() +
                 "'" :
             "");

    if (smfRegistration.registrationReasonIsSet()) {
      to_json(j, smfRegistration.getRegistrationReason());
      query += ",registrationReason='" + j.dump() + "'";
    }
    if (smfRegistration.contextInfoIsSet()) {
      to_json(j, smfRegistration.getContextInfo());
      query += ",contextInfo='" + j.dump() + "'";
    }

    to_json(j, smfRegistration.getSingleNssai());
    query += ",singleNssai='" + j.dump() + "'";
    to_json(j, smfRegistration.getPlmnId());
    query += ",plmnId='" + j.dump() + "'";
    query += " WHERE ueid='" + ue_id +
             "' AND subpduSessionId=" + std::to_string(pdu_session_id);
  } else {
    query =
        "INSERT INTO SmfRegistrations SET ueid='" + ue_id + "'" +
        ",subpduSessionId=" + std::to_string(pdu_session_id) +
        ",pduSessionId=" + std::to_string(smfRegistration.getPduSessionId()) +
        ",smfInstanceId='" + smfRegistration.getSmfInstanceId() + "'" +
        (smfRegistration.smfSetIdIsSet() ?
             ",smfSetId='" + smfRegistration.getSmfSetId() + "'" :
             "") +
        (smfRegistration.supportedFeaturesIsSet() ?
             ",supportedFeatures='" + smfRegistration.getSupportedFeatures() +
                 "'" :
             "") +
        (smfRegistration.dnnIsSet() ?
             ",dnn='" + smfRegistration.getDnn() + "'" :
             "") +
        (smfRegistration.emergencyServicesIsSet() ?
             (smfRegistration.isEmergencyServices() ? ",emergencyServices=1" :
                                                      ",emergencyServices=0") :
             "") +
        (smfRegistration.pcscfRestorationCallbackUriIsSet() ?
             ",pcscfRestorationCallbackUri='" +
                 smfRegistration.getPcscfRestorationCallbackUri() + "'" :
             "") +
        (smfRegistration.pgwFqdnIsSet() ?
             ",pgwFqdn='" + smfRegistration.getPgwFqdn() + "'" :
             "") +
        (smfRegistration.epdgIndIsSet() ?
             (smfRegistration.isEpdgInd() ? ",epdgInd=1" : ",epdgInd=0") :
             "") +
        (smfRegistration.deregCallbackUriIsSet() ?
             ",deregCallbackUri='" + smfRegistration.getDeregCallbackUri() +
                 "'" :
             "") +
        (smfRegistration.registrationTimeIsSet() ?
             ",registrationTime='" + smfRegistration.getRegistrationTime() +
                 "'" :
             "");

    if (smfRegistration.registrationReasonIsSet()) {
      to_json(j, smfRegistration.getRegistrationReason());
      query += ",registrationReason='" + j.dump() + "'";
    }
    if (smfRegistration.contextInfoIsSet()) {
      to_json(j, smfRegistration.getContextInfo());
      query += ",contextInfo='" + j.dump() + "'";
    }

    to_json(j, smfRegistration.getSingleNssai());
    query += ",singleNssai='" + j.dump() + "'";
    to_json(j, smfRegistration.getPlmnId());
    query += ",plmnId='" + j.dump() + "'";
  }

  mysql_free_result(res);
  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  to_json(j, smfRegistration);
  json_data = j;

  Logger::udr_db().debug(
      "[UE Id %s] SmfRegistration PUT: %s", ue_id.c_str(), j.dump().c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::delete_smf_context(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  const std::string query =
      "DELETE FROM SmfRegistrations WHERE ueid='" + ue_id +
      "' AND subpduSessionId=" + std::to_string(pdu_session_id);

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  // r_data = {};
  // code          = HTTP_STATUS_CODE_204_NO_CONTENT;
  Logger::udr_db().debug(
      "[UE Id %s] SmfRegistration DELETE - successful", ue_id.c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_smf_registration(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res                  = nullptr;
  MYSQL_ROW row                   = {};
  MYSQL_FIELD* field              = nullptr;
  nlohmann::json j                = {};
  SmfRegistration smfregistration = {};
  const std::string query =
      "SELECT * FROM SmfRegistrations WHERE ueid='" + ue_id +
      "' AND subpduSessionId=" + std::to_string(pdu_session_id);

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  row = mysql_fetch_row(res);
  if (row != nullptr) {
    for (int i = 0; (field = mysql_fetch_field(res)); i++) {
      try {
        if (boost::iequals("smfInstanceId", field->name)) {
          smfregistration.setSmfInstanceId(row[i]);
        } else if (
            boost::iequals("smfSetId", field->name) && row[i] != nullptr) {
          smfregistration.setSmfSetId(row[i]);
        } else if (
            boost::iequals("supportedFeatures", field->name) &&
            row[i] != nullptr) {
          smfregistration.setSupportedFeatures(row[i]);
        } else if (boost::iequals("pduSessionId", field->name)) {
          int32_t a = std::stoi(row[i]);
          smfregistration.setPduSessionId(a);
        } else if (boost::iequals("singleNssai", field->name)) {
          Snssai singlenssai;
          nlohmann::json::parse(row[i]).get_to(singlenssai);
          smfregistration.setSingleNssai(singlenssai);
        } else if (boost::iequals("dnn", field->name) && row[i] != nullptr) {
          smfregistration.setDnn(row[i]);
        } else if (
            boost::iequals("emergencyServices", field->name) &&
            row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            smfregistration.setEmergencyServices(true);
          else
            smfregistration.setEmergencyServices(false);
        } else if (
            boost::iequals("pcscfRestorationCallbackUri", field->name) &&
            row[i] != nullptr) {
          smfregistration.setPcscfRestorationCallbackUri(row[i]);
        } else if (boost::iequals("plmnId", field->name)) {
          PlmnId plmnid;
          nlohmann::json::parse(row[i]).get_to(plmnid);
          smfregistration.setPlmnId(plmnid);
        } else if (
            boost::iequals("pgwFqdn", field->name) && row[i] != nullptr) {
          smfregistration.setPgwFqdn(row[i]);
        } else if (
            boost::iequals("epdgInd", field->name) && row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            smfregistration.setEpdgInd(true);
          else
            smfregistration.setEpdgInd(false);
        } else if (
            boost::iequals("deregCallbackUri", field->name) &&
            row[i] != nullptr) {
          smfregistration.setDeregCallbackUri(row[i]);
        } else if (
            boost::iequals("registrationReason", field->name) &&
            row[i] != nullptr) {
          RegistrationReason registrationreason;
          nlohmann::json::parse(row[i]).get_to(registrationreason);
          smfregistration.setRegistrationReason(registrationreason);
        } else if (
            boost::iequals("registrationTime", field->name) &&
            row[i] != nullptr) {
          smfregistration.setRegistrationTime(row[i]);
        } else if (
            boost::iequals("contextInfo", field->name) && row[i] != nullptr) {
          ContextInfo contextinfo;
          nlohmann::json::parse(row[i]).get_to(contextinfo);
          smfregistration.setContextInfo(contextinfo);
        }
      } catch (std::exception e) {
        Logger::udr_db().error(
            "[UE Id %s] Cannot set values for SMF Registration: %s",
            ue_id.c_str(), e.what());
      }
    }
    to_json(j, smfregistration);
    json_data = j;

    Logger::udr_db().debug(
        "[UE Id %s] SmfRegistration GET: %s", ue_id.c_str(), j.dump().c_str());
  } else {
    Logger::udr_db().error(
        "[UE Id %s] SmfRegistration no data！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
  }
  mysql_free_result(res);

  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_smf_reg_list(
    const std::string& ue_id, nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res     = nullptr;
  MYSQL_ROW row      = {};
  MYSQL_FIELD* field = nullptr;
  std::vector<std::string> fields;
  nlohmann::json j   = {};
  nlohmann::json tmp = {};

  const std::string query =
      "SELECT * FROM SmfRegistrations WHERE ueid='" + ue_id + "'";

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  while ((field = mysql_fetch_field(res))) {
    fields.push_back(field->name);
  }

  j.clear();
  while (row = mysql_fetch_row(res)) {
    SmfRegistration smfregistration = {};

    tmp.clear();

    for (int i = 0; i < fields.size(); i++) {
      try {
        if (boost::iequals("smfInstanceId", fields[i].c_str())) {
          smfregistration.setSmfInstanceId(row[i]);
        } else if (
            boost::iequals("smfSetId", fields[i].c_str()) &&
            row[i] != nullptr) {
          smfregistration.setSmfSetId(row[i]);
        } else if (
            boost::iequals("supportedFeatures", fields[i].c_str()) &&
            row[i] != nullptr) {
          smfregistration.setSupportedFeatures(row[i]);
        } else if (boost::iequals("pduSessionId", fields[i].c_str())) {
          int32_t a = std::stoi(row[i]);
          smfregistration.setPduSessionId(a);
        } else if (boost::iequals("singleNssai", fields[i].c_str())) {
          Snssai singlenssai;
          nlohmann::json::parse(row[i]).get_to(singlenssai);
          smfregistration.setSingleNssai(singlenssai);
        } else if (
            boost::iequals("dnn", fields[i].c_str()) && row[i] != nullptr) {
          smfregistration.setDnn(row[i]);
        } else if (
            boost::iequals("emergencyServices", fields[i].c_str()) &&
            row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            smfregistration.setEmergencyServices(true);
          else
            smfregistration.setEmergencyServices(false);
        } else if (
            boost::iequals("pcscfRestorationCallbackUri", fields[i].c_str()) &&
            row[i] != nullptr) {
          smfregistration.setPcscfRestorationCallbackUri(row[i]);
        } else if (boost::iequals("plmnId", fields[i].c_str())) {
          PlmnId plmnid;
          nlohmann::json::parse(row[i]).get_to(plmnid);
          smfregistration.setPlmnId(plmnid);
        } else if (
            boost::iequals("pgwFqdn", fields[i].c_str()) && row[i] != nullptr) {
          smfregistration.setPgwFqdn(row[i]);
        } else if (
            boost::iequals("epdgInd", fields[i].c_str()) && row[i] != nullptr) {
          if (strcmp(row[i], "0"))
            smfregistration.setEpdgInd(true);
          else
            smfregistration.setEpdgInd(false);
        } else if (
            boost::iequals("deregCallbackUri", fields[i].c_str()) &&
            row[i] != nullptr) {
          smfregistration.setDeregCallbackUri(row[i]);
        } else if (
            boost::iequals("registrationReason", fields[i].c_str()) &&
            row[i] != nullptr) {
          RegistrationReason registrationreason;
          nlohmann::json::parse(row[i]).get_to(registrationreason);
          smfregistration.setRegistrationReason(registrationreason);
        } else if (
            boost::iequals("registrationTime", fields[i].c_str()) &&
            row[i] != nullptr) {
          smfregistration.setRegistrationTime(row[i]);
        } else if (
            boost::iequals("contextInfo", fields[i].c_str()) &&
            row[i] != nullptr) {
          ContextInfo contextinfo;
          nlohmann::json::parse(row[i]).get_to(contextinfo);
          smfregistration.setContextInfo(contextinfo);
        }
      } catch (std::exception e) {
        Logger::udr_db().error(
            "[UE Id %s] Cannot set values for SMF Registration: %s",
            ue_id.c_str(), e.what());
      }
    }
    to_json(tmp, smfregistration);
    j += tmp;
  }

  mysql_free_result(res);

  json_data = j;
  // code          = HTTP_STATUS_CODE_200_OK;

  Logger::udr_db().debug(
      "[UE Id %s] SmfRegistrations GET: %s", ue_id.c_str(), j.dump().c_str());
  return true;
}

//------------------------------------------------------------------------------
bool mysql_db::query_smf_select_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  // Check the connection with DB first
  if (!check_connection_status()) return false;

  MYSQL_RES* res                                            = nullptr;
  MYSQL_ROW row                                             = {};
  MYSQL_FIELD* field                                        = nullptr;
  nlohmann::json j                                          = {};
  SmfSelectionSubscriptionData smfselectionsubscriptiondata = {};
  const std::string query =
      "SELECT * FROM SmfSelectionSubscriptionData WHERE ueid='" + ue_id +
      "' AND servingPlmnid='" + serving_plmn_id + "'";

  if (mysql_real_query(
          &mysql_connector, query.c_str(), (unsigned long) query.size())) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_real_query failure！ SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  res = mysql_store_result(&mysql_connector);
  if (res == nullptr) {
    Logger::udr_db().error(
        "[UE Id %s] mysql_store_result failure！SQL Query: %s", ue_id.c_str(),
        query.c_str());
    return false;
  }

  row = mysql_fetch_row(res);

  if (row != nullptr) {
    for (int i = 0; (field = mysql_fetch_field(res)); i++) {
      if (boost::iequals("supportedFeatures", field->name) &&
          row[i] != nullptr) {
        smfselectionsubscriptiondata.setSupportedFeatures(row[i]);
      } else if (
          boost::iequals("subscribedSnssaiInfos", field->name) &&
          row[i] != nullptr) {
        std ::map<std ::string, SnssaiInfo> subscribedsnssaiinfos;
        nlohmann::json::parse(row[i]).get_to(subscribedsnssaiinfos);
        smfselectionsubscriptiondata.setSubscribedSnssaiInfos(
            subscribedsnssaiinfos);
      } else if (
          boost::iequals("sharedSnssaiInfosId", field->name) &&
          row[i] != nullptr) {
        smfselectionsubscriptiondata.setSharedSnssaiInfosId(row[i]);
      }
    }
    to_json(j, smfselectionsubscriptiondata);
    json_data = j;

    Logger::udr_db().debug(
        "[UE Id %s] SmfSelectionSubscriptionData GET: %s", ue_id.c_str(),
        j.dump().c_str());
  } else {
    Logger::udr_db().error(
        "[UE Id %s] SmfSelectionSubscriptionData no data！SQL Query: %s",
        ue_id.c_str(), query.c_str());
  }

  mysql_free_result(res);
  return true;
}
