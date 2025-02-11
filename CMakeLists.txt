cmake_minimum_required(VERSION 3.16)

project(
    CellularPi
    VERSION 1.0.0
    LANGUAGES CXX C
    DESCRIPTION "Cellular Pi Test App"
)

message("CMAKE_SYSROOT " ${CMAKE_SYSROOT})
message("CMAKE_LIBRARY_ARCHITECTURE " ${CMAKE_LIBRARY_ARCHITECTURE})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6Core)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wl,-rpath-link, ${CMAKE_SYSROOT}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE} -L${CMAKE_SYSROOT}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wl,-rpath-link,${CMAKE_SYSROOT}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE} -L${CMAKE_SYSROOT}/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")

list(APPEND PROJECT_QT_LIBS
        Quick
        DBus
        Concurrent
        Network
)

find_package(Qt6 6.7 REQUIRED COMPONENTS ${PROJECT_QT_LIBS})
set(APP_TARGET appCellularPi)

qt_standard_project_setup()

qt_add_executable(${APP_TARGET}
    main.cpp
)

add_subdirectory(Modem)
add_subdirectory(Qml)
add_subdirectory(REST)

list(APPEND PROJECT_MODULE_LIBS
        ModemLibplugin
        QmlLibplugin
        RESTLibplugin
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(${APP_TARGET} PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${APP_TARGET}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

foreach(lib ${PROJECT_QT_LIBS})
    list(APPEND QT6_PROJECT_LIBS Qt6::${lib})
endforeach()

target_link_libraries(${APP_TARGET}
        PRIVATE
        ${QT6_PROJECT_LIBS}
        ${PROJECT_MODULE_LIBS}
)

include(GNUInstallDirs)
install(TARGETS ${APP_TARGET}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
