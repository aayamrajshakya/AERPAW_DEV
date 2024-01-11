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

/*! \file smf_pfcp_association.hpp
 \author  Lionel GAUTHIER
 \date 2019
 \email: lionel.gauthier@eurecom.fr
 */

#ifndef FILE_SMF_PFCP_ASSOCIATION_HPP_SEEN
#define FILE_SMF_PFCP_ASSOCIATION_HPP_SEEN

#include <map>
#include <mutex>
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "3gpp_29.244.h"
#include "itti.hpp"
#include "smf_profile.hpp"
#include "SmPolicyDecision.h"
#include "3gpp_24.007.h"

namespace smf {

#define PFCP_ASSOCIATION_HEARTBEAT_INTERVAL_SEC 10
#define PFCP_ASSOCIATION_HEARTBEAT_MAX_RETRIES 2
#define PFCP_ASSOCIATION_GRACEFUL_RELEASE_PERIOD 5

struct edge;

class pfcp_association {
 public:
  static iface_type iface_type_from_string(const std::string& input);
  static std::string string_from_iface_type(const iface_type& type);

  pfcp::node_id_t node_id;
  std::size_t hash_node_id;
  pfcp::recovery_time_stamp_t recovery_time_stamp;
  std::pair<bool, pfcp::up_function_features_s> function_features;
  //
  mutable std::mutex m_sessions;
  std::set<pfcp::fseid_t> sessions;
  //
  timer_id_t timer_heartbeat;
  int num_retries_timer_heartbeat;
  uint64_t trxn_id_heartbeat;

  bool is_restore_sessions_pending;

  timer_id_t timer_association;
  timer_id_t timer_graceful_release;

  upf_profile upf_node_profile;
  bool upf_profile_is_set;

  explicit pfcp_association(const pfcp::node_id_t& node_id)
      : node_id(node_id),
        recovery_time_stamp(),
        function_features(),
        m_sessions(),
        sessions(),
        upf_node_profile(),
        upf_profile_is_set(false) {
    hash_node_id                = std::hash<pfcp::node_id_t>{}(node_id);
    timer_heartbeat             = ITTI_INVALID_TIMER_ID;
    num_retries_timer_heartbeat = 0;
    trxn_id_heartbeat           = 0;
    is_restore_sessions_pending = false;
    timer_association           = ITTI_INVALID_TIMER_ID;
    timer_graceful_release      = ITTI_INVALID_TIMER_ID;
  }

  pfcp_association(
      const pfcp::node_id_t& node_id,
      pfcp::recovery_time_stamp_t& recovery_time_stamp)
      : node_id(node_id),
        recovery_time_stamp(recovery_time_stamp),
        function_features(),
        m_sessions(),
        sessions(),
        upf_node_profile(),
        upf_profile_is_set(false) {
    hash_node_id                = std::hash<pfcp::node_id_t>{}(node_id);
    timer_heartbeat             = ITTI_INVALID_TIMER_ID;
    num_retries_timer_heartbeat = 0;
    trxn_id_heartbeat           = 0;
    timer_association           = ITTI_INVALID_TIMER_ID;
    is_restore_sessions_pending = false;
    timer_graceful_release      = ITTI_INVALID_TIMER_ID;
  }
  pfcp_association(
      const pfcp::node_id_t& ni, pfcp::recovery_time_stamp_t& rts,
      pfcp::up_function_features_s& uff)
      : node_id(ni),
        recovery_time_stamp(rts),
        m_sessions(),
        sessions(),
        upf_node_profile(),
        upf_profile_is_set(false) {
    hash_node_id                = std::hash<pfcp::node_id_t>{}(node_id);
    function_features.first     = true;
    function_features.second    = uff;
    timer_heartbeat             = ITTI_INVALID_TIMER_ID;
    num_retries_timer_heartbeat = 0;
    trxn_id_heartbeat           = 0;
    is_restore_sessions_pending = false;
    timer_association           = ITTI_INVALID_TIMER_ID;
    timer_graceful_release      = ITTI_INVALID_TIMER_ID;
  }

  pfcp_association(pfcp_association const& p)
      : node_id(p.node_id),
        hash_node_id(p.hash_node_id),
        recovery_time_stamp(p.recovery_time_stamp),
        function_features(p.function_features),
        timer_heartbeat(p.timer_heartbeat),
        num_retries_timer_heartbeat(p.num_retries_timer_heartbeat),
        trxn_id_heartbeat(p.trxn_id_heartbeat),
        is_restore_sessions_pending(p.is_restore_sessions_pending),
        timer_association(0),
        timer_graceful_release(0),
        upf_node_profile(p.upf_node_profile),
        upf_profile_is_set(p.upf_profile_is_set) {}

  void notify_add_session(const pfcp::fseid_t& cp_fseid);
  bool has_session(const pfcp::fseid_t& cp_fseid);
  void notify_del_session(const pfcp::fseid_t& cp_fseid);
  void del_sessions();
  void restore_n4_sessions();
  void set(const pfcp::up_function_features_s& ff) {
    function_features.first  = true;
    function_features.second = ff;
  };
  void set_upf_node_profile(const upf_profile& profile) {
    upf_node_profile   = profile;
    upf_profile_is_set = true;
  };
  bool is_upf_profile_set() { return upf_profile_is_set; }
  void get_upf_node_profile(upf_profile& profile) const {
    profile = upf_node_profile;
  };
  upf_profile get_upf_node_profile() const { return upf_node_profile; };

  size_t operator()(const pfcp_association&) const { return hash_node_id; }

  /**
   * @brief: Returns true and all the edges of N3 interfaces
   * @param edges
   * @return
   */
  bool find_n3_edge(std::vector<edge>& edges);

  /**
   * @brief: Returns true and all the edges of N6 interfaces
   * @param nw_instance
   * @return
   */
  bool find_n6_edge(std::vector<edge>& edges);

  /**
   * @brief Compares the FQDN or IP address value of all the interfaces of the
   * current PFCP association with the FQDN/ip addresses of other_upf.
   *
   * @param other_upf
   * @param out_edge info of the found edge
   * @return true    when one FQDN or IP address of this interface list matches
   * with FQDN and profile is set in other_upf
   */
  bool find_upf_edge(
      const std::shared_ptr<pfcp_association>& other_upf, edge& out_edge);

  /**
   * @brief Get the readble name of the UPF associated with this association
   * @param void
   * @return string representing the name of the UPF associated with this
   * association
   */
  std::string get_printable_name();

  /*
   * Print related-information for this association
   * @param void
   * @return void:
   */
  void display();

 private:
  bool find_interface_edge(
      const iface_type& type_match, std::vector<edge>& edges);
};

enum node_selection_criteria_e {
  NODE_SELECTION_CRITERIA_BEST_MAX_HEARBEAT_RTT = 0,
  NODE_SELECTION_CRITERIA_MIN_PFCP_SESSIONS     = 1,
  NODE_SELECTION_CRITERIA_MIN_UP_TIME           = 2,
  NODE_SELECTION_CRITERIA_MAX_AVAILABLE_BW      = 3,
  NODE_SELECTION_CRITERIA_NONE                  = 4
};

// TODO moved here from smf_context.hpp
// this is not nice but otherwise we have circular dependencies
// and forward declaration does not work as it is an incomplete type
class smf_qos_flow {
 public:
  smf_qos_flow() { clear(); }

  void clear() {
    ul_fteid    = {};
    dl_fteid    = {};
    pdr_id_ul   = {};
    pdr_id_dl   = {};
    precedence  = {};
    far_id_ul   = {};
    far_id_dl   = {};
    urr_id      = {};
    released    = false;
    qos_profile = {};
    cause_value = 0;
  }

  /**
   * Remove all values associated with a PFCP session, but keep QoS profile
   */
  void clear_session() {
    ul_fteid  = {};
    dl_fteid  = {};
    pdr_id_dl = {};
    pdr_id_ul = {};
    far_id_dl = {};
    far_id_ul = {};
    urr_id    = {};
  }

  /*
   * Release resources associated with this flow
   * @param void
   * @return void
   */
  void deallocate_ressources();

  /*
   * Mark this flow as released
   * @param void
   * @return void
   */
  void mark_as_released();

  /*
   * Represent flow as string to be printed
   * @param void
   * @return void
   */
  std::string toString() const;

  [[nodiscard]] std::string toString(const std::string& indent) const;

  pfcp::qfi_t qfi;           // QoS Flow Identifier
  pfcp::fteid_t ul_fteid{};  // fteid of UPF
  pfcp::fteid_t dl_fteid{};  // fteid of AN
  pfcp::pdr_id_t pdr_id_ul;  // Packet Detection Rule ID, UL
  pfcp::pdr_id_t pdr_id_dl;  // Packet Detection Rule ID, DL
  pfcp::urr_id_t urr_id{};   // Usage reporting Rule, use same for UL and DL
  pfcp::precedence_t precedence{};
  std::pair<bool, pfcp::far_id_t> far_id_ul;  // FAR ID, UL
  std::pair<bool, pfcp::far_id_t> far_id_dl;  // FAR ID, DL
  bool released{};  // finally seems necessary, TODO try to find heuristic ?
  pdu_session_id_t pdu_session_id{};
  qos_profile_t qos_profile;  // QoS profile
  uint8_t cause_value{};      // cause
};

const std::string DEFAULT_FLOW_DESCRIPTION =
    "permit out ip from any to assigned";

struct edge {
  // this might need to be replaced by FlowInformation model in the future, but
  // for now we know this is always uplink
  std::string used_dnai;
  std::string flow_description;
  pfcp::redirect_information_t redirect_information = {};
  unsigned int precedence                           = 0;
  std::string nw_instance;
  iface_type type;
  // Get first address from vector since multiple addresses on same interface
  // type is not under scope for now
  in_addr ip_addr;
  in6_addr ip6_addr;
  bool uplink = false;
  std::vector<std::shared_ptr<smf_qos_flow>> qos_flows;
  bool n4_sent = false;
  std::shared_ptr<pfcp_association> association;
  // we use parts of the upf_interface here
  // the reason why we do is that all the info is split up into several parts
  // in the API. It is more time-efficient to have all in one place
  std::unordered_set<snssai_upf_info_item_s, snssai_upf_info_item_s>
      snssai_dnns;

  static edge from_upf_info(
      const upf_info_t& upf_info, const interface_upf_info_item_t& interface);

  static edge from_upf_info(const upf_info_t& upf_info);

  bool serves_network(const std::string& dnn, const snssai_t& snssai) const;

  bool serves_network(
      const std::string& dnn, const snssai_t& snssai,
      const std::unordered_set<std::string>& dnais,
      std::string& found_dnai) const;

  bool get_qos_flows(std::vector<std::shared_ptr<smf_qos_flow>>& flows);
  bool get_qos_flows(
      pdu_session_id_t pid, std::vector<std::shared_ptr<smf_qos_flow>>& flows);

  std::shared_ptr<smf_qos_flow> get_qos_flow(const pfcp::pdr_id_t& pdr_id);

  std::shared_ptr<smf_qos_flow> get_qos_flow(const pfcp::qfi_t& qfi);

  std::shared_ptr<smf_qos_flow> get_qos_flow(const pfcp::far_id_t& far_id);

  bool operator==(const edge& other) const {
    return nw_instance == other.nw_instance && type == other.type &&
           uplink == other.uplink && association == other.association;
  }

  [[nodiscard]] std::string to_string() const {
    std::string output = pfcp_association::string_from_iface_type(type);
    output.append("(").append(nw_instance).append(")");
    // only print name of UPF when nw instance is empty
    // otherwise output is too long and redundant
    if (association && nw_instance.empty()) {
      output.append("(").append(association->get_printable_name()).append(") ");
    }
    if (!snssai_dnns.empty()) {
      output.append(", S-NSSAI UPF info list: { ");
      for (const auto& s : snssai_dnns) {
        output.append(" ").append(s.to_string()).append(", ");
      }
      output.append("}");
    }
    return output;
  }
};

class upf_graph {
 private:
  // Adjacency List representation of UPF graph, Index is the hashed value
  // of a PFCP association, then we have a list of edges
  // use std::hash of shared_ptr so that hash function of pfcp_association is
  // called
  std::unordered_map<
      std::shared_ptr<pfcp_association>, std::vector<edge>,
      std::hash<std::shared_ptr<pfcp_association>>>
      adjacency_list;

  mutable std::shared_mutex graph_mutex;

  std::stack<std::shared_ptr<pfcp_association>> stack_asynch;
  std::unordered_map<
      std::shared_ptr<pfcp_association>, bool,
      std::hash<std::shared_ptr<pfcp_association>>>
      visited_asynch;
  std::vector<edge> current_edges_ul_asynch;
  std::vector<edge> current_edges_dl_asynch;
  std::shared_ptr<pfcp_association> current_upf_asynch;
  bool uplink_asynch = false;
  smf_qos_flow qos_flow_asynch;

  // normal DFS temporary values
  std::unordered_set<std::string> dfs_all_dnais;
  std::string dfs_flow_description;
  pfcp::redirect_information_t dfs_redirect_information = {};
  snssai_t dfs_snssai;
  std::string dfs_dnn;
  uint32_t dfs_precedence{};

  /**
   * @brief Adds an edge in one direction, adds node if it does not exist
   * @param source
   * @param edge_info_src_dst
   */
  void add_upf_graph_edge(
      const std::shared_ptr<pfcp_association>& source, edge& edge_info_src_dst);

  /**
   * @brief Adds an edge to the graph in both direction, adds node if it does
   * not exist
   * @pre UPF profile needs to be set for both
   *
   * @param source
   * @param dest
   * @param edge_info_src_dest
   * @param edge_info_dst_src
   */
  void add_upf_graph_edge(
      const std::shared_ptr<pfcp_association>& source,
      const std::shared_ptr<pfcp_association>& dest, edge& edge_info_src_dest,
      edge& edge_info_dst_src);

  /**
   * @brief Adds a UPF graph node
   *
   * @param node
   */
  void add_upf_graph_node(const std::shared_ptr<pfcp_association>& node);

  /**
   * @brief Sets selection criteria for the subgraph_dfs function.
   * @pre Not thread-safe, lock before
   * @param all_dnais  All DNAIs that need to be present in the graph
   * @param flow_description The flow description that is used for the edges
   * @param precedence The precedence that is used for the edges
   * @param snssai The SNSSAI info to match against
   * @param dnn The DNN to match against
   */
  void set_dfs_selection_criteria(
      const std::unordered_set<std::string>& all_dnais,
      const std::string& flow_description, uint32_t precedence,
      const snssai_t& snssai, const std::string& dnn,
      const pfcp::redirect_information_t& redirect_information);

  /**
   * @brief Creates a subgraph based on all the UPFs in the current graph which
   * serve the DNAIs from all_dnais. Only UPFs are considered that serve the
   * SNSSAI and DNN
   * @pre Not thread-safe, lock before AND call set_dfs_selection_criteria
   *
   * @param sub_graph Existing graph. may already contain nodes
   * @param start_node Node to start the DFS search
   * @param visited visited map for DFS
   */
  void create_subgraph_dfs(
      std::shared_ptr<upf_graph>& sub_graph,
      const std::shared_ptr<pfcp_association>& start_node,
      std::unordered_map<
          std::shared_ptr<pfcp_association>, bool,
          std::hash<std::shared_ptr<pfcp_association>>>& visited);

  /**
   * @brief: Verifies this graph based on the selection criteria
   * @pre Not thread-safe, lock before
   * @return true if graph is correct, false if not
   */
  bool verify();

  /**
   * TODO
   * @param dnais
   * @return
   */
  static std::string get_dnai_list(const std::unordered_set<string>& dnais);

  /**
   * Traverse the graph in BFS from start and generate info
   * @param start Start node
   * @param indent Added in the beginning of each line
   * @return
   */
  std::string to_string_from_start_node(
      const std::string& indent,
      const std::shared_ptr<pfcp_association>& start) const;

 public:
  upf_graph() : adjacency_list(), visited_asynch(){};

  upf_graph(const upf_graph& g) {
    // TODO do I need to lock the other graph here?
    adjacency_list = g.adjacency_list;
    visited_asynch = g.visited_asynch;
    // do not copy mutex
  }

  /**
   * @brief Inserts a PFCP association into the UPF graph, based on the UPF
   * interface list and adds edges with other existing associations
   *
   * @param sa pointer to the pfcp_association, cannot be null
   */
  void insert_into_graph(const std::shared_ptr<pfcp_association>& sa);

  /**
   * @brief Get Association from an UPF node id hash
   * @param association_hash
   * @return shared_ptr to an association, may be null
   */
  std::shared_ptr<pfcp_association> get_association(
      std::size_t association_hash) const;

  /**
   * @brief Get Association from CP_FSEID
   * @param cp_fseid
   * @return shared_ptr to an association, may be null
   */
  std::shared_ptr<pfcp_association> get_association(
      const pfcp::fseid_t& cp_fseid) const;

  // cannot overload size_t and uint64_t
  /**
   * @brief Get Association from TRXN_ID
   * @param trxn_id
   * @return shared_ptr to an association, may be null
   */
  std::shared_ptr<pfcp_association> get_association_for_trxn_id(
      uint64_t trxn_id) const;
  /**
   * @brief Remove association from graph
   * @param association_hash
   * @return
   */
  bool remove_association(const std::shared_ptr<pfcp_association>& association);

  /**
   * @brief select one UPF node based on SNSSAI and DNAI. Returns a nullpointer
   * if no suitable UPF is found. Does not consider the graph structure
   *
   * @param snssai
   * @param dnn
   * @return std::shared_ptr<pfcp_association>&
   */
  std::shared_ptr<upf_graph> select_upf_node(
      const snssai_t& snssai, const std::string& dnn);

  /**
   * @brief Select UPF based on selection criteria. NOT IMPLEMENTED
   * @param node_selection_criteria
   * @return nullptr
   */
  std::shared_ptr<upf_graph> select_upf_node(int node_selection_criteria);

  /**
   *  @brief Select UPF nodes based on the policy decision and especially on the
   * PCC rules and the traffic descriptions from within. Tries to build a graph
   * where all the DNAIs from the traffic descriptions are contained. In case no
   * graph can be found, an empty pointer is returned
   *
   * @param policy_decision
   * @param snssai
   * @param dnn
   * @return std::shared_ptr<upf_graph>
   */
  std::shared_ptr<upf_graph> select_upf_nodes(
      const oai::model::pcf::SmPolicyDecision& policy_decision,
      const snssai_t& snssai, const std::string& dnn);

  /**
   * @brief Traverses the UPF asynch DFS procedure and returns the next UPF.
   * `start_asynch_dfs_procedure` needs to be started first!
   *
   * @param info_dl output_parameter: dl edge
   * @param info_ul output_parameter: ul edge
   * @param upf output_parameter: UPF
   */
  void dfs_next_upf(
      std::vector<edge>& info_dl, std::vector<edge>& info_ul,
      std::shared_ptr<pfcp_association>& upf);

  /**
   * @brief Gives the information from the UPF which has previously been
   * returned by the call to 'dfs_next_upf'
   *
   * @param info_dl output_parameter: dl edge
   * @param info_ul output_parameter: ul edge
   * @param upf output_parameter: UPF
   */
  void dfs_current_upf(
      std::vector<edge>& info_dl, std::vector<edge>& info_ul,
      std::shared_ptr<pfcp_association>& upf);

  /**
   * @brief Starts asynchronous DFS procedure,
   * @param uplink if uplink or downlink direction
   * @param qos_flow SMF flow to be used for this procedure
   */
  void start_asynch_dfs_procedure(bool uplink, smf_qos_flow& qos_flow);

  /**
   * @brief Returns the access edge of this graph. If multiple exist, first is
   * returned (random order)
   * @return
   */
  edge get_access_edge() const;

  /**
   * Update edge information in the graph
   * @param upf UPF for which edge info should be updated
   * @param info info to update
   */
  void update_edge_info(
      const std::shared_ptr<pfcp_association>& upf, const edge& info);

  /**
   * @brief: Debug-prints the current graph
   */
  void print_graph();
  /**
   * Returns true if graph exceeds MAX_PFCP_ASSOCIATIONS
   * @return
   */
  bool full() const;

  /**
   * Traverses the graph in BFS starting at the N3 interface and returns graph
   * information: For each QFI: UL/DL FTEID for each edge
   * @param indent Added in the beginning of each line
   * @return string representation of this graph
   *
   */
  [[nodiscard]] std::string to_string(const std::string& indent) const;
};

#define PFCP_MAX_ASSOCIATIONS 16

class pfcp_associations {
 private:
  std::vector<std::shared_ptr<pfcp_association>> pending_associations;
  mutable std::mutex m_mutex;

  upf_graph associations_graph;

  pfcp_associations() : pending_associations(), associations_graph(){};

  void trigger_heartbeat_request_procedure(
      std::shared_ptr<pfcp_association>& s);

  std::shared_ptr<pfcp_association> check_association_on_add(
      pfcp::node_id_t& node_id,
      pfcp::recovery_time_stamp_t& recovery_time_stamp,
      bool& restore_n4_sessions, const bool use_function_features,
      pfcp::up_function_features_s& function_features);

  bool resolve_upf_hostname(pfcp::node_id_t& node_id);

  void associate_with_upf_profile(
      std::shared_ptr<pfcp_association>& sa, const pfcp::node_id_t& node_id);

 public:
  static pfcp_associations& get_instance() {
    static pfcp_associations instance;
    return instance;
  }

  pfcp_associations(pfcp_associations const&) = delete;
  void operator=(pfcp_associations const&) = delete;

  bool add_association(
      pfcp::node_id_t& node_id,
      pfcp::recovery_time_stamp_t& recovery_time_stamp,
      bool& restore_n4_sessions);
  bool add_association(
      pfcp::node_id_t& node_id,
      pfcp::recovery_time_stamp_t& recovery_time_stamp,
      pfcp::up_function_features_s& function_features,
      bool& restore_n4_sessions);
  bool add_association(
      pfcp::node_id_t& node_id,
      pfcp::recovery_time_stamp_t& recovery_time_stamp,
      pfcp::up_function_features_s& function_features,
      pfcp::enterprise_specific_s& enterprise_specific,
      bool& restore_n4_sessions);
  bool update_association(
      pfcp::node_id_t& node_id,
      pfcp::up_function_features_s& function_features);
  bool get_association(
      const pfcp::node_id_t& node_id, std::shared_ptr<pfcp_association>& sa);
  bool get_association(
      const pfcp::fseid_t& cp_fseid,
      std::shared_ptr<pfcp_association>& sa) const;

  void notify_add_session(
      const pfcp::node_id_t& node_id, const pfcp::fseid_t& cp_fseid);
  void notify_del_session(const pfcp::fseid_t& cp_fseid);

  void restore_n4_sessions(const pfcp::node_id_t& node_id);

  void initiate_heartbeat_request(timer_id_t timer_id, uint64_t arg2_user);
  void timeout_heartbeat_request(timer_id_t timer_id, uint64_t arg2_user);
  void timeout_release_request(timer_id_t timer_id, uint64_t arg2_user);
  void handle_receive_heartbeat_response(const uint64_t trxn_id);

  std::shared_ptr<upf_graph> select_up_node(const int node_selection_criteria);
  std::shared_ptr<upf_graph> select_up_node(
      const snssai_t& snssai, const std::string& dnn);

  std::shared_ptr<upf_graph> select_up_node(
      const oai::model::pcf::SmPolicyDecision& decision, const snssai_t& snssai,
      const std::string& dnn);

  bool add_peer_candidate_node(const pfcp::node_id_t& node_id);
  bool add_peer_candidate_node(
      const pfcp::node_id_t& node_id, const upf_profile& profile);
  bool remove_association(const std::string& node_instance_id);
  bool remove_association(const int32_t& hash_node_id);
};

}  // namespace smf

#endif /* FILE_SMF_PFCP_ASSOCIATION_HPP_SEEN */
