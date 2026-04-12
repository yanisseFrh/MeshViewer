cmake_minimum_required(VERSION 3.19)
project(MeshViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

qt_standard_project_setup()

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/mainwindow.ui
    src/mainwindow.cpp
    src/openGLWidget.cpp
    src/camera.cpp
    src/vertex.cpp
    src/triangle.cpp
    src/mesh.cpp
    src/edgeKeyHash.cpp
    src/data.cpp
)

set(HEADERS
    include/mainwindow.h
    include/openGLWidget.h
    include/camera.h
    include/vertex.h
    include/triangle.h
    include/mesh.h
    include/edgeKeyHash.h
    include/shaders.h
    include/data.h
)

qt_add_executable(MeshViewer WIN32 MACOSX_BUNDLE
    ${SOURCES} ${HEADERS}
)

target_include_directories(MeshViewer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MeshViewer
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::OpenGL
        Qt::OpenGLWidgets
)

include(GNUInstallDirs)

install(TARGETS MeshViewer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


qt_generate_deploy_app_script(
    TARGET MeshViewer
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

enable_testing()
add_subdirectory(tests)
