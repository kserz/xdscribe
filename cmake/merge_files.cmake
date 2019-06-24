# - Script to merge multiple files into one
#
# User must provide the following arguments one by one on the command line:
#     input reports file names
#     output file name

cmake_minimum_required(VERSION 3.10...3.14)

# We skip "cmake -P <script_name>" and proceed to the actual arguments
if(${CMAKE_ARGC} LESS 4)
    message(
        FATAL_ERROR
        "Please specify output file on the command line"
    )
endif()

math(EXPR OUTPUT_NUM "${CMAKE_ARGC}-1")
set(OUTPUT_PATH "${CMAKE_ARGV${OUTPUT_NUM}}")

file(WRITE "${OUTPUT_PATH}" "")

if(${CMAKE_ARGC} GREATER 4)
    math(EXPR LAST_INPUT_NUM "${CMAKE_ARGC}-2")
    foreach(INPUT_NUM RANGE 3 ${LAST_INPUT_NUM})
        set(FILENAME "${CMAKE_ARGV${INPUT_NUM}}")
        file(READ "${FILENAME}" CONTENTS)
        file(APPEND "${OUTPUT_PATH}" "${CONTENTS}")
    endforeach()
endif()
