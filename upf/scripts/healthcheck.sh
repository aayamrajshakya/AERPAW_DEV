#!/bin/bash
set -eo pipefail

STATUS=0
SGW_PORT_FOR_N3=2152
SGW_PORT_FOR_SX=8805
SGW_IP_N3_INTERFACE=$(ifconfig $SGW_INTERFACE_NAME_FOR_N3 | grep inet | awk {'print $2'})
SGW_IP_SX_INTERFACE=$(ifconfig $SGW_INTERFACE_NAME_FOR_SX | grep inet | awk {'print $2'})
if [[ ${ENABLE_BPF_DATAPATH} == "no" ]]; then
	N3_PORT_STATUS=$(netstat -unpl | grep -o "$SGW_IP_N3_INTERFACE:$SGW_PORT_FOR_N3")
fi
SX_PORT_STATUS=$(netstat -unpl | grep -o "$SGW_IP_SX_INTERFACE:$SGW_PORT_FOR_SX")
#Check if entrypoint properly configured the conf file and no parameter is unset (optional)
NB_UNREPLACED_AT=`cat /openair-upf/etc/*.conf | grep -v contact@openairinterface.org | grep -c @ || true`
if [ $NB_UNREPLACED_AT -ne 0 ]; then
	STATUS=1
	echo "Healthcheck error: UNHEALTHY configuration file is not configured properly"
fi

if [[ ${ENABLE_BPF_DATAPATH} == "no" ]]; then
	if [[ -z $N3_PORT_STATUS ]]; then
		STATUS=1
		echo "Healthcheck error: UNHEALTHY N3 port $SGW_PORT_FOR_N3 is not listening."
	fi
fi

if [[ -z $SX_PORT_STATUS ]]; then
	STATUS=1
	echo "Healthcheck error: UNHEALTHY SX port $SGW_PORT_FOR_SX is not listening."
fi

exit $STATUS
