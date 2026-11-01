cmake_minimum_required(VERSION 3.16)
project(TanksGameEngine LANGUAGES CXX)

add_library(tanks_engine src/GameEngine.cpp)
target_include_directories(tanks_engine PUBLIC include)
target_compile_features(tanks_engine PUBLIC cxx_std_17)

add_executable(tanks_cli app/main.cpp)
target_link_libraries(tanks_cli PRIVATE tanks_engine)
