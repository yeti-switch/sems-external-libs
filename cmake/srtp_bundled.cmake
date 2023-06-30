MESSAGE(STATUS "Enable building of the bundled libsrtp")

set (SRTP_DIR third/srtp)
set (SRTP_SRC_DIR ${PROJECT_SOURCE_DIR}/${SRTP_DIR})
set (SRTP_BIN_DIR ${PROJECT_BINARY_DIR}/${SRTP_DIR})
set (SRTP_BUNDLED_LIB ${SRTP_BIN_DIR}/libsrtp2.a)

set(SRTP_CONFIG_ARGS --disable-openssl --enable-debug-logging CPPFLAGS='-fPIC -fcommon')

add_custom_target(libsrtp ALL DEPENDS ${SRTP_BUNDLED_LIB})

file(MAKE_DIRECTORY ${SRTP_BIN_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRTP_SRC_DIR} ${SRTP_BIN_DIR})

add_custom_command(OUTPUT ${SRTP_BUNDLED_LIB}
    COMMAND ./configure ${SRTP_CONFIG_ARGS}
    COMMAND make
    WORKING_DIRECTORY ${SRTP_BIN_DIR})

install(DIRECTORY ${SRTP_BIN_DIR}/include/ DESTINATION ${SEMS_THIRD_INCLUDE_PATH}/srtp FILES_MATCHING PATTERN "*.h")
install(FILES ${SRTP_BUNDLED_LIB} DESTINATION ${SEMS_THIRD_LIB_PATH})
