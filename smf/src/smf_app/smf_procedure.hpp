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

/*! \file smf_procedure.hpp
 \author  Lionel GAUTHIER, Tien-Thinh NGUYEN
 \company Eurecom
 \date 2019
 \email: lionel.gauthier@eurecom.fr, tien-thinh.nguyen@eurecom.fr
 */

#ifndef FILE_SMF_PROCEDURE_HPP_SEEN
#define FILE_SMF_PROCEDURE_HPP_SEEN

#include <memory>
#include <mutex>
#include <set>

#include "3gpp_29.244.hpp"
#include "itti_msg_n11.hpp"
#include "itti_msg_n4.hpp"
#include "itti_msg_n4_restore.hpp"
#include "itti_msg_nx.hpp"
#include "msg_pfcp.hpp"
#include "smf_msg.hpp"
#include "uint_generator.hpp"
#include "smf_pfcp_association.hpp"

namespace smf {

class smf_context;
class smf_pdu_session;

enum class smf_procedure_code : uint8_t { CONTINUE, ERROR, OK };

//------------------------------------------------------------------------------
class smf_procedure {
 private:
  static uint64_t trxn_id_generator;

  static uint64_t generate_trxn_id() {
    return util::uint_uid_generator<uint64_t>::get_instance().get_uid();
  }

 public:
  uint64_t trxn_id;
  smf_procedure() { trxn_id = generate_trxn_id(); }
  explicit smf_procedure(uint64_t tx) { trxn_id = tx; }
  virtual ~smf_procedure() {}
  virtual itti_msg_type_t get_procedure_type() { return ITTI_MSG_TYPE_NONE; }
  virtual smf_procedure_code handle_itti_msg(
      itti_n4_session_establishment_response& resp,
      std::shared_ptr<smf::smf_context> pc) {
    return smf_procedure_code::OK;
  }
  virtual smf_procedure_code handle_itti_msg(
      itti_n4_session_modification_response& resp,
      std::shared_ptr<smf::smf_context> pc) {
    return smf_procedure_code::OK;
  }
  virtual smf_procedure_code handle_itti_msg(
      itti_n4_session_deletion_response& resp,
      std::shared_ptr<smf::smf_context> pc) {
    return smf_procedure_code::OK;
  }
};

class smf_qos_flow;
class smf_pdu_session;

class smf_session_procedure : public smf_procedure {
 public:
  explicit smf_session_procedure(std::shared_ptr<smf_pdu_session>& ps)
      : smf_procedure(), sps(ps) {}

  std::shared_ptr<smf_pdu_session> sps;

  pfcp::create_far pfcp_create_far(edge& edge, const pfcp::qfi_t& qfi);

  pfcp::create_pdr pfcp_create_pdr(
      edge& edge, const pfcp::qfi_t& qfi,
      const pfcp::up_function_features_s up_features);

  pfcp::create_urr pfcp_create_urr(edge& edge, const pfcp::qfi_t& qfi);

  pfcp::create_pdr pfcp_create_pdr_dl(edge& edge, const pfcp::qfi_t& qfi);

  // TODO eventuell if used more than once
 private:
  // pfcp::destination_interface_value_e get_interface_value(const edge& edge);

  pfcp::ue_ip_address_t pfcp_ue_ip_address(const edge& edge);

  pfcp::fteid_t pfcp_prepare_fteid(
      pfcp::fteid_t& fteid, const bool& ftup_supported);

 protected:
  void synch_ul_dl_edges(
      const vector<edge>& dl_edges, const vector<edge>& ul_edges,
      const pfcp::qfi_t& qfi, bool synch_pdr_for_uplink);

  /**
   * Helper function to get current UPF from graph in a safe way
   * @param dl_edges
   * @param ul_edges
   * @param current_upf
   * @return ERROR in case not successful
   */
  smf_procedure_code get_current_upf(
      std::vector<edge>& dl_edges, std::vector<edge>& ul_edges,
      std::shared_ptr<pfcp_association>& current_upf);

  /**
   * Helper function to get next UPF from graph in a safe way
   * @param dl_edges
   * @param ul_edges
   * @param current_upf
   * @return ERROR in case not successful, OK when UPF graph is empty and
   * CONTINUE if UPF is not null
   */
  smf_procedure_code get_next_upf(
      std::vector<edge>& dl_edges, std::vector<edge>& ul_edges,
      std::shared_ptr<pfcp_association>& next_upf);
};

//------------------------------------------------------------------------------
class n4_session_restore_procedure : public smf_procedure {
 public:
  explicit n4_session_restore_procedure(
      std::set<pfcp::fseid_t>& sessions2restore)
      : smf_procedure(),
        pending_sessions(sessions2restore),
        restored_sessions() {
    sessions2restore.clear();
  }

  int run();
  //~n4_session_restore_procedure() {}
  std::set<pfcp::fseid_t> pending_sessions;
  std::set<pfcp::fseid_t> restored_sessions;
};

//------------------------------------------------------------------------------
class session_create_sm_context_procedure : public smf_session_procedure {
 public:
  explicit session_create_sm_context_procedure(
      std::shared_ptr<smf_pdu_session>& ps)
      : smf_session_procedure(ps),
        n4_triggered(),
        n11_triggered_pending(),
        n11_trigger() {}

  /*
   * Execute N11 Create SM Context Request procedure
   * @param [itti_n11_create_sm_context_request] req
   * @param [itti_n11_create_sm_context_response] resp
   * @param [std::shared_ptr<smf::smf_context>] sc: smf context
   * @return
   */
  smf_procedure_code run(
      std::shared_ptr<itti_n11_create_sm_context_request> req,
      std::shared_ptr<itti_n11_create_sm_context_response> resp,
      std::shared_ptr<smf::smf_context> sc);

  /*
   * Handle N4 Session Establishment Response from UPF
   * @param [itti_n4_session_establishment_response] resp
   * @param [std::shared_ptr<smf::smf_context>] sc smf context
   * @return void
   */
  smf_procedure_code handle_itti_msg(
      itti_n4_session_establishment_response& resp,
      std::shared_ptr<smf::smf_context> sc) override;

  std::shared_ptr<itti_n4_session_establishment_request> n4_triggered;

  std::shared_ptr<itti_n11_create_sm_context_request> n11_trigger;
  std::shared_ptr<itti_n11_create_sm_context_response> n11_triggered_pending;

 private:
  smf_qos_flow current_flow{};

  /**
   * Sends a session establishment request, based on current UPF graph
   * @return
   */
  smf_procedure_code send_n4_session_establishment_request();
};

//------------------------------------------------------------------------------
class session_update_sm_context_procedure : public smf_session_procedure {
 public:
  explicit session_update_sm_context_procedure(
      std::shared_ptr<smf_pdu_session>& ps)
      : smf_session_procedure(ps),
        n4_triggered(),
        n11_triggered_pending(),
        n11_trigger(),
        session_procedure_type() {}

  /*
   * Execute N11 Update SM Context Request procedure
   * @param [itti_n11_update_sm_context_request] req
   * @param [itti_n11_update_sm_context_response] resp
   * @param [std::shared_ptr<smf::smf_context>] sc: smf context
   * @return
   */
  smf_procedure_code run(
      std::shared_ptr<itti_n11_update_sm_context_request> req,
      std::shared_ptr<itti_n11_update_sm_context_response> resp,
      std::shared_ptr<smf::smf_context> sc);

  /*
   * Handle N4 Session Modification Response from UPF
   * @param [itti_n4_session_modification_response] resp
   * @param [std::shared_ptr<smf::smf_context>] sc smf context
   * @return void
   */
  smf_procedure_code handle_itti_msg(
      itti_n4_session_modification_response& resp,
      std::shared_ptr<smf::smf_context> sc) override;

  std::shared_ptr<itti_n4_session_modification_request> n4_triggered;

  std::shared_ptr<itti_n11_update_sm_context_request> n11_trigger;
  std::shared_ptr<itti_n11_update_sm_context_response> n11_triggered_pending;
  session_management_procedures_type_e session_procedure_type;

 private:
  // TODO currently support only one flow
  smf_qos_flow current_flow{};
  /**
   * Sends a session modification request, based on the graph
   * Does only consider normal DL procedures
   * @return OK when successful, ERROR otherwise
   */
  smf_procedure_code send_n4_session_modification_request();
};

//------------------------------------------------------------------------------
class session_release_sm_context_procedure : public smf_session_procedure {
 public:
  explicit session_release_sm_context_procedure(
      std::shared_ptr<smf_pdu_session>& ps)
      : smf_session_procedure(ps),
        n4_triggered(),
        n11_triggered_pending(),
        n11_trigger(),
        session_procedure_type() {}

  /*
   * Execute N11 Release SM Context Request procedure
   * @param [itti_n11_release_sm_context_request] req
   * @param [itti_n11_release_sm_context_response] resp
   * @param [std::shared_ptr<smf::smf_context>] sc: smf context
   * @return
   */
  smf_procedure_code run(
      std::shared_ptr<itti_n11_release_sm_context_request> req,
      std::shared_ptr<itti_n11_release_sm_context_response> resp,
      std::shared_ptr<smf::smf_context> sc);

  /*
   * Handle N4 Session Modification Response from UPF
   * @param [itti_n4_session_modification_response] resp
   * @param [std::shared_ptr<smf::smf_context>] sc smf context
   * @return void
   */
  smf_procedure_code handle_itti_msg(
      itti_n4_session_deletion_response& resp,
      std::shared_ptr<smf::smf_context> sc) override;

  std::shared_ptr<itti_n4_session_deletion_request> n4_triggered;

  std::shared_ptr<itti_n11_release_sm_context_request> n11_trigger;
  std::shared_ptr<itti_n11_release_sm_context_response> n11_triggered_pending;
  session_management_procedures_type_e session_procedure_type;

 private:
  smf_procedure_code send_n4_session_deletion_request();
};

}  // namespace smf
#include "../smf_app/smf_context.hpp"

#endif
