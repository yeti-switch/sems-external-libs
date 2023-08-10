MESSAGE(STATUS "Enable building of the bundled Spandsp")

set (SPANDSP_DIR third/spandsp)
set (SPANDSP_PATCH_FILE ${PROJECT_SOURCE_DIR}/third/spandsp.patch)
set (SPANDSP_SRC_DIR ${PROJECT_SOURCE_DIR}/${SPANDSP_DIR})
set (SPANDSP_BIN_DIR ${PROJECT_BINARY_DIR}/${SPANDSP_DIR})
set (SPANDSP_BUNDLED_PREFIX ${SPANDSP_BIN_DIR}/src/.libs)
set (SPANDSP_BUNDLED_LIB ${SPANDSP_BUNDLED_PREFIX}/libspandsp.a)
# set (SPANDSP_INCLUDE_HEADER ${SPANDSP_BIN_DIR}/src/spandsp.h)

set(SPANDSP_CONFIG_ARGS --enable-static --with-pic --enable-sse4-2 CFLAGS=-msse4.2\ -O2\ -g)

add_custom_target(libspandsp ALL DEPENDS ${SPANDSP_BUNDLED_LIB})

IF(NOT EXISTS ${SPANDSP_BIN_DIR}/configure_stdout)
    file(MAKE_DIRECTORY ${SPANDSP_BIN_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${SPANDSP_SRC_DIR} ${SPANDSP_BIN_DIR} WORKING_DIRECTORY ${SPANDSP_BIN_DIR})
    execute_process(COMMAND patch --silent --strip=1 --directory=${SPANDSP_BIN_DIR} --input=${SPANDSP_PATCH_FILE})
    execute_process(COMMAND autoreconf -fi OUTPUT_FILE configure_stdout WORKING_DIRECTORY ${SPANDSP_BIN_DIR})
    execute_process(COMMAND ./configure ${SPANDSP_CONFIG_ARGS} OUTPUT_FILE configure_stdout WORKING_DIRECTORY ${SPANDSP_BIN_DIR})
ENDIF(NOT EXISTS ${SPANDSP_BIN_DIR}/configure_stdout)


add_custom_command(OUTPUT ${SPANDSP_BUNDLED_LIB}
    COMMAND make
    WORKING_DIRECTORY ${SPANDSP_BIN_DIR})

install(FILES ${SPANDSP_BIN_DIR}/src/spandsp.h DESTINATION ${SEMS_THIRD_INCLUDE_PATH})
install(DIRECTORY ${SPANDSP_BIN_DIR}/src/spandsp DESTINATION ${SEMS_THIRD_INCLUDE_PATH}
        FILES_MATCHING PATTERN "*.h")
install(FILES ${SPANDSP_BUNDLED_LIB} DESTINATION ${SEMS_THIRD_LIB_PATH})
