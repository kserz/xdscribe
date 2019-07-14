# - Measure performance of xdscribe different algorithms

cmake_minimum_required(VERSION 3.10...3.14)

if(NOT SELECTED_METHODS)
    # ns gives constantly better performance among NLopt-based methods
    # when searching for proven solution
    set(SELECTED_METHODS "gfhbrg" "gfhbrl" "gfpbrbrg" "hs" "ns" "nr")
endif()

if(NOT SELECTED_FIELDS)
    set(SELECTED_FIELDS "runtime" "objective_call")
endif()

# Adds a path to the captured output to REPORTS list
function(add_xdscribe_run PATTERN CONTOUR PRECISION METHOD TRIES)
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/reports")

    string(
        REGEX REPLACE "[/\\]" "_"
        REPORT_NAME
        "${PATTERN}_"
        "${CONTOUR}_"
        "${PRECISION}_"
        "${METHOD}"
    )
    set(REPORT_PATH "${PROJECT_BINARY_DIR}/reports/${REPORT_NAME}.txt")

    add_custom_command(
        OUTPUT "${REPORT_PATH}"
        COMMAND "${PROJECT_BINARY_DIR}/xdscribe"
        "${PROJECT_SOURCE_DIR}/${PATTERN}.obj"
        "${PROJECT_SOURCE_DIR}/${CONTOUR}.obj"
        "${PRECISION}"
        "${METHOD}"
        "${TRIES}"
        > "${REPORT_PATH}"
        DEPENDS xdscribe
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
        COMMENT
        "Running ${METHOD} ${PRECISION} on ${PATTERN}, ${CONTOUR}"
        VERBATIM
    )

    set(REPORTS ${REPORTS} "${REPORT_PATH}" PARENT_SCOPE)
endfunction()

function(add_script_run SCRIPT ARGS INPUTS OUTPUT_PATH COMMENT)
    get_filename_component(
        TARGET_DIRECTORY
        "${OUTPUT_PATH}"
        DIRECTORY
        )
    file(MAKE_DIRECTORY "${TARGET_DIRECTORY}")

    add_custom_command(
        OUTPUT "${OUTPUT_PATH}"
        COMMAND "${CMAKE_COMMAND}"
        "-P" "${SCRIPT}"
        ${ARGS}
        ${INPUTS}
        "${OUTPUT_PATH}"
        MAIN_DEPENDENCY "${SCRIPT}"
        DEPENDS ${INPUTS}
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
        COMMENT "${COMMENT}"
        VERBATIM
    )
endfunction()

# Adds a base file names of comparison lines to COMPARISONS list
# Actual field comparison file paths are "<base_filename>_<field>s.txt"
function(add_methods_comparison PATTERN CONTOUR STOPPING TRIES)
    set(REPORTS "")
    foreach(METHOD ${SELECTED_METHODS})
        add_xdscribe_run(
            "${PATTERN}"
            "${CONTOUR}"
            "${STOPPING}"
            "${METHOD}"
            "${TRIES}")
    endforeach()

    string(
        REGEX REPLACE "[/\\]" "_"
        COMPARISON_NAME
        "${PATTERN}_"
        "${CONTOUR}_"
        "${STOPPING}"
    )
    set(
        COMPARISON_BASE_PATH
        "${PROJECT_BINARY_DIR}/reports/${COMPARISON_NAME}"
    )

    foreach(FIELD ${SELECTED_FIELDS})
        add_script_run(
            "${PROJECT_SOURCE_DIR}/cmake/compare_${FIELD}s.cmake"
            "${PATTERN} ${CONTOUR} ${STOPPING}"
            "${REPORTS}"
            "${COMPARISON_BASE_PATH}_${FIELD}s.txt"
            "Comparing ${FIELD}s on ${PATTERN} ${CONTOUR} ${STOPPING}"
        )
    endforeach()

    set(COMPARISONS ${COMPARISONS} "${COMPARISON_BASE_PATH}" PARENT_SCOPE)
endfunction()

message(STATUS "Adding measure_performance target")
set(COMPARISONS "")

# ====== Easy problems
add_methods_comparison(
    "examples/box_12"
    "examples/tetrahedron_4"
    "1e-7"
    "100"
)

add_methods_comparison(
    "examples/heart_320"
    "examples/heart_320"
    "1e-3"
    "1"
)

#add_methods_comparison(
#    "examples/tetrahedron_4"
#    "examples/heart_320"
#    "1e-3"
#    "1"
#)

# ====== Small problems
add_methods_comparison(
    "examples/heart_320"
    "examples/tetra_w_shear_576"
    "1e-2"
    "1"
)
add_methods_comparison(
    "examples/heart_320"
    "examples/tetra_w_shear_576"
    "v0.26"
    "1"
)
#add_methods_comparison(
#    "examples/heart_320"
#    "examples/tetra_w_shear_576"
#    "1e-3"
#    "1"
#)
#add_methods_comparison(
#    "examples/heart_320"
#    "examples/tetra_w_shear_576"
#    "v0.269"
#    "1"
#)

add_methods_comparison(
    "examples/cuts/standard_126"
    "examples/stones/rhombic_rough_576"
    "1e-2"
    "1"
)
#add_methods_comparison(
#    "examples/cuts/standard_126"
#    "examples/stones/rhombic_rough_576"
#    "1e-3"
#    "1"
#)
#add_methods_comparison(
#    "examples/cuts/standard_126"
#    "examples/stones/rhombic_rough_576"
#    "1e-4"
#    "1"
#)
add_methods_comparison(
    "examples/cuts/standard_126"
    "examples/stones/rhombic_rough_576"
    "v1.429"
    "1"
)

# ====== Hard problems
#add_methods_comparison(
#    "examples/cuts/bstilltrue_194"
#    "examples/stones/rhombic_rough_2304"
#    "1e-2"
#    "1"
#)
#add_methods_comparison(
#    "examples/cuts/bstilltrue_194"
#    "examples/stones/rhombic_rough_2304"
#    "1e-3"
#    "1"
#)
#add_methods_comparison(
#    "examples/cuts/bstilltrue_194"
#    "examples/stones/rhombic_rough_2304"
#    "v0.312"
#    "1"
#)

#add_methods_comparison(
#    "examples/cuts/rosehrttrue_104"
#    "examples/stones/tetra_rough_912"
#    "1e-2"
#    "1"
#)
#add_methods_comparison(
#    "examples/cuts/rosehrttrue_104"
#    "examples/stones/tetra_rough_912"
#    "1e-3"
#    "1"
#)
#add_methods_comparison(
#    "examples/cuts/rosehrttrue_104"
#    "examples/stones/tetra_rough_912"
#    "v0.288"
#    "1"
#)

#add_methods_comparison(
#    "examples/cuts/1stwave_172"
#    "examples/stones/tetra_flat_3648"
#    "1e-2"
#    "1"
#)
#add_methods_comparison(
#    "examples/cuts/1stwave_172"
#    "examples/stones/tetra_flat_3648"
#    "1e-3"
#    "1"
#)
#add_methods_comparison(
#    "examples/cuts/1stwave_172"
#    "examples/stones/tetra_flat_3648"
#    "v5.089"
#    "1"
#)

#add_methods_comparison(
#    "examples/cuts/novice7_86"
#    "examples/stones/octa_rough_6432"
#    "1e-2"
#    "1"
#)
#add_methods_comparison(
#    "examples/cuts/novice7_86"
#    "examples/stones/octa_rough_6432"
#    "1e-3"
#    "1"
#)
#add_methods_comparison(
#    "examples/cuts/novice7_86"
#    "examples/stones/octa_rough_6432"
#    "v1.279"
#    "1"
#)

set(HEADER_FILE "${PROJECT_BINARY_DIR}/reports_header.txt")
FILE(WRITE "${HEADER_FILE}" "Sample \\ Method | ")
foreach(METHOD ${SELECTED_METHODS})
    FILE(APPEND "${HEADER_FILE}" "${METHOD} | ")
endforeach()
file(APPEND "${HEADER_FILE}" "\n --- | ")
foreach(METHOD ${SELECTED_METHODS})
    FILE(APPEND "${HEADER_FILE}" "--- | ")
endforeach()
file(APPEND "${HEADER_FILE}" "\n")

set(TARGET_DEPENDS "")
foreach(FIELD ${SELECTED_FIELDS})
    set(FILES ${HEADER_FILE})

    foreach(COMPARISON_BASE_PATH ${COMPARISONS})
        set(FILES ${FILES} "${COMPARISON_BASE_PATH}_${FIELD}s.txt")
    endforeach()

    set(REPORT_PATH "${PROJECT_BINARY_DIR}/reports/${FIELD}s.txt")

    add_script_run(
        "${PROJECT_SOURCE_DIR}/cmake/merge_files.cmake"
        ""
        "${FILES}"
        "${REPORT_PATH}"
        "Merging ${FIELD}s from all reports"
    )

    set(TARGET_DEPENDS ${TARGET_DEPENDS} "${REPORT_PATH}")
endforeach()

add_custom_target(
    measure_performance
    DEPENDS ${TARGET_DEPENDS}
)
