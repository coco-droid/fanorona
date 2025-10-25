#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../utils/asset_manager.h"
#include "../window/window.h"
#include "../pions/pions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure pour les données du composant avatar selector
typedef struct AvatarSelectorData {
    AvatarID selected_avatar;
    bool visual_selection_updated;  // Flag pour forcer la mise à jour visuelle
    UINode* main_avatar;
    AtomicElement* mini_avatars[6];
    void (*on_change_callback)(AvatarID avatar, void* user_data);
    void* callback_user_data;
    bool initialized;
} AvatarSelectorData;

// === HELPERS INTERNES ===

static UINode* create_main_avatar_circle(UITree* tree, const char* id, AvatarID avatar_id, int size) {
    UINode* avatar = UI_DIV(tree, id);
    if (!avatar) return NULL;
    
    SET_SIZE(avatar, size, size);
    
    const char* avatar_path = avatar_id_to_filename(avatar_id);
    
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            SDL_Texture* avatar_texture = asset_load_texture(renderer, avatar_path);
            if (avatar_texture) {
                atomic_set_background_image(avatar->element, avatar_texture);
                atomic_set_background_size_str(avatar->element, "cover");
            }
        }
    }
    
    atomic_set_border(avatar->element, 3, 255, 215, 0, 255);
    return avatar;
}

static AtomicElement* create_mini_avatar_atomic(const char* id, AvatarID avatar_id, int size) {
    AtomicElement* avatar = atomic_create(id);
    if (!avatar) return NULL;
    
    atomic_set_size(avatar, size, size);
    
    const char* avatar_path = avatar_id_to_filename(avatar_id);
    
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            // 🔧 FIX: Utiliser atomic_set_background_image_with_path pour charger directement
            SDL_Texture* avatar_texture = asset_load_texture(renderer, avatar_path);
            if (avatar_texture) {
                atomic_set_background_image(avatar, avatar_texture);
                atomic_set_background_size_str(avatar, "cover");
                printf("✅ Avatar texture loaded: %s\n", avatar_path);
            } else {
                printf("❌ Failed to load avatar texture: %s\n", avatar_path);
            }
        }
    }
    
    atomic_set_border(avatar, 2, 255, 255, 255, 180);
    // 🔧 FIX: Remove the non-existent function call
    // atomic_set_interactive(avatar, true);  // REMOVED - function doesn't exist
    
    return avatar;
}

// Mise à jour de la bordure de sélection pour les mini-avatars
static void update_mini_avatar_selection(AvatarSelectorData* data) {
    for (int i = 0; i < 6; i++) {
        if (data->mini_avatars[i]) {
            // 🔧 FIX: Cast to avoid signed/unsigned comparison warning
            bool is_selected = (i == ((int)data->selected_avatar - 1));
            atomic_set_border(data->mini_avatars[i], 3, 
                            is_selected ? 255 : 255, 
                            is_selected ? 215 : 255, 
                            is_selected ? 0 : 255, 255);
        }
    }
}

static void update_main_avatar_texture(AvatarSelectorData* data) {
    if (!data || !data->main_avatar) return;
    
    const char* avatar_path = avatar_id_to_filename(data->selected_avatar);
    
    GameWindow* window = use_mini_window();
    if (!window) return;
    
    SDL_Renderer* renderer = window_get_renderer(window);
    if (!renderer) return;
    
    SDL_Texture* avatar_texture = asset_load_texture(renderer, avatar_path);
    if (avatar_texture) {
        atomic_set_background_image(data->main_avatar->element, avatar_texture);
        data->visual_selection_updated = true;
    }
}

// === CALLBACK POUR LA SÉLECTION D'AVATAR ===

static void on_mini_avatar_click(void* element, SDL_Event* event) {
    (void)event;
    
    AtomicElement* atomic = (AtomicElement*)element;
    AvatarSelectorData* data = (AvatarSelectorData*)atomic_get_custom_data(atomic, "selector_data");
    
    if (!data || !atomic->id) {
        printf("❌ Click callback: No data or ID\n");
        return;
    }
    
    AvatarID avatar_map[] = {
        AVATAR_WARRIOR,     // mini-1
        AVATAR_STRATEGIST,  // mini-2
        AVATAR_DIPLOMAT,    // mini-3
        AVATAR_EXPLORER,    // mini-4
        AVATAR_MERCHANT,    // mini-5
        AVATAR_SAGE         // mini-6
    };
    
    // 🔧 FIX: Parse the correct pattern for IDs like "profile-avatar-selector-mini-3"
    int avatar_index = 0;
    char* mini_pos = strstr(atomic->id, "-mini-");
    if (mini_pos && sscanf(mini_pos, "-mini-%d", &avatar_index) == 1) {
        if (avatar_index >= 1 && avatar_index <= 6) {
            AvatarID new_avatar = avatar_map[avatar_index - 1];
            
            printf("🎭 Mini-avatar %d clicked, selecting avatar ID=%d (%s)\n", 
                   avatar_index, new_avatar, avatar_id_to_filename(new_avatar));
            
            // 🔧 FIX: Toujours mettre à jour la sélection même si c'est le même avatar
            data->selected_avatar = new_avatar;
            update_main_avatar_texture(data);
            update_mini_avatar_selection(data);
            
            if (data->on_change_callback) {
                data->on_change_callback(new_avatar, data->callback_user_data);
            }
            
            printf("✅ Avatar selection updated successfully\n");
        }
    } else {
        printf("❌ Failed to parse avatar index from ID: %s\n", atomic->id);
    }
}

// Fonction de nettoyage pour les données du composant
static void avatar_selector_destroy_data(void* data) {
    if (data) {
        AvatarSelectorData* selector_data = (AvatarSelectorData*)data;
        
        // Nettoyer les atomic elements
        for (int i = 0; i < 6; i++) {
            if (selector_data->mini_avatars[i]) {
                atomic_destroy_safe(selector_data->mini_avatars[i]);
                selector_data->mini_avatars[i] = NULL;
            }
        }
        
        free(selector_data);
        printf("🧹 AvatarSelector data destroyed\n");
    }
}

// === FONCTION PRINCIPALE DE CRÉATION ===

UINode* ui_avatar_selector(UITree* tree, const char* id) {
    if (!tree) {
        ui_log_event("AvatarSelector", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    // Container principal
    UINode* selector_container = ui_div(tree, id);
    if (!selector_container) {
        ui_log_event("AvatarSelector", "CreateError", id, "Failed to create container");
        return NULL;
    }
    
    // Allouer les données du composant
    AvatarSelectorData* data = (AvatarSelectorData*)calloc(1, sizeof(AvatarSelectorData));
    if (!data) {
        ui_log_event("AvatarSelector", "CreateError", id, "Failed to allocate selector data");
        return selector_container;
    }
    
    // Initialiser les données
    data->selected_avatar = AVATAR_WARRIOR;
    data->main_avatar = NULL;
    data->on_change_callback = NULL;
    data->callback_user_data = NULL;
    data->initialized = false;
    data->visual_selection_updated = false;
    
    for (int i = 0; i < 6; i++) {
        data->mini_avatars[i] = NULL;
    }
    
    // Attacher les données au container
    selector_container->component_data = data;
    selector_container->component_destroy = avatar_selector_destroy_data;
    
    // Configuration du container principal
    SET_SIZE(selector_container, 500, 120);
    ui_set_display_flex(selector_container);
    FLEX_COLUMN(selector_container);
    ui_set_justify_content(selector_container, "flex-start");
    ui_set_align_items(selector_container, "center");
    ui_set_flex_gap(selector_container, 15);
    
    // === AVATAR PRINCIPAL ===
    char main_id[128];
    snprintf(main_id, sizeof(main_id), "%s-main", id);
    data->main_avatar = create_main_avatar_circle(tree, main_id, AVATAR_WARRIOR, 45);
    if (data->main_avatar) {
        ui_animate_pulse(data->main_avatar, 2.0f);
        atomic_set_border(data->main_avatar->element, 3, 255, 215, 0, 255);
        APPEND(selector_container, data->main_avatar);
        printf("✅ Main avatar created and configured\n");
    }
    
    // === CONTAINER MINI-AVATARS ===
    char minis_id[128];
    snprintf(minis_id, sizeof(minis_id), "%s-minis", id);
    UINode* minis_container = ui_div(tree, minis_id);
    if (minis_container) {
        SET_SIZE(minis_container, 400, 50);
        ui_set_display_flex(minis_container);
        FLEX_ROW(minis_container);
        ui_set_justify_content(minis_container, "space-between");
        ui_set_align_items(minis_container, "center");
        
        AvatarID avatar_ids[] = {
            AVATAR_WARRIOR, AVATAR_STRATEGIST, AVATAR_DIPLOMAT,
            AVATAR_EXPLORER, AVATAR_MERCHANT, AVATAR_SAGE
        };
        
        for (int i = 0; i < 6; i++) {
            char mini_id[128];
            snprintf(mini_id, sizeof(mini_id), "%s-mini-%d", id, i + 1);
            
            data->mini_avatars[i] = create_mini_avatar_atomic(mini_id, avatar_ids[i], 45);
            if (data->mini_avatars[i]) {
                atomic_set_custom_data(data->mini_avatars[i], "selector_data", data);
                atomic_set_click_handler(data->mini_avatars[i], on_mini_avatar_click);
                
                // 🔧 FIX: S'assurer que l'élément peut recevoir des événements
                atomic_set_z_index(data->mini_avatars[i], 10);
                
                // Ajouter comme enfant atomic au container
                atomic_add_child(minis_container->element, data->mini_avatars[i]);
                printf("✅ Mini avatar %d created with click handler\n", i + 1);
            }
        }
        
        APPEND(selector_container, minis_container);
    }
    
    // Initialiser la sélection visuelle
    update_mini_avatar_selection(data);
    
    data->initialized = true;
    
    ui_log_event("AvatarSelector", "Create", id, "Avatar selector created with 6 clickable avatars");
    printf("✨ AvatarSelector '%s' créé avec 6 avatars cliquables\n", id ? id : "NoID");
    
    return selector_container;
}

// === FONCTIONS UTILITAIRES ===

void ui_avatar_selector_set_callback(UINode* selector, 
                                     void (*callback)(AvatarID avatar, void* user_data),
                                     void* user_data) {
    if (!selector || !selector->component_data) return;
    
    AvatarSelectorData* data = (AvatarSelectorData*)selector->component_data;
    data->on_change_callback = callback;
    data->callback_user_data = user_data;
    
    ui_log_event("AvatarSelector", "Callback", selector->id, "Change callback set");
}

AvatarID ui_avatar_selector_get_selected(UINode* selector) {
    if (!selector || !selector->component_data) return AVATAR_WARRIOR;
    
    AvatarSelectorData* data = (AvatarSelectorData*)selector->component_data;
    return data->selected_avatar;
}

void ui_avatar_selector_set_selected(UINode* selector, AvatarID avatar) {
    if (!selector || !selector->component_data) return;
    
    AvatarSelectorData* data = (AvatarSelectorData*)selector->component_data;
    if (data->selected_avatar != avatar) {
        data->selected_avatar = avatar;
        update_mini_avatar_selection(data);
        update_main_avatar_texture(data);
        
        if (data->on_change_callback) {
            data->on_change_callback(avatar, data->callback_user_data);
        }
    }
    
    ui_log_event("AvatarSelector", "Selection", selector->id, "Avatar selection changed programmatically");
}

void ui_avatar_selector_reset(UINode* selector) {
    if (!selector || !selector->component_data) return;
    
    AvatarSelectorData* data = (AvatarSelectorData*)selector->component_data;
    
    // Reset à AVATAR_WARRIOR (valeur par défaut)
    data->selected_avatar = AVATAR_WARRIOR;
    data->visual_selection_updated = false;
    
    // Mettre à jour les visuels
    update_main_avatar_texture(data);
    update_mini_avatar_selection(data);
    
    printf("🔄 AvatarSelector reset to default (AVATAR_WARRIOR)\n");
    
    ui_log_event("AvatarSelector", "Reset", selector->id, "Avatar selector reset to WARRIOR");
}

void ui_avatar_selector_reset_to_defaults(UINode* selector) {
    if (!selector || !selector->component_data) return;
    
    AvatarSelectorData* data = (AvatarSelectorData*)selector->component_data;
    
    // Reset complet aux valeurs par défaut
    data->selected_avatar = AVATAR_WARRIOR;
    data->visual_selection_updated = false;
    data->on_change_callback = NULL;
    data->callback_user_data = NULL;
    
    // Mettre à jour les visuels
    update_main_avatar_texture(data);
    update_mini_avatar_selection(data);
    
    printf("🔄 AvatarSelector reset to full defaults\n");
}

void ui_avatar_selector_register_events(UINode* selector, EventManager* event_manager) {
    if (!selector || !selector->component_data || !event_manager) return;
    
    AvatarSelectorData* data = (AvatarSelectorData*)selector->component_data;
    
    int registered_count = 0;
    for (int i = 0; i < 6; i++) {
        if (data->mini_avatars[i]) {
            atomic_register_with_event_manager(data->mini_avatars[i], event_manager);
            registered_count++;
            printf("🔗 Mini avatar %d registered with EventManager (ID: %s)\n", 
                   i + 1, data->mini_avatars[i]->id);
        }
    }
    
    ui_log_event("AvatarSelector", "Events", selector->id, "All mini-avatars registered with EventManager");
    printf("🔗 AvatarSelector '%s': %d mini-avatars enregistrés dans EventManager\n", 
           selector->id, registered_count);
}

void ui_avatar_selector_update(UINode* selector, float delta_time) {
    (void)selector;
    (void)delta_time;
    // Placeholder pour futures animations si nécessaire
}
