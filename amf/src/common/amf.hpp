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

#ifndef __AMF_HPP
#define __AMF_HPP

#include "3gpp_23.003.h"
#include "inttypes.h"
#include "stdio.h"
#include "string.h"

// for CURL
constexpr auto CURL_MIME_BOUNDARY = "----Boundary";
#define CURL_TIMEOUT_MS 10000L

#define BUFFER_SIZE_8192 8192
#define BUFFER_SIZE_4096 4096
#define BUFFER_SIZE_2048 2048
#define BUFFER_SIZE_1024 1024
#define BUFFER_SIZE_512 512
#define BUFFER_SIZE_256 256

#define GNB_ID_FMT "%" PRIu32
#define GNB_UE_NGAP_ID_FMT "%" PRIu32
#define AMF_UE_NGAP_ID_FMT "%" PRIu64

constexpr long INVALID_AMF_UE_NGAP_ID = -1;

// Event Subscription IDs)
typedef uint32_t evsub_id_t;
#define EVSUB_ID_FMT "0x%" PRIx32
#define EVSUB_ID_SCAN_FMT SCNx32
#define INVALID_EVSUB_ID ((evsub_id_t) 0x00000000)
#define UNASSIGNED_EVSUB_ID ((evsub_id_t) 0x00000000)

typedef uint32_t n1n2sub_id_t;

constexpr uint64_t SECONDS_SINCE_FIRST_EPOCH = 2208988800;

#define UE_AGGREGATE_MAXIMUM_BIT_RATE_DL 1000000000
#define UE_AGGREGATE_MAXIMUM_BIT_RATE_UL 1000000000

#define NAMF_COMMUNICATION_BASE "/namf-comm/"
#define NAMF_COMMUNICATION_N1N2_MESSAGE_TRANSFER_URL "n1-n2-messages"
#define NAMF_COMMUNICATION_NON_UE_N2_MESSAGE_TRANSFER_URL                      \
  "/non-ue-n2-messages/transfer"
#define NAMF_COMMUNICATION_N1_MESSAGE_NOTIFY_URL "n1-message-notify"

#define NAMF_EVENT_EXPOSURE_BASE "/namf-evts/"
#define NAMF_EVENT_EXPOSURE_SUBSCRIPTION "/subscriptions"

#define NLMF_BASE "/nlmf-loc/"
#define NLMF_DETERMINE_LOCATION "/determine-location"

#define NAMF_CUSTOMIZED_API_BASE "/namf-oai/"
#define NAMF_CUSTOMIZED_API_CONFIGURATION_URL "/configuration"

#define NAMF_STATUS_NOTIFY_API_BASE "/namf-status-notify/"
#define NAMF_STATUS_NOTIFY_API_URL "/pdu-session-release/callback/"

#define NAS_MESSAGE_DOWNLINK 1
#define NAS_MESSAGE_UPLINK 0

#define JSON_CONTENT_ID_MIME "root"
#define N1_SM_CONTENT_ID "n1SmMsg"
#define N2_SM_CONTENT_ID "n2msg"
#define N2_NRPPa_CONTENT_ID "n2NrppaMsg"

typedef enum {
  PlainNasMsg                                              = 0x0,
  IntegrityProtected                                       = 0x1,
  IntegrityProtectedAndCiphered                            = 0x2,
  IntegrityProtectedWithNew5GNASSecurityContext            = 0x3,
  IntegrityProtectedAndCipheredWithNew5GNASSecurityContext = 0x4,
} SecurityHeaderType_t;

constexpr uint32_t DEFAULT_HTTP1_PORT  = 80;
constexpr uint32_t DEFAULT_HTTP2_PORT  = 8080;
constexpr auto DEFAULT_SBI_API_VERSION = "v1";
constexpr auto DEFAULT_SUPI_TYPE =
    "imsi";  // Set to "imsi" when supporting both IMSI and NAI as SUPI

constexpr auto DEFAULT_SST = 1;
constexpr auto DEFAULT_DNN = "default";
#endif
