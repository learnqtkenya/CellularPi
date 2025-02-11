set(MODULE_NAME Modem)
set(LIB_NAME ${MODULE_NAME}Lib)

qt_add_library(${LIB_NAME} STATIC)

set_target_properties(${LIB_NAME} PROPERTIES AUTOMOC ON)

target_link_libraries(${LIB_NAME} PRIVATE
    Qt6::SerialPort
    Qt6::DBus
    Qt6::Concurrent
)

list(APPEND MODULE_QML_FILES

)
list(APPEND MODULE_SOURCE_FILES
    modem.h modem.cpp
    modemdbusmanager.h modemdbusmanager.cpp
)

qt_add_qml_module(${LIB_NAME}
        URI ${MODULE_NAME}
        VERSION 1.0
        RESOURCE_PREFIX /
        QML_FILES  ${MODULE_QML_FILES}
        RESOURCES  ${MODULE_RESOURCES}
        SOURCES ${MODULE_SOURCE_FILES}
        OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${MODULE_NAME}"
)
