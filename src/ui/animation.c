#define _POSIX_C_SOURCE 200809L
#include "animation.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// === STRUCTURES INTERNES ===

typedef struct {
    UINode* node;
    Animation* animation;
    float current_time;
    int current_iteration;
    bool is_playing;
    bool is_reversed;
    float start_value;   // Valeur de départ pour certaines propriétés
    bool start_value_cached;
} AnimationInstance;

// Storage global pour les animations actives
static AnimationInstance** g_animations = NULL;
static int g_animation_count = 0;
static int g_animation_capacity = 0;

// === FONCTIONS D'EASING ===

static float ease_linear(float t) { 
    return t; 
}

static float ease_in_quad(float t) { 
    return t * t; 
}

static float ease_out_quad(float t) { 
    return 1.0f - (1.0f - t) * (1.0f - t); 
}

static float ease_in_out_quad(float t) { 
    return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f; 
}

static float apply_easing(float t, const char* easing) {
    if (!easing || strcmp(easing, "linear") == 0) return ease_linear(t);
    if (strcmp(easing, "ease-in") == 0) return ease_in_quad(t);
    if (strcmp(easing, "ease-out") == 0) return ease_out_quad(t);
    if (strcmp(easing, "ease-in-out") == 0) return ease_in_out_quad(t);
    return ease_linear(t);
}

// === GESTION DES ANIMATIONS ===

static void ensure_animation_capacity(void) {
    if (g_animation_count >= g_animation_capacity) {
        int new_capacity = g_animation_capacity == 0 ? 16 : g_animation_capacity * 2;
        AnimationInstance** new_animations = (AnimationInstance**)realloc(g_animations, 
                                                                         new_capacity * sizeof(AnimationInstance*));
        if (new_animations) {
            g_animations = new_animations;
            g_animation_capacity = new_capacity;
        }
    }
}

// Trouver les keyframes entre lesquels interpoler
static void find_surrounding_keyframes(Animation* anim, float time, int* left_idx, int* right_idx) {
    *left_idx = 0;
    *right_idx = anim->keyframe_count - 1;
    
    for (int i = 0; i < anim->keyframe_count - 1; i++) {
        if (time >= anim->keyframes[i].time && time <= anim->keyframes[i + 1].time) {
            *left_idx = i;
            *right_idx = i + 1;
            return;
        }
    }
}

// Interpoler entre deux keyframes
static float interpolate_keyframes(Animation* anim, float time) {
    if (anim->keyframe_count == 0) return 0.0f;
    if (anim->keyframe_count == 1) return anim->keyframes[0].value;
    
    int left_idx, right_idx;
    find_surrounding_keyframes(anim, time, &left_idx, &right_idx);
    
    if (left_idx == right_idx) {
        return anim->keyframes[left_idx].value;
    }
    
    Keyframe* left = &anim->keyframes[left_idx];
    Keyframe* right = &anim->keyframes[right_idx];
    
    float time_span = right->time - left->time;
    if (time_span == 0.0f) return left->value;
    
    float local_time = (time - left->time) / time_span;
    float eased_time = apply_easing(local_time, right->easing);
    
    return left->value + (right->value - left->value) * eased_time;
}

// Obtenir/définir la valeur de propriété d'un nœud
static float get_property_value(UINode* node, AnimationProperty property) {
    if (!node || !node->element) return 0.0f;
    
    switch (property) {
        case ANIMATION_PROPERTY_X:
            return (float)node->element->style.x;
        case ANIMATION_PROPERTY_Y:
            return (float)node->element->style.y;
        case ANIMATION_PROPERTY_WIDTH:
            return (float)node->element->style.width;
        case ANIMATION_PROPERTY_HEIGHT:
            return (float)node->element->style.height;
        case ANIMATION_PROPERTY_OPACITY:
            return (float)node->element->style.opacity;
        default:
            return 0.0f;
    }
}

static void set_property_value(UINode* node, AnimationProperty property, float value) {
    if (!node || !node->element) return;
    
    switch (property) {
        case ANIMATION_PROPERTY_X:
            atomic_set_position(node->element, (int)value, node->element->style.y);
            break;
        case ANIMATION_PROPERTY_Y:
            atomic_set_position(node->element, node->element->style.x, (int)value);
            break;
        case ANIMATION_PROPERTY_WIDTH:
            atomic_set_size(node->element, (int)value, node->element->style.height);
            break;
        case ANIMATION_PROPERTY_HEIGHT:
            atomic_set_size(node->element, node->element->style.width, (int)value);
            break;
        case ANIMATION_PROPERTY_OPACITY:
            atomic_set_opacity(node->element, (Uint8)value);
            break;
        default:
            break;
    }
}

// === API PUBLIQUE ===

Animation* animation_create(const char* name, AnimationProperty property, float duration) {
    Animation* anim = (Animation*)calloc(1, sizeof(Animation));
    if (!anim) return NULL;
    
    anim->name = strdup(name ? name : "unnamed");
    anim->property = property;
    anim->duration = duration;
    anim->iterations = 1;
    anim->alternate = false;
    anim->fill_mode = strdup("none");
    
    anim->keyframes = (Keyframe*)calloc(8, sizeof(Keyframe));
    anim->keyframe_count = 0;
    anim->keyframe_capacity = 8;
    
    printf("✨ Animation '%s' created (property: %d, duration: %.2fs)\n", anim->name, property, duration);
    return anim;
}

void animation_destroy(Animation* anim) {
    if (!anim) return;
    
    printf("🧹 Destroying animation '%s'\n", anim->name ? anim->name : "unnamed");
    
    // Libérer les keyframes
    for (int i = 0; i < anim->keyframe_count; i++) {
        if (anim->keyframes[i].easing) {
            free(anim->keyframes[i].easing);
        }
    }
    
    free(anim->keyframes);
    free(anim->name);
    free(anim->fill_mode);
    free(anim);
}

void animation_add_keyframe(Animation* anim, float time, float value, const char* easing) {
    if (!anim) return;
    
    // Assurer la capacité
    if (anim->keyframe_count >= anim->keyframe_capacity) {
        int new_capacity = anim->keyframe_capacity * 2;
        Keyframe* new_keyframes = (Keyframe*)realloc(anim->keyframes, new_capacity * sizeof(Keyframe));
        if (new_keyframes) {
            anim->keyframes = new_keyframes;
            anim->keyframe_capacity = new_capacity;
        } else {
            return; // Échec d'allocation
        }
    }
    
    Keyframe* kf = &anim->keyframes[anim->keyframe_count++];
    kf->time = time;
    kf->value = value;
    kf->easing = strdup(easing ? easing : "linear");
    
    printf("🔧 Keyframe added to '%s': time=%.2f, value=%.2f, easing=%s\n", 
           anim->name, time, value, kf->easing);
}

void ui_node_add_animation(UINode* node, Animation* anim) {
    if (!node || !anim) return;
    
    ensure_animation_capacity();
    if (g_animation_count >= g_animation_capacity) return;
    
    AnimationInstance* instance = (AnimationInstance*)calloc(1, sizeof(AnimationInstance));
    if (!instance) return;
    
    instance->node = node;
    instance->animation = anim;
    instance->current_time = 0.0f;
    instance->current_iteration = 0;
    instance->is_playing = true;
    instance->is_reversed = false;
    instance->start_value = get_property_value(node, anim->property);
    instance->start_value_cached = true;
    
    g_animations[g_animation_count++] = instance;
    
    printf("🎬 Animation '%s' started on node '%s' (start value: %.2f)\n", 
           anim->name, node->id ? node->id : "NoID", instance->start_value);
}

void ui_node_stop_animations(UINode* node) {
    if (!node) return;
    
    int removed_count = 0;
    for (int i = g_animation_count - 1; i >= 0; i--) {
        if (g_animations[i] && g_animations[i]->node == node) {
            free(g_animations[i]);
            
            // Décaler les animations restantes
            for (int j = i; j < g_animation_count - 1; j++) {
                g_animations[j] = g_animations[j + 1];
            }
            g_animation_count--;
            removed_count++;
        }
    }
    
    if (removed_count > 0) {
        printf("⏹️ Stopped %d animations for node '%s'\n", removed_count, node->id ? node->id : "NoID");
    }
}

void ui_update_animations(float delta_time) {
    int completed_count = 0;
    
    for (int i = g_animation_count - 1; i >= 0; i--) {
        AnimationInstance* instance = g_animations[i];
        if (!instance || !instance->is_playing) continue;
        
        instance->current_time += delta_time;
        
        // Calculer le temps normalisé (0-1)
        float normalized_time = instance->current_time / instance->animation->duration;
        
        // Gérer les itérations et le va-et-vient
        if (normalized_time >= 1.0f) {
            instance->current_iteration++;
            
            if (instance->animation->iterations > 0 && 
                instance->current_iteration >= instance->animation->iterations) {
                // Animation terminée
                instance->is_playing = false;
                normalized_time = 1.0f;
                completed_count++;
            } else {
                // Nouvelle itération
                instance->current_time = 0.0f;
                normalized_time = 0.0f;
                
                if (instance->animation->alternate) {
                    instance->is_reversed = !instance->is_reversed;
                }
            }
        }
        
        // Appliquer le va-et-vient
        float final_time = instance->is_reversed ? (1.0f - normalized_time) : normalized_time;
        
        // Interpoler et appliquer la valeur
        float new_value = interpolate_keyframes(instance->animation, final_time);
        set_property_value(instance->node, instance->animation->property, new_value);
        
        // Supprimer les animations terminées
        if (!instance->is_playing) {
            free(instance);
            
            // Décaler les animations restantes
            for (int j = i; j < g_animation_count - 1; j++) {
                g_animations[j] = g_animations[j + 1];
            }
            g_animation_count--;
        }
    }
    
    // Log occasionnel pour le debug
    static float debug_timer = 0.0f;
    debug_timer += delta_time;
    if (debug_timer > 5.0f && g_animation_count > 0) {
        printf("🎬 Active animations: %d (completed: %d this frame)\n", g_animation_count, completed_count);
        debug_timer = 0.0f;
    }
}

void ui_animations_cleanup(void) {
    printf("🧹 Cleaning up %d animations\n", g_animation_count);
    
    for (int i = 0; i < g_animation_count; i++) {
        if (g_animations[i]) {
            free(g_animations[i]);
        }
    }
    
    free(g_animations);
    g_animations = NULL;
    g_animation_count = 0;
    g_animation_capacity = 0;
    
    printf("✅ Animation system cleaned up\n");
}

// === ANIMATIONS PRÉDÉFINIES ===

Animation* animation_fade_in(float duration) {
    Animation* anim = animation_create("fade-in", ANIMATION_PROPERTY_OPACITY, duration);
    animation_add_keyframe(anim, 0.0f, 0.0f, "ease-out");
    animation_add_keyframe(anim, 1.0f, 255.0f, "ease-out");
    return anim;
}

Animation* animation_fade_out(float duration) {
    Animation* anim = animation_create("fade-out", ANIMATION_PROPERTY_OPACITY, duration);
    animation_add_keyframe(anim, 0.0f, 255.0f, "ease-in");
    animation_add_keyframe(anim, 1.0f, 0.0f, "ease-in");
    return anim;
}

Animation* animation_slide_in_left(float duration, float distance) {
    Animation* anim = animation_create("slide-in-left", ANIMATION_PROPERTY_X, duration);
    animation_add_keyframe(anim, 0.0f, -distance, "ease-out");
    animation_add_keyframe(anim, 1.0f, 0.0f, "ease-out");
    return anim;
}

Animation* animation_slide_in_right(float duration, float distance) {
    Animation* anim = animation_create("slide-in-right", ANIMATION_PROPERTY_X, duration);
    animation_add_keyframe(anim, 0.0f, distance, "ease-out");
    animation_add_keyframe(anim, 1.0f, 0.0f, "ease-out");
    return anim;
}

Animation* animation_slide_out_left(float duration, float distance) {
    Animation* anim = animation_create("slide-out-left", ANIMATION_PROPERTY_X, duration);
    animation_add_keyframe(anim, 0.0f, 0.0f, "ease-in");
    animation_add_keyframe(anim, 1.0f, -distance, "ease-in");
    return anim;
}

Animation* animation_scale_bounce(float duration) {
    // Utilise WIDTH comme approximation du scale (simplified)
    Animation* anim = animation_create("scale-bounce", ANIMATION_PROPERTY_WIDTH, duration);
    animation_add_keyframe(anim, 0.0f, 0.0f, "ease-out");
    animation_add_keyframe(anim, 0.6f, 120.0f, "ease-out"); // 120% de la taille
    animation_add_keyframe(anim, 0.8f, 90.0f, "ease-in");   // 90% de la taille
    animation_add_keyframe(anim, 1.0f, 100.0f, "ease-out"); // 100% de la taille
    return anim;
}

Animation* animation_shake_x(float duration, float intensity) {
    Animation* anim = animation_create("shake-x", ANIMATION_PROPERTY_X, duration);
    animation_set_iterations(anim, 4); // 4 secousses
    animation_set_alternate(anim, true);
    
    animation_add_keyframe(anim, 0.0f, 0.0f, "linear");
    animation_add_keyframe(anim, 0.5f, intensity, "linear");
    animation_add_keyframe(anim, 1.0f, -intensity, "linear");
    return anim;
}

Animation* animation_pulse(float duration) {
    Animation* anim = animation_create("pulse", ANIMATION_PROPERTY_OPACITY, duration);
    animation_set_iterations(anim, -1); // Infini
    animation_set_alternate(anim, true);
    
    animation_add_keyframe(anim, 0.0f, 255.0f, "ease-in-out");
    animation_add_keyframe(anim, 1.0f, 128.0f, "ease-in-out");
    return anim;
}

// === FONCTIONS UTILITAIRES ===

int ui_get_active_animations_count(void) {
    return g_animation_count;
}

bool ui_node_has_active_animations(UINode* node) {
    if (!node) return false;
    
    for (int i = 0; i < g_animation_count; i++) {
        if (g_animations[i] && g_animations[i]->node == node && g_animations[i]->is_playing) {
            return true;
        }
    }
    return false;
}

void animation_set_fill_mode(Animation* anim, const char* fill_mode) {
    if (!anim || !fill_mode) return;
    
    free(anim->fill_mode);
    anim->fill_mode = strdup(fill_mode);
}

void animation_set_iterations(Animation* anim, int iterations) {
    if (!anim) return;
    anim->iterations = iterations;
}

void animation_set_alternate(Animation* anim, bool alternate) {
    if (!anim) return;
    anim->alternate = alternate;
}
