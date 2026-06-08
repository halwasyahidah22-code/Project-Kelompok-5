cmake_minimum_required(VERSION 3.16)
project(RestaurantSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(RestaurantSystem
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/datastructures.h
)

target_link_libraries(RestaurantSystem PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Jika Qt5
# find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
# target_link_libraries(RestaurantSystem PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
