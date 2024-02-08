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

import argparse
import logging
import re
import sys
import time
import cls_cmd

logging.basicConfig(
    level=logging.DEBUG,
    stream=sys.stdout,
    format="[%(asctime)s] %(levelname)8s: %(message)s"
)

def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Checking Build Pod Status')

    parser.add_argument(
        '--pod-name', '-pn',
        action='store',
        required=True,
        help='Build Pod name',
    )

    parser.add_argument(
        '--log-file', '-l',
        action='store',
        required=True,
        help='Log file location',
    )
    return parser.parse_args()

if __name__ == '__main__':
    args = _parse_args()
    status = 0
    cnt = 0
    myCmds = cls_cmd.LocalCmd()
    while cnt < 6*20:
        ret = myCmds.run(f'oc get pods | grep {args.pod_name}', silent = True)
        if ret.stdout.count('Completed') > 0:
            cnt = 1000
            status = 0
        elif ret.stdout.count('Error') > 0:
            cnt = 1000
            status = -1
        else:
            cnt += 1
            time.sleep(10)
    if cnt < 500:
        status = -2
    # Storing the build logs all the time
    myCmds.run(f'oc logs {args.pod_name} > {args.log_file} 2>&1')
    myCmds.close()
    sys.exit(status)
