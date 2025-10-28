# CMakeLists.txt pour le projet Fanorona

cmake_minimum_required(VERSION 3.10)
project(fanorona LANGUAGES C)

# Définir les options de compilation
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Ajouter les options de compilation spécifiques au compilateur
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # Options pour GCC et Clang
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
elseif(MSVC)
    # Options pour Visual Studio
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
endif()

# Ajouter les répertoires d'inclusion
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/ai
    ${PROJECT_SOURCE_DIR}/src/plateau
    ${PROJECT_SOURCE_DIR}/src/pions
    ${PROJECT_SOURCE_DIR}/src/utils
    ${PROJECT_SOURCE_DIR}/src/window
    ${PROJECT_SOURCE_DIR}/src/logic
)

# Ajouter les fichiers sources
set(SOURCES
    src/main.c
    src/utils/log_console.c
    src/window/window.c
    src/plateau/plateau.c
    src/pions/pions.c
    src/logic/logic.c
    src/ai/ai.c
    src/ai/minimax.c
    src/ai/markov.c
    src/ai/mcts.c
    src/sound/sound.c
)

# Créer l'exécutable
add_executable(fanorona ${SOURCES})

# Lier les bibliothèques nécessaires
target_link_libraries(fanorona
    PRIVATE
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        SDL2_mixer::SDL2_mixer
)

# Définir les propriétés de l'exécutable
set_target_properties(fanorona PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# Installer l'exécutable
install(TARGETS fanorona DESTINATION bin)