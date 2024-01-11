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

#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#include <string>
#include <vector>

#include "amf.hpp"
#include "ngap_app.hpp"

typedef struct {
  uint32_t gnb_id;
  // TODO: list of PLMNs
  std::vector<SupportedTaItem_t> plmn_list;
  std::string mcc;
  std::string mnc;
  std::string gnb_name;
  uint32_t tac;
  // long nrCellId;
  std::string plmn_to_string() const {
    std::string s = {};
    for (auto supported_item : plmn_list) {
      s.append("TAC " + std::to_string(supported_item.tac));
      for (auto plmn_slice : supported_item.b_plmn_list) {
        s.append("( MCC " + plmn_slice.mcc);
        s.append(", MNC " + plmn_slice.mnc);
        for (auto slice : plmn_slice.slice_list) {
          s.append("(SST " + slice.sst + ", SD " + slice.sd + "),");
        }
        s.append(")");
      }
      s.append("),");
    }
    return s;
  }
} gnb_infos;

typedef struct ue_info_s {
  std::string connStatus;
  std::string registerStatus;
  uint32_t ranid;
  long amfid;
  std::string imsi;
  std::string guti;
  std::string mcc;
  std::string mnc;
  uint32_t cellId;
} ue_info_t;

class statistics {
 public:
  statistics();
  ~statistics();

  /*
   * Display the AMF configuration parameters
   * @param void
   * @return void
   */
  void display();

  /*
   * Update UE information
   * @param [const ue_info_t&] ue_info: UE information
   * @return void
   */
  void update_ue_info(const ue_info_t& ue_info);

  /*
   * Update UE 5GMM state
   * @param [const std::string&] imsi: UE IMSI
   * @param [const std::string&] state: UE State
   * @return void
   */
  void update_5gmm_state(const std::string& imsi, const std::string& state);

  /*
   * Remove gNB from the list connected gNB to this AMF
   * @param [const uint32_t] gnb_id: gNB ID
   * @return void
   */
  void remove_gnb(const uint32_t& gnb_id);

  /*
   * Add gNB to the list connected gNB to this AMF
   * @param [const uint32_t&] gnb_id: gNB ID
   * @param [const gnb_infos&] gnb: gNB Info
   * @return void
   */
  void add_gnb(const uint32_t& gnb_id, const gnb_infos& gnb);

  /*
   * Add gNB to the list connected gNB to this AMF
   * @param [const std::shared_ptr<gnb_context> &] gc: pointer to gNB Context
   * @return void
   */
  void add_gnb(const std::shared_ptr<gnb_context>& gc);

  /*
   * Update gNB info
   * @param [const uint32_t&] gnb_id: gNB ID
   * @param [const gnb_infos&] gnb: gNB Info
   * @return void
   */
  void update_gnb(const uint32_t& gnb_id, const gnb_infos& gnb);

  /*
   * Get number of connected gNBs
   * @param void
   * @return number of connected gNBs
   */
  uint32_t get_number_connected_gnbs() const;

 private:
  uint32_t gNB_connected;
  uint32_t UE_connected;
  uint32_t UE_registred;
  std::map<uint32_t, gnb_infos> gnbs;
  mutable std::shared_mutex m_gnbs;
  std::map<std::string, ue_info_t> ue_infos;
  mutable std::shared_mutex m_ue_infos;
};

#endif
