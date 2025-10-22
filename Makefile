# Makefile pour Fanorona
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -g
LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

# Dossiers
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Nom de l'exécutable
TARGET = $(BUILD_DIR)/fanorona

# Fichiers sources
SOURCES = main.c \
          src/core/core.c \
          src/event/event.c \
          src/scene/scene_manager.c \
          src/scene/scene_registry.c \
          src/scene/home_scene.c \
          src/scene/choice_scene.c \
          src/scene/menu_scene.c \
          src/scene/profile_scene.c \
          src/scene/game_scene.c \
          src/scene/ai_scene.c \
          src/ui/ui_tree.c \
          src/ui/ui_components.c \
          src/ui/animation.c \
          src/ui/cnt_ui.c \
          src/ui/cnt_playable.c \
          src/ui/sidebar.c \
          src/ui/plateau_cnt.c \
          src/ui/neon_btn.c \
          src/ui/text_input.c \
          src/ui/components/ui_link.c \
          src/ui/native/atomic.c \
          src/ui/native/optimum.c \
          src/window/window.c \
          src/plateau/plateau.c \
          src/pions/pions.c \
          src/logic/rules.c \
          src/config.c \
          src/utils/asset_manager.c \
          src/utils/log_console.c

# Fichiers objets
OBJECTS = $(SOURCES:%.c=$(OBJ_DIR)/%.o)

# Règle par défaut
all: $(TARGET)

# Création de l'exécutable
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LIBS)
	@echo "✓ Compilation réussie: $(TARGET)"

# Compilation des fichiers objets
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Création des dossiers
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/core
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/event
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/window
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/scene
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/utils
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/logic
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/plateau
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/pions
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/ui/native
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/ui/components

# Nettoyage
clean:
	rm -rf $(BUILD_DIR)
	@echo "✓ Nettoyage terminé"

# Compilation et exécution
run: $(TARGET)
	./$(TARGET)

# Installation des dépendances SDL2 (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev

# Règles factices
.PHONY: all clean run install-deps