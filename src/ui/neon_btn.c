#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Structure pour les données du neon button
typedef struct {
    float glow_intensity;     // Intensité de la lueur (0.0 - 1.0)
    float animation_time;     // Temps d'animation
    bool is_hovered;          // État de survol
    bool is_animating;        // Animation en cours
} NeonButtonData;

// === ANIMATIONS NEON ===

// Calculer la couleur de lueur selon l'intensité
static void calculate_neon_color(float intensity, int* r, int* g, int* b) {
    // Orange néon avec variation d'intensité
    *r = (int)(255 * intensity);
    *g = (int)(165 * intensity);
    *b = 0;
}

// Mettre à jour l'animation neon
static void update_neon_animation(UINode* neon_btn, float delta_time) {
    if (!neon_btn || !neon_btn->component_data) return;
    
    NeonButtonData* data = (NeonButtonData*)neon_btn->component_data;
    
    if (data->is_animating) {
        data->animation_time += delta_time * 3.0f; // Vitesse d'animation
        
        if (data->is_hovered) {
            // Animation d'entrée : intensité croissante
            data->glow_intensity = fminf(1.0f, data->glow_intensity + delta_time * 4.0f);
            
            // Effet de pulsation
            float pulse = (sinf(data->animation_time) + 1.0f) * 0.5f; // 0.0 - 1.0
            float final_intensity = data->glow_intensity * (0.7f + 0.3f * pulse);
            
            // Appliquer la couleur neon
            int r, g, b;
            calculate_neon_color(final_intensity, &r, &g, &b);
            atomic_set_border(neon_btn->element, 2, r, g, b, (int)(255 * final_intensity));
            
        } else {
            // Animation de sortie : intensité décroissante
            data->glow_intensity = fmaxf(0.0f, data->glow_intensity - delta_time * 5.0f);
            
            if (data->glow_intensity > 0.0f) {
                int r, g, b;
                calculate_neon_color(data->glow_intensity, &r, &g, &b);
                atomic_set_border(neon_btn->element, 2, r, g, b, (int)(255 * data->glow_intensity));
            } else {
                // Retour à la bordure normale
                atomic_set_border(neon_btn->element, 1, 255, 165, 0, 128); // Orange translucide
                data->is_animating = false;
            }
        }
    }
}

// === CALLBACKS NEON ===

static void neon_button_hovered(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    UINode* neon_btn = (UINode*)atomic_element->user_data;
    
    if (!neon_btn || !neon_btn->component_data) return;
    
    NeonButtonData* data = (NeonButtonData*)neon_btn->component_data;
    data->is_hovered = true;
    data->is_animating = true;
    data->animation_time = 0.0f;
    
    ui_log_event("NeonButton", "HoverStart", neon_btn->id, "Neon glow animation started");
    (void)event;
}

static void neon_button_unhovered(void* element, SDL_Event* event) {
    AtomicElement* atomic_element = (AtomicElement*)element;
    UINode* neon_btn = (UINode*)atomic_element->user_data;
    
    if (!neon_btn || !neon_btn->component_data) return;
    
    NeonButtonData* data = (NeonButtonData*)neon_btn->component_data;
    data->is_hovered = false;
    data->is_animating = true; // Continuer l'animation pour la sortie
    
    ui_log_event("NeonButton", "HoverEnd", neon_btn->id, "Neon glow fade-out started");
    (void)event;
}

// Fonction de mise à jour personnalisée pour l'animation
static void neon_button_update(UINode* neon_btn, float delta_time) {
    if (!neon_btn) return;
    
    // Mettre à jour l'animation neon
    update_neon_animation(neon_btn, delta_time);
}

// Fonction de nettoyage pour les données du composant
static void neon_button_destroy(void* data) {
    if (data) {
        free(data);
    }
}

// === FONCTION PRINCIPALE DE CRÉATION ===

UINode* ui_neon_button(UITree* tree, const char* id, const char* text, 
                       void (*onClick)(UINode* node, void* user_data), void* user_data) {
    if (!tree) {
        ui_log_event("NeonButton", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    // Créer un bouton de base
    UINode* neon_btn = ui_button(tree, id, text, onClick, user_data);
    if (!neon_btn) {
        ui_log_event("NeonButton", "CreateError", id, "Failed to create base button");
        return NULL;
    }
    
    // Allouer les données spécifiques au neon button
    NeonButtonData* data = (NeonButtonData*)calloc(1, sizeof(NeonButtonData));
    if (!data) {
        ui_log_event("NeonButton", "CreateError", id, "Failed to allocate neon data");
        return neon_btn; // Retourner le bouton de base
    }
    
    // Initialiser les données
    data->glow_intensity = 0.0f;
    data->animation_time = 0.0f;
    data->is_hovered = false;
    data->is_animating = false;
    
    // Attacher les données au nœud
    neon_btn->component_data = data;
    neon_btn->component_destroy = neon_button_destroy;
    
    // Configurer le style de base
    atomic_set_background_color(neon_btn->element, 0, 0, 0, 0); // Fond transparent
    atomic_set_border(neon_btn->element, 1, 255, 165, 0, 128); // Bordure orange translucide
    atomic_set_text_color_rgba(neon_btn->element, 255, 255, 255, 255); // Texte blanc
    
    // Connecter les événements neon
    atomic_set_hover_handler(neon_btn->element, neon_button_hovered);
    atomic_set_unhover_handler(neon_btn->element, neon_button_unhovered);
    
    // Établir la liaison pour les callbacks
    neon_btn->element->user_data = neon_btn;
    
    ui_log_event("NeonButton", "Create", id, "Neon button created with hover animation");
    printf("✨ Neon Button '%s' créé avec animation de lueur\n", id ? id : "NoID");
    
    return neon_btn;
}

// === FONCTIONS UTILITAIRES ===

void ui_neon_button_set_glow_color(UINode* neon_btn, int r, int g, int b) {
    if (!neon_btn || !neon_btn->component_data) return;
    
    // Pour l'instant, fonction placeholder - pourrait être étendue plus tard
    atomic_set_border(neon_btn->element, 1, r, g, b, 128);
    ui_log_event("NeonButton", "Style", neon_btn->id, "Custom glow color set");
}

void ui_neon_button_set_animation_speed(UINode* neon_btn, float speed) {
    if (!neon_btn || !neon_btn->component_data) return;
    
    // Fonction placeholder pour contrôler la vitesse d'animation
    ui_log_event("NeonButton", "Style", neon_btn->id, "Animation speed set");
    (void)speed; // Éviter warning unused
}

// Forcer la mise à jour de l'animation (appelée depuis le system de rendu)
void ui_neon_button_update_all(UITree* tree, float delta_time) {
    if (!tree || !tree->root) return;
    
    // Cette fonction parcourt tous les neon buttons et les met à jour
    void update_neon_buttons_recursive(UINode* node, float dt) {
        if (!node) return;
        
        // Si c'est un neon button avec des données de composant
        if (node->component_data && node->component_destroy == neon_button_destroy) {
            // Utiliser la fonction update qu'on avait définie
            neon_button_update(node, dt);
        }
        
        // Parcourir les enfants
        for (int i = 0; i < node->children_count; i++) {
            update_neon_buttons_recursive(node->children[i], dt);
        }
    }
    
    // Démarrer le parcours récursif depuis la racine
    update_neon_buttons_recursive(tree->root, delta_time);
}
