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

/*! \file smf_n7.hpp
 \author  Stefan Spettel
 \company Openairinterface Software Alliance
 \date 2022
 \email: stefan.spettel@eurecom.fr
 */

#ifndef FILE_SMF_N7_HPP_SEEN
#define FILE_SMF_N7_HPP_SEEN

#include <string>
#include <memory>
#include <unordered_map>
#include <shared_mutex>

#include "Snssai.h"
#include "PlmnId.h"
#include "SmPolicyDecision.h"
#include "SmPolicyContextData.h"
#include "SmPolicyUpdateContextData.h"
#include "SmPolicyDeleteData.h"
#include "smf.h"
#include "3gpp_29.500.h"

namespace smf::n7 {

/**
 * @brief Status codes for the communication with the PCF
 *
 */
enum class sm_policy_status_code {
  CREATED,
  USER_UNKOWN,
  INVALID_PARAMETERS,
  CONTEXT_DENIED,
  NOT_FOUND,
  OK,
  PCF_NOT_AVAILABLE,
  INTERNAL_ERROR
};

/**
 * @brief Used to store all policy-related fields in one struct.
 *
 */
struct policy_association {
  oai::model::pcf::SmPolicyDecision decision;
  oai::model::pcf::SmPolicyContextData context;
  uint64_t id     = 0;
  uint64_t pcf_id = 0;
  std::string pcf_location;

  void set_context(
      const std::string& supi, const std::string& supi_prefix,
      const std::string& dnn, const snssai_t& snssai, const plmn_t& plmn,
      const uint8_t pdu_session_id,
      const pdu_session_type_t& pdu_session_type) {
    oai::model::common::Snssai snssai_model;
    snssai_model.setSst(snssai.sst);
    snssai_model.setSd(std::to_string(snssai.sd));
    oai::model::common::PlmnIdNid plmn_id_model;
    std::string mnc_string = std::to_string(plmn.mnc_digit1) +
                             std::to_string(plmn.mnc_digit2) +
                             std::to_string(plmn.mnc_digit3);
    std::string mcc_string = std::to_string(plmn.mcc_digit1) +
                             std::to_string(plmn.mcc_digit2) +
                             std::to_string(plmn.mcc_digit3);
    plmn_id_model.setMnc(mnc_string);
    plmn_id_model.setMcc(mcc_string);
    context = {};

    context.setPduSessionId(pdu_session_id);
    context.setSupi(smf_get_supi_with_prefix(supi_prefix, supi));
    oai::model::common::PduSessionType pdu_session_type_model;
    // hacky
    from_json(pdu_session_type.to_string(), pdu_session_type_model);
    context.setPduSessionType(pdu_session_type_model);
    context.setDnn(dnn);
    context.setSliceInfo(snssai_model);
    context.setServingNetwork(plmn_id_model);
  }

  std::string toString() const {
    std::string s;
    if (decision.pccRulesIsSet()) {
      s.append("\t\tPCC Rules:\n");
      for (const auto& it : decision.getPccRules()) {
        s.append("\t\t\t\t").append(it.second.getPccRuleId()).append("\n");
      }
    }
    if (decision.traffContDecsIsSet()) {
      s.append("\t\tTraffic Control Descriptions:\n");
      for (const auto& it : decision.getTraffContDecs()) {
        s.append("\t\t\t\t").append(it.second.getTcId()).append("\n");
      }
    }
    return s;
  }
};

/**
 * @brief Abstract class to receive policies based on the source (either PCF or
 * local files)
 *
 */
class policy_storage {
 public:
  /**
   * @brief Creates a policy association based on the given context and provides
   * a policy decision.
   *
   * Precondition: context data needs to be set
   * Postcondition: In case of success, return CREATED,
   * association.decision and policy_association.id is set
   *
   * @param association when the ID is already set, this ID is used and the
   * client ensures uniqueness
   * @return sm_policy_status_code CREATED in case of success, otherwise
   * USER_UNKNOWN, INVALID_PARAMETERS, CONTEXT_DENIED, INTERNAL_ERROR
   */
  virtual sm_policy_status_code create_policy_association(
      policy_association& association) = 0;

  /**
   * @brief Removes a policy association, identified by the ID,
   *
   * @param policy_association input: must contain PCF location, object is not
   * removed
   * @param delete_data input: must not be null, but values are optional
   * @return sm_policy_status_code OK in case of success, otherwise NOT_FOUND,
   * INTERNAL_ERROR, PCF_NOT_AVAILABLE
   */
  virtual sm_policy_status_code remove_policy_association(
      const policy_association& association,
      const oai::model::pcf::SmPolicyDeleteData& delete_data) = 0;

  /**
   * @brief Updates a policy association, identified by the ID
   *
   * @param update_data input: must not be null and set accordingly to the
   * triggers
   * @param policy_association Updated policy association with new
   * context and decision in case of OK status code, must contain PCF location
   * @return sm_policy_status_code OK in case of success, otherwise NOT_FOUND,
   * INTERNAL_ERROR, PCF_NOT_AVAILABLE
   */
  virtual sm_policy_status_code update_policy_association(
      const oai::model::pcf::SmPolicyUpdateContextData& update_data,
      policy_association& association) = 0;
  /**
   * @brief Get the the policy association together with the original context
   *
   * @param association contains the original context and the policy
   * decision, must contain policy ID and PCF location
   * @return sm_policy_status_code OK in case of success, otherwise NOT_FOUND,
   * ITERNAL_ERROR, PCF_NOT_AVAILABLE
   */
  virtual sm_policy_status_code get_policy_association(
      policy_association& association) = 0;
};

/**
 * @brief Implements Npcf_SMPolicyControlAPI to interact with a PCF to recieve
 * policies
 */
class smf_pcf_client : public policy_storage {
 public:
  const std::string sm_api_name                 = "npcf-smpolicycontrol";
  const std::string sm_api_policy_resource_part = "sm-policies";
  const std::string delete_suffix               = "delete";
  const std::string update_suffix               = "update";

  explicit smf_pcf_client(
      const std::string& pcf_addr, const std::string& pcf_api_version) {
    root_uri = pcf_addr + "/" + sm_api_name + "/" + pcf_api_version + "/" +
               sm_api_policy_resource_part;
  }

  virtual ~smf_pcf_client();

  /**
   * @brief Discover PCF either based on NRF or local configuration (based on
   * the config file)
   *
   * @param snssai
   * @param plmn_id
   * @param dnn
   * @return & smf_pcf_client nullptr in case of an error
   */
  static std::shared_ptr<smf_pcf_client> discover_pcf(
      const oai::model::common::Snssai& snssai,
      const oai::model::common::PlmnId& plmn_id, const std::string& dnn);

  sm_policy_status_code create_policy_association(
      policy_association& association) override;

  sm_policy_status_code remove_policy_association(
      const policy_association& association,
      const oai::model::pcf::SmPolicyDeleteData& delete_data) override;

  sm_policy_status_code update_policy_association(
      const oai::model::pcf::SmPolicyUpdateContextData& update_data,
      policy_association& association) override;

  sm_policy_status_code get_policy_association(
      policy_association& association) override;

 private:
  static bool discover_pcf_with_nrf(
      std::string& addr, std::string& api_version,
      const oai::model::common::Snssai& snssai,
      const oai::model::common::PlmnId& plmn_id, const std::string& dnn);

  static bool discover_pcf_from_config_file(
      std::string& addr, std::string& api_version,
      const oai::model::common::Snssai& snssai,
      const oai::model::common::PlmnId& plmn_id, const std::string& dnn);

  http_status_code_e send_request(
      const std::string& uri, const std::string& body,
      const std::string& method, std::string& response_body,
      std::string& response_headers, bool use_response_headers = false);

  std::string root_uri;
};

// TODO implement for file based policy rules
// class smf_file_pcc_rules : public policy_storage {};

/**
 * @brief Implements the N7 procedures (communication between SMF and PCF). It
 * is the interface for PCF communication that should be used by other
 * components. Depending on the configuration, the policy rules may come from a
 * PCF or from local files (currently not supported)
 *
 */
class smf_n7 {
 public:
  smf_n7() : policy_storages(){};
  smf_n7(smf_n7 const&) = delete;
  void operator=(smf_n7 const&) = delete;
  virtual ~smf_n7();

  static smf_n7& get_instance() {
    static smf_n7 instance;
    return instance;
  }

  /**
   * @brief Creates a SM Policy Association (as defined in 3GPP TS 29.512).
   * The PCF is selected based on the configuration file (NRF or pre-configured)
   * The PCC rules may also be selected from local configuration (depending on
   * option in config file)
   *  @param association: context needs to be set, if id is set, client ensures
   * that the value is unique, if not, other associations may be overwritten
   *
   *
   * @param context input: context data, the mandatory parameters need to be set
   * @param policy_decision output: policy decision
   * @param policy_id output: ID of the policy association
   * @return sm_policy_status_code
   */
  sm_policy_status_code create_sm_policy_association(
      policy_association& association);

  /**
   * @brief Removes an SM Policy Association on the PCF. The params pcf_id and
   * id of the association parameter need to be set
   *
   * @param association input: pcf_id and id need to be set and exist
   * @param delete_data input: Values are optional but cannot be null
   * @return sm_policy_status_code OK in case of success, otherwise NOT_FOUND,
   * INTERNAL_ERROR, PCF_NOT_AVAILABLE
   */
  sm_policy_status_code remove_sm_policy_association(
      const policy_association& association,
      const oai::model::pcf::SmPolicyDeleteData& delete_data);

  /**
   * @brief Updates an SM Policy Association, requires the triggers to be set as
   * defined in 3GPP TS 29.512
   *
   * @param association The association to update
   * @param update_data The update context data
   * @return sm_policy_status_code OK in case of success, otherwise NOT_FOUND,
   * INTERNAL_ERROR, PCF_NOT_AVAILABLE
   */
  sm_policy_status_code update_sm_policy_association(
      policy_association& association,
      const oai::model::pcf::SmPolicyUpdateContextData& update_data);

 private:
  /**
   * @brief Allows the discovery of a PCF, either via NRF or local
   * configuration, depending on the DISCOVER_PCF option in the configuration
   * file.
   * In case the input parameters are not set, they are ignored.
   *
   * @param context: Context containing at least Snssai, plmn ID and DNN
   * @return 0 in case of failure, otherwise ID > 0
   */
  uint32_t select_pcf(const oai::model::pcf::SmPolicyContextData& context);

  /**
   * @brief Helper method to receive the policy storage (thread safe)
   *
   * @param pcf_id ID of the policy storage
   * @return std::shared_ptr<policy_storage>  -> nullptr in case not found
   */
  std::shared_ptr<policy_storage> get_policy_storage(uint32_t pcf_id);

  std::unordered_map<uint32_t, std::shared_ptr<policy_storage>> policy_storages;
  mutable std::shared_mutex policy_storages_mutex;
};
}  // namespace smf::n7
#endif /* FILE_SMF_N7_HPP_SEEN */
