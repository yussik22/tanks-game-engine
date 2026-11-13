cmake_minimum_required(VERSION 3.16)
project(TanksGameEngine LANGUAGES CXX)

add_library(tanks_engine STATIC
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

option(TANKS_BUILD_RENDERER "Build the SFML symbol sprite renderer" OFF)

if(TANKS_BUILD_RENDERER)
    include(FetchContent)

    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(sfml)

    add_executable(tanks_render
        app/graphics/main.cpp
        app/graphics/GameDisplayData.cpp
        app/graphics/PixelTextures.cpp
        app/graphics/SpriteRenderer.cpp
        app/graphics/WindowInput.cpp
    )
    target_link_libraries(tanks_render PRIVATE tanks_engine sfml-graphics)

    if(WIN32)
        add_custom_command(TARGET tanks_render POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:sfml-graphics>
                $<TARGET_FILE:sfml-window>
                $<TARGET_FILE:sfml-system>
                $<TARGET_FILE_DIR:tanks_render>
        )
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(tanks_tests tests/engine_tests.cpp)
    target_link_libraries(tanks_tests PRIVATE tanks_engine)
    add_test(NAME tanks_engine_tests COMMAND tanks_tests)
endif()
