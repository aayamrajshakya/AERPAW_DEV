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
import os
import re
import sys

from common.python.generate_html import (
    generate_header,
    generate_footer,
    generate_chapter,
    generate_button_header,
    generate_button_footer,
    generate_image_table_header,
    generate_image_table_footer,
    generate_image_table_row,
    generate_image_table_separator,
    generate_list_header,
    generate_list_footer,
    generate_list_row,
    generate_list_sub_header,
    generate_list_sub_footer,
    generate_list_sub_row,
)

REPORT_NAME = 'test_results_oai_cn5g_ngap_tester.html'

class HtmlReport():
    def __init__(self):
        pass

    def generate(self, args):
        cwd = os.getcwd()
        status = True
        with open(os.path.join(cwd, REPORT_NAME), 'w') as wfile:
            wfile.write(generate_header(args))
            (status, testSummary) = self.testSummary('NGAP-Tester')
            wfile.write(testSummary)
            wfile.write(generate_footer())
        if status:
            sys.exit(0)
        else:
            sys.exit(-1)

    def testSummary(self, testName):
        cwd = os.getcwd()

        if not os.path.isdir(cwd + '/archives/'):
            return ''

        log_files = sorted(os.listdir(cwd + '/archives/'))
        deployedContainerImages = []
        for log_file in log_files:
            if not log_file.endswith(".log"):
                continue
            if re.search('image-info', log_file) is not None:
                continue
            containerName = re.sub('.log.*$', '', log_file)
            imageRootName = f'{containerName}:'
            fileRootName = containerName
            imageTag = ''
            imageSize = ''
            imageDate = ''
            if not os.path.isfile(cwd + '/archives/' + fileRootName + '-image-info.log'):
                continue
            with open(cwd + '/archives/' + fileRootName + '-image-info.log','r') as imageDetailsFile:
                for line in imageDetailsFile:
                    result = re.search('Tested Tag is .*.*:(?P<tag>[a-zA-Z0-9\-\_]+)', line)
                    if result is not None:
                        imageTag = result.group('tag')
                    result = re.search('Size = (?P<size>[0-9]+) bytes', line)
                    if result is not None:
                        sizeInt = int(result.group('size'))
                        if sizeInt < 1000000:
                            sizeInt = int(sizeInt / 1000)
                            imageSize = str(sizeInt) + ' kB'
                        else:
                            sizeInt = int(sizeInt / 1000000)
                            imageSize = str(sizeInt) + ' MB'
                    result = re.search('Date = (?P<date>[a-zA-Z0-9\-\_:]+)', line)
                    if result is not None:
                        imageDate = re.sub('T', '  ', result.group('date'))
            imageDetailsFile.close()
            deployedContainerImages.append((containerName, f'{imageRootName}{imageTag}', imageSize, imageDate))

        mandatoryTests = []
        if os.path.isfile(cwd + '/ci-scripts/docker-compose/ngap-tester/list-mandatory.txt'):
            print('found the list of mandatory tests')
            with open(cwd + '/ci-scripts/docker-compose/ngap-tester/list-mandatory.txt','r') as mandatoryFile:
                for line in mandatoryFile:
                    mandatoryTests.append(line.strip())

        testCaseDetails = []
        globalStatus = True
        globalMandatoryStatus = True
        for log_file in log_files:
            if not log_file.endswith(".log"):
                continue
            if re.search('TC', log_file) is None:
                continue
            testCaseEnded = False
            testCaseStatus = False
            testCaseName = re.sub('.log.*$', '', log_file)
            stringStatus = 'UNKNOWN'
            description  = 'UNKNOWN'
            if len(mandatoryTests) > 0:
                try:
                    index = mandatoryTests.index(testCaseName)
                    mandatory = True
                except:
                    mandatory = False
            else:
                mandatory = False
            with open(cwd + '/archives/' + log_file,'r') as imageDetailsFile:
                for line in imageDetailsFile:
                    result = re.search('Scenario *: Status *: Description', line)
                    if result is not None:
                        testCaseEnded = True
                    result = re.search(f'{testCaseName} *: (?P<status>[A-Z]+) *: (?P<description>.*$)', line)
                    if result is not None and testCaseEnded:
                        if result.group('status') == 'PASSED':
                            testCaseStatus = True
                        stringStatus = result.group('status')
                        description = result.group('description')
                        description = re.sub('NOT YET VALIDATED - ', '', description)
                        description = re.sub('NOT YET VALIDATED, HAVE TO BE IMPLEMENTED IN OAI CN -', '', description)
            if (not testCaseEnded or not testCaseStatus):
                globalStatus = False
                if mandatory:
                    globalMandatoryStatus = False
            testCaseDetails.append((testCaseName, testCaseStatus, mandatory, stringStatus, description))
        testDetails = ''
        if globalStatus:
            message = f'All Tests Passed'
        elif globalMandatoryStatus and len(mandatoryTests) > 0:
            message = f'All Mandatory Tests Passed'
        else:
            message = f'Some Tests Failed'
        if len(mandatoryTests) > 0:
            testDetails += generate_chapter(f'Load Test Summary for {testName}', message, globalMandatoryStatus)
        else:
            testDetails += generate_chapter(f'Load Test Summary for {testName}', message, globalStatus)
        testDetails += generate_button_header(f'tc-suite-details', 'More details on ngap-tester results')
        testDetails += generate_image_table_header()
        for (cName,iTag,iSize,iDate) in deployedContainerImages:
            testDetails += generate_image_table_row(cName, iTag, 'N/A', iDate, iSize)
            if cName == 'ngap-tester':
                testDetails += generate_image_table_separator()
        testDetails += generate_image_table_footer()
        testDetails += generate_list_header()
        for (name, status, mandatory, stringStatus, description) in testCaseDetails:
            if mandatory and len(mandatoryTests) > 0:
                name += ' <span class="label label-default">MANDATORY</span>'
            if status:
                testDetails += generate_list_row(f'{name}', 'info-sign')
            else:
                testDetails += generate_list_row(f'{name}', 'remove-sign')
            testDetails += generate_list_sub_header()
            if status:
                testDetails += generate_list_sub_row(f'{description}', stringStatus, 'primary')
            else:
                testDetails += generate_list_sub_row(f'{description}', stringStatus, 'danger')
            testDetails += generate_list_sub_footer()
        testDetails += generate_list_footer()
        testDetails += generate_list_row(f'Logs on private CI server at `oaicicd@selfix:/opt/ngap-tester-logs/cn5g_fed-{args.job_name}-{args.job_id}.zip`', 'info-sign')
        testDetails += generate_button_footer()

        # If there is no mandatory list, the test is always passing
        if len(mandatoryTests) == 0:
            globalMandatoryStatus = True

        return (globalMandatoryStatus, testDetails)

def _parse_args() -> argparse.Namespace:
    """Parse the command line args

    Returns:
        argparse.Namespace: the created parser
    """
    example_text = '''example:
        ./ci-scripts/checkNgapTesterHtmlReport.py --help
        ./ci-scripts/checkNgapTesterHtmlReport.py --job_name NameOfPipeline --job_id BuildNumber --job_url BuildURL'''

    parser = argparse.ArgumentParser(description='OAI 5G CORE NETWORK NGAP-Tester HTML report',
                                    epilog=example_text,
                                    formatter_class=argparse.RawDescriptionHelpFormatter)

    # Job Name
    parser.add_argument(
        '--job_name', '-n',
        action='store',
        help='Pipeline is called JOB_NAME',
    )

    # Job Build Id
    parser.add_argument(
        '--job_id', '-id',
        action='store',
        help='Build # JOB_ID',
    )

    # Job URL
    parser.add_argument(
        '--job_url', '-u',
        action='store',
        help='Pipeline provides an URL for this run',
    )

    return parser.parse_args()

#--------------------------------------------------------------------------------------------------------
#
# Start of main
#
#--------------------------------------------------------------------------------------------------------

if __name__ == '__main__':
    # Parse the arguments
    args = _parse_args()

    # Generate report
    HTML = HtmlReport()
    HTML.generate(args)
