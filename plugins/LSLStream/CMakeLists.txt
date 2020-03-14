add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)

#QT5_WRAP_UI ( UI_SRC  ../common/selectlistdialog.ui  )

add_library(LSL SHARED IMPORTED) # or STATIC instead of SHARED

if(APPLE)
    # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
    set_target_properties(LSL PROPERTIES
      IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/LSL/lib/liblsl64.dylib"
      INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/LSL/include"
    )
endif()

if(UNIX AND NOT APPLE)
	message(${CMAKE_CURRENT_SOURCE_DIR})
    # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
    set_target_properties(LSL PROPERTIES
      IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/LSL/lib/liblsl.so"
      INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/LSL/include"
    )
endif()


include_directories( ./ ../  ../../include  ../../common)

SET( SRC
    lslnode.cpp
    ../../include/PlotJuggler/datastreamer_base.h
    )

add_library(LSLStream SHARED ${SRC} ${UI_SRC} )
target_link_libraries(LSLStream ${Qt5Widgets_LIBRARIES} ${Qt5Xml_LIBRARIES} LSL)

if(COMPILING_WITH_CATKIN)
    install(TARGETS DataStreamSample
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION} )
else()
    install(TARGETS LSLStream DESTINATION bin )
endif()


########### add UI components ##################







