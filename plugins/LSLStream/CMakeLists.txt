include_directories( ./ ../  ../../include  ../../common)

# Qt related stuff
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)


QT5_WRAP_UI ( LSL_UI_SRC
    dialogselectlslstream.ui
    )

SET( SRC
    lslnode.cpp
    lslnode.h
    lslstream.h
    lslstream.cpp
    dialogselectlslstream.cpp
    dialogselectlslstream.h
    ../../include/PlotJuggler/datastreamer_base.h
    )

add_library(LSL SHARED IMPORTED) # or STATIC instead of SHARED

if(APPLE)
    set_target_properties(LSL PROPERTIES
      IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/LSL/lib/liblsl64.dylib"
      INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/LSL/include"
    )
endif()

if(UNIX AND NOT APPLE)
    set_target_properties(LSL PROPERTIES
      IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/LSL/lib/liblsl.so"
      INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/LSL/include"
    )
endif()


add_library(LSLStream SHARED ${SRC} ${LSL_UI_SRC} )
target_link_libraries(LSLStream ${Qt5Widgets_LIBRARIES} ${Qt5Xml_LIBRARIES} LSL)


########### add UI components ##################






