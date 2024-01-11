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

SET(SMF_MODEL_DIR ${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/smf)

include_directories(${SMF_MODEL_DIR})

list(REMOVE_DUPLICATES USED_SMF_MODEL_SRC_FILES)

# Here, we expect that in NF build directory or in other models (e.g. NRF) selected common_models are written to
# ${USED_COMMON_MODEL_SRC_FILES} to speed up the build (more performant than just adding all models)
if (TARGET ${NF_TARGET})
    target_include_directories(${NF_TARGET} PUBLIC ${SMF_MODEL_DIR})
    target_sources(${NF_TARGET} PRIVATE
            ${USED_SMF_MODEL_SRC_FILES}
            )
endif()