MESSAGE(STATUS "Enable building of the bundled libconfuse")

set(CONFUSE_DIR third/confuse)
set(CONFUSE_DEPENDENCY_TARGET libconfuse)
set(CONFUSE_SRC_DIR ${PROJECT_SOURCE_DIR}/${CONFUSE_DIR})
set(CONFUSE_BIN_DIR ${PROJECT_BINARY_DIR}/${CONFUSE_DIR})
set(CONFUSE_BUNDLED_LIB ${CONFUSE_BIN_DIR}/src/.libs/libconfuse.a)

set(CONFUSE_CONFIG_ARGS --enable-shared=no --enable-static=yes --with-pic=yes)

file(MAKE_DIRECTORY ${CONFUSE_BIN_DIR})

add_custom_target(libconfuse ALL DEPENDS ${CONFUSE_BUNDLED_LIB})
add_custom_command(OUTPUT ${CONFUSE_BUNDLED_LIB}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CONFUSE_SRC_DIR} ${CONFUSE_BIN_DIR}
    COMMAND ./configure ${CONFUSE_CONFIG_ARGS}
    COMMAND make
    WORKING_DIRECTORY ${CONFUSE_BIN_DIR})

install(FILES ${CONFUSE_BIN_DIR}/src/confuse.h DESTINATION ${SEMS_THIRD_INCLUDE_PATH})
install(FILES ${CONFUSE_BUNDLED_LIB} DESTINATION ${SEMS_THIRD_LIB_PATH})
