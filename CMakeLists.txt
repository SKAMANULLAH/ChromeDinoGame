cmake_minimum_required(VERSION 3.16)

project(ChromeDinoRaster LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

set(SOURCES
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/graphics/Framebuffer.h
    src/graphics/Framebuffer.cpp
    src/graphics/Sprite.h
    src/graphics/Sprite.cpp
    src/graphics/SpriteData.h
    src/graphics/SpriteData.cpp
    src/graphics/SoftwareRasterizer.h
    src/graphics/SoftwareRasterizer.cpp
    src/graphics/Font5x7.h
    src/game/Entity.h
    src/game/Dino.h
    src/game/Dino.cpp
    src/game/Obstacle.h
    src/game/Obstacle.cpp
    src/game/Environment.h
    src/game/Environment.cpp
    src/game/ParticleSystem.h
    src/game/ParticleSystem.cpp
    src/game/Weather.h
    src/game/Weather.cpp
    src/game/PowerUp.h
    src/game/PowerUp.cpp
    src/game/Achievements.h
    src/game/Achievements.cpp
    src/game/DinoGame.h
    src/game/DinoGame.cpp
    src/audio/RetroAudio.h
    src/audio/RetroAudio.cpp
    src/lab/LabInspector.h
    src/lab/LabInspector.cpp
    src/lab/AlgorithmShowcase.h
    src/lab/AlgorithmShowcase.cpp
)

add_executable(ChromeDinoRaster ${SOURCES})

target_link_libraries(ChromeDinoRaster PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)
