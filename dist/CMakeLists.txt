execute_process(COMMAND env LC_TIME="en_US.utf8" date "+%a %b %d %Y" OUTPUT_VARIABLE SAYONARA_DATE_TODAY)
string(REGEX REPLACE "\n$" "" SAYONARA_DATE_TODAY "${SAYONARA_DATE_TODAY}")
string(REGEX MATCH "[0-9]+(\.[0-9])+" SAYONARA_VERSION_BASE "${SAYONARA_VERSION}")
string(REGEX MATCH "[a-z]+[0-9]+" SAYONARA_VERSION_RELEASE ${SAYONARA_VERSION})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rpm/sayonara.spec.in ${CMAKE_CURRENT_BINARY_DIR}/sayonara.spec @ONLY)

