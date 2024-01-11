
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

# Add common model dependencies from SMF API model
# TODO here we also have stuff from NRF and PCF still mixed-in
list(APPEND USED_COMMON_MODEL_SRC_FILES
        ${COMMON_MODEL_DIR}/ProblemDetails.cpp
        ${COMMON_MODEL_DIR}/AccessTokenErr.cpp
        ${COMMON_MODEL_DIR}/AccessTokenReq.cpp
        ${COMMON_MODEL_DIR}/NFType.cpp
        ${COMMON_MODEL_DIR}/NFType_anyOf.cpp
        ${COMMON_MODEL_DIR}/LineType.cpp
        ${COMMON_MODEL_DIR}/LineType_anyOf.cpp
        ${COMMON_MODEL_DIR}/DnaiChangeType.cpp
        ${COMMON_MODEL_DIR}/DddTrafficDescriptor.cpp
        ${COMMON_MODEL_DIR}/NgApCause.cpp
        ${COMMON_MODEL_DIR}/DnaiChangeType_anyOf.cpp
        ${COMMON_MODEL_DIR}/InvalidParam.cpp
        ${COMMON_MODEL_DIR}/PatchOperation.cpp
        ${COMMON_MODEL_DIR}/PatchOperation_anyOf.cpp
        ${COMMON_MODEL_DIR}/PatchItem.cpp
        ${COMMON_MODEL_DIR}/RefToBinaryData.cpp
        ${COMMON_MODEL_DIR}/PlmnIdNid.cpp
        ${COMMON_MODEL_DIR}/ChangeItem.cpp
        ${COMMON_MODEL_DIR}/ChangeType.cpp
        ${COMMON_MODEL_DIR}/ChangeType_anyOf.cpp
        ${COMMON_MODEL_DIR}/BackupAmfInfo.cpp
        # SM Policy
        ${COMMON_MODEL_DIR}/QosResourceType.cpp
        ${COMMON_MODEL_DIR}/QosResourceType_anyOf.cpp
        ${COMMON_MODEL_DIR}/Arp.cpp
        ${COMMON_MODEL_DIR}/PreemptionVulnerability.cpp
        ${COMMON_MODEL_DIR}/PreemptionVulnerability_anyOf.cpp
        ${COMMON_MODEL_DIR}/PreemptionCapability.cpp
        ${COMMON_MODEL_DIR}/PreemptionCapability_anyOf.cpp
        ${COMMON_MODEL_DIR}/Ambr.cpp
        ${COMMON_MODEL_DIR}/AtsssCapability.cpp
        ${COMMON_MODEL_DIR}/PresenceState.cpp
        ${COMMON_MODEL_DIR}/PresenceState_anyOf.cpp
        ${COMMON_MODEL_DIR}/PduSessionType.cpp
        ${COMMON_MODEL_DIR}/PduSessionType_anyOf.cpp
        ${COMMON_MODEL_DIR}/RouteToLocation.cpp
        ${COMMON_MODEL_DIR}/RouteInformation.cpp
        ${COMMON_MODEL_DIR}/Ipv6Prefix.cpp
        ${COMMON_MODEL_DIR}/Ipv6Addr.cpp
        ${COMMON_MODEL_DIR}/Guami.cpp
        ${COMMON_MODEL_DIR}/AccessType.cpp
        ${COMMON_MODEL_DIR}/AccessTypeRm.cpp
        ${COMMON_MODEL_DIR}/DlDataDeliveryStatus.cpp
        ${COMMON_MODEL_DIR}/DlDataDeliveryStatus_anyOf.cpp
        ${COMMON_MODEL_DIR}/PresenceInfoRm.cpp
        ${COMMON_MODEL_DIR}/PresenceInfo.cpp
        ${COMMON_MODEL_DIR}/SubscribedDefaultQos.cpp
        # RAN Node ID dependencies
        ${COMMON_MODEL_DIR}/GlobalRanNodeId.cpp
        ${COMMON_MODEL_DIR}/GNbId.cpp
        ${COMMON_MODEL_DIR}/TnapId.cpp
        # User Location Dependencies
        ${COMMON_MODEL_DIR}/UserLocation.cpp
        ${COMMON_MODEL_DIR}/NrLocation.cpp
        ${COMMON_MODEL_DIR}/Ncgi.cpp
        ${COMMON_MODEL_DIR}/N3gaLocation.cpp
        ${COMMON_MODEL_DIR}/TransportProtocol.cpp
        ${COMMON_MODEL_DIR}/TransportProtocol_anyOf.cpp
        ${COMMON_MODEL_DIR}/TwapId.cpp
        ${COMMON_MODEL_DIR}/HfcNodeId.cpp
        ${COMMON_MODEL_DIR}/EutraLocation.cpp
        ${COMMON_MODEL_DIR}/Ecgi.cpp
        # UTRAN/GERAN Location Dependencies
        ${COMMON_MODEL_DIR}/UtraLocation.cpp
        ${COMMON_MODEL_DIR}/GeraLocation.cpp
        ${COMMON_MODEL_DIR}/ServiceAreaId.cpp
        ${COMMON_MODEL_DIR}/LocationAreaId.cpp
        ${COMMON_MODEL_DIR}/RoutingAreaId.cpp
        ${COMMON_MODEL_DIR}/CellGlobalId.cpp
        # other dependencies
        ${COMMON_MODEL_DIR}/TraceData.cpp
        ${COMMON_MODEL_DIR}/TraceDepth.cpp
        ${COMMON_MODEL_DIR}/TraceDepth_anyOf.cpp
        ${COMMON_MODEL_DIR}/RatType.cpp
        ${COMMON_MODEL_DIR}/RatType_anyOf.cpp
        ${COMMON_MODEL_DIR}/SecondaryRatUsageReport.cpp
        ${COMMON_MODEL_DIR}/SecondaryRatUsageInfo.cpp
        ${COMMON_MODEL_DIR}/QosFlowUsageReport.cpp
        ${COMMON_MODEL_DIR}/VolumeTimedReport.cpp
        )

include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/smf/smf_model.cmake)

list(APPEND USED_SMF_MODEL_SRC_FILES
        ${SMF_MODEL_DIR}/DnnSelectionMode.cpp
        ${SMF_MODEL_DIR}/DnnSelectionMode_anyOf.cpp
        ${SMF_MODEL_DIR}/VplmnQos.cpp
        )

include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/pcf/pcf_model.cmake)

list(APPEND USED_PCF_MODEL_SRC_FILES
        # SM Policy Decision
        ${PCF_MODEL_DIR}/QosData.cpp
        ${PCF_MODEL_DIR}/SmPolicyDecision.cpp
        ${PCF_MODEL_DIR}/ChargingInformation.cpp
        ${PCF_MODEL_DIR}/RequestedUsageData.cpp
        ${PCF_MODEL_DIR}/QosFlowUsage.cpp
        ${PCF_MODEL_DIR}/QosFlowUsage_anyOf.cpp
        ${PCF_MODEL_DIR}/SmPolicyAssociationReleaseCause.cpp
        ${PCF_MODEL_DIR}/SmPolicyAssociationReleaseCause_anyOf.cpp
        ${PCF_MODEL_DIR}/BridgeManagementContainer.cpp
        ${PCF_MODEL_DIR}/PortManagementContainer.cpp
        ${PCF_MODEL_DIR}/RequestedRuleData.cpp
        ${PCF_MODEL_DIR}/RequestedRuleDataType.cpp
        ${PCF_MODEL_DIR}/RequestedRuleDataType_anyOf.cpp
        ${PCF_MODEL_DIR}/PolicyControlRequestTrigger.cpp
        ${PCF_MODEL_DIR}/PolicyControlRequestTrigger_anyOf.cpp
        ${PCF_MODEL_DIR}/SessionRule.cpp
        ${PCF_MODEL_DIR}/AuthorizedDefaultQos.cpp
        ${PCF_MODEL_DIR}/PccRule.cpp
        ${PCF_MODEL_DIR}/AfSigProtocol.cpp
        ${PCF_MODEL_DIR}/AfSigProtocol_anyOf.cpp
        ${PCF_MODEL_DIR}/FlowInformation.cpp
        ${PCF_MODEL_DIR}/EthFlowDescription.cpp
        ${PCF_MODEL_DIR}/FlowDirectionRm.cpp
        ${PCF_MODEL_DIR}/FlowDirection_anyOf.cpp
        ${PCF_MODEL_DIR}/TscaiInputContainer.cpp
        ${PCF_MODEL_DIR}/DownlinkDataNotificationControl.cpp
        ${PCF_MODEL_DIR}/NotificationControlIndication.cpp
        ${PCF_MODEL_DIR}/NotificationControlIndication_anyOf.cpp
        ${PCF_MODEL_DIR}/DownlinkDataNotificationControlRm.cpp
        ${PCF_MODEL_DIR}/ChargingData.cpp
        ${PCF_MODEL_DIR}/ReportingLevel.cpp
        ${PCF_MODEL_DIR}/ReportingLevel_anyOf.cpp
        ${PCF_MODEL_DIR}/MeteringMethod.cpp
        ${PCF_MODEL_DIR}/MeteringMethod_anyOf.cpp
        ${PCF_MODEL_DIR}/UsageMonitoringData.cpp
        ${PCF_MODEL_DIR}/QosCharacteristics.cpp
        ${PCF_MODEL_DIR}/TrafficControlData.cpp
        ${PCF_MODEL_DIR}/FlowStatus.cpp
        ${PCF_MODEL_DIR}/FlowStatus_anyOf.cpp
        ${PCF_MODEL_DIR}/SteeringFunctionality.cpp
        ${PCF_MODEL_DIR}/SteeringFunctionality_anyOf.cpp
        ${PCF_MODEL_DIR}/SteeringMode.cpp
        ${PCF_MODEL_DIR}/SteerModeValue.cpp
        ${PCF_MODEL_DIR}/SteerModeValue_anyOf.cpp
        ${PCF_MODEL_DIR}/MulticastAccessControl.cpp
        ${PCF_MODEL_DIR}/MulticastAccessControl_anyOf.cpp
        ${PCF_MODEL_DIR}/RedirectInformation.cpp
        ${PCF_MODEL_DIR}/RedirectAddressType.cpp
        ${PCF_MODEL_DIR}/RedirectAddressType_anyOf.cpp
        ${PCF_MODEL_DIR}/UpPathChgEvent.cpp
        ${PCF_MODEL_DIR}/QosMonitoringData.cpp
        ${PCF_MODEL_DIR}/ReportingFrequency.cpp
        ${PCF_MODEL_DIR}/ReportingFrequency_anyOf.cpp
        ${PCF_MODEL_DIR}/RequestedQosMonitoringParameter.cpp
        ${PCF_MODEL_DIR}/RequestedQosMonitoringParameter_anyOf.cpp
        ${PCF_MODEL_DIR}/ConditionData.cpp
        ${PCF_MODEL_DIR}/SmPolicyControl.cpp
        # SM Policy Context
        ${PCF_MODEL_DIR}/SmPolicyContextData.cpp
        ${PCF_MODEL_DIR}/AccNetChId.cpp
        ${PCF_MODEL_DIR}/AccNetChargingAddress.cpp
        ${PCF_MODEL_DIR}/AdditionalAccessInfo.cpp
        ${PCF_MODEL_DIR}/ServingNfIdentity.cpp
        ${PCF_MODEL_DIR}/AnGwAddress.cpp
        ${PCF_MODEL_DIR}/MaPduIndication.cpp
        ${PCF_MODEL_DIR}/MaPduIndication_anyOf.cpp
        ${PCF_MODEL_DIR}/PcfAtsssCapability.cpp
        ${PCF_MODEL_DIR}/PcfAtsssCapability_anyOf.cpp
        # SmPolicyUpdateContextData
        ${PCF_MODEL_DIR}/SmPolicyUpdateContextData.cpp
        ${PCF_MODEL_DIR}/UeInitiatedResourceRequest.cpp
        ${PCF_MODEL_DIR}/RuleOperation.cpp
        ${PCF_MODEL_DIR}/RuleOperation_anyOf.cpp
        ${PCF_MODEL_DIR}/RequestedQos.cpp
        ${PCF_MODEL_DIR}/PacketFilterInfo.cpp
        ${PCF_MODEL_DIR}/FlowDirection.cpp
        ${PCF_MODEL_DIR}/FlowDirection_anyOf.cpp
        ${PCF_MODEL_DIR}/CreditManagementStatus.cpp
        ${PCF_MODEL_DIR}/CreditManagementStatus_anyOf.cpp
        ${PCF_MODEL_DIR}/TsnBridgeInfo.cpp
        ${PCF_MODEL_DIR}/AppDetectionInfo.cpp
        ${PCF_MODEL_DIR}/RuleReport.cpp
        ${PCF_MODEL_DIR}/RuleStatus.cpp
        ${PCF_MODEL_DIR}/RuleStatus_anyOf.cpp
        ${PCF_MODEL_DIR}/FailureCode.cpp
        ${PCF_MODEL_DIR}/FailureCode_anyOf.cpp
        ${PCF_MODEL_DIR}/RanNasRelCause.cpp
        ${PCF_MODEL_DIR}/FinalUnitAction.cpp
        ${PCF_MODEL_DIR}/FinalUnitAction_anyOf.cpp
        ${PCF_MODEL_DIR}/SessionRuleReport.cpp
        ${PCF_MODEL_DIR}/SessionRuleFailureCode.cpp
        ${PCF_MODEL_DIR}/SessionRuleFailureCode_anyOf.cpp
        ${PCF_MODEL_DIR}/QosNotificationControlInfo.cpp
        ${PCF_MODEL_DIR}/QosNotifType.cpp
        ${PCF_MODEL_DIR}/QosNotifType_anyOf.cpp
        ${PCF_MODEL_DIR}/QosMonitoringReport.cpp
        ${PCF_MODEL_DIR}/IpMulticastAddressInfo.cpp
        ${PCF_MODEL_DIR}/PolicyDecisionFailureCode.cpp
        ${PCF_MODEL_DIR}/PolicyDecisionFailureCode_anyOf.cpp
        ${PCF_MODEL_DIR}/CreditManagementStatus.cpp
        ${PCF_MODEL_DIR}/TsnBridgeInfo.cpp
        ${PCF_MODEL_DIR}/AccuUsageReport.cpp
        # Sm Policy Delete Data
        ${PCF_MODEL_DIR}/SmPolicyDeleteData.cpp
        ${PCF_MODEL_DIR}/PduSessionRelCause.cpp
        ${PCF_MODEL_DIR}/PduSessionRelCause_anyOf.cpp
        )

# we also use SMF models
include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/smf/smf_model.cmake)

# we also use PCF models
include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/pcf/pcf_model.cmake)

# we also use NRF models
include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/nrf/nrf_model.cmake)

# finally, we have to include common_model.cmake (has to be last
include(${SRC_TOP_DIR}/${MOUNTED_COMMON}/model/common_model/common_model.cmake)