cmake_minimum_required(VERSION 3.30)
project(BehaviourTree)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

file(GLOB PROJECT_SOURCES "src/*.cpp")

add_library(BehaviourTree SHARED ${PROJECT_SOURCES})

target_include_directories(BehaviourTree
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(BehaviourTree PRIVATE nlohmann_json::nlohmann_json)

enable_testing()

add_executable(RunSaveLoadTest test/test_save_load_cycle.cpp)
add_executable(RunEmptyTest test/test_empty_tree.cpp)

target_link_libraries(RunSaveLoadTest PRIVATE BehaviourTree GTest::gtest GTest::gtest_main)
target_link_libraries(RunEmptyTest PRIVATE BehaviourTree GTest::gtest GTest::gtest_main)

add_test(
        NAME BehaviorTree.SaveLoad
        COMMAND RunSaveLoadTest
)

add_test(
        NAME BehaviorTree.Empty
        COMMAND RunEmptyTest
)

