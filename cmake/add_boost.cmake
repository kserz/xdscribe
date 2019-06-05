# - Download and extract recent boost source
#
# User must set EXTERNAL_CACHE_DIR to a directory where to place result
#
# This module sets BOOST_SOURCE_DIR to a full path
# of the downloaded boost source directory.
#
# NB. Archive tarball is stored in binary directory

cmake_minimum_required(VERSION 3.10...3.14)

if(NOT EXTERNAL_CACHE_DIR)
    message(
        FATAL_ERROR
        "Please set EXTERNAL_CACHE_DIR before including add_boost")
endif()

set(BOOST_VERSION 1.70.0)
string(REPLACE "." "_" BOOST_BASE_PATH "boost_${BOOST_VERSION}")
string(
    CONCAT
    BOOST_SOURCE_URL
    "https://dl.bintray.com/boostorg/release/"
    "${BOOST_VERSION}/source/${BOOST_BASE_PATH}.tar.bz2"
)

set(BOOST_SOURCE_DIR "${EXTERNAL_CACHE_DIR}/${BOOST_BASE_PATH}")

if(NOT EXISTS "${BOOST_SOURCE_DIR}/boost/version.hpp")
    set(ARCHIVE_PATH "${PROJECT_BINARY_DIR}/${BOOST_BASE_PATH}.tar.bz2")

    if(NOT EXISTS "${ARCHIVE_PATH}")
        message(STATUS "Downloading boost")
        file(
            DOWNLOAD
            "${BOOST_SOURCE_URL}"
            "${ARCHIVE_PATH}"
            SHOW_PROGRESS
            STATUS DOWNLOAD_FULL_STATUS
            )
        list(GET DOWNLOAD_FULL_STATUS 0 DOWNLOAD_STATUS)
        list(GET DOWNLOAD_FULL_STATUS 1 DOWNLOAD_MESSAGE)
        if(NOT DOWNLOAD_STATUS EQUAL "0")
            message(
                FATAL_ERROR
                "Could not download ${BOOST_SOURCE_URL}: ${DOWNLOAD_MESSAGE}"
            )
        endif()
    endif()

    message(STATUS "Extracting boost")
    file(MAKE_DIRECTORY "${EXTERNAL_CACHE_DIR}")
    execute_process(
        COMMAND
        ${CMAKE_COMMAND} -E tar xjf
        "${ARCHIVE_PATH}"
        WORKING_DIRECTORY "${EXTERNAL_CACHE_DIR}"
        RESULT_VARIABLE EXTRACT_STATUS
    )
    if(NOT EXTRACT_STATUS EQUAL "0")
        message(
            FATAL_ERROR
            "Could not extract ${ARCHIVE_PATH}")
    endif()
endif()
