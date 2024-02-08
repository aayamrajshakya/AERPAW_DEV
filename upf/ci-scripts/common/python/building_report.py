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
from common.python.generate_html import (
    generate_chapter,
    generate_subchapter,
    generate_button_header,
    generate_button_footer,
    generate_build_table_header,
    generate_build_table_footer,
    generate_build_table_row,
    generate_build_table_double_row,
    generate_warning_table_header,
    generate_warning_table_footer,
    generate_warning_table_row,
)

class InstallationStatus():
    def __init__(self):
        self.fStatus=False
        self.isCached=False
        self.package_install=False
        self.fmtToBeInstalled=False
        self.fmtStatus=False
        self.spdlogToBeInstalled=False
        self.spdlogStatus=False
        self.nghttp2ToBeInstalled=False
        self.nghttp2Status=False
        self.njsonToBeInstalled=False
        self.njsonStatus=False
        self.pistacheToBeInstalled=False
        self.pistacheStatus=False
        self.yamlcppToBeInstalled=False
        self.yamlcppStatus=False
        self.lttngToBeInstalled=False
        self.lttngStatus=False
        self.cppjwtToBeInstalled=False
        self.cppjwtStatus=False
        self.libbpfToBeInstalled=False
        self.libbpfStatus=False
        self.bpftoolToBeInstalled=False
        self.bpftoolStatus=False

class BuildStatus():
    def __init__(self):
        self.fStatus=False
        self.nb_errors=0
        self.nb_warnings=0

def build_summary(args, nfName, ubuntuVersion, rhelVersion):
    cwd = os.getcwd()
    details = ''
    chapterName = 'Container Images Build Summary'
    details += generate_build_table_header(nfName.upper(), ubuntuVersion, rhelVersion)
    details += initial_base_preparation(args, nfName)
    details += nf_base_image_creation(args, nfName)
    (buildStatus, errorList) = nf_build_log_check(nfName)
    details += buildStatus
    (status, imageRow) = nf_target_image_size(nfName)
    details += imageRow
    details += generate_build_table_footer()
    if len(errorList) > 0:
        details += errorList
    if status:
        details = generate_chapter(chapterName, 'All Container Target Images were created.', True) + details
    else:
        details = generate_chapter(chapterName, 'Some/All Container Target Images were NOT created.', False) + details

    return details

def initial_base_preparation(args, nfName):
    cwd = os.getcwd()
    details = ''
    variants = ['ubuntu', 'rhel']
    messages = ['', '']
    idx = 0
    for variant in variants:
        logFileName = f'{nfName}_{variant}_image_build.log'
        if os.path.isfile(f'{cwd}/archives/{logFileName}'):
            status = False
            isCached = False
            section_status = True
            section_end_pattern = f'RUN ./build_{nfName} --install-deps --force'
            main_update_step = f'^#([0-9]+).*oai-{nfName}-base.*RUN apt-get update'
            build_stage_id = 'NotAcorrectBuildStageId'
            with open(f'{cwd}/archives/{logFileName}', 'r') as logfile:
                for line in logfile:
                    result = re.search(section_end_pattern, line)
                    if result is not None and section_status:
                        section_status = False
                        status = True
                    result = re.search(main_update_step, line)
                    if result is not None and section_status:
                        build_stage_id = result.group(1)
                    result = re.search(f'#{build_stage_id} *CACHED', line)
                    if result is not None:
                        isCached = True
                        status = True
            if status:
                messages[idx] = f'OK:\n -- All initial operations went alright'
                if isCached:
                    messages[idx] += '\n   * Some were cached'
            else:
                messages[idx] = f'KO:\n -- Some initial operations went wrong'
        else:
            messages[idx] = f'KO:\n -- logfile ({logFileName}) not found'
        idx += 1
    details += generate_build_table_row('Initial Stages', 'Base Image', messages[0], messages[1])
    return details

def nf_base_image_creation(args, nfName):
    cwd = os.getcwd()
    details = ''
    variants = ['ubuntu', 'rhel']
    messages = ['', '']
    idx = 0
    for variant in variants:
        logFileName = f'{nfName}_{variant}_image_build.log'
        if os.path.isfile(f'{cwd}/archives/{logFileName}'):
            status = InstallationStatus()
            section_start_pattern = f'RUN ./build_{nfName} --install-deps --force'
            section_end_pattern = f'{nfName.upper()} not compiled, to compile it, re-run build_{nfName} without -I option'
            section_status = False
            main_update_step = f'^#([0-9]+).*oai-{nfName}-base.*RUN ./build_{nfName} --install-deps --force'
            build_stage_id = 'NotAcorrectBuildStageId'
            with open(f'{cwd}/archives/{logFileName}', 'r') as logfile:
                for line in logfile:
                    result = re.search(section_start_pattern, line)
                    if result is not None:
                        section_status = True
                    result = re.search(section_end_pattern, line)
                    if result is not None and section_status:
                        section_status = False
                        status.fStatus = True
                    result = re.search(main_update_step, line)
                    if result is not None:
                        build_stage_id = result.group(1)
                    result = re.search(f'#{build_stage_id} *CACHED', line)
                    if result is not None:
                        status.isCached = True
                        status.fStatus = True
                        section_status = False
                    if section_status:
                        if re.search('distro libs installation complete', line) is not None:
                            status.package_install = True
                        if re.search('Starting to install fmt', line) is not None:
                            status.fmtToBeInstalled = True
                        if re.search('fmt installation complete', line) is not None and status.fmtToBeInstalled:
                            status.fmtStatus = True
                        if re.search('Starting to install spdlog', line) is not None:
                            status.spdlogToBeInstalled = True
                        if re.search('spdlog installation complete', line) is not None and status.spdlogToBeInstalled:
                            status.spdlogStatus = True
                        if re.search('Starting to install nghttp2', line) is not None:
                            status.nghttp2ToBeInstalled = True
                        if re.search('nghttp2 installation complete', line) is not None and status.nghttp2ToBeInstalled:
                            status.nghttp2Status = True
                        if re.search('Starting to install Nlohmann Json', line) is not None:
                            status.njsonToBeInstalled = True
                        if re.search('Nlohmann Json installation complete', line) is not None and status.njsonToBeInstalled:
                            status.njsonStatus = True
                        if re.search('Starting to install pistache', line) is not None:
                            status.pistacheToBeInstalled = True
                        if re.search('pistache installation complete', line) is not None and status.pistacheToBeInstalled:
                            status.pistacheStatus = True
                        if re.search('Starting to install libyaml_cpp', line) is not None:
                            status.yamlcppToBeInstalled = True
                        if re.search('yaml-cpp installation complete', line) is not None and status.yamlcppToBeInstalled:
                            status.yamlcppStatus = True
                        if re.search('Starting to install cpp_jwt', line) is not None:
                            status.cppjwtToBeInstalled = True
                        if re.search('cpp_jwt installation complete', line) is not None and status.cppjwtToBeInstalled:
                            status.cppjwtStatus = True
                        if re.search('Starting to install lttng', line) is not None:
                            status.lttngToBeInstalled = True
                        if re.search('lttng installation complete', line) is not None and status.lttngToBeInstalled:
                            status.lttngStatus = True
                        if re.search('Starting to install libbpf', line) is not None:
                            status.libbpfToBeInstalled = True
                        if re.search('libbpf installation complete', line) is not None and status.libbpfToBeInstalled:
                            status.libbpfStatus = True
                        if re.search('Starting to install bpftool', line) is not None:
                            status.bpftoolToBeInstalled = True
                        if re.search('bpftool installation complete', line) is not None and status.bpftoolToBeInstalled:
                            status.bpftoolStatus = True
            if status.fStatus:
                messages[idx] = f'OK:\n -- ./build_{nfName} --install-deps --force'
                if status.isCached:
                    messages[idx] += '\n   * was cached'
            else:
                messages[idx] = f'KO:\n -- ./build_{nfName} --install-deps --force'
            if not status.isCached:
                if status.package_install:
                    messages[idx] += '\n   * Packages Installation: OK'
                else:
                    messages[idx] += '\n   * Packages Installation: KO'
                if status.fmtToBeInstalled:
                    if status.fmtStatus:
                        messages[idx] += '\n   * fmt Installation: OK'
                    else:
                        messages[idx] += '\n   * fmt Installation: KO'
                if status.spdlogToBeInstalled:
                    if status.spdlogStatus:
                        messages[idx] += '\n   * spdlog Installation: OK'
                    else:
                        messages[idx] += '\n   * spdlog Installation: KO'
                if status.nghttp2ToBeInstalled:
                    if status.nghttp2Status:
                        messages[idx] += '\n   * nghttp2-asio Installation: OK'
                    else:
                        messages[idx] += '\n   * nghttp2-asio Installation: KO'
                if status.njsonToBeInstalled:
                    if status.njsonStatus:
                        messages[idx] += '\n   * Nlohmann-Json Installation: OK'
                    else:
                        messages[idx] += '\n   * Nlohmann-Json Installation: KO'
                if status.pistacheToBeInstalled:
                    if status.pistacheStatus:
                        messages[idx] += '\n   * pistache Installation: OK'
                    else:
                        messages[idx] += '\n   * pistache Installation: KO'
                if status.yamlcppToBeInstalled:
                    if status.yamlcppStatus:
                        messages[idx] += '\n   * yaml-cpp Installation: OK'
                    else:
                        messages[idx] += '\n   * yaml-cpp Installation: KO'
                if status.cppjwtToBeInstalled:
                    if status.cppjwtStatus:
                        messages[idx] += '\n   * cpp-jwt Installation: OK'
                    else:
                        messages[idx] += '\n   * cpp-jwt Installation: KO'
                if status.lttngToBeInstalled:
                    if status.lttngStatus:
                        messages[idx] += '\n   * lttng Installation: OK'
                    else:
                        messages[idx] += '\n   * lttng Installation: KO'
                if status.libbpfToBeInstalled:
                    if status.libbpfStatus:
                        messages[idx] += '\n   * libbpf Installation: OK'
                    else:
                        messages[idx] += '\n   * libbpf Installation: KO'
                if status.bpftoolToBeInstalled:
                    if status.bpftoolStatus:
                        messages[idx] += '\n   * bpftool Installation: OK'
                    else:
                        messages[idx] += '\n   * bpftool Installation: KO'
        else:
            messages[idx] = f'KO:\n -- logfile ({logFileName}) not found'
        idx += 1
    details += generate_build_table_row('SW libs and packages Installation', 'Base Image', messages[0], messages[1])
    return details

def nf_build_log_check(nfName):
    cwd = os.getcwd()
    details = ''
    extraDetails = ''
    variants = ['ubuntu', 'rhel']
    messages = ['', '', '', '']
    idx = 0
    errorMessages = []
    for variant in variants:
        logFileName = f'{nfName}_{variant}_image_build.log'
        if os.path.isfile(f'{cwd}/archives/{logFileName}'):
            status = BuildStatus()
            section_start_pattern = f'./build_{nfName} --clean --Verbose --build-type Release --jobs'
            section_end_pattern = f'{nfName} installed'
            section_status = False
            with open(f'{cwd}/archives/{logFileName}', 'r') as logfile:
                for line in logfile:
                    if re.search(section_start_pattern, line) is not None:
                        section_status = True
                    if re.search(section_end_pattern, line) is not None and section_status:
                        section_status = False
                        status.fStatus = True
                    if section_status:
                        if re.search('error:', line) is not None:
                            status.nb_errors += 1
                        if re.search('warning:', line) is not None:
                            status.nb_warnings += 1
                        if re.search('error:|warning:', line) is not None:
                            correctLine = re.sub(f'^.*/openair-{nfName}',f'/openair-{nfName}',line.strip())
                            wordsList = correctLine.split(None,2)
                            filename = re.sub(":[0-9]*:[0-9]*:","", wordsList[0])
                            linenumber = re.sub(filename + ':',"", wordsList[0])
                            linenumber = re.sub(':[0-9]*:',"", linenumber)
                            error_warning_status = re.sub(':',"", wordsList[1])
                            error_warning_msg = re.sub('^.*' + error_warning_status + ':', '', correctLine)
                            errorMessages.append((filename, linenumber, error_warning_status, error_warning_msg))

            if status.fStatus:
                messages[2*idx] = f'OK:\n -- build_{nfName} --clean --Verbose --build-type Release --jobs'
            else:
                messages[2*idx] = f'KO:\n -- build_{nfName} --clean --Verbose --build-type Release --jobs'
            if status.nb_errors == 0 and status.nb_warnings == 0:
                messages[2*idx+1] = 'Perfect:\n  -- 0 errors and 0 warnings found in compile log'
            elif status.nb_errors == 0 and status.nb_warnings < 20:
                messages[2*idx+1] = f'OK:\n  -- 0 errors and {status.nb_warnings} warnings found in compile log'
            elif status.nb_errors == 0:
                messages[2*idx+1] = f'ToBeImproved:\n  -- 0 errors and {status.nb_warnings} warnings found in compile log'
            else:
                messages[2*idx+1] = f'KO:\n  -- {status.nb_errors} errors and {status.nb_warnings} warnings found in compile log'
        else:
            messages[2*idxidx] = f'KO:\n -- logfile ({logFileName}) not found'
        idx += 1
    details += generate_build_table_double_row('cNF Compile / Build', 'Builder Image', messages[0], messages[1], messages[2], messages[3])
    if len(errorMessages) > 0:
        extraDetails += generate_subchapter('Compilation Warnings Details')
        extraDetails += generate_button_header('oai-compilation-details', 'Details for Compilation Errors and Warnings')
        extraDetails += generate_warning_table_header()
        for (filename, linenumber, error_warning_status, error_warning_msg) in errorMessages:
            extraDetails += generate_warning_table_row(filename, linenumber, error_warning_status, error_warning_msg)
        extraDetails += generate_warning_table_footer()
        extraDetails += generate_button_footer()
    return (details, extraDetails)

def nf_target_image_size(nfName):
    cwd = os.getcwd()
    details = ''
    variants = ['ubuntu', 'rhel']
    messages = ['', '']
    idx = 0
    imagesStatus = True
    for variant in variants:
        logFileName = f'{nfName}_{variant}_image_build.log'
        if os.path.isfile(f'{cwd}/archives/{logFileName}'):
            status = False
            imageTag = 'notAcorrectTagForTheMoment'
            if variant == 'ubuntu':
                section_start_pattern = f'naming to docker.io/library/oai-{nfName}:'
                section_end_pattern = f'OAI-{nfName.upper()} UBUNTU IMAGE BUILD'
            else:
                section_start_pattern = f'COMMIT temp.builder.openshift.io/oaicicd-core/{nfName}'
                section_end_pattern = f'OAI-{nfName.upper()} RHEL IMAGE BUILD'
            section_status = False
            with open(f'{cwd}/archives/{logFileName}', 'r') as logfile:
                for line in logfile:
                    result = re.search(f'{section_start_pattern}([0-9a-zA-Z\-\_\.]+)', line)
                    if result is not None:
                        section_status = True
                        imageTag = result.group(1)
                    result = re.search(section_end_pattern, line)
                    if result is not None and section_status:
                        section_status = False
                    if section_status:
                        result = re.search(f'oai-{nfName} *{imageTag}', line)
                        if result is not None and not status:
                            result = re.search('ago  *([0-9A-Z ]+)', line)
                            if result is not None:
                                size = result.group(1)
                                if variant == 'ubuntu':
                                    size = re.sub('MB', ' MB', size)
                                status = True
                        result = re.search('Image Size:\\t*([0-9\.]+)MB', line)
                        if result is not None and not status:
                            fSize = float(result.group(1)) * 2.6
                            size = f'~ {fSize:.1f} MB'
                            status = True
            if status:
                messages[idx] = f'OK: {size}'
            else:
                messages[idx] = f'KO:\n -- Target Image was not created'
                imagesStatus = False
        else:
            messages[idx] = f'KO:\n -- logfile ({logFileName}) not found'
            imagesStatus = False
        idx += 1
    details += generate_build_table_row('Image Size', 'Target Image', messages[0], messages[1])
    return (imagesStatus, details)
