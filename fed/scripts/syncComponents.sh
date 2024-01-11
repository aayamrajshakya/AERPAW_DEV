#!/bin/bash
#/*
# * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
# * contributor license agreements.  See the NOTICE file distributed with
# * this work for additional information regarding copyright ownership.
# * The OpenAirInterface Software Alliance licenses this file to You under
# * the OAI Public License, Version 1.1  (the "License"); you may not use this file
# * except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *      http://www.openairinterface.org/?page_id=698
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *-------------------------------------------------------------------------------
# * For more information about the OpenAirInterface (OAI) Software Alliance:
# *      contact@openairinterface.org
# */

function usage {
    echo "Openair-CN components synchronization"
    echo "   Original Author: Raphael Defosseux"
    echo ""
    echo "   Requirement: git shall be installed"
    echo ""
    echo "   By default (no options) all components will be synchronized to"
    echo "     the 'master' branch."
    echo "   Each component can be synchronized a dedicated branch."
    echo ""
    echo "Usage:"
    echo "------"
    echo "    syncComponents.sh [OPTIONS]"
    echo ""
    echo "Options:"
    echo "--------"
    echo "    --nrf-branch ####"
    echo "    Specify the source branch for the OAI-NRF component"
    echo ""
    echo "    --amf-branch ####"
    echo "    Specify the source branch for the OAI-AMF component"
    echo ""
    echo "    --smf-branch ####"
    echo "    Specify the source branch for the OAI-SMF component"
    echo ""
    echo "    --upf-branch ####"
    echo "    Specify the source branch for the OAI-UPF component"
    echo ""
    echo "    --ausf-branch ####"
    echo "    Specify the source branch for the OAI-AUSF component"
    echo ""
    echo "    --udm-branch ####"
    echo "    Specify the source branch for the OAI-UDM component"
    echo ""
    echo "    --udr-branch ####"
    echo "    Specify the source branch for the OAI-UDR component"
    echo ""
    echo "    --upf-vpp-branch ####"
    echo "    Specify the source branch for the OAI-UPF-VPP component"
    echo ""
    echo "    --nssf-branch ####"
    echo "    Specify the source branch for the OAI-UDR component"
    echo ""
    echo "    --nef-branch ####"
    echo "    Specify the source branch for the OAI-NEF component"
    echo ""
    echo "    --pcf-branch ####"
    echo "    Specify the source branch for the OAI-PCF component"
    echo ""
    echo "    --verbose"
    echo "    Will show all operations results"
    echo ""
    echo "    --help OR -h"
    echo "    Print this help message."
    echo ""
}

BRANCH_NAMES=("master" "master" "master" "master" "master" "master" "master" "master" "master" "master" "master")
#BRANCH_NAMES=("develop" "develop" "develop" "develop" "develop" "develop" "develop" "develop" "develop" "develop" "develop")
COMPONENT_PATHS=("oai-nrf" "oai-amf" "oai-smf" "oai-upf" "oai-ausf" "oai-udm" "oai-udr" "oai-upf-vpp" "oai-nssf" "oai-nef" "oai-pcf")

NRF_IDX=0
AMF_IDX=1
SMF_IDX=2
UPF_IDX=3
AUSF_IDX=4
UDM_IDX=5
UDR_IDX=6
UPF_VPP_IDX=7
NSSF_IDX=8
NEF_IDX=9
PCF_IDX=10

doDefault=0
verbose=0

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -h|--help)
    shift
    usage
    exit 0
    ;;
    --nrf-branch)
    BRANCH_NAMES[NRF_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --amf-branch)
    BRANCH_NAMES[AMF_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --smf-branch)
    BRANCH_NAMES[SMF_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --upf-branch)
    BRANCH_NAMES[UPF_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --ausf-branch)
    BRANCH_NAMES[AUSF_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --udm-branch)
    BRANCH_NAMES[UDM_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --udr-branch)
    BRANCH_NAMES[UDR_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --upf-vpp-branch)
    BRANCH_NAMES[UPF_VPP_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --nssf-branch)
    BRANCH_NAMES[NSSF_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --nef-branch)
    BRANCH_NAMES[NEF_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --pcf-branch)
    BRANCH_NAMES[PCF_IDX]="$2"
    doDefault=0
    shift
    shift
    ;;
    --verbose)
    verbose=1
    shift
    shift
    ;;
    *)
    echo "Syntax Error: unknown option: $key"
    echo ""
    usage
    exit 1
esac

done

echo "---------------------------------------------------------"
echo "OAI-NRF     component branch : ${BRANCH_NAMES[NRF_IDX]}"
echo "OAI-AMF     component branch : ${BRANCH_NAMES[AMF_IDX]}"
echo "OAI-SMF     component branch : ${BRANCH_NAMES[SMF_IDX]}"
echo "OAI-UPF     component branch : ${BRANCH_NAMES[UPF_IDX]}"
echo "OAI-AUSF    component branch : ${BRANCH_NAMES[AUSF_IDX]}"
echo "OAI-UDM     component branch : ${BRANCH_NAMES[UDM_IDX]}"
echo "OAI-UDR     component branch : ${BRANCH_NAMES[UDR_IDX]}"
echo "OAI-UPF-VPP component branch : ${BRANCH_NAMES[UPF_VPP_IDX]}"
echo "OAI-NSSF    component branch : ${BRANCH_NAMES[NSSF_IDX]}"
echo "OAI-NEF     component branch : ${BRANCH_NAMES[NEF_IDX]}"
echo "OAI-PCF     component branch : ${BRANCH_NAMES[PCF_IDX]}"
echo "---------------------------------------------------------"

# First do a clean-up
echo "git submodule deinit --force ."
if [ $verbose -eq 1 ]; then
    git submodule deinit --force .
else
    git submodule deinit --force . > /dev/null 2>&1
fi
echo "git submodule update --init --recursive"
if [ $verbose -eq 1 ]; then
    git submodule update --init --recursive
else
    git submodule update --init --recursive > /dev/null 2>&1
fi

if [ $doDefault -eq 1 ]; then
    # should be enough now.
    echo "git submodule foreach 'git clean -x -d -ff'"
    if [ $verbose -eq 1 ]; then
        git submodule foreach 'git clean -x -d -ff'
    else
        git submodule foreach 'git clean -x -d -ff' > /dev/null 2>&1
    fi
    exit
else
    length=${#BRANCH_NAMES[@]}
    for (( idx=0; idx<${length}; idx++ ));
    do
        pushd component/${COMPONENT_PATHS[idx]}
        if [ $verbose -eq 1 ]; then
            git fetch --prune
            git branch -D ${BRANCH_NAMES[idx]}
            if [[ $? -ne 0 ]]; then
                git checkout ${BRANCH_NAMES[idx]}
            else
                git checkout -b ${BRANCH_NAMES[idx]} origin/${BRANCH_NAMES[idx]}
            fi
            git submodule update --init --recursive
            git clean -x -d -ff
        else
            git fetch --prune > /dev/null 2>&1
            git branch -D ${BRANCH_NAMES[idx]} > /dev/null 2>&1
            if [[ $? -ne 0 ]]; then
                git checkout ${BRANCH_NAMES[idx]} > /dev/null 2>&1
            else
                git checkout -b ${BRANCH_NAMES[idx]} origin/${BRANCH_NAMES[idx]} > /dev/null 2>&1
            fi
            git submodule update --init --recursive > /dev/null 2>&1
            git clean -x -d -ff > /dev/null 2>&1
        fi
        popd
    done
fi
