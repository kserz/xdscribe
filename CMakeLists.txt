cmake_minimum_required(VERSION 3.10...3.14)

project(
    xdscribe
    LANGUAGES CXX
)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Ensure optimized code by default
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)

add_library(
    xdscribe_lib OBJECT

    source/geometry/kernel.h

    source/geometry/convex_decomposition/convex_decomposition.cpp
    source/geometry/convex_decomposition/convex_decomposition.h
    source/geometry/convex_decomposition/convex_part_builder.cpp
    source/geometry/convex_decomposition/convex_part_builder.h
    source/geometry/convex_decomposition/floodfill_convex_decomposition.cpp
    source/geometry/convex_decomposition/floodfill_convex_decomposition.h
    source/geometry/convex_decomposition/star_convex_decomposition.cpp
    source/geometry/convex_decomposition/star_convex_decomposition.h

    source/geometry/entity/bounding_box.h
    source/geometry/entity/perpendicular.h
    source/geometry/entity/placement.h
    source/geometry/entity/polytope.cpp
    source/geometry/entity/polytope.h

    source/geometry/location/axis_distance.cpp
    source/geometry/location/axis_distance.h
    source/geometry/location/linear_test.h
    source/geometry/location/location.cpp
    source/geometry/location/location.h
    source/geometry/location/simplex_facet_overlap.cpp
    source/geometry/location/simplex_facet_overlap.h

    source/geometry/utility/cgal.h

    source/grid/rasterization/bbox_facet_rasterizer.cpp
    source/grid/rasterization/bbox_facet_rasterizer.h
    source/grid/rasterization/facet_box_overlap.cpp
    source/grid/rasterization/facet_box_overlap.h
    source/grid/rasterization/facet_rasterizer.cpp
    source/grid/rasterization/facet_rasterizer.h
    source/grid/rasterization/inner_region_rasterizer.cpp
    source/grid/rasterization/inner_region_rasterizer.h
    source/grid/rasterization/polytope_rasterizer.cpp
    source/grid/rasterization/polytope_rasterizer.h

    source/grid/sampling/mapper.h
    source/grid/sampling/refinement.cpp
    source/grid/sampling/refinement.h
    source/grid/sampling/sampling.h
    source/grid/sampling/sparse_raster.h
    source/grid/sampling/vector_sampling.h
    source/grid/sampling/vector_sparse_raster.h
    source/grid/sampling/xd_iterator.h

    source/helper/image_stats.cpp
    source/helper/image_stats.h
    source/helper/io.cpp
    source/helper/io.h
    source/helper/stats.cpp
    source/helper/stats.h
    source/helper/stopwatch.cpp
    source/helper/stopwatch.h

    source/solver/inscribed_radius.cpp
    source/solver/inscribed_radius.h
    source/solver/inscriber.h
    source/solver/iterative_inscriber.cpp
    source/solver/iterative_inscriber.h
    source/solver/objective.h

    source/solver/conventional/combined_objective_bounder.cpp
    source/solver/conventional/combined_objective_bounder.h
    source/solver/conventional/dlib_inscriber.cpp
    source/solver/conventional/dlib_inscriber.h
    source/solver/conventional/ghj_inscriber.cpp
    source/solver/conventional/ghj_inscriber.h
    source/solver/conventional/nlopt_inscriber.cpp
    source/solver/conventional/nlopt_inscriber.h
    source/solver/conventional/objective_bounder.h

    source/solver/inverse/accuracy_estimator.cpp
    source/solver/inverse/accuracy_estimator.h
    source/solver/inverse/domain_estimator.cpp
    source/solver/inverse/domain_estimator.h
    source/solver/inverse/general_accuracy_estimator.cpp
    source/solver/inverse/general_accuracy_estimator.h
    source/solver/inverse/graphic_inscriber.cpp
    source/solver/inverse/graphic_inscriber.h
    source/solver/inverse/halfspace_part_rasterizer.cpp
    source/solver/inverse/halfspace_part_rasterizer.h
    source/solver/inverse/minkowski_sum.cpp
    source/solver/inverse/minkowski_sum.h
    source/solver/inverse/minkowski_sum_rasterizer.cpp
    source/solver/inverse/minkowski_sum_rasterizer.h

    source/utility/generator.h
    source/utility/lazy.h
    source/utility/noncopyable.h
    source/utility/subsets.h
)
target_compile_features(xdscribe_lib PUBLIC cxx_std_17)
target_include_directories(xdscribe_lib PUBLIC "${PROJECT_SOURCE_DIR}/source")

add_executable(
    xdscribe

    source/inscriber_factory.cpp
    source/inscriber_factory.h
    source/main.cpp

    $<TARGET_OBJECTS:xdscribe_lib>
)
target_link_libraries(xdscribe PRIVATE xdscribe_lib)

set(EXTERNAL_CACHE_DIR "${PROJECT_SOURCE_DIR}/extern/cache")
include(add_boost)
include(external_utils)
include(helpers)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_external_lib(
    xdscribe_lib
    eigen
    "https://github.com/eigenteam/eigen-git-mirror.git"
    "3.3.7"
    ""
)

set(Boost_INCLUDE_DIR "${BOOST_SOURCE_DIR}")
turnon(CGAL_HEADER_ONLY)
turnon(CGAL_DISABLE_GMP)
turnon(WITH_Eigen3)
turnoff(WITH_CGAL_ImageIO)
turnoff(WITH_CGAL_Qt5)
turnoff(WITH_GMP)
turnoff(WITH_MPFR)
# CGAL does not work with add_subdirectory
get_external_source(
    CGAL
    "https://github.com/CGAL/cgal.git"
    "releases/CGAL-4.14"
    ""
)
set(CGAL_DIR "${EXTERNAL_CACHE_DIR}/CGAL")
find_package(CGAL)
target_link_libraries(xdscribe_lib PRIVATE CGAL::CGAL)

turnoff(NLOPT_FORTRAN)
turnoff(NLOPT_GUILE)
turnoff(NLOPT_MATLAB)
turnoff(NLOPT_OCTAVE)
turnoff(NLOPT_PYTHON)
turnoff(NLOPT_SWIG)
add_external_lib(
    xdscribe_lib
    nlopt
    "https://github.com/stevengj/nlopt.git"
    "v2.6.1"
    "${PROJECT_SOURCE_DIR}/extern/nlopt.patch"
)

turnoff(DLIB_GIF_SUPPORT)
turnoff(DLIB_JPEG_SUPPORT)
turnoff(DLIB_LINK_WITH_SQLITE3)
turnon(DLIB_NO_GUI_SUPPORT)
turnoff(DLIB_PNG_SUPPORT)
turnoff(DLIB_USE_BLAS)
turnoff(DLIB_USE_CUDA)
turnoff(DLIB_USE_LAPACK)
turnoff(DLIB_USE_LAPACK)
turnoff(DLIB_USE_MKL_FFT)
add_external_lib(
    xdscribe_lib
    dlib
    "https://github.com/davisking/dlib.git"
    "v19.17"
    "${PROJECT_SOURCE_DIR}/extern/dlib.patch"
)

add_executable(
    xdscribe_tests

    tests/integration_test.cpp
    tests/tests_main.cpp

    tests/geometry/axis_distance_test.cpp
    tests/geometry/helpers.h
    tests/geometry/location_test.cpp
    tests/geometry/polytope_test.cpp
    tests/geometry/simplex_facet_overlap_test.cpp

    tests/grid/mapper_test.cpp
    tests/grid/refinement_test.cpp
    tests/grid/xd_iterator_test.cpp

    tests/utility/generator_test.cpp
    tests/utility/lazy_test.cpp
    tests/utility/subsets_test.cpp

    $<TARGET_OBJECTS:xdscribe_lib>
)
target_include_directories(xdscribe_tests PRIVATE "${PROJECT_SOURCE_DIR}")
target_link_libraries(xdscribe_tests PRIVATE xdscribe_lib)

turnoff(CATCH_BUILD_TESTING)
add_external_lib(
    xdscribe_tests
    Catch2
    "https://github.com/catchorg/Catch2.git"
    "v2.11.1"
    ""
)

include(CTest)
include("${EXTERNAL_CACHE_DIR}/Catch2/contrib/Catch.cmake")
catch_discover_tests(xdscribe_tests)

file(
    COPY "${PROJECT_SOURCE_DIR}/examples"
    DESTINATION "${PROJECT_BINARY_DIR}")

include(measure_performance)
