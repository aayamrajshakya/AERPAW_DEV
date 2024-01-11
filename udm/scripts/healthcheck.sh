#!/bin/bash
set -eo pipefail

STATUS=0
# Determine if we are still using conf file or new yaml file
COMMAND_ARGS=$(pgrep oai_udm | xargs ps -fp | grep -c udm.conf || true)

if [ $COMMAND_ARGS -ne 0 ]; then
  # Still using old config file
  #Check if entrypoint properly configured the conf file and no parameter is unset(optional)
  NB_UNREPLACED_AT=`cat /openair-udm/etc/*.conf | grep -v contact@openairinterface.org | grep -c @ || true`
else
  # Now using the new yaml file
  SBI_IF_NAME=$(yq '.nfs.udm.sbi.interface_name' /openair-udm/etc/config.yaml)
  SBI_PORT=$(yq '.nfs.udm.sbi.port' /openair-udm/etc/config.yaml)
  NB_UNREPLACED_AT=0
fi

UDM_IP_SBI_INTERFACE=$(ifconfig $SBI_IF_NAME | grep inet | grep -v inet6 | awk {'print $2'})
UDM_SBI_PORT_STATUS=$(netstat -tnpl | grep -o "$UDM_IP_SBI_INTERFACE:$SBI_PORT")

if [ $NB_UNREPLACED_AT -ne 0 ]; then
        STATUS=1
        echo "Healthcheck error: UNHEALTHY configuration file is not configured properly"
fi

if [[ -z $UDM_SBI_PORT_STATUS ]]; then
        STATUS=1
        echo "Healthcheck error: UNHEALTHY SBI TCP/HTTP port $SBI_PORT is not listening."
fi

exit $STATUS
