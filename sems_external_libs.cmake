set(SEMS_THIRD_INCLUDE_PATH /usr/include/sems/third)
set(SEMS_THIRD_LIB_PATH /usr/lib/sems/third)

function(import_static_lib PREFIX LIBNAME)
    set(${PREFIX}_BUNDLED_LIB ${SEMS_THIRD_LIB_PATH}/${LIBNAME})
    add_library(${PREFIX}_bundled STATIC IMPORTED)
    set_property(TARGET ${PREFIX}_bundled PROPERTY IMPORTED_LOCATION ${${PREFIX}_BUNDLED_LIB})
    MESSAGE(STATUS "import static library ${${PREFIX}_BUNDLED_LIB} as ${PREFIX}_bundled")
endfunction()

import_static_lib(BOTAN libbotan-3.a)
import_static_lib(CONFUSE libconfuse.a)
import_static_lib(S3 libs3.a)
import_static_lib(SRTP libsrtp2.a)

import_static_lib(CURL libcurl.a)
set(CURL_BUNDLED_LIBS CURL_bundled gssapi_krb5 ssl cares crypto z nghttp2 brotlidec)

import_static_lib(STUN_CORE libstuncore.a)
import_static_lib(STUN_COMMON libcommon.a)
set(STUN_bundled STUN_CORE_bundled STUN_COMMON_bundled)

include_directories(${SEMS_THIRD_INCLUDE_PATH})
