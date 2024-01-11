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
include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/common_model/common_model.cmake)

# Add common model dependencies from UDR API model
list(APPEND USED_COMMON_MODEL_SRC_FILES
        ${COMMON_MODEL_DIR}/Ambr.cpp
        ${COMMON_MODEL_DIR}/AmbrRm.cpp
        ${COMMON_MODEL_DIR}/AccessTokenErr.cpp
        ${COMMON_MODEL_DIR}/AccessTokenReq.cpp
        ${COMMON_MODEL_DIR}/Arp.cpp
        ${COMMON_MODEL_DIR}/Area.cpp
        ${COMMON_MODEL_DIR}/AreaScope.cpp
        ${COMMON_MODEL_DIR}/AcsInfo.cpp
        ${COMMON_MODEL_DIR}/BatteryIndication.cpp
        ${COMMON_MODEL_DIR}/BackupAmfInfo.cpp
        ${COMMON_MODEL_DIR}/CoreNetworkType.cpp
        ${COMMON_MODEL_DIR}/CoreNetworkType_anyOf.cpp
        ${COMMON_MODEL_DIR}/CollectionPeriodRmmLteMdt.cpp
        ${COMMON_MODEL_DIR}/CollectionPeriodRmmLteMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/CollectionPeriodRmmNrMdt.cpp
        ${COMMON_MODEL_DIR}/CollectionPeriodRmmNrMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/Ecgi.cpp
        ${COMMON_MODEL_DIR}/EventForMdt.cpp
        ${COMMON_MODEL_DIR}/EventForMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/GNbId.cpp
        ${COMMON_MODEL_DIR}/GlobalRanNodeId.cpp
        ${COMMON_MODEL_DIR}/Guami.cpp
        ${COMMON_MODEL_DIR}/InterFreqTargetInfo.cpp
        ${COMMON_MODEL_DIR}/InvalidParam.cpp
        ${COMMON_MODEL_DIR}/JobType.cpp
        ${COMMON_MODEL_DIR}/JobType_anyOf.cpp
        ${COMMON_MODEL_DIR}/LoggingIntervalMdt.cpp
        ${COMMON_MODEL_DIR}/LoggingIntervalMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/LoggingIntervalNrMdt.cpp
        ${COMMON_MODEL_DIR}/LoggingIntervalNrMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/LoggingDurationMdt.cpp
        ${COMMON_MODEL_DIR}/LoggingDurationMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/LoggingDurationNrMdt.cpp
        ${COMMON_MODEL_DIR}/LteV2xAuth.cpp
        ${COMMON_MODEL_DIR}/MeasurementPeriodLteMdt.cpp
        ${COMMON_MODEL_DIR}/MeasurementNrForMdt.cpp
        ${COMMON_MODEL_DIR}/MeasurementLteForMdt.cpp
        ${COMMON_MODEL_DIR}/MeasurementLteForMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/MdtConfiguration.cpp
        ${COMMON_MODEL_DIR}/MbsfnArea.cpp
        ${COMMON_MODEL_DIR}/Ncgi.cpp
        ${COMMON_MODEL_DIR}/NFType.cpp
        ${COMMON_MODEL_DIR}/NFType_anyOf.cpp
        ${COMMON_MODEL_DIR}/NrV2xAuth.cpp
        ${COMMON_MODEL_DIR}/OdbPacketServices.cpp
        ${COMMON_MODEL_DIR}/OdbPacketServices_anyOf.cpp
        ${COMMON_MODEL_DIR}/PatchItem.cpp
        ${COMMON_MODEL_DIR}/PatchOperation.cpp
        ${COMMON_MODEL_DIR}/PatchOperation_anyOf.cpp
        ${COMMON_MODEL_DIR}/PlmnIdNid.cpp
        ${COMMON_MODEL_DIR}/PreemptionCapability.cpp
        ${COMMON_MODEL_DIR}/PreemptionCapability_anyOf.cpp
        ${COMMON_MODEL_DIR}/PreemptionVulnerability.cpp
        ${COMMON_MODEL_DIR}/PreemptionVulnerability_anyOf.cpp
        ${COMMON_MODEL_DIR}/ProblemDetails.cpp
        ${COMMON_MODEL_DIR}/RatType.cpp
        ${COMMON_MODEL_DIR}/RatType_anyOf.cpp
        ${COMMON_MODEL_DIR}/ReportAmountMdt.cpp
        ${COMMON_MODEL_DIR}/ReportAmountMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/ReportingTrigger.cpp
        ${COMMON_MODEL_DIR}/ReportingTrigger_anyOf.cpp
        ${COMMON_MODEL_DIR}/ReportTypeMdt.cpp
        ${COMMON_MODEL_DIR}/ReportTypeMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/ReportIntervalMdt.cpp
        ${COMMON_MODEL_DIR}/ReportIntervalMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/ReportIntervalNrMdt.cpp
        ${COMMON_MODEL_DIR}/ReportIntervalNrMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/RestrictionType.cpp
        ${COMMON_MODEL_DIR}/RestrictionType_anyOf.cpp
        ${COMMON_MODEL_DIR}/PositioningMethodMdt.cpp
        ${COMMON_MODEL_DIR}/PositioningMethodMdt_anyOf.cpp
        ${COMMON_MODEL_DIR}/ScheduledCommunicationType.cpp
        ${COMMON_MODEL_DIR}/ScheduledCommunicationType_anyOf.cpp
        ${COMMON_MODEL_DIR}/ScheduledCommunicationTime.cpp
        ${COMMON_MODEL_DIR}/ServiceAreaRestriction.cpp
        ${COMMON_MODEL_DIR}/SensorMeasurement.cpp
        ${COMMON_MODEL_DIR}/SensorMeasurement_anyOf.cpp
        ${COMMON_MODEL_DIR}/Snssai.cpp
        ${COMMON_MODEL_DIR}/SscMode.cpp
        ${COMMON_MODEL_DIR}/SscMode_anyOf.cpp
        ${COMMON_MODEL_DIR}/StationaryIndication.cpp
        ${COMMON_MODEL_DIR}/StationaryIndication_anyOf.cpp
        ${COMMON_MODEL_DIR}/SubscribedDefaultQos.cpp
        ${COMMON_MODEL_DIR}/TacInfo.cpp
        ${COMMON_MODEL_DIR}/TraceData.cpp
        ${COMMON_MODEL_DIR}/TraceDepth.cpp
        ${COMMON_MODEL_DIR}/TraceDepth_anyOf.cpp
        ${COMMON_MODEL_DIR}/TrafficProfile.cpp
        ${COMMON_MODEL_DIR}/TrafficProfile_anyOf.cpp
        ${COMMON_MODEL_DIR}/UeAuth.cpp
        ${COMMON_MODEL_DIR}/UeAuth_anyOf.cpp
        ${COMMON_MODEL_DIR}/UpConfidentiality.cpp
        ${COMMON_MODEL_DIR}/UpIntegrity.cpp
        ${COMMON_MODEL_DIR}/UpIntegrity_anyOf.cpp
        ${COMMON_MODEL_DIR}/UpSecurity.cpp
        ${COMMON_MODEL_DIR}/WirelineArea.cpp
        ${COMMON_MODEL_DIR}/WirelineServiceAreaRestriction.cpp
)

# we also use NRF models
include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/nrf/nrf_model.cmake)

# finally, we have to include common_model.cmake (has to be last
include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/common_model/common_model.cmake)
