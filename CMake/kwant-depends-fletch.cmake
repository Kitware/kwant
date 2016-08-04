find_package( fletch REQUIRED )
include( ${fletch_DIR}/fletchConfig.cmake )

find_package( TinyXML REQUIRED )
include_directories( SYSTEM ${TinyXML_INCLUDE_DIR} )

find_package( LIBJSON REQUIRED )
include_directories( SYSTEM ${LIBJSON_INCLUDE_DIR} )
