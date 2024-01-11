#!/bin/bash
set -eo pipefail

STATUS=0
NRF_INTERFACE_NAME_FOR_SBI=$(yq '.nfs.nrf.sbi.interface_name' /openair-nrf/etc/config.yaml)
NRF_INTERFACE_PORT_FOR_SBI=$(yq '.nfs.nrf.sbi.port' /openair-nrf/etc/config.yaml)

NRF_IP_SBI_INTERFACE=$(ifconfig $NRF_INTERFACE_NAME_FOR_SBI | grep inet | awk {'print $2'})
NRF_SBI_PORT_STATUS=$(netstat -tnpl | grep -o "$NRF_IP_SBI_INTERFACE:$NRF_INTERFACE_PORT_FOR_SBI")

if [[ -z $NRF_SBI_PORT_STATUS ]]; then
	STATUS=1
	echo "Healthcheck error: UNHEALTHY SBI TCP/HTTP port $NRF_INTERFACE_PORT_FOR_SBI is not listening."
fi

exit $STATUS
