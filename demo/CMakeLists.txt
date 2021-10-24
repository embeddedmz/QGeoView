# To disable command line prompt under Windows
if(WIN32)
set(GUI_TYPE WIN32)
elseif(APPLE)
# Set some Apple MacOS Specific settings
set(GUI_TYPE MACOSX_BUNDLE)
endif()

# Executable icon
set(APP_ICON_RESOURCE_WINDOWS windowExeIcon.rc)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Widgets
)

set(DEMO_SRCS
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    demoitem.h
    demoitem.cpp
    selectordialog.h
    selectordialog.cpp
    samples/widgets.cpp
    samples/widgets.h
    samples/background.cpp
    samples/background.h
    samples/mouse.cpp
    samples/mouse.h
    samples/items.cpp
    samples/items.h
    samples/flags.cpp
    samples/flags.h
    samples/customtiles.cpp
    samples/customtiles.h
    samples/utilities.cpp
    samples/utilities.h
    samples/waveanimation.cpp
    samples/waveanimation.h
    samples/ellipse.cpp
    samples/ellipse.h
    samples/rectangle.cpp
    samples/rectangle.h
    samples/placemark.cpp
    samples/placemark.h
    samples/mytiles.cpp
    samples/mytiles.h
    samples/mytile.cpp
    samples/mytile.h
    samples/polyline.cpp
    samples/polyline.h
    samples/placemarkCluster.cpp
    samples/placemarkCluster.h
    samples/placemarkSetLayer.cpp
    samples/placemarkSetLayer.h
    samples/placemarkPixmap.cpp
    samples/placemarkPixmap.h
    samples/colorMap.cpp
    samples/colorMap.h
    samples/colorMapPresetDialog.cpp
    samples/colorMapPresetDialog.h
    samples/colorMapPresets.cpp
    samples/colorMapPresets.h
    samples/colorMapPresetToPixmap.cpp
    samples/colorMapPresetToPixmap.h
)

if(BUILD_GDAL_EXAMPLE)
    find_package(GDAL REQUIRED)
	include_directories(${GDAL_INCLUDE_DIRS})

    list(APPEND DEMO_SRCS samples/raster.cpp)
    list(APPEND DEMO_SRCS samples/raster.h)
endif(BUILD_GDAL_EXAMPLE)

set(UI_SRCS samples/colorMapPresetDialog.ui)

add_executable(qgeoview-demo ${GUI_TYPE} ${DEMO_SRCS} ${APP_ICON_RESOURCE_WINDOWS} ${UI_SRCS})

target_link_libraries(qgeoview-demo
    PRIVATE
        Qt5::Core
        Qt5::Network
        Qt5::Gui
        Qt5::Widgets
        QGeoView
		${GDAL_LIBRARY}
)

set_target_properties(qgeoview-demo
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)