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
from datetime import datetime
import logging
import re
import sys
import common.python.cls_cmd as cls_cmd

logging.basicConfig(
    level=logging.INFO,
    stream=sys.stdout,
    format="[%(asctime)s] %(levelname)8s: %(message)s"
)

PRIVATE_LOCAL_REGISTRY_URL='https://selfix.sboai.cs.eurecom.fr:443'

def main() -> None:
    args = _parse_args()
    if args.repo_name == '5gc-gnbsim':
        tagRoot = 'main'
        nbChars = 11
    else:
        tagRoot = 'develop'
        nbChars = 15

    myCmds = cls_cmd.LocalCmd()
    cmd = f'curl --insecure -Ss -u oaicicd:oaicicd {PRIVATE_LOCAL_REGISTRY_URL}/v2/{args.repo_name}/tags/list | jq .'
    tagList = myCmds.run(cmd, silent=True)
    latestTag = ''
    latestDate = datetime.strptime('2022-01-01T00:00:00', '%Y-%m-%dT%H:%M:%S')
    for line in tagList.stdout.split('\n'):
        res = re.search(f'"(?P<tag>{tagRoot}-[0-9a-zA-Z]+)"', line)
        if res is not None:
            tag = res.group('tag')
            # on SPGWU / GitHub     `git log -1 --pretty=format:"%h"` returns 7 characters
            # on other NF / GitLab  `git log -1 --pretty=format:"%h"` returns 8 characters
            if len(tag) == nbChars or len(tag) == (nbChars+1):
                cmd = f'curl --insecure -Ss -u oaicicd:oaicicd {PRIVATE_LOCAL_REGISTRY_URL}/v2/{args.repo_name}/manifests/{tag} | jq .history'
                tagInfo = myCmds.run(cmd, silent=True)
                res2 = re.search('"created.*(?P<date>202[0-9-]\-[0-9]+-[0-9]+T[0-9]+:[0-9]+:[0-9]+).*docker_version', tagInfo.stdout)
                if res2 is not None:
                    date = datetime.strptime(res2.group('date'), '%Y-%m-%dT%H:%M:%S')
                    if date > latestDate:
                        latestDate = date
                        latestTag = tag
                res2 = re.search('"created.*(?P<date>202[0-9-]\-[0-9]+-[0-9]+T[0-9]+:[0-9]+:[0-9]+).*container_config.*WORKDIR', tagInfo.stdout)
                if res2 is not None:
                    date = datetime.strptime(res2.group('date'), '%Y-%m-%dT%H:%M:%S')
                    if date > latestDate:
                        latestDate = date
                        latestTag = tag

    #logging.info(f'Latest Tag = {latestTag} made on {latestDate}')
    if latestTag == '':
        sys.exit(-1)
    print(latestTag)
    sys.exit(0)

def _parse_args() -> argparse.Namespace:
    """Parse the command line args

    Returns:
        argparse.Namespace: the created parser
    """
    parser = argparse.ArgumentParser(description='Script to retrieve on a given image repository, the latest (newest) develop image.')
    parser.add_argument(
        '--repo-name', '-rn',
        action='store',
        help='Image Repository Name (for example oai-amf)'
    )

    return parser.parse_args()

if __name__ == '__main__':
    main()
