cmake_minimum_required(VERSION 3.10)
project(operators)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenCV REQUIRED)

# Coverage configuration
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "Coverage enabled")
endif()

# Handle OpenMP
if(APPLE)
    find_package(OpenMP)
    if(NOT OpenMP_CXX_FOUND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor -fopenmp")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lomp")
        include_directories(/opt/homebrew/Cellar/libomp/21.1.0/include)
        link_directories(/opt/homebrew/Cellar/libomp/21.1.0/lib)
    endif()
else()
    find_package(OpenMP REQUIRED)
endif()

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(include)
include_directories(src)

# Main application executable
add_executable(operators
        src/gradient/ocv_sobel.cpp
        include/gradient/ocv_sobel.h
        main.cpp
        include/gradient/gradient_operator.h
        src/gradient/alt_sobel.cpp
        include/gradient/alt_sobel.h
        src/utils/image_utils.cpp
        include/utils/image_utils.h
        include/utils/kernels_util.h
        src/gradient/omp_sobel.cpp
        include/gradient/omp_sobel.h
        src/gradient/ocv_prewitt.cpp
        include/gradient/ocv_prewitt.h
        src/gradient/ocv_roberts_cross.cpp
        include/gradient/ocv_roberts_cross.h
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(operators ${OpenCV_LIBS} OpenMP::OpenMP_CXX)
else()
    target_link_libraries(operators ${OpenCV_LIBS})
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(operators_test
        test/gradient/test_ocv_sobel.cpp
        test/gradient/test_alt_sobel.cpp
        test/gradient/test_omp_sobel.cpp
        test/gradient/test_ocv_prewitt.cpp
        test/gradient/test_ocv_roberts_cross.cpp
        test/gradient/test_utils.cpp
        src/gradient/ocv_sobel.cpp
        src/gradient/alt_sobel.cpp
        src/gradient/omp_sobel.cpp
        src/gradient/ocv_prewitt.cpp
        src/gradient/ocv_roberts_cross.cpp
        src/utils/image_utils.cpp
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(operators_test
            gtest
            gtest_main
            ${OpenCV_LIBS}
            OpenMP::OpenMP_CXX
    )
else()
    target_link_libraries(operators_test
            gtest
            gtest_main
            ${OpenCV_LIBS}
    )
endif()

include(GoogleTest)
# gtest_discover_tests(operators_test)

# Coverage target
if(ENABLE_COVERAGE)
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory coverage
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/test/*' '*/googletest/*' --output-file coverage_filtered.info --ignore-errors unsupported,inconsistent
            COMMAND ${GENHTML_PATH} coverage_filtered.info --output-directory coverage
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in coverage/index.html"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS operators_test
            COMMENT "Generating coverage report"
        )
        
        add_custom_target(coverage-clean
            COMMAND ${CMAKE_COMMAND} -E remove_directory coverage
            COMMAND ${CMAKE_COMMAND} -E remove coverage.info coverage_filtered.info
            COMMENT "Cleaning coverage files"
        )
    else()
        message(WARNING "Coverage tools not found. Install lcov and genhtml for HTML reports.")
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage data generated. Use gcov manually for reports."
            DEPENDS operators_test
            COMMENT "Generating coverage data"
        )
    endif()
endif()