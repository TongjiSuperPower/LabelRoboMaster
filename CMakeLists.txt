cmake_minimum_required(VERSION 3.12)
project(LabelMaster)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets Svg)

# 自动查找 Qt，避免手动设置路径
find_package(Qt${QT_VERSION} REQUIRED COMPONENTS ${REQUIRED_LIBS})
find_package(OpenCV REQUIRED)

include_directories(.)

aux_source_directory(. src)

if(MSVC)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:gbk>")
endif()

qt5_add_big_resources(resource resource.qrc)

message("${src}")
add_executable(${PROJECT_NAME} ${src} ${resource})

# 链接 Qt 和 OpenCV 库
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# 指定包的版本号
set(_VERSION_MAJOR 0)
set(_VERSION_MINOR 8)
set(_VERSION_PATCH 0)
set(CPACK_PACKAGE_VERSION_MAJOR "${_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${_VERSION_PATCH}")

IF(UNIX)
    ##---------------- 以下为用CPack打包 (Ubuntu) 的内容 ----------------##
    set(CPACK_GENERATOR "7Z")
    set(CPACK_PACKAGE_NAME "LabelMaster")
    set(CPACK_DEBIAN_PACKAGE_NAME "LabelMaster")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_PACKAGE_CONTACT "supporter@gmail.com")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "members of support@gmail.cn")

    INSTALL(TARGETS LabelMaster RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

ELSEIF(WIN32)

ENDIF()
