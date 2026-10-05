# ----------------------------------------------------
# Chrome Dino: Pure Software Raster Graphics Engine
# Qt 6 / Qt 5 qmake Project Configuration File
# ----------------------------------------------------

QT       += core gui widgets multimedia

CONFIG   += c++20
CONFIG   += release

TARGET    = ChromeDinoRaster
TEMPLATE  = app

DEFINES  += QT_DEPRECATED_WARNINGS

# Source Files
SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/graphics/Framebuffer.cpp \
    src/graphics/Sprite.cpp \
    src/graphics/SpriteData.cpp \
    src/graphics/SoftwareRasterizer.cpp \
    src/game/Dino.cpp \
    src/game/Obstacle.cpp \
    src/game/Environment.cpp \
    src/game/ParticleSystem.cpp \
    src/game/Weather.cpp \
    src/game/PowerUp.cpp \
    src/game/Achievements.cpp \
    src/game/DinoGame.cpp \
    src/audio/RetroAudio.cpp \
    src/lab/LabInspector.cpp \
    src/lab/AlgorithmShowcase.cpp

# Header Files
HEADERS += \
    src/MainWindow.h \
    src/graphics/Framebuffer.h \
    src/graphics/Sprite.h \
    src/graphics/SpriteData.h \
    src/graphics/SoftwareRasterizer.h \
    src/graphics/Font5x7.h \
    src/game/Entity.h \
    src/game/Dino.h \
    src/game/Obstacle.h \
    src/game/Environment.h \
    src/game/ParticleSystem.h \
    src/game/Weather.h \
    src/game/PowerUp.h \
    src/game/Achievements.h \
    src/game/DinoGame.h \
    src/audio/RetroAudio.h \
    src/lab/LabInspector.h \
    src/lab/AlgorithmShowcase.h

# Windows Application Icon
win32 {
    RC_ICONS = standalone/icon.ico
}
