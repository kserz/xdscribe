# - Script template to gather information from multiple xdscribe reports
#
# This file should be included from the actual script setting REPORT_REGEX
# variable to some regular expression. The regular expression is expected
# to match first subexpression with the needed value.
#
# The resulting script is used as follows:
#
# User must provide the following arguments one by one on the command line:
#     prefix string
#     input reports file names
#     output file name
#
# The script then writes to the output file a TAB-separated line containing
# all the values gathered from input reports prepended by the prefix string

cmake_minimum_required(VERSION 3.10...3.14)

if(NOT REPORT_REGEX)
    message(STATUS "Source ${PROJECT_SOURCE_DIR}")
    message(
        FATAL_ERROR
        "This is a script template, not to be used directly!\n"
        "You should set REPORT_REGEX variable "
        "in order to get the working script.")
endif()

# We skip "cmake -P <script_name>" and proceed to the arguments
if(${CMAKE_ARGC} LESS 5)
    message(
        FATAL_ERROR
        "Please specify prefix string and output file name on the command line"
    )
endif()

math(EXPR OUTPUT_NUM "${CMAKE_ARGC}-1")
set(OUTPUT_PATH "${CMAKE_ARGV${OUTPUT_NUM}}")

file(WRITE "${OUTPUT_PATH}" "${CMAKE_ARGV3} | ")

if(${CMAKE_ARGC} GREATER 5)
    math(EXPR LAST_INPUT_NUM "${CMAKE_ARGC}-2")
    foreach(INPUT_NUM RANGE 4 ${LAST_INPUT_NUM})
        set(FILENAME "${CMAKE_ARGV${INPUT_NUM}}")
        file(
            STRINGS
            "${FILENAME}"
            RESULT
            REGEX ${REPORT_REGEX}
#            REGEX "^Runtime of a single try:.*$"
        )
        string(
            REGEX REPLACE ${REPORT_REGEX} "\\1"
#            REGEX REPLACE "^Runtime.*:\ " ""
            RUNNING_TIME
            "${RESULT}"
        )
        file(APPEND "${OUTPUT_PATH}" "${RUNNING_TIME} | ")
    endforeach()
endif()

file(APPEND "${OUTPUT_PATH}" "\n")
