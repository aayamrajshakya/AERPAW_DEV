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

import os
import common.python.cls_cmd as cls_cmd
from common.python.generate_html import (
    generate_chapter,
    generate_button_header,
    generate_button_footer,
    generate_list_header,
    generate_list_footer,
    generate_list_row,
)

def coding_formatting_log_check(args):
    cwd = os.getcwd()
    details = ''
    chapterName = 'OAI Coding / Formatting Guidelines Check'
    if os.path.isfile(f'{cwd}/src/oai_rules_result.txt'):
        myCmd = cls_cmd.LocalCmd()
        cmd = f'grep NB_FILES_FAILING_CHECK {cwd}/src/oai_rules_result.txt | sed -e "s#NB_FILES_FAILING_CHECK=##"'
        nbFailRet = myCmd.run(cmd)
        cmd = f'grep NB_FILES_CHECKED {cwd}/src/oai_rules_result.txt | sed -e "s#NB_FILES_CHECKED=##"'
        nbTotalRet = myCmd.run(cmd)
        myCmd.close()
        if int(nbFailRet.stdout) == 0:
            if args.git_merge_request:
                message = f'All modified files in Merge-Request follow OAI rules. ({nbTotalRet.stdout} were checked)'
            else:
                message = f'All files in repository follow OAI rules. ({nbTotalRet.stdout} were checked)'
        else:
            if args.git_merge_request:
                message = f'{nbFailRet.stdout} modified files in Merge-Request DO NOT follow OAI rules. ({nbTotalRet.stdout} were checked)'
            else:
                message = f'{nbFailRet.stdout} files in repository DO NO follow OAI rules. ({nbTotalRet.stdout} were checked)'
        details += generate_chapter(chapterName, message, (int(nbFailRet.stdout) == 0))

        if os.path.isfile(f'{cwd}/src/oai_rules_result_list.txt'):
            details += generate_button_header('oai-formatting-details', 'More details on formatting check')
            details += '  <p>Please apply the following command to this(ese) file(s): </p>\n'
            details += '  <p style="margin-left: 30px"><strong><code>cd src && clang-format -i filename(s)</code></strong></p>\n'
            details += generate_list_header()
            with open(cwd + '/src/oai_rules_result_list.txt', 'r') as filelist:
                for line in filelist:
                    details += generate_list_row(line.strip(), 'indent-left')
            details += generate_list_footer()
            details += generate_button_footer()
    else:
        details += generate_chapter(chapterName, 'Was NOT performed (with CLANG-FORMAT tool).', False)

    return details
