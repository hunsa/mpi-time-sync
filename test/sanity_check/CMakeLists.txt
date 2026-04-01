
add_executable(measure_clock_drift_rep
clock_drift/measure_clock_drift_rep.c
clock_drift/clock_drift_utils.c
)

SET_TARGET_PROPERTIES(measure_clock_drift_rep PROPERTIES LINKER_LANGUAGE CXX )
SET_TARGET_PROPERTIES(measure_clock_drift_rep PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
TARGET_LINK_LIBRARIES(measure_clock_drift_rep MPITS::mpits MPI::MPI_C)

add_executable(sync_duration 
sync_duration/sync_duration.c
sync_duration/parse_test_options.c
)

TARGET_LINK_LIBRARIES(sync_duration MPITS::mpits MPI::MPI_C)
SET_TARGET_PROPERTIES(sync_duration PROPERTIES LINKER_LANGUAGE CXX )
SET_TARGET_PROPERTIES(sync_duration PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")

add_executable(measure_rtt
rtt/measure_rtt.c
)

TARGET_LINK_LIBRARIES(measure_rtt MPITS::mpits MPI::MPI_C)
SET_TARGET_PROPERTIES(measure_rtt PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")


add_executable(clock_sync_stress_test
clock_sync_stress/clock_sync_stress_test.c
)

SET_TARGET_PROPERTIES(clock_sync_stress_test PROPERTIES LINKER_LANGUAGE CXX )
SET_TARGET_PROPERTIES(clock_sync_stress_test PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
TARGET_LINK_LIBRARIES(clock_sync_stress_test MPITS::mpits MPI::MPI_C)
