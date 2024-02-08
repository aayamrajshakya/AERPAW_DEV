#!/usr/bin/env python3
"""
Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
contributor license agreements.  See the NOTICE file distributed with
this work for additional information regarding copyright ownership.
The OpenAirInterface Software Alliance licenses this file to You under
the OAI Public License, Version 1.1  (the "License"); you may not use this file
except in compliance with the License.
You may obtain a copy of the License at

  http://www.openairinterface.org/?page_id=698

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
------------------------------------------------------------------------------
For more information about the OpenAirInterface (OAI) Software Alliance:
  contact@openairinterface.org
---------------------------------------------------------------------
"""

import logging
import re
import sys
import cls_cmd

logging.basicConfig(
    level=logging.DEBUG,
    stream=sys.stdout,
    format="[%(asctime)s] %(levelname)8s: %(message)s"
)

if __name__ == '__main__':
    myCmds = cls_cmd.LocalCmd()
    myCmds.run('oc delete configmap rhsm-ca')
    myCmds.run('oc delete configmap rhsm-conf')
    myCmds.run('oc create configmap rhsm-conf --from-file /etc/rhsm/rhsm.conf')
    myCmds.run('oc create configmap rhsm-ca --from-file /etc/rhsm/ca/redhat-uep.pem')
    ret = myCmds.run('oc get configmap | grep rhsm')
    print(ret.stdout)
    myCmds.run('oc delete secret etc-pki-entitlement')
    ret = myCmds.run('ls /etc/pki/entitlement/???????????????????.pem | tail -1', silent=True)
    regres1 = re.search(r"/etc/pki/entitlement/[0-9]+.pem", ret.stdout)
    ret = myCmds.run('ls /etc/pki/entitlement/???????????????????-key.pem | tail -1', silent=True)
    regres2 = re.search(r"/etc/pki/entitlement/[0-9]+-key.pem", ret.stdout)
    if regres1 is None or regres2 is None:
        logging.error("could not find entitlements")
        sys.exit(-1)
    file1 = regres1.group(0)
    file2 = regres2.group(0)
    ret = myCmds.run(f'oc create secret generic etc-pki-entitlement --from-file {file1} --from-file {file2}')
    regres = re.search(r"secret/etc-pki-entitlement created", ret.stdout)
    if ret.returncode != 0 or regres is None:
        logging.error("could not create secret/etc-pki-entitlement")
        sys.exit(-1)
    ret = myCmds.run('oc get secret | grep pki')
    print(ret.stdout)

    myCmds.close()
    sys.exit(0)
