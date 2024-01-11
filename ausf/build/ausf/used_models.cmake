################################################################################
# Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The OpenAirInterface Software Alliance licenses this file to You under
# the OAI Public License, Version 1.1  (the "License"); you may not use this file
# except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.openairinterface.org/?page_id=698
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#-------------------------------------------------------------------------------
# For more information about the OpenAirInterface (OAI) Software Alliance:
#      contact@openairinterface.org
################################################################################

## This file is used to specify the common models and utils this library is using
## DO NOT JUST COPY THIS FILE FROM OTHER NFs. The reasoning behind this is to only compile used files to optimize
## build speed

# Add common model dependencies from AUSF API model
list(APPEND USED_COMMON_MODEL_SRC_FILES
        ${COMMON_MODEL_DIR}/Helpers.cpp
        ${COMMON_MODEL_DIR}/InvalidParam.cpp
        ${COMMON_MODEL_DIR}/Ipv6Addr.cpp
        ${COMMON_MODEL_DIR}/Link.cpp
        ${COMMON_MODEL_DIR}/LinksValueSchema.cpp
        ${COMMON_MODEL_DIR}/PatchItem.cpp
        ${COMMON_MODEL_DIR}/PatchOperation.cpp
        ${COMMON_MODEL_DIR}/PatchOperation_anyOf.cpp
        ${COMMON_MODEL_DIR}/TraceData.cpp
        ${COMMON_MODEL_DIR}/TraceDepth.cpp
        ${COMMON_MODEL_DIR}/TraceDepth_anyOf.cpp
        ${COMMON_MODEL_DIR}/ProblemDetails.cpp
        ${COMMON_MODEL_DIR}/AccessTokenErr.cpp
        ${COMMON_MODEL_DIR}/AccessTokenReq.cpp
        ${COMMON_MODEL_DIR}/NFType.cpp
        ${COMMON_MODEL_DIR}/NFType_anyOf.cpp
        ${COMMON_MODEL_DIR}/PlmnId.cpp
        ${COMMON_MODEL_DIR}/PlmnIdNid.cpp
        ${COMMON_MODEL_DIR}/Snssai.cpp
        ${COMMON_MODEL_DIR}/Tai.cpp
)

# we also use NRF models
#include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/nrf/nrf_model.cmake)

# finally, we have to include common_model.cmake (has to be last
include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/common_model/common_model.cmake)