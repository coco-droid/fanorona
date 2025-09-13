#include "button_demo_scene.h"
#include "../ui/button.h"
#include <stdlib.h>
#include <stdio.h>

// Données pour la scène de démonstration des boutons
typedef struct ButtonDemoSceneData {
    bool initialized;
    Button* primary_button;
    Button* secondary_button;
    Button* success_button;
    Button* warning_button;
    Button* danger_button;
    EventManager* event_manager;
} ButtonDemoSceneData;

// Callbacks pour les boutons
static void primary_button_clicked(void* button, void* user_data) {
    printf("Bouton PRIMARY cliqué!\n");
    (void)button; (void)user_data;
}

static void secondary_button_clicked(void* button, void* user_data) {
    printf("Bouton SECONDARY cliqué!\n");
    (void)button; (void)user_data;
}

static void success_button_clicked(void* button, void* user_data) {
    printf("Bouton SUCCESS cliqué!\n");
    (void)button; (void)user_data;
}

static void warning_button_clicked(void* button, void* user_data) {
    printf("Bouton WARNING cliqué!\n");
    (void)button; (void)user_data;
}

static void danger_button_clicked(void* button, void* user_data) {
    printf("Bouton DANGER cliqué!\n");
    (void)button; (void)user_data;
}

// Initialisation de la scène de démonstration
static void button_demo_scene_init(Scene* scene) {
    printf("Initialisation de la scène Button Demo\n");
    
    ButtonDemoSceneData* data = (ButtonDemoSceneData*)malloc(sizeof(ButtonDemoSceneData));
    if (!data) {
        printf("Erreur: Impossible d'allouer la mémoire pour ButtonDemoSceneData\n");
        return;
    }
    
    data->initialized = true;
    data->event_manager = event_manager_create();
    
    // Créer les boutons avec différents thèmes
    data->primary_button = button_create("btn_primary", "Primary");
    button_set_type(data->primary_button, BUTTON_TYPE_PRIMARY);
    button_set_position(data->primary_button, 50, 50);
    button_set_size(data->primary_button, 100, 40);
    button_set_click_callback(data->primary_button, primary_button_clicked, NULL);
    
    data->secondary_button = button_create("btn_secondary", "Secondary");
    button_set_type(data->secondary_button, BUTTON_TYPE_SECONDARY);
    button_set_position(data->secondary_button, 170, 50);
    button_set_size(data->secondary_button, 100, 40);
    button_set_click_callback(data->secondary_button, secondary_button_clicked, NULL);
    
    data->success_button = button_create("btn_success", "Success");
    button_set_type(data->success_button, BUTTON_TYPE_SUCCESS);
    button_set_position(data->success_button, 290, 50);
    button_set_size(data->success_button, 100, 40);
    button_set_click_callback(data->success_button, success_button_clicked, NULL);
    
    data->warning_button = button_create("btn_warning", "Warning");
    button_set_type(data->warning_button, BUTTON_TYPE_WARNING);
    button_set_position(data->warning_button, 410, 50);
    button_set_size(data->warning_button, 100, 40);
    button_set_click_callback(data->warning_button, warning_button_clicked, NULL);
    
    data->danger_button = button_create("btn_danger", "Danger");
    button_set_type(data->danger_button, BUTTON_TYPE_DANGER);
    button_set_position(data->danger_button, 230, 120);
    button_set_size(data->danger_button, 100, 40);
    button_set_click_callback(data->danger_button, danger_button_clicked, NULL);
    
    // Enregistrer les boutons avec l'event manager
    if (data->event_manager) {
        button_register_events(data->primary_button, data->event_manager);
        button_register_events(data->secondary_button, data->event_manager);
        button_register_events(data->success_button, data->event_manager);
        button_register_events(data->warning_button, data->event_manager);
        button_register_events(data->danger_button, data->event_manager);
    }
    
    scene->data = data;
}

// Mise à jour de la scène de démonstration
static void button_demo_scene_update(Scene* scene, float delta_time) {
    if (!scene || !scene->data) return;
    
    ButtonDemoSceneData* data = (ButtonDemoSceneData*)scene->data;
    
    // Mettre à jour tous les boutons
    button_update(data->primary_button, delta_time);
    button_update(data->secondary_button, delta_time);
    button_update(data->success_button, delta_time);
    button_update(data->warning_button, delta_time);
    button_update(data->danger_button, delta_time);
}

// Rendu de la scène de démonstration
static void button_demo_scene_render(Scene* scene, GameWindow* window) {
    if (!scene || !scene->data || !window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    ButtonDemoSceneData* data = (ButtonDemoSceneData*)scene->data;
    
    // Fond blanc
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderClear(renderer);
    
    // Rendre tous les boutons
    button_render(data->primary_button, renderer);
    button_render(data->secondary_button, renderer);
    button_render(data->success_button, renderer);
    button_render(data->warning_button, renderer);
    button_render(data->danger_button, renderer);
    
    // Dessiner un titre simple
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect title_rect = {200, 20, 200, 20};
    SDL_RenderFillRect(renderer, &title_rect);
}

// Nettoyage de la scène de démonstration
static void button_demo_scene_cleanup(Scene* scene) {
    printf("Nettoyage de la scène Button Demo\n");
    if (scene->data) {
        ButtonDemoSceneData* data = (ButtonDemoSceneData*)scene->data;
        
        // Désenregistrer et détruire les boutons
        if (data->event_manager) {
            button_unregister_events(data->primary_button, data->event_manager);
            button_unregister_events(data->secondary_button, data->event_manager);
            button_unregister_events(data->success_button, data->event_manager);
            button_unregister_events(data->warning_button, data->event_manager);
            button_unregister_events(data->danger_button, data->event_manager);
            
            event_manager_destroy(data->event_manager);
        }
        
        button_destroy(data->primary_button);
        button_destroy(data->secondary_button);
        button_destroy(data->success_button);
        button_destroy(data->warning_button);
        button_destroy(data->danger_button);
        
        free(scene->data);
        scene->data = NULL;
    }
}

// Créer la scène de démonstration des boutons
Scene* create_button_demo_scene(void) {
    Scene* scene = (Scene*)malloc(sizeof(Scene));
    if (!scene) {
        printf("Erreur: Impossible d'allouer la mémoire pour la scène Button Demo\n");
        return NULL;
    }
    
    scene->name = "Button Demo";
    scene->init = button_demo_scene_init;
    scene->update = button_demo_scene_update;
    scene->render = button_demo_scene_render;
    scene->cleanup = button_demo_scene_cleanup;
    scene->data = NULL;
    
    return scene;
}