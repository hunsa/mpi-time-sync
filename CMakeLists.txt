cmake_minimum_required(VERSION 3.24)

project(mpi_time_sync)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(MPI REQUIRED)

include_directories(include src)
include_directories(${MPI_INCLUDE_PATH})

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")
SET(MY_COMPILE_FLAGS "")
SET(MY_ADDITIONAL_LIBS "")

find_package(GSL REQUIRED)
if (GSL_INCLUDE_DIR)
    message (STATUS "GSL INCLUDES: ${GSL_INCLUDE_DIR}")
    include_directories(${GSL_INCLUDE_DIR})
else(GSL_INCLUDE_DIR)
    message (FATAL_ERROR "GSL libraries not found.")
endif(GSL_INCLUDE_DIR)

find_package(Hwloc)
if (HWLOC_FOUND)
    message (STATUS "HWLOC INCLUDES: ${HWLOC_INCLUDE_DIR}")
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DHAVE_HWLOC")
    SET(MY_ADDITIONAL_LIBS ${HWLOC_LIBRARY} ${MY_ADDITIONAL_LIBS})
else(HWLOC_FOUND)
    message (STATUS "The HWLOC library was not found. Compiling without HWLOC support.")
endif(HWLOC_FOUND)

option(ENABLE_GETTIME_REALTIME "Use clock_gettime(CLOCK_REALTIME) for time measurements [default: MPI_Wtime()]" off)
option(ENABLE_GETTIME_MONOTONIC "Use clock_gettime(CLOCK_MONOTONIC) for time measurements [default: MPI_Wtime()]" off)

if(ENABLE_GETTIME_REALTIME)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_GETTIME_REALTIME")
endif()

if(ENABLE_GETTIME_MONOTONIC)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_GETTIME_MONOTONIC")
endif()


set(MPITS_LIBRARY_SRC_FILES
    src/time_provider/mpi_time_provider.c
    src/clock_sync/mpi_clock_sync.cpp
    src/helpers/dict/mpits_dict.cpp
    src/helpers/dict/cli_param_parser.cpp
    src/time_provider/clocks/GettimeClock.cpp
    src/time_provider/clocks/GlobalClock.cpp
    src/time_provider/clocks/GlobalClockLM.cpp
    src/time_provider/clocks/GlobalClockOffset.cpp
    src/time_provider/clocks/MPIClock.cpp
    src/clock_sync/clock_sync_common.cpp
    src/clock_sync/clock_sync_loader.cpp
    src/clock_sync/sync_algorithms/ClockPropagationSync.cpp
    src/clock_sync/sync_algorithms/HCA2ClockSync.cpp
    src/clock_sync/sync_algorithms/HCA3ClockSync.cpp
    src/clock_sync/sync_algorithms/HCAAbstractClockSync.cpp
    src/clock_sync/sync_algorithms/HCAClockSync.cpp
    #src/clock_sync/sync_algorithms/HierarchicalClockSync.cpp
    src/clock_sync/sync_algorithms/JKClockSync.cpp
    src/clock_sync/sync_algorithms/LinearModelFitterDebug.cpp
    src/clock_sync/sync_algorithms/LinearModelFitterStandard.cpp
    src/clock_sync/sync_algorithms/TwoLevelClockSync.cpp
    src/clock_sync/sync_algorithms/offset/HCA3OffsetClockSync.cpp
    src/clock_sync/sync_algorithms/offset/SKaMPIClockSync.cpp
    src/clock_sync/clock_offset_algs/ClockOffset.cpp
    src/clock_sync/clock_offset_algs/ClockOffsetAlg.cpp
    src/clock_sync/clock_offset_algs/PingpongClockOffsetAlg.cpp
    src/clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.cpp
    src/clock_sync/sync_algorithms/utils/communicator_utils.cpp
    src/clock_sync/sync_algorithms/utils/sync_errors.cpp
    src/clock_sync/sync_algorithms/utils/sync_utils.cpp
    src/log/zf_log.c
    src/clock_sync/sync_algorithms_adapters/hca_sync.cpp
    src/clock_sync/sync_algorithms_adapters/jk_sync.cpp
    src/clock_sync/sync_algorithms_adapters/no_sync.cpp
    src/clock_sync/sync_algorithms_adapters/sk_sync.cpp
    src/clock_sync/sync_algorithms_adapters/topo_aware_2l_sync.cpp
)

if (HWLOC_FOUND)
    message (STATUS "Compiling clock sync algs that need hwloc library.")
    set(MPITS_LIBRARY_SRC_FILES
        ${MPITS_LIBRARY_SRC_FILES}
        src/clock_sync/sync_algorithms/HierarchicalClockSync.cpp
        src/clock_sync/sync_algorithms_adapters/topo_aware_sync.cpp
            src/clock_sync/sync_algorithms/utils/hwloc_helpers.c
    )
endif (HWLOC_FOUND)

add_library(mpits SHARED  ${MPITS_LIBRARY_SRC_FILES})

SET_TARGET_PROPERTIES(mpits PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
target_link_libraries(mpits PUBLIC ${MPI_LIBRARIES} ${GSL_LIBRARIES} ${MY_ADDITIONAL_LIBS} m MPI::MPI_CXX)

add_executable(cs1
    test/clock_sync/01-basic_usage/cs1.cpp
)
TARGET_LINK_LIBRARIES(cs1 PRIVATE mpits MPI::MPI_CXX)

add_executable(check_clock_accuracy
        test/clock_sync/02-clock-check/check_clock_accuracy.cpp
        test/clock_sync/02-clock-check/clock_drift_utils.c
)
TARGET_LINK_LIBRARIES(check_clock_accuracy PRIVATE mpits MPI::MPI_CXX)

install(TARGETS mpits
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(FILES
        include/mpits.h
        DESTINATION include)
