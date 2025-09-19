#include "ui_link.h"
#include "../native/atomic.h"
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
    
    if (!node) {
        printf("❌ UINode est NULL dans ui_link_click_handler\n");
        return;
    }
    
    UILinkData* link_data = (UILinkData*)node->component_data;
    if (!link_data) {
        printf("❌ UILinkData est NULL\n");
        return;
    }
    
    // Effet visuel de clic
    atomic_set_background_color(atomic_element, 100, 150, 255, 200);
    
    // Exécuter le callback personnalisé s'il existe
    if (link_data->on_click) {
        link_data->on_click(node);
    }
    
    // Effectuer la transition vers la scène cible
    scene_manager_transition_to_scene_from_element(node);
    
    (void)event; // Éviter l'avertissement de compilation
}

// Effet visuel lorsque la souris survole le lien
static void ui_link_hover_handler(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // Effet de survol
    atomic_set_background_color(atomic_element, 200, 220, 255, 100);
    atomic_set_border(atomic_element, 1, 0, 120, 255, 255);
    
    (void)event; // Éviter l'avertissement de compilation
}

// Effet visuel lorsque la souris quitte le lien
static void ui_link_unhover_handler(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    
    // Retour à l'état normal
    atomic_set_background_color(atomic_element, 0, 0, 0, 0);
    atomic_set_border(atomic_element, 0, 0, 0, 0, 0);
    
    (void)event; // Éviter l'avertissement de compilation
}

// Créer un lien de navigation vers une scène
UINode* ui_create_link(UITree* tree, const char* id, const char* text, 
                      const char* target_scene_id, SceneTransitionOption transition) {
    // Créer le nœud UI
    UINode* link = ui_tree_create_node(tree, id, "link");
    if (!link) {
        printf("❌ Erreur: Impossible de créer le nœud UI pour le lien\n");
        return NULL;
    }
    
    // Créer les données du lien
    UILinkData* link_data = (UILinkData*)calloc(1, sizeof(UILinkData));
    if (!link_data) {
        ui_tree_destroy_node(link);
        printf("❌ Erreur: Impossible d'allouer les données du lien\n");
        return NULL;
    }
    
    // Initialiser les données
    link_data->target_scene_id = target_scene_id ? strdup(target_scene_id) : NULL;
    link_data->transition = transition;
    link_data->target_window = WINDOW_TYPE_MAIN; // Par défaut
    link_data->on_click = NULL;
    
    // Associer les données au nœud
    link->component_data = link_data;
    link->component_destroy = ui_link_data_destroy;
    
    // Configurer l'élément atomique
    atomic_set_text(link->element, text);
    atomic_set_text_color_rgba(link->element, 0, 100, 200, 255);
    atomic_set_padding(link->element, 5, 10, 5, 10);
    atomic_set_margin(link->element, 2, 2, 2, 2);
    
    // Associer les handlers d'événements
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
    
    if (link_data->target_scene_id) {
        free(link_data->target_scene_id);
    }
    
    link_data->target_scene_id = strdup(target_scene_id);
}

// Définir le type de transition du lien
void ui_link_set_transition(UINode* link, SceneTransitionOption transition) {
    if (!link) return;
    
    UILinkData* link_data = (UILinkData*)link->component_data;
    if (link_data) {
        link_data->transition = transition;
    }
}

// Définir la fenêtre cible du lien
void ui_link_set_target_window(UINode* link, WindowType window_type) {
    if (!link) return;
    
    UILinkData* link_data = (UILinkData*)link->component_data;
    if (link_data) {
        link_data->target_window = window_type;
    }
}

// Définir un handler personnalisé lors du clic
void ui_link_set_click_handler(UINode* link, void (*on_click)(UINode* link)) {
    if (!link) return;
    
    UILinkData* link_data = (UILinkData*)link->component_data;
    if (link_data) {
        link_data->on_click = on_click;
    }
}

// Connecter le lien au gestionnaire de scènes - VERSION SÉCURISÉE
void ui_link_connect_to_manager(UINode* link, SceneManager* manager) {
    if (!link || !manager || !link->element) {
        printf("❌ Paramètres invalides pour ui_link_connect_to_manager\n");
        return;
    }
    
    // 🔧 FIX PRINCIPAL: Utiliser l'EventManager via l'UITree plutôt que active_scenes
    if (link->tree && link->tree->event_manager) {
        atomic_register_with_event_manager(link->element, link->tree->event_manager);
        printf("🔗 Link '%s' registered via UITree EventManager\n", 
               link->id ? link->id : "NoID");
    } else {
        // 🔧 FALLBACK SÉCURISÉ: Vérifier active_scenes[WINDOW_TYPE_MAIN] avant accès
        if (manager->active_scenes[WINDOW_TYPE_MAIN] && 
            manager->active_scenes[WINDOW_TYPE_MAIN]->event_manager) {
            atomic_register_with_event_manager(link->element, 
                                             manager->active_scenes[WINDOW_TYPE_MAIN]->event_manager);
            printf("🔗 Link '%s' registered via SceneManager EventManager\n", 
                   link->id ? link->id : "NoID");
        } else {
            printf("❌ No EventManager available for link '%s'\n", 
                   link->id ? link->id : "NoID");
        }
    }
}

// Créer un lien avec style prédéfini
UINode* ui_create_navigation_link(UITree* tree, const char* id, const char* text, 
                                 const char* target_scene_id) {
    UINode* link = ui_create_link(tree, id, text, target_scene_id, SCENE_TRANSITION_REPLACE);
    if (!link) return NULL;
    
    // Style pour un bouton de navigation
    atomic_set_padding(link->element, 8, 15, 8, 15);
    atomic_set_text_color_rgba(link->element, 255, 255, 255, 255);
    atomic_set_background_color(link->element, 0, 100, 200, 255);
    atomic_set_border(link->element, 0, 0, 0, 0, 0);
    
    return link;
}

// 🆕 Nouvelle fonction pour récupérer les données de lien
UILinkData* ui_link_get_data(UINode* link) {
    if (!link) {
        printf("❌ UINode est NULL dans ui_link_get_data\n");
        return NULL;
    }
    
    return (UILinkData*)link->component_data;
}

// 🆕 Fonction pour valider un lien avant utilisation
bool ui_link_is_valid(UINode* link) {
    if (!link) return false;
    
    UILinkData* data = ui_link_get_data(link);
    if (!data || !data->target_scene_id) {
        printf("⚠️ Lien '%s' invalide: target_scene_id manquant\n", 
               link->id ? link->id : "NoID");
        return false;
    }
    
    return true;
}

// 🆕 Fonction helper pour éviter les dépendances circulaires
const char* ui_link_get_target_scene_id_from_data(void* data) {
    if (!data) return NULL;
    
    UILinkData* link_data = (UILinkData*)data;
    return link_data->target_scene_id;
}
