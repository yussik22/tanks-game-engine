cmake_minimum_required(VERSION 3.16)
project(TanksGameEngine LANGUAGES CXX)

add_library(tanks_engine
    src/BotAI.cpp
    src/CollisionSystem.cpp
    src/GameEngine.cpp
    src/GameMap.cpp
    src/KeyboardInput.cpp
    src/Layer.cpp
    src/Projectile.cpp
    src/Tank.cpp
    src/Tiles.cpp
    src/Wall.cpp
)
target_include_directories(tanks_engine PUBLIC include)
target_compile_features(tanks_engine PUBLIC cxx_std_17)

add_executable(tanks_cli app/main.cpp)
target_link_libraries(tanks_cli PRIVATE tanks_engine)

include(CTest)
if(BUILD_TESTING)
    add_executable(tanks_tests tests/engine_tests.cpp)
    target_link_libraries(tanks_tests PRIVATE tanks_engine)
    add_test(NAME tanks_engine_tests COMMAND tanks_tests)
endif()
