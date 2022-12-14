project(sayonara-test)
include("NewTest.cmake")
include(${CMAKE_SOURCE_DIR}/src/CMake/Compiler.cmake)

if(POLICY CMP0071)
	cmake_policy(SET CMP0071 NEW)
endif()

find_package(Qt5 COMPONENTS Core Gui Sql Network Xml Test REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(COPY ${CMAKE_SOURCE_DIR}/resources/player.db
	DESTINATION	${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Xml_INCLUDE_DIRS})
include_directories(${Qt5Sql_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${Qt5Test_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${QT_DEFINITIONS})

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src/3rdParty)
include_directories(${CMAKE_BINARY_DIR}/src/3rdParty/taglib)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR}/test)

set(RESOURCES
	resources/Test.qrc
	${CMAKE_CURRENT_SOURCE_DIR}/../resources/Database.qrc
	${CMAKE_CURRENT_SOURCE_DIR}/../resources/Resources.qrc
)

qt5_add_resources(RESOURCES_RCC
	${RESOURCES}
)

new_test(AudioConverter/AudioConverterTest.cpp)

new_test(Covers/CoverLocationTest.cpp)
new_test(Covers/CoverFetchManagerTest.cpp)

new_test(Database/PodcastTest.cpp)
new_test(Database/AlbumTest.cpp)
new_test(Database/LibraryDatabaseTest.cpp)
new_test(Database/TracksTest.cpp Common/TestTracks.cpp)
new_test(DynamicPlayback/ArtistMatchTest.cpp)

new_test(Equalizer/EqualizerTest.cpp)

new_test(Library/ImportCacheTest.cpp)
new_test(Library/LibraryManagerTest.cpp)
new_test(Lyrics/LyricsLogicTest.cpp)

new_test(Playlist/PlaylistTest.cpp Playlist/PlaylistTestUtils.cpp Common/TestTracks.cpp)
new_test(Playlist/PlaylistTrackModifyTest.cpp Playlist/PlaylistTestUtils.cpp Common/TestTracks.cpp)
new_test(Playlist/PlaylistFileTest.cpp Playlist/PlaylistTestUtils.cpp Common/TestTracks.cpp)
new_test(Playlist/PlaylistHandlerTest.cpp)
new_test(Playlist/PlaylistDbInterfaceTest.cpp Common/TestTracks.cpp)
new_test(Playlist/PlaylistShuffleTest.cpp Playlist/PlaylistTestUtils.cpp Common/TestTracks.cpp)
new_test(Playlist/ExternTracksPlaylistGeneratorTest.cpp Playlist/PlaylistTestUtils.cpp Common/TestTracks.cpp)
new_test(Playlist/PlaylistFromPathCreatorTest.cpp Playlist/PlaylistTestUtils.cpp Common/TestTracks.cpp)
new_test(Playlist/RepeatTest.cpp Playlist/PlaylistTestUtils.cpp Common/TestTracks.cpp)

new_test(PlayManager/PlayManagerTest.cpp)

new_test(Streaming/RadioStationParserTest.cpp)

new_test(StreamRecorder/StreamRecorderTest.cpp)
new_test(StreamRecorder/StreamRecorderUtilsTest.cpp)

new_test(Tagging/AlbumArtistTest.cpp Tagging/AbstractTaggingTest.cpp)
new_test(Tagging/CoverTest.cpp Tagging/AbstractTaggingTest.cpp)
new_test(Tagging/DiscnumberTest.cpp Tagging/AbstractTaggingTest.cpp)
new_test(Tagging/EditorTest.cpp)
new_test(Tagging/LyricsTest.cpp Tagging/AbstractTaggingTest.cpp)
new_test(Tagging/RatingTest.cpp Tagging/AbstractTaggingTest.cpp)
new_test(Tagging/TagFromPathTest.cpp)
new_test(Tagging/TaggingTest.cpp Tagging/AbstractTaggingTest.cpp)
new_test(Tagging/CoverEditTest.cpp Common/TestTracks.cpp)

new_test(Util/AlbumClassTest.cpp)
new_test(Util/UtilTest.cpp)
new_test(Util/AsyncWebAccessTest.cpp)
new_test(Util/CryptTest.cpp)
new_test(Util/DirectoryReaderTest.cpp Common/TestTracks.cpp)
new_test(Util/FileHelperTest.cpp)
new_test(Util/LanguageKeyTest.cpp)
new_test(Util/LanguageUtilTest.cpp)
new_test(Util/LibraryFilterTest.cpp)
new_test(Util/MetaDataTest.cpp)
new_test(Util/MetaDataListTest.cpp)
new_test(Util/MetaDataSortingTest.cpp)
new_test(Util/MimeDataTest.cpp ${CMAKE_SOURCE_DIR}/src/Gui/Utils/MimeData/CustomMimeData.cpp)
new_test(Util/PlaylistModeTest.cpp)
new_test(Util/SettingsTest.cpp)
new_test(Util/SetTest.cpp)
new_test(Util/CissearchTest.cpp)
new_test(Util/StandardPathTest.cpp)
new_test(Util/TreeTest.cpp)
new_test(Util/Ranges.cpp)
new_test(Util/StreamParserTest.cpp)
new_test(SmartPlaylists/ConverterTest.cpp)
new_test(SmartPlaylists/SmartPlaylistManagerTest.cpp)
new_test(Library/LocalLibraryTest.cpp)
new_test(Database/ArtistTest.cpp)
