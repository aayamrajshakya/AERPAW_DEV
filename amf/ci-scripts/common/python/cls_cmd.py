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
import subprocess as sp

class LocalCmd():
    def __init__(self, d = None):
        self.cwd = d
        self.cp = sp.CompletedProcess(args='', returncode=0, stdout='')

    def run(self, line, timeout=300, silent=False, reportNonZero=True):
        if not silent:
            logging.info(line)
        try:
            ret = sp.run(line, shell=True, cwd=self.cwd, stdout=sp.PIPE, stderr=sp.STDOUT, timeout=timeout)
        except Exception as e:
            ret = sp.CompletedProcess(args=line, returncode=255, stdout=f'Exception: {str(e)}'.encode('utf-8'))
        if ret.stdout is None:
            ret.stdout = b''
        ret.stdout = ret.stdout.decode('utf-8').strip()
        if reportNonZero and ret.returncode != 0:
            logging.warning(f'command "{line}" returned non-zero returncode {ret.returncode}: output:\n{ret.stdout}')
        self.cp = ret
        return ret

    def close(self):
        pass
