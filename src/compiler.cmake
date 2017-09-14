INCLUDE(CheckCXXCompilerFlag)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON) 

#SET(COMMON_FLAGS "-fno-diagnostics-show-caret -ftrack-macro-expansion=0")

MESSAGE("Build type = ${CMAKE_BUILD_TYPE}")

SET(COMMON_FLAGS_TEST 
		"-Woverloaded-virtual"
		"-Wall" 
		"-Wunreachable-code" 
		"-Wextra" 
		"-Wpedantic" 
		"-pthread" 
)

IF(NOT WIN32 OR NOT DEFINED WIN32)
	SET(COMMON_FLAGS_TEST
		${COMMON_FLAGS_TEST}
		"-fPIC"
	)
ENDIF()

FOREACH(FLAG ${COMMON_FLAGS_TEST})
		CHECK_CXX_COMPILER_FLAG(${FLAG} CXX_COMPILER_FLAG_AVAILABLE)
		IF( ${CXX_COMPILER_FLAG_AVAILABLE} )
			MESSAGE("Add ${FLAG}")
			SET(COMMON_FLAGS "${COMMON_FLAGS} ${FLAG}")
		ENDIF()
ENDFOREACH()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${COMMON_FLAGS}")

IF ( ${CMAKE_BUILD_TYPE} MATCHES "Debug" )
    MESSAGE("Debug Mode active")
    ADD_DEFINITIONS(-DDEBUG)
ENDIF()

SET(CMAKE_CXX_FLAGS_NONE "${CMAKE_CXX_FLAGS_NONE} ${COMMON_FLAGS}")


