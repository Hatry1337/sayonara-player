project(sayonara_components)

set(CMAKE_AUTOUIC OFF)

set(COMPONENTS
	Bookmarks
	Broadcasting
	Converter
	Covers
	Directories
	DynamicPlayback
	Equalizer
	Engine
	Library
	LibraryManagement
	Lyrics
	MetaDataInfo
	Playlist
	PlaylistChooser
	PlayManager
	Preferences
	RemoteControl
	Session
	Shutdown
	Streaming
	Tagging
)

foreach(COMPONENT ${COMPONENTS})
	file(GLOB_RECURSE SOURCES ${COMPONENT} *.cpp)
	file(GLOB_RECURSE HEADERS ${COMPONENT} *.h)
endforeach()

qt5_add_resources(COMPONENT_RESOURCES_RCC
	${CMAKE_CURRENT_SOURCE_DIR}/Broadcasting/Broadcasting.qrc
	${CMAKE_CURRENT_SOURCE_DIR}/Lyrics/Lyrics.qrc
)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES} ${COMPONENT_RESOURCES_RCC} Equalizer/Equalizer.cpp Equalizer/Equalizer.h)

target_link_libraries(${PROJECT_NAME}
	sayonara_database
	sayonara_interfaces

	${GSTREAMER_LIBRARIES}
	${GSTREAMER_BASE_LIBRARIES}
	${GSTREAMER_BASE_APP_LIBRARIES}

	Qt5::Svg
)

if( WITH_COTIRE )
	cotire(${PROJECT_NAME})
endif()

