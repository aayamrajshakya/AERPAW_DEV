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

HEADER_TEMPLATE = 'ci-scripts/common/html-templates/file-header.htm'
FOOTER_TEMPLATE = 'ci-scripts/common/html-templates/file-footer.htm'
CHAPT_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/chapter-header.htm'
SUBCHAPT_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/sub-chapter-header.htm'
BUTTON_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/button-header.htm'
BUTTON_FOOTER_TEMPLATE = 'ci-scripts/common/html-templates/button-footer.htm'
IMAGE_TABLE_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/image-table-header.htm'
IMAGE_TABLE_FOOTER_TEMPLATE = 'ci-scripts/common/html-templates/image-table-footer.htm'
IMAGE_TABLE_ROW_TEMPLATE = 'ci-scripts/common/html-templates/image-table-row.htm'
IMAGE_TABLE_SEP_TEMPLATE = 'ci-scripts/common/html-templates/image-table-sep.htm'
COMMAND_TABLE_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/command-table-header.htm'
COMMAND_TABLE_FOOTER_TEMPLATE = 'ci-scripts/common/html-templates/command-table-footer.htm'
COMMAND_TABLE_ROW_TEMPLATE = 'ci-scripts/common/html-templates/command-table-row.htm'
LIST_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/list-header.htm'
LIST_FOOTER_TEMPLATE = 'ci-scripts/common/html-templates/list-footer.htm'
LIST_ROW_TEMPLATE = 'ci-scripts/common/html-templates/list-row.htm'
LIST_SUB_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/list-sub-header.htm'
LIST_SUB_FOOTER_TEMPLATE = 'ci-scripts/common/html-templates/list-sub-footer.htm'
LIST_SUB_ROW_TEMPLATE = 'ci-scripts/common/html-templates/list-sub-row.htm'
GIT_INFO_PUSH_TEMPLATE = 'ci-scripts/common/html-templates/git-info-table-push.htm'
GIT_INFO_MR_TEMPLATE = 'ci-scripts/common/html-templates/git-info-table-mr.htm'
CPPCHECK_TABLE_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/cppcheck-table-header.htm'
CPPCHECK_TABLE_FOOTER_TEMPLATE = 'ci-scripts/common/html-templates/cppcheck-table-footer.htm'
CPPCHECK_TABLE_ROW_TEMPLATE = 'ci-scripts/common/html-templates/cppcheck-table-row.htm'
BUILD_TABLE_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/build-table-header.htm'
BUILD_TABLE_FOOTER_TEMPLATE = 'ci-scripts/common/html-templates/build-table-footer.htm'
BUILD_TABLE_ROW_TEMPLATE = 'ci-scripts/common/html-templates/build-table-row.htm'
BUILD_TABLE_DOUBLE_ROW_TEMPLATE = 'ci-scripts/common/html-templates/build-table-double-row.htm'
WARNING_TABLE_HEADER_TEMPLATE = 'ci-scripts/common/html-templates/warning-table-header.htm'
WARNING_TABLE_FOOTER_TEMPLATE = 'ci-scripts/common/html-templates/warning-table-footer.htm'
WARNING_TABLE_ROW_TEMPLATE = 'ci-scripts/common/html-templates/warning-table-row.htm'

import os
import re

def generate_header(args):
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
        if hasattr(args, 'job_name'):
            header = re.sub('JOB_NAME', args.job_name, header)
        if hasattr(args, 'job_id'):
            header = re.sub('BUILD_ID', args.job_id, header)
        if hasattr(args, 'build_id'):
            header = re.sub('BUILD_ID', args.build_id, header)
        if hasattr(args, 'job_url'):
            header = re.sub('BUILD_URL', args.job_url, header)
        if hasattr(args, 'build_url'):
            header = re.sub('BUILD_URL', args.build_url, header)
    return header

def generate_git_info(args):
    cwd = os.getcwd()
    header = ''
    fileToUse = GIT_INFO_PUSH_TEMPLATE
    isMr = False
    if hasattr(args, 'git_merge_request'):
        if args.git_merge_request:
            fileToUse = GIT_INFO_MR_TEMPLATE
            isMr = True
    with open(os.path.join(cwd, fileToUse), 'r') as temp:
        header = temp.read()
        if hasattr(args, 'git_src_branch'):
            header = re.sub('SRC_BRANCH', args.git_src_branch, header)
        if hasattr(args, 'git_src_commit'):
            header = re.sub('SRC_COMMIT', args.git_src_commit, header)
        if hasattr(args, 'git_url'):
            header = re.sub('GIT_URL', args.git_url, header)
        if isMr:
            if hasattr(args, 'git_dst_branch'):
                header = re.sub('DST_BRANCH', args.git_dst_branch, header)
            if hasattr(args, 'git_dst_commit'):
                header = re.sub('DST_COMMIT', args.git_dst_commit, header)
    return header

def generate_footer():
    cwd = os.getcwd()
    footer = ''
    with open(os.path.join(cwd, FOOTER_TEMPLATE), 'r') as temp:
        footer = temp.read()
    return footer

def generate_chapter(name, message, status):
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, CHAPT_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
        header = re.sub('CHAPTER_NAME', name, header)
        if status:
            header = re.sub('ALERT_LEVEL', 'success', header)
        else:
            header = re.sub('ALERT_LEVEL', 'danger', header)
        header = re.sub('MESSAGE', message, header)
    return header

def generate_subchapter(name):
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, SUBCHAPT_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
        header = re.sub('CHAPTER_NAME', name, header)
    return header

def generate_button_header(name, message):
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, BUTTON_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
        header = re.sub('BUTTON_NAME', name, header)
        header = re.sub('BUTTON_MESSAGE', message, header)
    return header

def generate_button_footer():
    cwd = os.getcwd()
    footer = ''
    with open(os.path.join(cwd, BUTTON_FOOTER_TEMPLATE), 'r') as temp:
        footer = temp.read()
    return footer

def generate_image_table_header():
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, IMAGE_TABLE_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
    return header

def generate_image_table_footer():
    cwd = os.getcwd()
    footer = ''
    with open(os.path.join(cwd, IMAGE_TABLE_FOOTER_TEMPLATE), 'r') as temp:
        footer = temp.read()
    return footer

def generate_image_table_row(name, tag, ocTag, creationDate, size):
    cwd = os.getcwd()
    row = ''
    with open(os.path.join(cwd, IMAGE_TABLE_ROW_TEMPLATE), 'r') as temp:
        row = temp.read()
        row = re.sub('CONTAINER_NAME', name, row)
        row = re.sub('IMAGE_TAG', tag, row)
        row = re.sub('OC_TAG', ocTag, row)
        row = re.sub('CREATION_DATE', creationDate, row)
        row = re.sub('IMAGE_SIZE', size, row)
    return row

def generate_image_table_separator():
    cwd = os.getcwd()
    row = ''
    with open(os.path.join(cwd, IMAGE_TABLE_SEP_TEMPLATE), 'r') as temp:
        row = temp.read()
    return row

def generate_command_table_header():
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, COMMAND_TABLE_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
    return header

def generate_command_table_footer():
    cwd = os.getcwd()
    footer = ''
    with open(os.path.join(cwd, COMMAND_TABLE_FOOTER_TEMPLATE), 'r') as temp:
        footer = temp.read()
    return footer

def generate_command_table_row(command, status):
    cwd = os.getcwd()
    row = ''
    with open(os.path.join(cwd, COMMAND_TABLE_ROW_TEMPLATE), 'r') as temp:
        row = temp.read()
        row = re.sub('COMMAND', command, row)
        if status:
            row = re.sub('STATUS', 'PASS', row)
            row = re.sub('COLOR', 'lightgreen', row)
        else:
            row = re.sub('STATUS', 'FAIL', row)
            row = re.sub('COLOR', 'lightcoral', row)
    return row

def generate_list_header():
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, LIST_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
    return header

def generate_list_footer():
    cwd = os.getcwd()
    footer = ''
    with open(os.path.join(cwd, LIST_FOOTER_TEMPLATE), 'r') as temp:
        footer = temp.read()
    return footer

def generate_list_row(message, iconName):
    cwd = os.getcwd()
    row = ''
    with open(os.path.join(cwd, LIST_ROW_TEMPLATE), 'r') as temp:
        row = temp.read()
        row = re.sub('ROW_MESSAGE', message, row)
        row = re.sub('ICON_NAME', iconName, row)
    return row

def generate_list_sub_header():
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, LIST_SUB_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
    return header

def generate_list_sub_footer():
    cwd = os.getcwd()
    footer = ''
    with open(os.path.join(cwd, LIST_SUB_FOOTER_TEMPLATE), 'r') as temp:
        footer = temp.read()
    return footer

# bagdeColor can be 'primary', 'secondary', 'success', 'danger', 'warning', 'info', 'light', 'dark'
def generate_list_sub_row(message, nbInBadge, bagdeColor):
    cwd = os.getcwd()
    row = ''
    with open(os.path.join(cwd, LIST_SUB_ROW_TEMPLATE), 'r') as temp:
        row = temp.read()
        row = re.sub('ROW_MESSAGE', message, row)
        row = re.sub('NUMBER', nbInBadge, row)
        row = re.sub('LEVEL', bagdeColor, row)
    return row

def generate_cppcheck_table_header():
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, CPPCHECK_TABLE_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
    return header

def generate_cppcheck_table_footer():
    cwd = os.getcwd()
    footer = ''
    with open(os.path.join(cwd, CPPCHECK_TABLE_FOOTER_TEMPLATE), 'r') as temp:
        footer = temp.read()
    return footer

def generate_cppcheck_table_row(cppcheckErr):
    cwd = os.getcwd()
    row = ''
    if cppcheckErr.severity == 'error':
        severityColor = 'Tomato'
    else:
        severityColor = 'Orange'
    with open(os.path.join(cwd, CPPCHECK_TABLE_ROW_TEMPLATE), 'r') as temp:
        row = temp.read()
        row = re.sub('ERROR_TYPE', cppcheckErr.severity, row)
        row = re.sub('SEVERITY_COLOR', severityColor, row)
        row = re.sub('ERROR_MESSAGE', cppcheckErr.message, row)
        row = re.sub('FILENAME', cppcheckErr.filename, row)
        row = re.sub('LINE_NUMBER', cppcheckErr.lineNb, row)
    return row

def generate_build_table_header(nfName, ubuntuVersion, rhelVersion):
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, BUILD_TABLE_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
        header = re.sub('NF_NAME', nfName, header)
        header = re.sub('UBUNTU_VERSION', ubuntuVersion, header)
        header = re.sub('RHEL_VERSION', rhelVersion, header)
    return header

def generate_build_table_footer():
    cwd = os.getcwd()
    footer = ''
    with open(os.path.join(cwd, BUILD_TABLE_FOOTER_TEMPLATE), 'r') as temp:
        footer = temp.read()
    return footer

def generate_build_table_row(stageName, imageKind, ubuntuMessage, rhelMessage):
    cwd = os.getcwd()
    row = ''
    if re.search('^OK', ubuntuMessage) is not None:
        ubuntuColor = 'LimeGreen'
    else:
        ubuntuColor = 'Tomato'
    if re.search('^OK', rhelMessage) is not None:
        rhelColor = 'LimeGreen'
    else:
        rhelColor = 'Tomato'
    with open(os.path.join(cwd, BUILD_TABLE_ROW_TEMPLATE), 'r') as temp:
        row = temp.read()
        row = re.sub('STAGE_NAME', stageName, row)
        row = re.sub('IMAGE_KIND', imageKind, row)
        row = re.sub('UBUNTU_MESSAGE', ubuntuMessage, row)
        row = re.sub('UBUNTU_COLOR', ubuntuColor, row)
        row = re.sub('RHEL_MESSAGE', rhelMessage, row)
        row = re.sub('RHEL_COLOR', rhelColor, row)
    return row

def generate_build_table_double_row(stageName, imageKind, ubuntuMessage0, ubuntuMessage1, rhelMessage0, rhelMessage1):
    cwd = os.getcwd()
    row = ''
    if re.search('^OK', ubuntuMessage0) is not None:
        ubuntu0Color = 'LimeGreen'
    else:
        ubuntu0Color = 'Tomato'
    if re.search('^Perfect', ubuntuMessage1) is not None:
        ubuntu1Color = 'LimeGreen'
    elif re.search('^OK', ubuntuMessage1) is not None:
        ubuntu1Color = 'Chartreuse'
    elif re.search('^ToBeImproved', ubuntuMessage1) is not None:
        ubuntu1Color = 'Orange'
    else:
        ubuntu1Color = 'Tomato'
    if re.search('^OK', rhelMessage0) is not None:
        rhel0Color = 'LimeGreen'
    else:
        rhel0Color = 'Tomato'
    if re.search('^Perfect', rhelMessage1) is not None:
        rhel1Color = 'LimeGreen'
    elif re.search('^OK', rhelMessage1) is not None:
        rhel1Color = 'Chartreuse'
    elif re.search('^ToBeImproved', rhelMessage1) is not None:
        rhel1Color = 'Orange'
    else:
        rhel1Color = 'Tomato'
    with open(os.path.join(cwd, BUILD_TABLE_DOUBLE_ROW_TEMPLATE), 'r') as temp:
        row = temp.read()
        row = re.sub('STAGE_NAME', stageName, row)
        row = re.sub('IMAGE_KIND', imageKind, row)
        row = re.sub('UBUNTU0_MESSAGE', ubuntuMessage0, row)
        row = re.sub('UBUNTU0_COLOR', ubuntu0Color, row)
        row = re.sub('RHEL0_MESSAGE', rhelMessage0, row)
        row = re.sub('RHEL0_COLOR', rhel0Color, row)
        row = re.sub('UBUNTU1_MESSAGE', ubuntuMessage1, row)
        row = re.sub('UBUNTU1_COLOR', ubuntu1Color, row)
        row = re.sub('RHEL1_MESSAGE', rhelMessage1, row)
        row = re.sub('RHEL1_COLOR', rhel1Color, row)
    return row

def generate_warning_table_header():
    cwd = os.getcwd()
    header = ''
    with open(os.path.join(cwd, WARNING_TABLE_HEADER_TEMPLATE), 'r') as temp:
        header = temp.read()
    return header

def generate_warning_table_footer():
    cwd = os.getcwd()
    footer = ''
    with open(os.path.join(cwd, WARNING_TABLE_FOOTER_TEMPLATE), 'r') as temp:
        footer = temp.read()
    return footer

def generate_warning_table_row(filename, lineNumber, status, message):
    cwd = os.getcwd()
    row = ''
    if re.search('warning', status) is not None:
        kindColor = 'Orange'
    else:
        kindColor = 'Tomato'
    with open(os.path.join(cwd, WARNING_TABLE_ROW_TEMPLATE), 'r') as temp:
        row = temp.read()
        row = re.sub('FILENAME', filename, row)
        row = re.sub('LINENUMBER', lineNumber, row)
        row = re.sub('STATUS_COLOR', kindColor, row)
        row = re.sub('STATUS_KIND', status, row)
        row = re.sub('STATUS_MESSAGE', message, row)
    return row

