project(say_resources)

string(TIMESTAMP APPSTREAM_BUILD_DATE "%Y-%m-%d")
configure_file(version.in ${CMAKE_BINARY_DIR}/version @ONLY)
configure_file(com.sayonara-player.Sayonara.appdata.xml.in
               ${CMAKE_CURRENT_BINARY_DIR}/com.sayonara-player.Sayonara.appdata.xml
               @ONLY)

# manpage
find_program(GZIP_BINARY
             NAMES gzip)

if (NOT GZIP_BINARY)
    message("gzip not found. No manpage will be created")
else ()
    ## create date for manpage
    string(TIMESTAMP SAYONARA_MANPAGE_DATE "%d %B %Y" UTC)

    ## create manpages
    configure_file(sayonara.1.man.in
                   ${CMAKE_CURRENT_BINARY_DIR}/sayonara.1
                   @ONLY)
    configure_file(sayonara-ctl.1.man.in
                   ${CMAKE_CURRENT_BINARY_DIR}/sayonara-ctl.1
                   @ONLY)
    configure_file(sayonara-query.1.man.in
                   ${CMAKE_CURRENT_BINARY_DIR}/sayonara-query.1
                   @ONLY)

    ## create zipped manpage
    execute_process(COMMAND gzip -kf ${CMAKE_CURRENT_BINARY_DIR}/sayonara.1)
    execute_process(COMMAND gzip -kf ${CMAKE_CURRENT_BINARY_DIR}/sayonara-ctl.1)
    execute_process(COMMAND gzip -kf ${CMAKE_CURRENT_BINARY_DIR}/sayonara-query.1)

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sayonara.1.gz"
            DESTINATION share/man/man1)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sayonara-ctl.1.gz"
            DESTINATION share/man/man1)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sayonara-query.1.gz"
            DESTINATION share/man/man1)
endif ()

install(FILES "com.sayonara-player.Sayonara.desktop"
        DESTINATION share/applications)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/com.sayonara-player.Sayonara.appdata.xml"
        DESTINATION share/metainfo)
install(DIRECTORY "logos/"
        DESTINATION share/icons/hicolor)
