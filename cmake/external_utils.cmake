# - Helper functions for adding external libraries
#
# User must set EXTERNAL_CACHE_DIR to a directory where to place sources

cmake_minimum_required(VERSION 3.10...3.14)

if(NOT EXTERNAL_CACHE_DIR)
    message(
        FATAL_ERROR
        "Please set EXTERNAL_CACHE_DIR before including external_utils")
endif()

find_package(Git)
if(NOT GIT_FOUND)
    message(FATAL_ERROR "Could not find git executable")
endif()

# Shallow clones of a specified branch are tricky in git submodules,
# so we get external sources directly
function(get_external_source NAME SOURCE_URL BRANCH PATCH_FILE)
    if(NOT EXISTS "${EXTERNAL_CACHE_DIR}/${NAME}")
        message(STATUS "Checking out ${NAME}")
        file(MAKE_DIRECTORY "${EXTERNAL_CACHE_DIR}")
        execute_process(
            COMMAND
            ${GIT_EXECUTABLE} clone --depth 1
            --branch "${BRANCH}"
            -- "${SOURCE_URL}"
            "${EXTERNAL_CACHE_DIR}/${NAME}"
            RESULT_VARIABLE CLONE_STATUS
        )
        if(NOT CLONE_STATUS EQUAL "0")
            message(
                FATAL_ERROR
                "Could not clone ${NAME} from ${BRANCH} of ${SOURCE_URL}"
            )
        endif()

        if(PATCH_FILE)
            message(STATUS "Patching ${NAME}")
            execute_process(
                COMMAND
                ${GIT_EXECUTABLE} apply "${PATCH_FILE}"
                WORKING_DIRECTORY "${EXTERNAL_CACHE_DIR}/${NAME}"
                RESULT_VARIABLE PATCH_STATUS
                )
            if(NOT PATCH_STATUS EQUAL "0")
                message(
                    FATAL_ERROR
                    "Could not patch ${NAME} with ${PATCH_FILE}"
                    )
            endif()
        endif()
    endif()
endfunction()

function(add_external_lib TARGET NAME SOURCE_URL BRANCH PATCH_FILE)
    get_external_source("${NAME}" "${SOURCE_URL}" "${BRANCH}" "${PATCH_FILE}")
    add_subdirectory("${EXTERNAL_CACHE_DIR}/${NAME}")
    target_link_libraries(${TARGET} PUBLIC ${NAME})
endfunction()

