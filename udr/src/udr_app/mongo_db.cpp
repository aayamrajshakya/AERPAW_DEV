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

/*! \file mongo_db.cpp
 \brief
 \author
 \company Eurecom, phine.tech
 \date 2023
 \email: contact@openairinterface.org, lukas.rotheneder@phine.tech
 */

#include "mongo_db.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <spdlog/spdlog.h>

#include "AccessAndMobilitySubscriptionData.h"
#include "AuthenticationSubscription.h"
#include "ProblemDetails.h"
#include "SdmSubscription.h"
#include "logger.hpp"
#include "udr_config.hpp"

#include <boost/algorithm/string.hpp>

using namespace oai::udr::app;
using namespace oai::udr::model;
using namespace oai::udr::config;
using namespace oai::model::common;

extern udr_config udr_cfg;

mongo_db::mongo_db(udr_event& ev)
    : database_wrapper<mongo_db>(), m_event_sub(ev), m_db_connection_status() {
  is_db_connection_active = false;
  start_event_connection_handling();
}

//------------------------------------------------------------------------------
mongo_db::~mongo_db() {
  if (db_connection_event.connected()) db_connection_event.disconnect();
  close_connection();
}

//------------------------------------------------------------------------------
bool mongo_db::initialize() {
  return true;
}

//------------------------------------------------------------------------------
bool mongo_db::connect(uint32_t num_retries) {
  Logger::udr_db().debug("Connecting to MongoDB");

  int i = 0;
  while (i < num_retries) {
    try {
      // Try to connect to MongoDB
      mongo_client = mongocxx::client{mongocxx::uri{
          "mongodb://" + udr_cfg.db_conf.user + ":" + udr_cfg.db_conf.pass +
          "@" + udr_cfg.db_conf.server + ":" +
          std::to_string(udr_cfg.db_conf.port)}};

      // Check if connection to MongoDB works
      bsoncxx::builder::stream::document ping;
      ping << "ping" << 1;
      auto db = mongo_client[udr_cfg.db_conf.db_name.c_str()];
      db.run_command(ping.view());

      Logger::udr_db().info("Connected to MongoDB");
      set_db_connection_status(true);
      Logger::udr_db().info("Mongo client created successfully");

      return true;
    } catch (const mongocxx::exception& ex) {
      std::cout << "Mongo client URI: " << mongo_client.uri().to_string()
                << std::endl;
      Logger::udr_db().error(
          "An error occurred when connecting to MongoDB (%s), retry ...",
          ex.what());
      i++;
      set_db_connection_status(false);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  Logger::udr_db().error(
      "Failed to connect to MongoDB after %d retries", num_retries);
  return false;
}

//------------------------------------------------------------------------------
bool mongo_db::close_connection() {
  Logger::udr_db().debug("Close the connection with MongoDB");
  // No need to explicitly close the MongoDB connection
  set_db_connection_status(false);
  return true;
}

//------------------------------------------------------------------------------
void mongo_db::set_db_connection_status(bool status) {
  std::unique_lock lock(m_db_connection_status);
  is_db_connection_active = status;
}

//------------------------------------------------------------------------------
bool mongo_db::get_db_connection_status() const {
  std::shared_lock lock(m_db_connection_status);
  return is_db_connection_active;
}

//---------------------------------------------------------------------------------------------
void mongo_db::start_event_connection_handling() {
  // create a time point representing the current time
  auto now = std::chrono::system_clock::now();

  // convert the time point to milliseconds
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
      std::bind(
          &mongo_db::trigger_connection_handling_procedure, this,
          std::placeholders::_1),
      interval, ms + interval);
}

//---------------------------------------------------------------------------------------------
void mongo_db::trigger_connection_handling_procedure(uint64_t ms) {
  _unused(ms);
  std::time_t current_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  Logger::udr_db().debug(
      "DB Connection handling, current time: %s", std::ctime(&current_time));

  try {
    if (mongo_client) {
      bsoncxx::builder::stream::document ping;
      ping << "ping" << 1;
      auto db = mongo_client[udr_cfg.db_conf.db_name.c_str()];
      db.run_command(ping.view());
      return;
    }
  } catch (const std::exception& e) {
    set_db_connection_status(false);
    Logger::udr_db().warn(
        "Could not establish the connection to the DB, reason: %s", e.what());
  }

  // If the connection couldn't be established
  // reset the connection and try again
  close_connection();
  initialize();
  if (!connect(MAX_CONNECTION_RETRY))
    Logger::udr_db().warn("Could not establish the connection to the DB");
}

//------------------------------------------------------------------------------
bool mongo_db::insert_authentication_subscription(
    const std::string& id,
    const oai::udr::model::AuthenticationSubscription& auth_subscription,
    nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Select the appropriate database and collection
  mongocxx::database db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  mongocxx::collection coll = db["AuthenticationSubscription"];
  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << id;

  try {
    auto cursor = coll.find_one(filter_builder.view());

    if (cursor) {
      Logger::udr_db().error(
          "[UE Id %s] AuthenticationSubscription for UE Id already exists.",
          id);
      problemDetails.setCause("SYSTEM_FAILURE");
      to_json(json_data, problemDetails);
      return false;
    }

    to_json(json_data, auth_subscription);
    json_data["ueid"] = id;
    coll.insert_one(bsoncxx::from_json(json_data.dump()));
    Logger::udr_db().debug(
        "[UE Id %s] AuthenticationSubscription inserted in MongoDB: %s", id,
        json_data.dump());
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while insert AuthenticationSubscription in "
        "MongoDB: %s",
        id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

bool mongo_db::delete_authentication_subscription(
    const std::string& id, nlohmann::json& json_data) {
  Logger::udr_db().info("[UE Id %s] Delete AuthenticationSubscription", id);
  ProblemDetails problemDetails = {};
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Select the appropriate database and collection
  mongocxx::database db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  mongocxx::collection coll = db["AuthenticationSubscription"];
  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << id;

  try {
    // Perform the delete operation
    auto result = coll.delete_one(filter_builder.view());

    if (!result) {
      Logger::udr_db().error(
          "[UE Id %s] Failed to delete document from MongoDB", id);
      return false;
    }
    if (result->deleted_count() == 0) {
      Logger::udr_db().error(
          "[UE Id %s] Failed to delete document from MongoDB: Document not "
          "found",
          id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
    Logger::udr_db().debug("[UE Id %s] Deleted document from MongoDB", id);
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while delete authentication subscription from "
        "MongoDB: %s",
        id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

bool mongo_db::query_authentication_subscription(
    const std::string& id, nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Query AuthenticationSubscription", id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Get the database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["AuthenticationSubscription"];
  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << id;

  try {
    // Execute the query and get the result
    auto result = coll.find_one(filter_builder.view());

    if (result) {
      AuthenticationSubscription authentication_subscription = {};
      from_json(
          nlohmann::json::parse(bsoncxx::to_json(result->view())),
          authentication_subscription);
      to_json(json_data, authentication_subscription);
      Logger::udr_db().debug(
          "[UE Id %s] AuthenticationSubscription: %s", id, json_data.dump());
      return true;
    } else {
      Logger::udr_db().error(
          "[UE Id %s] AuthenticationSubscription not Found！", id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query AuthenticationSubscription from "
        "MongoDB: %s",
        id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::update_authentication_subscription(
    const std::string& ue_id,
    const std::vector<oai::model::common::PatchItem>& patchItem,
    nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Update AuthenticationSubscription", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Get the database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["AuthenticationSubscription"];
  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << ue_id;

  try {
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        coll.find_one(filter_builder.view());
    if (result) {
      bsoncxx::document::view view = result->view();
      for (const auto& item : patchItem) {
        if (item.getOp().getEnumValue() ==
                PatchOperation_anyOf::ePatchOperation_anyOf::REPLACE &&
            item.valueIsSet()) {
          auto sequenceNumberValue =
              bsoncxx::from_json(item.getValue().c_str());
          auto sequenceNumberView = sequenceNumberValue.view();

          bsoncxx::builder::stream::document updateBuilder{};
          updateBuilder << "$set" << bsoncxx::builder::stream::open_document
                        << "sequenceNumber" << sequenceNumberView
                        << bsoncxx::builder::stream::close_document;

          auto updateResult =
              coll.update_one(filter_builder.view(), updateBuilder.view());

          if (!updateResult) {
            Logger::udr_db().error(
                "[UE Id %s] Failed to update AuthenticationSubscription",
                ue_id);
            problemDetails.setCause("SYSTEM_FAILURE");
            to_json(json_data, problemDetails);
            return false;
          }
        }
        nlohmann::json tmp_j;
        to_json(tmp_j, item);
        json_data += tmp_j;
      }
      Logger::udr_db().debug(
          "[UE Id %s] Update AuthenticationSubscription: %s", ue_id,
          json_data.dump());
      return true;
    } else {
      Logger::udr_db().error(
          "[UE Id %s] AuthenticationSubscription not found!", ue_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while update AuthenticationSubscription in "
        "MongoDB: %s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_am_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info(
      "[UE Id %s] Query AccessAndMobilitySubscriptionData", ue_id);
  // Establish MongoDB connection
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Get the database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["AccessAndMobilitySubscriptionData"];
  // Construct the MongoDB query
  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << ue_id << "servingPlmnid" << serving_plmn_id;

  try {
    auto result = coll.find_one(filter_builder.view());
    if (result) {
      oai::udr::model::AccessAndMobilitySubscriptionData subscription_data = {};
      const bsoncxx::document::view row = result.value().view();
      from_json(
          nlohmann::json::parse(bsoncxx::to_json(row)), subscription_data);
      to_json(json_data, subscription_data);
      Logger::udr_db().debug(
          "[UE Id %s] AccessAndMobilitySubscriptionData: %s", ue_id,
          json_data.dump());
    } else {
      // Handle query failure
      Logger::udr_db().error(
          "[UE Id %s] Failed to query AccessAndMobilitySubscriptionData from "
          "MongoDB",
          ue_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query AccessAndMobilitySubscriptionData "
        "from MongoDB: %s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
bool mongo_db::create_amf_context_3gpp(
    const std::string& ue_id,
    Amf3GppAccessRegistration& amf3GppAccessRegistration,
    nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Put Amf3GppAccessRegistration", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Select the appropriate database and collection
  mongocxx::database db = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto collection       = db["Amf3GppAccessRegistration"];
  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;

  try {
    auto document = collection.find_one(filter_builder.view());

    nlohmann::json json_data;
    to_json(json_data, amf3GppAccessRegistration);
    if (document) {
      bsoncxx::document::value update_doc =
          bsoncxx::from_json(json_data.dump());
      auto result = collection.update_one(
          filter_builder.view(),
          bsoncxx::builder::basic::make_document(
              bsoncxx::builder::basic::kvp("$set", update_doc.view())));
      if (!result) {
        Logger::udr_db().error(
            "[UE Id %s] Failed to update Amf3GppAccessRegistration in MongoDB",
            ue_id, bsoncxx::to_json(update_doc.view()).c_str());
        problemDetails.setCause("SYSTEM_FAILURE");
        to_json(json_data, problemDetails);
        return false;
      }
      Logger::udr_db().debug(
          "[UE Id %s] Amf3GppAccessRegistration is updated in MongoDB: %s",
          ue_id, bsoncxx::to_json(update_doc.view()));
      return true;
    } else {
      json_data["ueid"] = ue_id;
      bsoncxx::document::value insert_doc =
          bsoncxx::from_json(json_data.dump());
      collection.insert_one(insert_doc.view());
      Logger::udr_db().debug(
          "[UE Id %s] Amf3GppAccessRegistration is inserted in MongoDB: %s",
          ue_id, bsoncxx::to_json(insert_doc.view()));
      return true;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while create AMF context in MongoDB: %s", ue_id,
        e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_amf_context_3gpp(
    const std::string& ue_id, nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Get Amf3GppAccessRegistration", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  auto db         = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto collection = db["Amf3GppAccessRegistration"];
  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;

  try {
    auto cursor = collection.find_one(filter_builder.view());

    if (cursor) {
      oai::udr::model::Amf3GppAccessRegistration amf3gppaccessregistration = {};
      const bsoncxx::document::view row = cursor.value().view();
      from_json(
          nlohmann::json::parse(bsoncxx::to_json(row)),
          amf3gppaccessregistration);
      to_json(json_data, amf3gppaccessregistration);
      Logger::udr_db().debug(
          "[UE Id %s] Amf3GppAccessRegistration: %s", ue_id, json_data.dump());
      return true;
    } else {
      Logger::udr_db().info(
          "[UE Id %s] AMF 3GPP Access Registration for not found in MongoDB",
          ue_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query AMF context from MongoDB: %s", ue_id,
        e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::insert_authentication_status(
    const std::string& ue_id, const oai::udr::model::AuthEvent& authEvent,
    nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Put AuthenticationStatus", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Get the database and collection
  auto db         = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto collection = db["AuthenticationStatus"];
  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;

  try {
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        collection.find_one(filter_builder.view());
    nlohmann::json tmp_json;
    to_json(tmp_json, authEvent);
    tmp_json["ueid"] = ue_id;
    auto document    = bsoncxx::from_json(tmp_json.dump());

    if (result) {
      auto updateResult = collection.update_one(
          filter_builder.view(),
          bsoncxx::builder::basic::make_document(
              bsoncxx::builder::basic::kvp("$set", document.view())));
      if (updateResult && updateResult->matched_count() == 1) {
        json_data = tmp_json;
        Logger::udr_db().debug(
            "[UE Id %s] Successfully updated AuthenticationStatus: %s", ue_id,
            json_data.dump());
      } else {
        Logger::udr_db().error(
            "[UE Id %s] Failed to update AuthenticationStatus", ue_id);
        problemDetails.setCause("DATA_NOT_FOUND");
        to_json(json_data, problemDetails);
        return false;
      }
    } else {
      auto insertResult = collection.insert_one(document.view());
      if (!insertResult) {
        Logger::udr_db().error(
            "[UE Id %s] Failed to insert AuthenticationStatus", ue_id);
        problemDetails.setCause("DATA_NOT_FOUND");
        to_json(json_data, problemDetails);
        return false;
      }
      json_data = tmp_json;
      Logger::udr_db().debug(
          "[UE Id %s] Successfully inserted AuthenticationStatus: %s", ue_id,
          json_data.dump());
    }
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while put AuthenticationStatus in MongoDB: %s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::delete_authentication_status(
    const std::string& ue_id, nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Delete AuthenticationStatus", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  mongocxx::database db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  mongocxx::collection coll = db["AuthenticationStatus"];
  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;

  try {
    auto result = coll.delete_one(
        filter_builder.view());  // Delete the document matching the filter
    if (result && result->deleted_count() > 0) {
      Logger::udr_db().debug(
          "[UE Id %s] Successfully deleted AuthenticationStatus", ue_id);
      return true;
    } else {
      Logger::udr_db().info(
          "[UE Id %s] No AuthenticationStatus found", ue_id.c_str());
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while delete AuthenticationStatus from MongoDB: "
        "%s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_authentication_status(
    const std::string& ue_id, nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Get AuthenticationStatus", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Get the database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["AuthenticationStatus"];
  // Build the query
  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;

  try {
    // Execute the query and get the result
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        coll.find_one(filter_builder.view());

    // Check if the result is not empty
    if (result) {
      // Convert the result object to a JSON string
      std::string result_str = bsoncxx::to_json(result->view());
      Logger::udr_db().debug(
          "[UE Id %s] Get AuthenticationStatus: %s", result_str);

      AuthEvent authentication_status = {};
      from_json(nlohmann::json::parse(result_str), authentication_status);
      to_json(json_data, authentication_status);
      Logger::udr_db().debug(
          "[UE Id %s] AuthenticationStatus: %s", ue_id, json_data.dump());
      return true;
    } else {
      Logger::udr_db().error(
          "[UE Id %s] AuthenticationStatus not found", ue_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query AuthenticationStatus from MongoDB: "
        "%s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  Logger::udr_db().info(
      "[UE Id %s] Get SdmSubscription with subscription ID %s", ue_id, subs_id);
  ProblemDetails problemDetails = {};
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Select the appropriate database and collection
  mongocxx::database db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  mongocxx::collection coll = db["SdmSubscriptions"];
  bsoncxx::builder::stream::document filter;
  try {
    filter << "ueid" << ue_id << "subsId" << std::stoi(subs_id);
  } catch (std::exception& err) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query authentication data from MongoDB: %s",
        ue_id, err.what());
    problemDetails.setCause("INVALID_QUERY_PARAM");
    problemDetails.setInvalidParams(std::vector<InvalidParam>{{"subsId"}});
    to_json(json_data, problemDetails);
    return false;
  }
  try {
    auto result = coll.find_one(filter.view());
    if (result) {
      auto str_result = bsoncxx::to_json(result->view());
      oai::udr::model::SdmSubscription sdmSubscriptions = {};

      from_json(nlohmann::json::parse(str_result), sdmSubscriptions);
      to_json(json_data, sdmSubscriptions);
      Logger::udr_db().debug(
          "[UE Id %s] SdmSubscription with id %s: %s", ue_id, subs_id,
          json_data.dump());
      return true;
    } else {
      Logger::udr_db().info(
          "[UE Id %s] SdmSubscription with subscription ID %s not found", ue_id,
          subs_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query SdmSubscription with subscription ID "
        "%s: %s",
        ue_id, subs_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::delete_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    nlohmann::json& json_data) {
  Logger::udr_db().info("[UE Id %s] Delete SdmSubscription", ue_id);
  ProblemDetails problemDetails = {};
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Select the appropriate database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SdmSubscriptions"];
  bsoncxx::builder::stream::document filter;
  try {
    filter << "ueid" << ue_id << "subsId" << std::stoi(subs_id);
  } catch (std::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while delete SdmSubscription data from MongoDB: "
        "%s",
        ue_id, e.what());
    problemDetails.setCause("INVALID_QUERY_PARAM");
    problemDetails.setInvalidParams(std::vector<InvalidParam>{{"subsId"}});
    to_json(json_data, problemDetails);
    return false;
  }
  try {
    auto result = coll.delete_one(filter.view());
    if (!result || result->deleted_count() == 0) {
      Logger::udr_db().error(
          "[UE Id %s] Failed to delete SdmSubscription with subscription ID %s "
          "from MongoDB",
          ue_id, subs_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while delete sdm subscription from MongoDB: %s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  Logger::udr_db().debug(
      "[UE Id %s] Successfully deleted SdmSubscription with subscription ID %s",
      ue_id, subs_id);
  return true;
}

//------------------------------------------------------------------------------
bool mongo_db::update_sdm_subscription(
    const std::string& ue_id, const std::string& subs_id,
    oai::udr::model::SdmSubscription& sdmSubscription,
    nlohmann::json& json_data) {
  Logger::udr_db().info(
      "[UE Id %s] Update SdmSubscription with subscription ID %s", ue_id,
      subs_id);
  ProblemDetails problemDetails = {};
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Select the appropriate database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SdmSubscriptions"];

  // Prepare filter for update
  bsoncxx::builder::stream::document filter;
  try {
    filter << "ueid" << ue_id << "subsId" << std::stoi(subs_id);
  } catch (std::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while update SdmSubscription data in MongoDB: %s",
        ue_id, e.what());
    problemDetails.setCause("INVALID_QUERY_PARAM");
    problemDetails.setInvalidParams(std::vector<InvalidParam>{{"subsId"}});
    to_json(json_data, problemDetails);
    return false;
  }
  nlohmann::json sdmSubscriptionJson;
  to_json(sdmSubscriptionJson, sdmSubscription);
  bsoncxx::document::value update_doc =
      bsoncxx::from_json(sdmSubscriptionJson.dump());

  try {
    // Execute update query
    auto result = coll.update_one(
        filter.view(),
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("$set", update_doc.view())));

    // Check for update success
    if (result) {
      if (result->matched_count() == 1) {
        json_data = sdmSubscriptionJson;
        Logger::udr_db().debug(
            "[UE Id %s] Successfully updated SdmSubscription with subscription "
            "ID %s in MongoDB: %s",
            ue_id, subs_id, json_data.dump());
        return true;
      } else {
        problemDetails.setCause("DATA_NOT_FOUND");
        to_json(json_data, problemDetails);
        Logger::udr_db().error(
            "[UE Id %s] Failed to update SdmSubscription with subscription ID "
            "%s",
            ue_id, subs_id);
        return false;
      }
    } else {
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      Logger::udr_db().error(
          "[UE Id %s] Failed to update SdmSubscription with subscription ID %s",
          ue_id, subs_id);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    Logger::udr_db().error(
        "[UE Id %s] Exception while update SdmSubscription in MongoDB: %s",
        ue_id, e.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::create_sdm_subscriptions(
    const std::string& ue_id, oai::udr::model::SdmSubscription& sdmSubscription,
    nlohmann::json& json_data) {
  Logger::udr_db().info("[UE Id %s] Create SdmSubscription", ue_id);
  ProblemDetails problemDetails = {};
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info(
        "The connection to the MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  // Select the appropriate database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SdmSubscriptions"];

  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;

  nlohmann::json tmp_json;
  to_json(tmp_json, sdmSubscription);
  tmp_json["ueid"] = ue_id;

  try {
    auto cursor = coll.find(filter_builder.view());
    std::list<int32_t> subsIds;
    for (auto doc : cursor) {
      bsoncxx::document::element subs_id_elem = doc["subsId"];
      if (subs_id_elem) {
        int32_t subs_id = subs_id_elem.get_int32().value;
        subsIds.push_back(subs_id);
      }
    }
    subsIds.sort();
    int count = 0;
    for (auto el : subsIds) {
      if (el != count) {
        break;
      }
      count++;
    }
    tmp_json["subsId"] = count;
    auto document      = bsoncxx::from_json(tmp_json.dump());

    auto result = coll.insert_one(document.view());
    if (result) {
      json_data = tmp_json;
      Logger::udr_db().debug(
          "[UE Id %s] Successfully create SdmSubscriptions: %s", ue_id,
          json_data.dump());
      return true;
    } else {
      Logger::udr_db().error(
          "[UE Id %s] Failed to insert SdmSubscriptions into MongoDB", ue_id);
      problemDetails.setCause("SYSTEM_FAILURE");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while create SdmSubscriptions in MongoDB: %s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_sdm_subscriptions(
    const std::string& ue_id, nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Get SdmSubscriptions", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  bsoncxx::builder::stream::document filter_builder;
  filter_builder << "ueid" << ue_id;

  // Select the appropriate database and collection
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SdmSubscriptions"];

  try {
    mongocxx::cursor result = coll.find(filter_builder.view());

    nlohmann::json j   = {};
    nlohmann::json tmp = {};
    for (const bsoncxx::document::view& doc : result) {
      SdmSubscription sdmsubscriptions = {};
      tmp.clear();
      from_json(nlohmann::json::parse(bsoncxx::to_json(doc)), sdmsubscriptions);
      to_json(tmp, sdmsubscriptions);
      j.push_back(tmp);
    }
    if (j.empty()) {
      problemDetails.setCause("USER_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
    json_data = j;
    Logger::udr_db().debug(
        "[UE Id %s] SdmSubscriptions: %s", ue_id, json_data.dump());
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query sdm subscription from MongoDB: %s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_sm_data(nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("Get SessionManagementSubscriptionData");
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SessionManagementSubscriptionData"];

  try {
    auto result = coll.find({});

    auto row = result.begin();
    if (row == result.end()) {
      Logger::udr_db().error(
          "Empty document in MongoDB Collection "
          "SessionManagementSubscriptionData");
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }

    for (auto&& view : result) {
      nlohmann::json j = query_sm_data_helper(view);
      json_data += j;
      Logger::udr_db().debug(
          "SessionManagementSubscriptionData: %s", j.dump().c_str());
    }
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "Exception while query SessionManagementSubscriptionData from MongoDB: "
        "%s",
        e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_sm_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data,
    const std::optional<oai::model::common::Snssai>& snssai,
    const std::optional<std::string>& dnn) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info(
      "[UE Id %s] Get SessionManagementSubscriptionData", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }

  auto db     = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll   = db["SessionManagementSubscriptionData"];
  auto filter = bsoncxx::builder::stream::document{};
  filter << "ueid" << ue_id << "servingPlmnid" << serving_plmn_id;
  if (snssai.value().getSst() > 0) {
    filter << "singleNssai.sst" << snssai.value().getSst();
  }
  if (dnn.has_value()) {
    filter << "dnnConfigurations." + dnn.value()
           << bsoncxx::builder::stream::open_document << "$exists" << true
           << bsoncxx::builder::stream::close_document;
  }
  try {
    auto result = coll.find(filter.view());

    auto row = result.begin();
    if (row == result.end()) {
      Logger::udr_db().error(
          "[UE Id %s] Empty document for servingPlmnid=%s", ue_id,
          serving_plmn_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }

    for (auto&& view : result) {
      nlohmann::json j = query_sm_data_helper(view);
      json_data += j;
      Logger::udr_db().debug(
          "[UE Id %s] SessionManagementSubscriptionData: %s", ue_id, j.dump());
    }
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while SessionManagementSubscriptionData from "
        "MongoDB: %s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
nlohmann::json mongo_db::query_sm_data_helper(
    const bsoncxx::v_noabi::document::view& view) {
  // check if at least one document can be found
  SessionManagementSubscriptionData sessionmanagementsubscriptiondata = {};
  from_json(
      nlohmann::json::parse(bsoncxx::to_json(view)),
      sessionmanagementsubscriptiondata);

  nlohmann::json j;
  to_json(j, sessionmanagementsubscriptiondata);

  return j;
}

//------------------------------------------------------------------------------
bool mongo_db::insert_smf_context_non_3gpp(
    const std::string& ue_id, const int32_t& pdu_session_id,
    const oai::udr::model::SmfRegistration& smfRegistration,
    nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Insert SmfRegistrations", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfRegistrations"];

  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << ue_id << "subpduSessionId" << pdu_session_id;
  auto filter = filter_builder.view();

  try {
    auto result = coll.find_one(filter);

    nlohmann::json tmp_json;
    to_json(tmp_json, smfRegistration);
    tmp_json["ueid"]                    = ue_id;
    tmp_json["pdu_session_id"]          = pdu_session_id;
    bsoncxx::document::value update_doc = bsoncxx::from_json(tmp_json.dump());

    if (result) {
      auto update_result = coll.update_one(
          filter, bsoncxx::builder::basic::make_document(
                      bsoncxx::builder::basic::kvp("$set", update_doc.view())));
      if (!update_result) {
        Logger::udr_db().error(
            "[UE Id %s] Failed to update SmfRegistration", ue_id);
        problemDetails.setCause("SYSTEM_FAILURE");
        to_json(json_data, problemDetails);
        return false;
      }
    } else {
      auto insert_result = coll.insert_one(update_doc.view());
      if (!insert_result) {
        Logger::udr_db().error(
            "[UE Id %s] Failed to insert SmfRegistration", ue_id);
        problemDetails.setCause("SYSTEM_FAILURE");
        to_json(json_data, problemDetails);
        return false;
      }
    }

    json_data = tmp_json;
    Logger::udr_db().debug(
        "[UE Id %s] Successfully put SmfRegistration: %s", ue_id,
        json_data.dump());
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while insert smf context in MongoDB: %s", ue_id,
        e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::delete_smf_context(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info(
      "[UE Id %s] Delete SmfRegistrations for PduSessionId: %i", ue_id,
      pdu_session_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfRegistrations"];

  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << ue_id << "subpduSessionId" << pdu_session_id;

  try {
    auto result = coll.delete_one(filter_builder.view());
    if (result && result->deleted_count() == 0) {
      Logger::udr_db().warn(
          "[UE Id %s] SmfRegistration for PduSessionId %i not found", ue_id,
          pdu_session_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
    Logger::udr_db().debug(
        "[UE Id %s] Successfully delete SmfRegistration", ue_id);
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while delete smf context from MongoDB: %s", ue_id,
        e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_smf_registration(
    const std::string& ue_id, const int32_t& pdu_session_id,
    nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info(
      "[UE Id %s] Get SmfRegistrations for PduSessionId: %i", ue_id,
      pdu_session_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfRegistrations"];
  bsoncxx::builder::stream::document filter_builder{};
  filter_builder << "ueid" << ue_id << "subpduSessionId" << pdu_session_id;

  try {
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        coll.find_one(filter_builder.view());

    if (result) {
      SmfRegistration smfregistration = {};
      from_json(
          nlohmann::json::parse(bsoncxx::to_json(result->view())),
          smfregistration);
      to_json(json_data, smfregistration);

      Logger::udr_db().debug(
          "[UE Id %s] SmfRegistration for PduSessionId %i: %s", ue_id,
          pdu_session_id, json_data.dump());
      return true;
    } else {
      Logger::udr_db().error(
          "[UE Id %s] SmfRegistration not found for PduSessionId %i", ue_id,
          pdu_session_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query smf context from MongoDB: %s", ue_id,
        e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_smf_reg_list(
    const std::string& ue_id, nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info("[UE Id %s] Get SmfRegistrations", ue_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }

  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfRegistrations"];

  bsoncxx::builder::stream::document filter{};
  filter << "ueid" << ue_id;

  try {
    mongocxx::cursor result = coll.find(filter.view());

    nlohmann::json j = {};
    for (const bsoncxx::document::view& doc : result) {
      SmfRegistration smfregistration = {};
      nlohmann::json tmp              = {};
      from_json(nlohmann::json::parse(bsoncxx::to_json(doc)), smfregistration);
      to_json(tmp, smfregistration);
      j += tmp;
    }
    if (j.empty()) {
      Logger::udr_db().debug("[UE Id %s] No SmfRegistration found", ue_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }
    json_data = j;
    Logger::udr_db().debug("[UE Id %s] SmfRegistrations: %s", ue_id, j.dump());
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query smf registration list from MongoDB: "
        "%s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}

//------------------------------------------------------------------------------
bool mongo_db::query_smf_select_data(
    const std::string& ue_id, const std::string& serving_plmn_id,
    nlohmann::json& json_data) {
  ProblemDetails problemDetails = {};
  Logger::udr_db().info(
      "[UE Id %s] Get SmfSelectionSubscriptionData for ServingPlmnId %s", ue_id,
      serving_plmn_id);
  // Check the connection with DB first
  if (!get_db_connection_status()) {
    Logger::udr_db().info("The connection to MongoDB is currently inactive");
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
  auto db   = mongo_client[udr_cfg.db_conf.db_name.c_str()];
  auto coll = db["SmfSelectionSubscriptionData"];

  bsoncxx::builder::stream::document filter{};
  filter << "ueid" << ue_id;

  try {
    auto query_result = coll.find_one(filter.view());

    if (!query_result) {
      Logger::udr_db().error(
          "[UE Id %s] SmfSelectionSubscriptionData for ServingPlmnId %s not "
          "found",
          ue_id, serving_plmn_id);
      problemDetails.setCause("DATA_NOT_FOUND");
      to_json(json_data, problemDetails);
      return false;
    }

    auto smfselectionsubscriptiondata = SmfSelectionSubscriptionData();
    from_json(
        nlohmann::json::parse(bsoncxx::to_json(query_result.value().view())),
        smfselectionsubscriptiondata);
    to_json(json_data, smfselectionsubscriptiondata);

    Logger::udr_db().debug(
        "[UE Id %s] SmfSelectionSubscriptionData for ServingPlmnId %s: %s",
        ue_id, serving_plmn_id, json_data.dump());
    return true;
  } catch (const mongocxx::exception& e) {
    Logger::udr_db().error(
        "[UE Id %s] Exception while query smf selection subscription data from "
        "MongoDB: %s",
        ue_id, e.what());
    problemDetails.setCause("SYSTEM_FAILURE");
    to_json(json_data, problemDetails);
    return false;
  }
}