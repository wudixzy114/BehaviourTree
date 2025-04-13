cmake_minimum_required(VERSION 3.30)
project(BehaviourTree)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt6_DIR "D:/Qt/6.9.0/mingw_64/lib/cmake/Qt6")

find_package(nlohmann_json CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)

file(GLOB PROJECT_SOURCES "src/*.cpp")
file(GLOB QT_SOURCES "qt_src/*.cpp")
file(GLOB QT_HEADERS "qt_include/*.h")

add_library(BehaviourTree SHARED ${PROJECT_SOURCES})

target_include_directories(BehaviourTree
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/qt_include)

target_link_libraries(BehaviourTree PRIVATE nlohmann_json::nlohmann_json)

add_executable(BehaviourTreeEditor qt_src/main_qt.cpp ${QT_SOURCES} ${QT_HEADERS}
)
target_link_libraries(BehaviourTreeEditor
        PRIVATE
        BehaviourTree
        nlohmann_json::nlohmann_json
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
)

enable_testing()

add_executable(RunSaveLoadTest test/test_save_load_cycle.cpp)
add_executable(RunEmptyTest test/test_empty_tree.cpp)

target_link_libraries(RunSaveLoadTest PRIVATE BehaviourTree GTest::gtest GTest::gtest_main)
target_link_libraries(RunEmptyTest PRIVATE BehaviourTree GTest::gtest GTest::gtest_main)
