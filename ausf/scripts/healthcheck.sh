#!/bin/bash
set -eo pipefail

STATUS=0
COMMAND_ARGS=$(pgrep oai_ausf | xargs ps -fp | grep -c ausf.conf || true)

if [ $COMMAND_ARGS -ne 0 ]; then
  # Still using old config file
  #Check if entrypoint properly configured the conf file and no parameter is unset(optional)
  NB_UNREPLACED_AT=`cat /openair-ausf/etc/*.conf | grep -v contact@openairinterface.org | grep -c @ || true`
else
  AUSF_IP_SBI_INTERFACE=$(yq '.nfs.ausf.sbi.interface_name' /openair-ausf/etc/config.yaml)
  SBI_PORT=$(yq '.nfs.ausf.sbi.port' /openair-ausf/etc/config.yaml)
  NB_UNREPLACED_AT=0
fi

AUSF_IP_SBI_INTERFACE=$(ifconfig $SBI_IF_NAME | grep inet | grep -v inet6 | awk {'print $2'})
AUSF_SBI_PORT_STATUS=$(netstat -tnpl | grep -o "$AUSF_IP_SBI_INTERFACE:$SBI_PORT")

if [ $NB_UNREPLACED_AT -ne 0 ]; then
        STATUS=1
        echo "Healthcheck error: UNHEALTHY configuration file is not configured properly"
fi

if [[ -z $AUSF_SBI_PORT_STATUS ]]; then
        STATUS=1
        echo "Healthcheck error: UNHEALTHY SBI TCP/HTTP port $SBI_PORT is not listening."
fi

exit $STATUS
