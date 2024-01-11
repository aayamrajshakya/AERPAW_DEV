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

#include "amf_config.hpp"

#include "3gpp_29.502.h"
#include "amf_app.hpp"
#include "logger.hpp"

extern "C" {
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#include "common_defs.h"
}

using namespace libconfig;
using namespace amf_application;

namespace oai::config {

//------------------------------------------------------------------------------
amf_config::amf_config() {
  smf_addr.ipv4_addr.s_addr               = INADDR_ANY;
  smf_addr.port                           = DEFAULT_HTTP1_PORT;
  smf_addr.api_version                    = DEFAULT_SBI_API_VERSION;
  nrf_addr.ipv4_addr.s_addr               = INADDR_ANY;
  nrf_addr.port                           = DEFAULT_HTTP1_PORT;
  nrf_addr.api_version                    = DEFAULT_SBI_API_VERSION;
  ausf_addr.ipv4_addr.s_addr              = INADDR_ANY;
  ausf_addr.port                          = DEFAULT_HTTP1_PORT;
  ausf_addr.api_version                   = DEFAULT_SBI_API_VERSION;
  udm_addr.ipv4_addr.s_addr               = INADDR_ANY;
  udm_addr.port                           = DEFAULT_HTTP1_PORT;
  udm_addr.api_version                    = DEFAULT_SBI_API_VERSION;
  lmf_addr.ipv4_addr.s_addr               = INADDR_ANY;
  lmf_addr.port                           = DEFAULT_HTTP1_PORT;
  lmf_addr.api_version                    = DEFAULT_SBI_API_VERSION;
  nssf_addr.ipv4_addr.s_addr              = INADDR_ANY;
  nssf_addr.port                          = DEFAULT_HTTP1_PORT;
  nssf_addr.api_version                   = DEFAULT_SBI_API_VERSION;
  instance                                = 0;
  log_level                               = spdlog::level::debug;
  n2                                      = {};
  sbi                                     = {};
  sbi_api_version                         = DEFAULT_SBI_API_VERSION;
  sbi_http2_port                          = DEFAULT_HTTP2_PORT;
  statistics_interval                     = 0;
  guami                                   = {};
  guami_list                              = {};
  relative_amf_capacity                   = 0;
  plmn_list                               = {};
  auth_para                               = {};
  nas_cfg                                 = {};
  support_features.enable_nf_registration = false;
  support_features.enable_smf_selection   = false;
  support_features.enable_external_ausf   = false;
  support_features.enable_external_udm    = false;
  support_features.enable_nssf            = false;
  support_features.enable_lmf             = false;
  support_features.use_fqdn_dns           = false;
  support_features.use_http2              = false;
  support_features.enable_external_nrf    = true;
  is_emergency_support                    = false;
  // TODO:
}

//------------------------------------------------------------------------------
amf_config::~amf_config() {}

//------------------------------------------------------------------------------
void amf_config::display() {
  Logger::config().info(
      "==== OAI-CN5G %s v%s ====", PACKAGE_NAME, PACKAGE_VERSION);
  Logger::config().info(
      "======================    AMF   =====================");
  Logger::config().info("Configuration AMF:");
  Logger::config().info("- Instance ................: %d", instance);
  Logger::config().info("- PID dir .................: %s", pid_dir.c_str());
  Logger::config().info("- AMF NAME.................: %s", amf_name.c_str());
  Logger::config().info(
      "- GUAMI (MCC, MNC, Region ID, AMF Set ID, AMF pointer): ");
  Logger::config().info(
      "    (%s, %s, %d, %d, %d)", guami.mcc.c_str(), guami.mnc.c_str(),
      guami.region_id, guami.amf_set_id, guami.amf_pointer);
  Logger::config().info("- Served Guami List:");
  for (int i = 0; i < guami_list.size(); i++) {
    Logger::config().info(
        "    (%s, %s, %d , %d, %d)", guami_list[i].mcc.c_str(),
        guami_list[i].mnc.c_str(), guami_list[i].region_id,
        guami_list[i].amf_set_id, guami_list[i].amf_pointer);
  }
  Logger::config().info(
      "- Relative Capacity .......: %d", relative_amf_capacity);
  Logger::config().info("- PLMN Support: ");
  for (int i = 0; i < plmn_list.size(); i++) {
    Logger::config().info(
        "    MCC, MNC ..............: %s, %s", plmn_list[i].mcc.c_str(),
        plmn_list[i].mnc.c_str());
    Logger::config().info("    TAC ...................: %d", plmn_list[i].tac);
    Logger::config().info("    Slice Support .........:");
    for (int j = 0; j < plmn_list[i].slice_list.size(); j++) {
      if (plmn_list[i].slice_list[j].sd != SD_NO_VALUE) {
        Logger::config().info(
            "        SST, SD ...........: %d, %ld (0x%x)",
            plmn_list[i].slice_list[j].sst, plmn_list[i].slice_list[j].sd,
            plmn_list[i].slice_list[j].sd);
      } else {
        Logger::config().info(
            "        SST ...............: %d", plmn_list[i].slice_list[j].sst);
      }
    }
  }
  Logger::config().info(
      "- Emergency Support .......: %s", is_emergency_support ?
                                             AMF_CONFIG_OPTION_YES_STR :
                                             AMF_CONFIG_OPTION_NO_STR);

  if (!support_features.enable_external_ausf) {
    Logger::config().info("- Database: ");
    Logger::config().info(
        "    MySQL Server Addr .....: %s", auth_para.mysql_server.c_str());
    Logger::config().info(
        "    MySQL user ............: %s", auth_para.mysql_user.c_str());
    Logger::config().info(
        "    MySQL pass ............: %s", auth_para.mysql_pass.c_str());
    Logger::config().info(
        "    MySQL DB ..............: %s", auth_para.mysql_db.c_str());
  }

  Logger::config().info("- N2 Networking:");
  Logger::config().info("    Iface .................: %s", n2.if_name.c_str());
  Logger::config().info("    IP Addr ...............: %s", inet_ntoa(n2.addr4));
  Logger::config().info("    Port ..................: %d", n2.port);

  Logger::config().info("- SBI Networking:");
  Logger::config().info("    Iface .................: %s", sbi.if_name.c_str());
  Logger::config().info(
      "    IP Addr ...............: %s", inet_ntoa(sbi.addr4));
  Logger::config().info("    Port ..................: %d", sbi.port);
  Logger::config().info("    HTTP2 port ............: %d", sbi_http2_port);
  Logger::config().info(
      "    API version............: %s", sbi_api_version.c_str());

  if (!support_features.enable_smf_selection) {
    Logger::config().info("- SMF:");
    Logger::config().info(
        "    URI root ...............: %s", smf_addr.uri_root);
    Logger::config().info(
        "    API version ...........: %s", smf_addr.api_version.c_str());
  }

  if (support_features.enable_external_nrf) {
    Logger::config().info("- NRF:");
    Logger::config().info(
        "    URI root ...............: %s", nrf_addr.uri_root);
    Logger::config().info(
        "    API version ...........: %s", nrf_addr.api_version.c_str());
  }

  if (support_features.enable_nssf) {
    Logger::config().info("- NSSF:");
    Logger::config().info(
        "    URI root ...............: %s", nssf_addr.uri_root);
    Logger::config().info(
        "    API version ...........: %s", nssf_addr.api_version.c_str());
  }

  if (support_features.enable_external_ausf) {
    Logger::config().info("- AUSF:");
    Logger::config().info(
        "    URI root ...............: %s", ausf_addr.uri_root);
    Logger::config().info(
        "    API version ...........: %s", ausf_addr.api_version.c_str());
  }

  if (support_features.enable_external_udm) {
    Logger::config().info("- UDM:");
    Logger::config().info(
        "    URI root ...............: %s", udm_addr.uri_root);
    Logger::config().info(
        "    API version ...........: %s", udm_addr.api_version.c_str());
  }

  if (support_features.enable_lmf) {
    Logger::config().info("- LMF:");
    Logger::config().info(
        "    IP Addr ...............: %s", inet_ntoa(lmf_addr.ipv4_addr));
    Logger::config().info("    Port ..................: %d", lmf_addr.port);
    Logger::config().info(
        "    API version ...........: %s", lmf_addr.api_version.c_str());
  }

  Logger::config().info("- Supported Features:");
  Logger::config().info(
      "    NF Registration .......: %s",
      support_features.enable_nf_registration ? AMF_CONFIG_OPTION_YES_STR :
                                                AMF_CONFIG_OPTION_NO_STR);
  Logger::config().info(
      "    SMF Selection .........: %s", support_features.enable_smf_selection ?
                                             AMF_CONFIG_OPTION_YES_STR :
                                             AMF_CONFIG_OPTION_NO_STR);
  Logger::config().info(
      "    External AUSF .........: %s", support_features.enable_external_ausf ?
                                             AMF_CONFIG_OPTION_YES_STR :
                                             AMF_CONFIG_OPTION_NO_STR);
  Logger::config().info(
      "    External UDM ..........: %s", support_features.enable_external_udm ?
                                             AMF_CONFIG_OPTION_YES_STR :
                                             AMF_CONFIG_OPTION_NO_STR);
  Logger::config().info(
      "    External NSSF .........: %s", support_features.enable_nssf ?
                                             AMF_CONFIG_OPTION_YES_STR :
                                             AMF_CONFIG_OPTION_NO_STR);
  Logger::config().info(
      "    External LMF ..........: %s",
      support_features.enable_lmf ? "Yes" : "No");

  Logger::config().info(
      "    Use HTTP2 .............: %s", support_features.use_http2 ?
                                             AMF_CONFIG_OPTION_YES_STR :
                                             AMF_CONFIG_OPTION_NO_STR);

  Logger::config().info(
      "    Use FQDN .............: %s", support_features.use_fqdn_dns ?
                                            AMF_CONFIG_OPTION_YES_STR :
                                            AMF_CONFIG_OPTION_NO_STR);
  Logger::config().info(
      "- Log Level ...............: %s",
      spdlog::level::to_string_view(log_level));

  Logger::config().info("- Supported NAS Algorithm: ");
  std::string supported_integrity_alg = {};
  for (const auto& i : nas_cfg.prefered_integrity_algorithm) {
    supported_integrity_alg.append(get_5g_ia_str(i)).append(" ");
  }
  Logger::config().info(
      "    Ordered Integrity Algorithm: %s", supported_integrity_alg);
  std::string supported_ciphering_alg = {};
  for (const auto& i : nas_cfg.prefered_ciphering_algorithm) {
    supported_ciphering_alg.append(get_5g_ea_str(i)).append(" ");
  }
  Logger::config().info(
      "    Ordered Ciphering Algorithm: %s", supported_ciphering_alg);
}

//------------------------------------------------------------------------------
std::string amf_config::get_amf_n1n2_message_subscribe_uri(
    const std::string& ue_cxt_id) {
  unsigned int sbi_port = DEFAULT_HTTP1_PORT;
  if (support_features.use_http2) {
    sbi_port = sbi_http2_port;
  } else {
    sbi_port = sbi.port;
  }
  return std::string(inet_ntoa(*((struct in_addr*) &sbi.addr4))) + ":" +
         std::to_string(sbi_port) + NAMF_COMMUNICATION_BASE + sbi_api_version +
         "/ue-contexts/" + ue_cxt_id + "/n1-n2-messages/subscriptions";
}

//------------------------------------------------------------------------------
std::string amf_config::get_nrf_nf_discovery_service_uri() {
  return nrf_addr.uri_root + "/nnrf-disc/" + nrf_addr.api_version +
         "/nf-instances";
}

//------------------------------------------------------------------------------
std::string amf_config::get_nrf_nf_registration_uri(
    const std::string& nf_instance_id) {
  return nrf_addr.uri_root + "/nnrf-nfm/" + nrf_addr.api_version +
         "/nf-instances/" + nf_instance_id;
}

//------------------------------------------------------------------------------
std::string amf_config::get_udm_slice_selection_subscription_data_retrieval_uri(
    const std::string& supi) {
  return udm_addr.uri_root + "/nudm-sdm/" + udm_addr.api_version + "/" + supi +
         "/nssai";
}

//------------------------------------------------------------------------------
std::string amf_config::get_nssf_network_slice_selection_information_uri() {
  return nssf_addr.uri_root + "/nnssf-nsselection/" + nssf_addr.api_version +
         "/network-slice-information";
}

//------------------------------------------------------------------------------
std::string amf_config::get_ausf_ue_authentications_uri() {
  return ausf_addr.uri_root + "/nausf-auth/" + ausf_addr.api_version +
         "/ue-authentications";
}

//------------------------------------------------------------------------------
std::string amf_config::get_lmf_determine_location_uri() {
  return std::string(inet_ntoa(*((struct in_addr*) &lmf_addr.ipv4_addr))) +
         ":" + std::to_string(lmf_addr.port) + NLMF_BASE +
         lmf_addr.api_version + NLMF_DETERMINE_LOCATION;
}

bool amf_config::get_smf_pdu_session_context_uri(
    const std::shared_ptr<pdu_session_context>& psc, std::string& smf_uri) {
  if (!psc) return false;

  if (!psc->smf_info.info_available) {
    Logger::amf_sbi().error("No SMF is available for this PDU session");
    return false;
  }

  std::size_t found = psc->smf_info.context_location.find("/");
  if (found != 0)
    smf_uri = psc->smf_info.context_location;
  else
    smf_uri = psc->smf_info.uri_root + psc->smf_info.context_location;
  return true;
}

//------------------------------------------------------------------------------
std::string amf_config::get_smf_pdu_session_base_uri(
    const std::string& smf_uri_root, const std::string& smf_api_version) {
  return smf_uri_root + "/nsmf-pdusession/" + smf_api_version +
         NSMF_PDU_SESSION_CREATE;
}

//------------------------------------------------------------------------------
void amf_config::to_json(nlohmann::json& json_data) const {
  json_data["instance"]   = instance;
  json_data["log_level"]  = log_level;
  json_data["amf_name"]   = amf_name;
  json_data["guami"]      = guami.to_json();
  json_data["guami_list"] = nlohmann::json::array();
  for (auto s : guami_list) {
    json_data["guami_list"].push_back(s.to_json());
  }

  json_data["relative_amf_capacity"] = relative_amf_capacity;

  json_data["plmn_list"] = nlohmann::json::array();
  for (auto s : plmn_list) {
    json_data["plmn_list"].push_back(s.to_json());
  }
  json_data["is_emergency_support"] = is_emergency_support ?
                                          AMF_CONFIG_OPTION_YES_STR :
                                          AMF_CONFIG_OPTION_NO_STR;

  if (!support_features.enable_external_ausf) {
    json_data["auth_para"] = auth_para.to_json();
  }

  json_data["n2"]  = n2.to_json();
  json_data["sbi"] = sbi.to_json();

  json_data["support_features_options"] = support_features.to_json();

  json_data["smf"] = smf_addr.to_json();

  if (support_features.enable_external_nrf) {
    json_data["nrf"] = nrf_addr.to_json();
  }

  if (support_features.enable_nssf) {
    json_data["nssf"] = nssf_addr.to_json();
  }

  if (support_features.enable_external_ausf) {
    json_data["ausf"] = ausf_addr.to_json();
  }

  if (support_features.enable_external_udm) {
    json_data["udm"] = udm_addr.to_json();
  }

  json_data["supported_nas_algorithms"] = nas_cfg.to_json();
  if (support_features.enable_lmf) {
    json_data["lmf"] = lmf_addr.to_json();
  }
}

//------------------------------------------------------------------------------
bool amf_config::from_json(nlohmann::json& json_data) {
  try {
    if (json_data.find("instance") != json_data.end()) {
      instance = json_data["instance"].get<int>();
    }

    if (json_data.find("pid_dir") != json_data.end()) {
      pid_dir = json_data["pid_dir"].get<std::string>();
    }
    if (json_data.find("amf_name") != json_data.end()) {
      amf_name = json_data["amf_name"].get<std::string>();
    }
    if (json_data.find("log_level") != json_data.end()) {
      log_level = json_data["log_level"].get<spdlog::level::level_enum>();
    }
    if (json_data.find("guami") != json_data.end()) {
      guami.from_json(json_data["guami"]);
    }

    if (json_data.find("guami_list") != json_data.end()) {
      guami_list.clear();
      for (auto s : json_data["guami_list"]) {
        guami_t g = {};
        g.from_json(s);
        guami_list.push_back(g);
      }
    }

    if (json_data.find("relative_amf_capacity") != json_data.end()) {
      relative_amf_capacity = json_data["relative_amf_capacity"].get<int>();
    }

    if (json_data.find("plmn_list") != json_data.end()) {
      plmn_list.clear();
      for (auto s : json_data["plmn_list"]) {
        plmn_item_t p = {};
        p.from_json(s);
        plmn_list.push_back(p);
      }
    }

    if (json_data.find("is_emergency_support") != json_data.end()) {
      is_emergency_support = json_data["is_emergency_support"].get<bool>();
    }

    if (json_data.find("auth_para") != json_data.end()) {
      auth_para.from_json(json_data["auth_para"]);
    }

    if (json_data.find("n2") != json_data.end()) {
      n2.from_json(json_data["n2"]);
    }
    if (json_data.find("sbi") != json_data.end()) {
      sbi.from_json(json_data["sbi"]);
    }

    if (json_data.find("sbi_http2_port") != json_data.end()) {
      sbi_http2_port = json_data["sbi_http2_port"].get<int>();
    }

    if (json_data.find("support_features_options") != json_data.end()) {
      support_features.from_json(json_data["support_features_options"]);
    }

    if (json_data.find("smf") != json_data.end()) {
      smf_addr.from_json(json_data["smf"]);
    }

    if (support_features.enable_external_nrf) {
      if (json_data.find("nrf") != json_data.end()) {
        nrf_addr.from_json(json_data["nrf"]);
      }
    }

    if (support_features.enable_nssf) {
      if (json_data.find("nssf") != json_data.end()) {
        nssf_addr.from_json(json_data["nssf"]);
      }
    }

    if (support_features.enable_external_ausf) {
      if (json_data.find("ausf") != json_data.end()) {
        ausf_addr.from_json(json_data["ausf"]);
      }
    }

    if (support_features.enable_external_udm) {
      if (json_data.find("udm") != json_data.end()) {
        udm_addr.from_json(json_data["udm"]);
      }
    }
    if (support_features.enable_lmf) {
      if (json_data.find("lmf") != json_data.end()) {
        lmf_addr.from_json(json_data["lmf"]);
      }
    }

  } catch (nlohmann::detail::exception& e) {
    Logger::amf_app().error(
        "Exception when reading configuration from json %s", e.what());
    return false;
  } catch (std::exception& e) {
    Logger::amf_app().error(
        "Exception when reading configuration from json %s", e.what());
    return false;
  }

  return true;
}
}  // namespace oai::config
