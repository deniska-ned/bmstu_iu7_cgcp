cmake_minimum_required(VERSION 3.5)

project(deniska VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
  src/main.cpp

  src/mainwindow.ui

  src/mainwindow.h
  src/mainwindow.cpp

  src/manager_draw.h
  src/manager_draw.cpp
  )

add_executable(${PROJECT_NAME}
  ${PROJECT_SOURCES}
  )

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Widgets
  pthread
  )