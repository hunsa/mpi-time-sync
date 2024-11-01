cmake_minimum_required(VERSION 3.24)

project(mpi_time_sync VERSION 1.0 LANGUAGES C CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(MPI REQUIRED)

# Configure the installation paths
set(MPITS_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
set(MPITS_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(MPITS_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
set(MPITS_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBEXECDIR})
set(MPITS_INSTALL_DATADIR ${CMAKE_INSTALL_DATADIR})
set(MPITS_INSTALL_CMAKEDIR ${CMAKE_INSTALL_DATADIR}/cmake/mpits)

STRING(COMPARE EQUAL ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} MPITS_BUILD_INPLACE)
if(NOT MPITS_BUILD_INPLACE)
  include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}")
  include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/include")
  include_directories(BEFORE "${CMAKE_CURRENT_SOURCE_DIR}/src")
endif(NOT MPITS_BUILD_INPLACE)
include_directories(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")
include_directories(BEFORE "${CMAKE_CURRENT_BINARY_DIR}/include")
include_directories(BEFORE "${CMAKE_CURRENT_BINARY_DIR}/src")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")
SET(MY_COMPILE_FLAGS "")
SET(MY_ADDITIONAL_LIBS "")

set(MPITS_REQUIRES "")

find_package(GSL REQUIRED)
if (GSL_INCLUDE_DIR)
    message (STATUS "GSL INCLUDES: ${GSL_INCLUDE_DIR}")
    include_directories(${GSL_INCLUDE_DIR})
    set(MPITS_REQUIRES "${MPITS_REQUIRES} gsl")
else(GSL_INCLUDE_DIR)
    message (FATAL_ERROR "GSL libraries not found.")
endif(GSL_INCLUDE_DIR)

find_package(Hwloc)
if (HWLOC_FOUND)
    message (STATUS "HWLOC INCLUDES: ${HWLOC_INCLUDE_DIR}")
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DHAVE_HWLOC")
    SET(MY_ADDITIONAL_LIBS ${HWLOC_LIBRARY} ${MY_ADDITIONAL_LIBS})
    set(MPITS_REQUIRES "${MPITS_REQUIRES} hwloc")
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
    src/time_provider/clocks/BaseClock.cpp
    src/time_provider/clocks/GlobalClock.cpp
    src/time_provider/clocks/GlobalClockLM.cpp
    src/time_provider/clocks/GlobalClockOffset.cpp
    src/clock_sync/clock_sync_common.cpp
    src/clock_sync/clock_sync_loader.cpp
    src/clock_sync/sync_algorithms/ClockPropagationSync.cpp
    src/clock_sync/sync_algorithms/HCA2ClockSync.cpp
    src/clock_sync/sync_algorithms/HCA3ClockSync.cpp
    src/clock_sync/sync_algorithms/HCAAbstractClockSync.cpp
    src/clock_sync/sync_algorithms/HCAClockSync.cpp
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
add_library(MPITS::mpits ALIAS mpits)
set_target_properties(mpits PROPERTIES PUBLIC_HEADER include/mpits.h)

# make sure the include directory is part of the interface
target_include_directories(mpits
        INTERFACE
        $<INSTALL_INTERFACE:${MPITS_INSTALL_INCLUDEDIR}>
        $<$<NOT:${MPITS_BUILD_INPLACE}>:
          $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
          $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

SET_TARGET_PROPERTIES(mpits PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
target_link_libraries(mpits PUBLIC ${GSL_LIBRARIES} ${MY_ADDITIONAL_LIBS} m MPI::MPI_CXX)

add_executable(cs1
    test/clock_sync/01-basic_usage/cs1.cpp
)
TARGET_LINK_LIBRARIES(cs1 PRIVATE mpits MPI::MPI_CXX)

add_executable(check_clock_accuracy
        test/clock_sync/02-clock-check/check_clock_accuracy.cpp
        test/clock_sync/02-clock-check/clock_drift_utils.c
)
SET_TARGET_PROPERTIES(check_clock_accuracy PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
TARGET_LINK_LIBRARIES(check_clock_accuracy PRIVATE mpits MPI::MPI_CXX)


configure_package_config_file(cmake_modules/mpits-config.cmake.in
        ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mpits-config.cmake
        INSTALL_DESTINATION ${MPITS_INSTALL_CMAKEDIR}
        PATH_VARS   MPITS_INSTALL_LIBDIR
                    MPITS_INSTALL_INCLUDEDIR
                    MPITS_INSTALL_BINDIR
                    MPITS_INSTALL_LIBEXECDIR
                    MPITS_INSTALL_CMAKEDIR)

install(TARGETS mpits
        EXPORT mpits-targets
        LIBRARY DESTINATION ${MPITS_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${MPITS_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${MPITS_INSTALL_BINDIR}
		PUBLIC_HEADER DESTINATION ${MPITS_INSTALL_INCLUDEDIR})

install(FILES ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mpits-config.cmake
        DESTINATION ${MPITS_INSTALL_CMAKEDIR})

install(EXPORT mpits-targets # rules to export the targets built here
        FILE MPITSTargets.cmake
        NAMESPACE MPITS::
        DESTINATION ${MPITS_INSTALL_CMAKEDIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/mpits.pc.in ${CMAKE_CURRENT_BINARY_DIR}/mpits.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mpits.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)