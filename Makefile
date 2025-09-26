# Makefile pour Fanorona
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

# Dossiers
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Nom de l'exécutable
TARGET = $(BUILD_DIR)/fanorona

# Fichiers sources
SOURCES = main.c \
          $(SRC_DIR)/core/core.c \
          $(SRC_DIR)/event/event.c \
          $(SRC_DIR)/window/window.c \
          $(SRC_DIR)/scene/scene.c \
          $(SRC_DIR)/scene/scene_manager.c \
          $(SRC_DIR)/scene/home_scene.c \
          $(SRC_DIR)/scene/menu_scene.c \
          $(SRC_DIR)/utils/asset_manager.c \
          $(SRC_DIR)/utils/log_console.c \
          $(SRC_DIR)/ui/native/atomic.c \
          $(SRC_DIR)/ui/native/optimum.c \
          $(SRC_DIR)/ui/ui_tree.c \
          $(SRC_DIR)/ui/ui_components.c \
          $(SRC_DIR)/ui/cnt_ui.c \
          $(SRC_DIR)/ui/neon_btn.c \
          $(SRC_DIR)/ui/components/ui_link.c

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
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/ui/native
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)/ui

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