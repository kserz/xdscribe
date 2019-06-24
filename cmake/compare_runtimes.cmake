# - Script comparing running times from multiple xdscribe reports
#
# User must provide the following arguments one by one on the command line:
#     prefix string
#     input reports file names
#     output file name
#
# This script writes to the output file a TAB-separated line containing
# all the values gathered from input reports prepended by the prefix string

cmake_minimum_required(VERSION 3.10...3.14)

get_filename_component(
    SCRIPTS_DIRECTORY
    "${CMAKE_SCRIPT_MODE_FILE}"
    DIRECTORY
)

set(REPORT_REGEX "^Runtime of a single try: *([0-9.]+)$")

include("${SCRIPTS_DIRECTORY}/reports_comparison_line.cmake")

