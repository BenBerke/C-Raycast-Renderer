cmake_minimum_required(VERSION 4.1)
project(Raycast_Renderer LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(Raycast_Renderer main.c
        config.h
        src/Systems/Renderer.c
        Headers/Systems/Renderer.h
        src/Math/Vector2.c
        Headers/Tools/Vector2.h
        Headers/Objects/Wall.h
        Headers/Tools/Vector3.h
        src/Math/Vector3.c
)

find_package(SDL3 CONFIG REQUIRED)

target_link_libraries(Raycast_Renderer PRIVATE SDL3::SDL3)