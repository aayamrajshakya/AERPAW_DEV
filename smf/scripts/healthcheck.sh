#!/bin/bash
set -eo pipefail

STATUS=0
SMF_INTERFACE_NAME_FOR_SBI=$(yq '.nfs.smf.sbi.interface_name' /openair-smf/etc/config.yaml)
SMF_INTERFACE_PORT_FOR_SBI=$(yq '.nfs.smf.sbi.port' /openair-smf/etc/config.yaml)

SMF_IP_SBI_INTERFACE=$(ifconfig $SMF_INTERFACE_NAME_FOR_SBI | grep inet | grep -v inet6 | awk {'print $2'})
#Check if entrypoint properly configured the conf file and no parameter is unset(optional)
SMF_SBI_PORT_STATUS=$(netstat -tnpl | grep -o "$SMF_IP_SBI_INTERFACE:$SMF_INTERFACE_PORT_FOR_SBI")

if [[ -z $SMF_SBI_PORT_STATUS ]]; then
	STATUS=-1
	echo "Healthcheck error: UNHEALTHY SBI TCP/HTTP port $SMF_INTERFACE_PORT_FOR_SBI is not listening."
fi

exit $STATUS
