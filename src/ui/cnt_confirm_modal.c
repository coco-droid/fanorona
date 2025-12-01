#define _POSIX_C_SOURCE 200809L
#include "ui_components.h"
#include "ui_tree.h"
#include "native/atomic.h"
#include "../utils/log_console.h"
#include <stdio.h>
#include <stdlib.h>

// === CONFIRM MODAL COMPONENT ===

UINode* ui_confirm_modal(UITree* tree, const char* id, const char* title, const char* message, 
                         void (*on_yes)(void*, SDL_Event*), void (*on_no)(void*, SDL_Event*)) {
    if (!tree) return NULL;

    // Overlay (plein écran, semi-transparent)
    UINode* overlay = ui_div(tree, id);
    if (!overlay) return NULL;

    // Style overlay : couvre tout l'écran (et plus pour être sûr)
    SET_SIZE(overlay, 2000, 2000); 
    atomic_set_background_color(overlay->element, 0, 0, 0, 200); // Fond noir semi-transparent
    
    // 🔧 FIX: Utiliser ui_center pour centrer l'overlay géant par rapport à la fenêtre active
    // Cela calcule automatiquement (WindowWidth - 2000)/2, ce qui centre le point (1000,1000) de l'overlay
    ui_center(overlay);
    
    ui_set_z_index(overlay, 2000); // Au-dessus de tout (sidebar = 10, playable = 5)
    
    // Flexbox pour centrer la boîte
    ui_set_display_flex(overlay);
    ui_set_justify_content(overlay, "center");
    ui_set_align_items(overlay, "center");
    
    // Masqué par défaut
    atomic_set_display(overlay->element, DISPLAY_NONE);

    // Boîte de dialogue
    char box_id[128];
    snprintf(box_id, sizeof(box_id), "%s-box", id);
    UINode* box = ui_div(tree, box_id);
    SET_SIZE(box, 500, 300); // 🔧 AUGMENTÉ: 450x250 -> 500x300
    atomic_set_background_color(box->element, 40, 40, 40, 255); // Gris foncé
    atomic_set_border(box->element, 2, 255, 69, 0, 255); // Bordure rouge/orange
    atomic_set_border_radius(box->element, 10);
    atomic_set_padding(box->element, 20, 20, 20, 20); // 🆕 AJOUT: Padding interne
    
    ui_set_display_flex(box);
    FLEX_COLUMN(box);
    ui_set_justify_content(box, "space-between"); // 🔧 CHANGÉ: space-evenly -> space-between pour mieux répartir
    ui_set_align_items(box, "center");
    
    // Titre
    UINode* title_node = ui_text(tree, "modal-title", title);
    ui_set_text_color(title_node, "rgb(255, 69, 0)"); // Rouge orangé
    ui_set_text_size(title_node, 24);
    ui_set_text_style(title_node, true, false);
    
    // Message
    UINode* msg_node = ui_text(tree, "modal-msg", message);
    ui_set_text_color(msg_node, "rgb(255, 255, 255)");
    ui_set_text_size(msg_node, 18); // 🔧 AUGMENTÉ: 16 -> 18
    ui_set_text_align(msg_node, "center");
    
    // Conteneur boutons
    UINode* btns = ui_div(tree, "modal-btns");
    SET_SIZE(btns, 460, 60); // 🔧 AUGMENTÉ largeur
    ui_set_display_flex(btns);
    ui_set_flex_direction(btns, "row");
    ui_set_justify_content(btns, "center"); // 🔧 CHANGÉ: space-around -> center avec gap
    ui_set_align_items(btns, "center");
    ui_set_flex_gap(btns, 40); // 🆕 AJOUT: Gap explicite
    
    // Bouton OUI (Danger)
    char yes_id[128];
    snprintf(yes_id, sizeof(yes_id), "%s-yes", id);
    UINode* btn_yes = ui_button(tree, yes_id, "OUI, QUITTER", NULL, NULL);
    SET_SIZE(btn_yes, 150, 45);
    BUTTON_DANGER(btn_yes); 
    if (on_yes) {
        atomic_set_click_handler(btn_yes->element, on_yes);
    }
    
    // Bouton NON (Info/Success)
    char no_id[128];
    snprintf(no_id, sizeof(no_id), "%s-no", id);
    UINode* btn_no = ui_button(tree, no_id, "NON, RESTER", NULL, NULL);
    SET_SIZE(btn_no, 150, 45);
    BUTTON_SUCCESS(btn_no); // Vert pour encourager à rester
    if (on_no) {
        atomic_set_click_handler(btn_no->element, on_no);
    }
    
    APPEND(btns, btn_yes);
    APPEND(btns, btn_no);
    
    APPEND(box, title_node);
    APPEND(box, msg_node);
    APPEND(box, btns);
    
    APPEND(overlay, box);
    
    printf("✅ Modal de confirmation '%s' créé (masqué par défaut)\n", id);
    return overlay;
}
