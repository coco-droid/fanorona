#include "ui_link.h"
#include "../native/atomic.h"
#include "../../utils/log_console.h"
#include "../../scene/scene.h" // 🔧 FIX: Include scene.h for transition constants
#include "../../sound/sound.h" // 🆕 AJOUT: Pour le son
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declaration - implemented in scene_manager.c
extern void scene_manager_transition_to_scene_from_element(UINode* element);

// Fonction de destruction des données du lien
static void ui_link_data_destroy(void* data) {
    UILinkData* link_data = (UILinkData*)data;
    if (!link_data) return;
    
    if (link_data->target_scene_id) {
        free(link_data->target_scene_id);
    }
    
    free(link_data);
}

// Callback lors du clic sur le lien
static void ui_link_click_handler(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    UINode* node = (UINode*)atomic_element->user_data;
    
    if (!node) return;
    
    UILinkData* link_data = (UILinkData*)node->component_data;
    if (!link_data) return;
    
    // 🆕 Vérifier si le lien est prêt
    if (!link_data->is_ready) {
        printf("⏳ Clic sur lien '%s' ignoré - pas encore prêt\n", node->id ? node->id : "NoID");
        return;
    }
    
    // 🆕 AJOUT: Jouer le son de clic par défaut
    sound_play_button_click();
    
    printf("🎯 CLIC SUR LE LIEN UI '%s' → Cible: '%s'\n", 
           node->id ? node->id : "NoID", 
           link_data->target_scene_id ? link_data->target_scene_id : "NULL");
    
    // Exécuter le callback personnalisé s'il existe
    if (link_data->on_click) {
        link_data->on_click(node);
    }
    
    // 🔧 FIX: Effectuer la transition via le SceneManager stocké
    if (link_data->manager && link_data->target_scene_id) {
        printf("🚀 TRANSITION via SceneManager vers '%s'...\n", link_data->target_scene_id);
        
        bool success = scene_manager_transition_to_scene(link_data->manager, 
                                                        link_data->target_scene_id, 
                                                        link_data->transition);
        
        if (success) {
            // Associer explicitement la nouvelle scène à la fenêtre source si nécessaire
            Scene* current = scene_manager_get_current_scene(link_data->manager);
            WindowType source_window = current ? current->target_window : WINDOW_TYPE_MAIN;
            
            Scene* new_scene = scene_manager_get_scene_by_id(link_data->manager, link_data->target_scene_id);
            if (new_scene) {
                scene_manager_set_scene_for_window(link_data->manager, new_scene, source_window);
            }
        }
    } else {
        printf("❌ ERREUR: Transition impossible sans SceneManager ou target_scene_id\n");
    }
    
    (void)event;
}

// Effet visuel lorsque la souris survole le lien
static void ui_link_hover_handler(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    atomic_set_background_color(atomic_element, 200, 220, 255, 100);
    (void)event;
}

// Effet visuel lorsque la souris quitte le lien
static void ui_link_unhover_handler(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    atomic_set_background_color(atomic_element, 0, 0, 0, 0);
    (void)event;
}

// Créer un lien de navigation vers une scène
UINode* ui_create_link(UITree* tree, const char* id, const char* text, 
                      const char* target_scene_id, SceneTransitionOption transition) {
    UINode* link = ui_tree_create_node(tree, id, "link");
    if (!link) return NULL;
    
    UILinkData* link_data = (UILinkData*)calloc(1, sizeof(UILinkData));
    if (!link_data) {
        ui_tree_destroy_node(link);
        return NULL;
    }
    
    link_data->target_scene_id = target_scene_id ? strdup(target_scene_id) : NULL;
    link_data->transition = transition;
    link_data->target_window = WINDOW_TYPE_MAIN;
    link_data->is_ready = false;
    link_data->activation_delay = 0.5f;
    
    link->component_data = link_data;
    link->component_destroy = ui_link_data_destroy;
    
    atomic_set_text(link->element, text);
    atomic_set_text_color_rgba(link->element, 0, 100, 200, 255);
    atomic_set_padding(link->element, 5, 10, 5, 10);
    
    link->element->user_data = link;
    atomic_set_click_handler(link->element, ui_link_click_handler);
    atomic_set_hover_handler(link->element, ui_link_hover_handler);
    atomic_set_unhover_handler(link->element, ui_link_unhover_handler);
    
    return link;
}

// Définir la scène cible du lien
void ui_link_set_target(UINode* link, const char* target_scene_id) {
    if (!link || !target_scene_id) return;
    UILinkData* link_data = (UILinkData*)link->component_data;
    if (!link_data) return;
    if (link_data->target_scene_id) free(link_data->target_scene_id);
    link_data->target_scene_id = strdup(target_scene_id);
}

// Définir le type de transition du lien
void ui_link_set_transition(UINode* link, SceneTransitionOption transition) {
    if (!link) return;
    UILinkData* link_data = (UILinkData*)link->component_data;
    if (link_data) link_data->transition = transition;
}

// Définir la fenêtre cible du lien
void ui_link_set_target_window(UINode* link, WindowType window_type) {
    if (!link) return;
    UILinkData* link_data = (UILinkData*)link->component_data;
    if (link_data) link_data->target_window = window_type;
}

// Définir un handler personnalisé lors du clic
void ui_link_set_click_handler(UINode* link, void (*on_click)(UINode* link)) {
    if (!link) return;
    UILinkData* link_data = (UILinkData*)link->component_data;
    if (link_data) link_data->on_click = on_click;
}

// Configurer le délai d'activation du lien
void ui_link_set_activation_delay(UINode* link, float seconds) {
    if (!link) return;
    UILinkData* link_data = (UILinkData*)link->component_data;
    if (link_data) {
        link_data->activation_delay = seconds;
        link_data->is_ready = (seconds <= 0.0f);
    }
}

// Mettre à jour le lien (gestion des délais)
void ui_link_update(UINode* link, float delta_time) {
    if (!link) return;
    UILinkData* link_data = (UILinkData*)link->component_data;
    if (!link_data || link_data->is_ready) return;
    
    link_data->time_since_creation += delta_time;
    if (link_data->time_since_creation >= link_data->activation_delay) {
        link_data->is_ready = true;
        atomic_set_background_color(link->element, 0, 0, 0, 0);
        atomic_set_text_alpha(link->element, 255);
    }
}

// Connecter le lien au gestionnaire de scènes
void ui_link_connect_to_manager(UINode* link, SceneManager* manager) {
    if (!link || !link->component_data) return;
    
    UILinkData* data = (UILinkData*)link->component_data;
    data->manager = manager;
}

// 🆕 Nouvelle fonction pour récupérer les données de lien
UILinkData* ui_link_get_data(UINode* link) {
    if (!link) return NULL;
    return (UILinkData*)link->component_data;
}

// 🆕 Attacher un comportement de lien à un nœud existant
void ui_link_attach_to_node(UINode* node, const char* target_scene_id) {
    if (!node) return;
    
    if (node->component_data && node->component_destroy) {
        node->component_destroy(node->component_data);
    }
    
    UILinkData* link_data = (UILinkData*)calloc(1, sizeof(UILinkData));
    if (!link_data) return;
    
    link_data->target_scene_id = target_scene_id ? strdup(target_scene_id) : NULL;
    link_data->transition = SCENE_TRANSITION_REPLACE;
    link_data->target_window = WINDOW_TYPE_MAIN;
    link_data->is_ready = true;
    link_data->activation_delay = 0.0f;
    
    node->component_data = link_data;
    node->component_destroy = ui_link_data_destroy;
    
    node->element->user_data = node;
    atomic_set_click_handler(node->element, ui_link_click_handler);
    atomic_set_hover_handler(node->element, ui_link_hover_handler);
    atomic_set_unhover_handler(node->element, ui_link_unhover_handler);
}

// Valider un lien
bool ui_link_is_valid(UINode* link) {
    if (!link) return false;
    UILinkData* data = ui_link_get_data(link);
    return (data && data->target_scene_id);
}

// Helper pour éviter les dépendances circulaires
const char* ui_link_get_target_scene_id_from_data(void* data) {
    if (!data) return NULL;
    UILinkData* link_data = (UILinkData*)data;
    return link_data->target_scene_id;
}

// Activer manuellement la transition
void ui_link_activate(UINode* link) {
    if (!link) return;
    UILinkData* data = (UILinkData*)link->component_data;
    if (!data || !data->manager || !data->target_scene_id) return;
    
    scene_manager_transition_to_scene(data->manager, data->target_scene_id, data->transition);
}
