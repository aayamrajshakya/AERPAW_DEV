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
import re
import common.python.cls_cmd as cls_cmd
from common.python.generate_html import (
    generate_chapter,
    generate_button_header,
    generate_button_footer,
    generate_cppcheck_table_header,
    generate_cppcheck_table_footer,
    generate_cppcheck_table_row,
)

class CppCheckError():
    def __init__(self):
        self.severity=''
        self.message=''
        self.filename=''
        self.lineNb=''

def analyze_sca_log_check():
    cwd = os.getcwd()
    details = ''
    chapterName = 'Static Code Analysis'
    if os.path.isfile(cwd + '/archives/cppcheck.xml'):
        nb_errors = 0
        nb_warnings = 0
        fullDesc = False
        listOfErrors = []
        myError = CppCheckError()
        with open(f'{cwd}/archives/cppcheck.xml', 'r') as xmlfile:
            for line in xmlfile:
                if re.search('severity="warning"', line) is not None:
                    nb_warnings += 1
                if re.search('severity="error"', line) is not None:
                    nb_errors += 1
                result = re.search('severity="(?P<severity>[a-z]+)" msg="(?P<message>.*)" verbose="', line)
                if result is not None:
                    myError = CppCheckError()
                    fullDesc = True
                    myError.severity=result.group('severity')
                    myError.message=result.group('message')
                # Case with only one file
                result = re.search('location file="(?P<filename>.*)" line="(?P<lineNb>[0-9]+)" column="', line)
                if result is not None and fullDesc:
                    myError.filename=result.group('filename')
                    myError.lineNb=result.group('lineNb')
                    listOfErrors.append(myError)
                    fullDesc = False
                # Case with two files
                result = re.search('location file0=.* file="(?P<filename>.*)" line="(?P<lineNb>[0-9]+)" column="', line)
                if result is not None and fullDesc:
                    myError.filename=result.group('filename')
                    myError.lineNb=result.group('lineNb')
                    listOfErrors.append(myError)
                    fullDesc = False

        if (nb_errors == 0) and (nb_warnings == 0):
            details += generate_chapter(chapterName, 'CPPCHECK found NO error and NO warning.', True)
        elif nb_errors == 0:
            details += generate_chapter(chapterName, f'CPPCHECK found NO error and {nb_warnings} warning(s).', True)
        else:
            details += generate_chapter(chapterName, f'CPPCHECK found {nb_errors} error(s) and {nb_warnings} warning(s).', False)

        if (nb_errors > 0) or (nb_warnings > 0):
            details += generate_button_header('oai-cppcheck-details', 'More details on CPPCHECK results')
            details += generate_cppcheck_table_header()
            for myError in listOfErrors:
                details += generate_cppcheck_table_row(myError)
            details += generate_cppcheck_table_footer()
            details += generate_button_footer()
    else:
        details += generate_chapter(chapterName, 'Was NOT performed (with CPPCHECK tool).', False)
    return details
