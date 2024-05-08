MESSAGE(STATUS "Enable building of the bundled libbotan")

set(BOTAN_DIR third/botan)
set(BOTAN_PATCH_FILE ${PROJECT_SOURCE_DIR}/third/botan.patch)
set(BOTAN_SRC_DIR ${PROJECT_SOURCE_DIR}/${BOTAN_DIR})
set(BOTAN_BIN_DIR ${PROJECT_BINARY_DIR}/${BOTAN_DIR})
set(BOTAN_BUNDLED_LIB ${BOTAN_BIN_DIR}/libbotan-3.a)

set(BOTAN_CONFIG_ARGS --cxxflags=-fPIC --disable-shared-library --without-documentation --build-targets=static)

#use execute_process instead of custom_command to generate headers before cmake install command invocation
IF(NOT EXISTS ${BOTAN_BIN_DIR}/configure_stdout)
    file(MAKE_DIRECTORY ${BOTAN_BIN_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${BOTAN_SRC_DIR} ${BOTAN_BIN_DIR})
    execute_process(COMMAND patch --silent --strip=1 --directory=${BOTAN_BIN_DIR} --input=${BOTAN_PATCH_FILE})
    execute_process(COMMAND ./configure.py ${BOTAN_CONFIG_ARGS} OUTPUT_FILE configure_stdout WORKING_DIRECTORY ${BOTAN_BIN_DIR})
ENDIF(NOT EXISTS ${BOTAN_BIN_DIR}/configure_stdout)

add_custom_target(libbotan ALL DEPENDS ${BOTAN_BUNDLED_LIB})
add_custom_command(USES_TERMINAL OUTPUT ${BOTAN_BUNDLED_LIB}
    COMMAND make
    WORKING_DIRECTORY ${BOTAN_BIN_DIR})

#needed to add as dependency for libstuncore
add_library(BOTAN_bundled STATIC IMPORTED)
set_property(TARGET BOTAN_bundled PROPERTY IMPORTED_LOCATION ${BOTAN_BUNDLED_LIB})
set(BOTAN_BUNDLED_INCLUDE_DIRS ${BOTAN_BIN_DIR}/build/include/public)

file(GLOB BOTAN_INCLUDE_FILES "${BOTAN_BIN_DIR}/build/include/public/botan/*.h")
FOREACH(rel_file ${BOTAN_INCLUDE_FILES})
    get_filename_component(abs_file ${rel_file} REALPATH)
    install(FILES ${abs_file} DESTINATION ${SEMS_THIRD_INCLUDE_PATH}/botan)
ENDFOREACH(rel_file ${BOTAN_INCLUDE_FILES})

file(GLOB BOTAN_INCLUDE_FILES "${BOTAN_BIN_DIR}/build/include/external/*.h")
FOREACH(rel_file ${BOTAN_INCLUDE_FILES})
    get_filename_component(abs_file ${rel_file} REALPATH)
    install(FILES ${abs_file} DESTINATION ${SEMS_THIRD_INCLUDE_PATH}/botan/external)
ENDFOREACH(rel_file ${BOTAN_INCLUDE_FILES})

file(GLOB BOTAN_INCLUDE_FILES "${BOTAN_BIN_DIR}/build/include/internal/botan/internal/*.h")
FOREACH(rel_file ${BOTAN_INCLUDE_FILES})
    get_filename_component(abs_file ${rel_file} REALPATH)
    install(FILES ${abs_file} DESTINATION ${SEMS_THIRD_INCLUDE_PATH}/botan/internal)
ENDFOREACH(rel_file ${BOTAN_INCLUDE_FILES})

install(FILES ${BOTAN_BUNDLED_LIB} DESTINATION ${SEMS_THIRD_LIB_PATH})
