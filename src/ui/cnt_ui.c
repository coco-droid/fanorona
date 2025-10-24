#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include "../window/window.h"      
#include "../utils/asset_manager.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 🔧 FIX: Déclarer les fonctions AVANT leur utilisation
static void ui_container_add_logo_to_section(UITree* tree, UINode* logo_container);
static void ui_container_add_subtitle_to_section(UITree* tree, UINode* subtitle_container);

// 🆕 NOUVELLE ARCHITECTURE: Container avec sous-containers spécialisés
UINode* ui_container(UITree* tree, const char* id) {
    if (!tree) {
        ui_log_event("UIComponent", "CreateError", id, "Tree is NULL");
        return NULL;
    }
    
    // Créer le container principal
    UINode* main_container = ui_div(tree, id);
    if (!main_container) {
        ui_log_event("UIComponent", "CreateError", id, "Failed to create main container");
        return NULL;
    }
    
    // Style du container principal : modal avec bordure orange
    atomic_set_background_color(main_container->element, 0, 0, 0, 180);
    atomic_set_border(main_container->element, 2, 255, 165, 0, 255);
    atomic_set_padding(main_container->element, 15, 15, 15, 15); // Padding uniforme
    atomic_set_overflow(main_container->element, OVERFLOW_HIDDEN);
    
    // Configuration flexbox du container principal (colonne)
    ui_set_display_flex(main_container);
    FLEX_COLUMN(main_container);
    ui_set_justify_content(main_container, "flex-start"); // Alignement du haut
    ui_set_align_items(main_container, "center"); // Centrage horizontal
    //ui_set_flex_gap(main_container, 0); // Espacement entre les sections
    
    // === 1. CONTAINER POUR LE LOGO ===
    char logo_container_id[128];
    snprintf(logo_container_id, sizeof(logo_container_id), "%s-logo-container", id);
    UINode* logo_container = ui_div(tree, logo_container_id);
    
    if (logo_container) {
        SET_SIZE(logo_container, 400, 100); // Taille dédiée au logo
        ui_set_display_flex(logo_container);
        ui_set_justify_content(logo_container, "center");
        ui_set_align_items(logo_container, "center");
        
        // Ajouter le logo dans son container dédié
        ui_container_add_logo_to_section(main_container->tree, logo_container);
        APPEND(main_container, logo_container);
        
        ui_log_event("UIComponent", "ContainerSection", id, "Logo container created and added");
    }
    
    // === 2. CONTAINER POUR LE SOUS-TITRE ===
    char subtitle_container_id[128];
    snprintf(subtitle_container_id, sizeof(subtitle_container_id), "%s-subtitle-container", id);
    UINode* subtitle_container = ui_div(tree, subtitle_container_id);
    
    if (subtitle_container) {
        SET_SIZE(subtitle_container, 400, 50); // Taille dédiée au sous-titre
        ui_set_display_flex(subtitle_container);
        ui_set_justify_content(subtitle_container, "center");
        ui_set_align_items(subtitle_container, "center");
        
        // Ajouter le sous-titre dans son container dédié
        ui_container_add_subtitle_to_section(main_container->tree, subtitle_container);
        APPEND(main_container, subtitle_container);
        
        ui_log_event("UIComponent", "ContainerSection", id, "Subtitle container created and added");
    }
    
    // === 3. CONTAINER POUR LES BOUTONS (sera ajouté via ui_container_add_content) ===
    char content_container_id[128];
    snprintf(content_container_id, sizeof(content_container_id), "%s-content-container", id);
    UINode* content_container = ui_div(tree, content_container_id);
    
    if (content_container) {
        SET_SIZE(content_container, 450, 250); // Taille généreuse pour le contenu
        ui_set_display_flex(content_container);
        ui_set_justify_content(content_container, "center");
        ui_set_align_items(content_container, "center");
        
        // Stocker la référence du content_container dans l'élément principal pour un accès ultérieur
        // On utilisera un attribut personnalisé ou une méthode pour le retrouver
        atomic_set_custom_data(main_container->element, "content_container", content_container);
        
        APPEND(main_container, content_container);
        
        // 🔧 FIX: Forcer un recalcul initial du layout après ajout des enfants
        atomic_calculate_layout(main_container->element);
        
        ui_log_event("UIComponent", "ContainerSection", id, "Content container created and ready for user content");
    }
    
    ui_log_event("UIComponent", "Create", id, "Container created with 3 specialized sub-containers (logo, subtitle, content)");
    printf("✅ Container '%s' créé avec architecture modulaire :\n", id);
    printf("   📦 Container principal : flexbox column, gap 20px\n");
    printf("   🖼️  Section logo : 400x100, centré\n");
    printf("   📝  Section sous-titre : 400x50, centré\n");
    printf("   🎮  Section contenu : 450x250, centré (prêt pour boutons)\n");
    
    return main_container;
}

// 🆕 FONCTION HELPER: Ajouter le logo à sa section dédiée
static void ui_container_add_logo_to_section(UITree* tree, UINode* logo_container) {
    if (!tree || !logo_container) return;
    
    // Charger le logo Fanorona
    SDL_Texture* logo_texture = NULL;
    GameWindow* window = use_mini_window();
    if (window) {
        SDL_Renderer* renderer = window_get_renderer(window);
        if (renderer) {
            logo_texture = asset_load_texture(renderer, "fanorona_text.png");
        }
    }
    
    UINode* logo = NULL;
    if (logo_texture) {
        logo = ui_image(tree, "container-section-logo", logo_texture);
        if (logo) {
            SET_SIZE(logo, 300, 80); // Taille optimisée pour la section
            atomic_set_background_color(logo->element, 0, 0, 0, 0); // Transparent
            APPEND(logo_container, logo);
        }
    } else {
        // Fallback texte
        logo = ui_text(tree, "container-section-logo-text", "FANORONA");
        if (logo) {
            ui_set_text_color(logo, "rgb(255, 165, 0)");
            ui_set_text_size(logo, 28);
            ui_set_text_align(logo, "center");
            APPEND(logo_container, logo);
        }
    }
    
    printf("🖼️ Logo ajouté dans sa section dédiée (centré automatiquement)\n");
}

// 🆕 FONCTION HELPER: Ajouter le sous-titre à sa section dédiée
static void ui_container_add_subtitle_to_section(UITree* tree, UINode* subtitle_container) {
    if (!tree || !subtitle_container) return;
    
    UINode* subtitle = ui_text(tree, "container-section-subtitle", "STRATEGIE ET TRADITION");
    if (subtitle) {
        ui_set_text_color(subtitle, "rgb(255, 255, 255)");
        ui_set_text_size(subtitle, 16);
        ui_set_text_align(subtitle, "center");
        ui_set_text_style(subtitle, false, true); // Italique
        APPEND(subtitle_container, subtitle);
        
        printf("📝 Sous-titre ajouté dans sa section dédiée (centré automatiquement)\n");
    }
}

// 🔧 REFACTORISER: ui_container_add_content pour utiliser le content_container
void ui_container_add_content(UINode* container, UINode* content) {
    if (!container || !content) {
        ui_log_event("UIComponent", "ContainerError", container ? container->id : "null", 
                    "Invalid parameters for content");
        return;
    }
    
    // Récupérer le content_container stocké
    UINode* content_container = (UINode*)atomic_get_custom_data(container->element, "content_container");
    
    if (content_container) {
        // Le contenu sera automatiquement centré dans le content_container
        APPEND(content_container, content);
        
        ui_log_event("UIComponent", "ContainerContent", container->id, 
                    "Content added to dedicated content section (auto-centered)");
        printf("📦 Contenu ajouté à la section dédiée (centrage automatique via flexbox)\n");
    } else {
        // Fallback : ajouter directement au container principal
        APPEND(container, content);
        ui_log_event("UIComponent", "ContainerContent", container->id, 
                    "Content added directly to main container (fallback)");
        printf("⚠️ Content_container non trouvé, ajout direct au container principal\n");
    }
}

// === FONCTIONS EXISTANTES (compatibilité) ===

UINode* ui_container_with_size(UITree* tree, const char* id, int width, int height) {
    UINode* container = ui_container(tree, id);
    if (container) {
        SET_SIZE(container, width, height);
        ui_log_event("UIComponent", "Style", id, "Container size set");
    }
    return container;
}

UINode* ui_container_centered(UITree* tree, const char* id, int width, int height) {
    UINode* container = ui_container_with_size(tree, id, width, height);
    if (container) {
        ALIGN_SELF_BOTH(container); // 🆕 UTILISER align-self pour centrage
        ui_log_event("UIComponent", "Style", id, "Container centered with align-self");
    }
    return container;
}

void ui_container_add_header(UINode* container, const char* header_text) {
    if (!container || !header_text) {
        ui_log_event("UIComponent", "ContainerError", container ? container->id : "null", 
                    "Invalid parameters for header");
        return;
    }
    
    UINode* header = UI_TEXT(container->tree, "container-header", header_text);
    if (header) {
        ui_set_text_color(header, "rgb(255, 165, 0)"); // Orange pour contraste
        ui_set_text_size(header, 20);
        ui_set_text_align(header, "center");
        
        // 🆕 UTILISER align-self pour centrage
        ALIGN_SELF_X(header);
        APPEND(container, header);
        
        ui_log_event("UIComponent", "ContainerHeader", container->id, "Header added to container with align-self");
    }
}

void ui_container_set_modal_style(UINode* container, bool is_modal) {
    if (!container) return;
    
    if (is_modal) {
        // Style modal complet : overlay sombre
        atomic_set_background_color(container->element, 0, 0, 0, 200);
        atomic_set_border(container->element, 3, 255, 165, 0, 255); // Bordure orange de 3px
        atomic_set_padding(container->element, 2, 2, 2, 2); // 🔧 FIX: Maintenir padding 2px en mode modal
        ui_set_z_index(container, 1000); // Au-dessus de tout
        
        ui_log_event("UIComponent", "ContainerStyle", container->id, "Modal style applied with 2px padding");
    } else {
        // Style normal
        atomic_set_background_color(container->element, 255, 255, 255, 230);
        atomic_set_border(container->element, 1, 128, 128, 128, 255); // Bordure grise de 1px
        atomic_set_padding(container->element, 2, 2, 2, 2); // 🔧 FIX: Maintenir padding 2px en mode normal
        
        ui_log_event("UIComponent", "ContainerStyle", container->id, "Normal style applied with 2px padding");
    }
}
