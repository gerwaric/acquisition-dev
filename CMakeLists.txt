cmake_minimum_required(VERSION 3.19)

project(acquisition
    VERSION 0.12.0
    DESCRIPTION "Stash and forum shop management for Path of Exile \\256"
    HOMEPAGE_URL "https://github.com/gerwaric/acquisition"
    LANGUAGES CXX
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core HttpServer Network Sql Widgets)

if(MSVC)
    add_compile_options(/EHsc)
    add_compile_options(/W4)
endif()

qt_standard_project_setup(REQUIRES 6.5.3)

qt_add_executable(acquisition WIN32 MACOSX_BUNDLE)

add_subdirectory(acquisition)
add_subdirectory(external/boost-headers-only)
add_subdirectory(external/json_struct)
add_subdirectory(external/QsLog)

target_link_libraries(acquisition PRIVATE
    Qt6::Core
    Qt6::HttpServer
    Qt6::Network
    Qt6::Sql
    Qt6::Widgets
    boost-headers-only
    json_struct
    QsLog
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(acquisition PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS acquisition
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
