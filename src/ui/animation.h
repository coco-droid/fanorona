#ifndef UI_ANIMATION_H
#define UI_ANIMATION_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Forward declarations
typedef struct UINode UINode;

typedef enum {
    ANIMATION_PROPERTY_X,
    ANIMATION_PROPERTY_Y,
    ANIMATION_PROPERTY_WIDTH,
    ANIMATION_PROPERTY_HEIGHT,
    ANIMATION_PROPERTY_OPACITY,
    ANIMATION_PROPERTY_SCALE_X,
    ANIMATION_PROPERTY_SCALE_Y,
    ANIMATION_PROPERTY_ROTATION
} AnimationProperty;

typedef struct {
    float time;          // Temps en secondes (0.0 à 1.0)
    float value;         // Valeur à ce moment
    char* easing;        // "linear", "ease-in", "ease-out", "ease-in-out"
} Keyframe;

typedef struct {
    char* name;
    AnimationProperty property;
    Keyframe* keyframes;
    int keyframe_count;
    int keyframe_capacity;
    float duration;      // Durée totale en secondes
    int iterations;      // -1 = infinite, 0 = 1 fois, n = n fois
    bool alternate;      // true = va-et-vient
    char* fill_mode;     // "none", "forwards", "backwards", "both"
    float activation_delay; // 🆕 Délai avant démarrage en secondes
} Animation;

// === FONCTIONS PRINCIPALES ===

// Créer une animation simple
Animation* animation_create(const char* name, AnimationProperty property, float duration);

// Détruire une animation
void animation_destroy(Animation* anim);

// Ajouter un keyframe
void animation_add_keyframe(Animation* anim, float time, float value, const char* easing);

// Appliquer l'animation à un nœud UI
void ui_node_add_animation(UINode* node, Animation* anim);

// Arrêter toutes les animations d'un nœud
void ui_node_stop_animations(UINode* node);

// Mettre à jour toutes les animations (appeler dans la boucle de jeu)
void ui_update_animations(float delta_time);

// Nettoyer toutes les animations
void ui_animations_cleanup(void);

// === ANIMATIONS PRÉDÉFINIES ===

// Animation prédéfinie - fade in
Animation* animation_fade_in(float duration);

// Animation prédéfinie - fade out
Animation* animation_fade_out(float duration);

// Animation prédéfinie - slide in from left
Animation* animation_slide_in_left(float duration, float distance);

// Animation prédéfinie - slide in from right
Animation* animation_slide_in_right(float duration, float distance);

// Animation prédéfinie - slide out to left
Animation* animation_slide_out_left(float duration, float distance);

// Animation prédéfinie - scale bounce
Animation* animation_scale_bounce(float duration);

// Animation prédéfinie - shake horizontale
Animation* animation_shake_x(float duration, float intensity);

// Animation prédéfinie - pulse (scale up/down)
Animation* animation_pulse(float duration);

// === FONCTIONS UTILITAIRES ===

// Obtenir le nombre d'animations actives
int ui_get_active_animations_count(void);

// Vérifier si un nœud a des animations en cours
bool ui_node_has_active_animations(UINode* node);

// Définir le mode de remplissage d'une animation
void animation_set_fill_mode(Animation* anim, const char* fill_mode);

// Définir le nombre d'itérations
void animation_set_iterations(Animation* anim, int iterations);

// Définir le mode alternatif (va-et-vient)
void animation_set_alternate(Animation* anim, bool alternate);

// Définir un délai d'activation pour l'animation
void animation_set_activation_delay(Animation* anim, float delay_seconds);

// Obtenir le délai d'activation
float animation_get_activation_delay(Animation* anim);

#endif // UI_ANIMATION_H
