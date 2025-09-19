#ifndef UI_LINK_H
#define UI_LINK_H

#include "../ui_tree.h"
#include "../../scene/scene.h"

// 🔧 FIX: Forward declaration pour éviter la dépendance circulaire
typedef struct UILinkData UILinkData;

// Structure pour les données du lien
struct UILinkData {
    char* target_scene_id;               // ID de la scène cible
    SceneTransitionOption transition;    // Type de transition
    WindowType target_window;            // Fenêtre cible (si applicable)
    void (*on_click)(UINode* link);      // Callback personnalisé lors du clic
    SceneManager* manager;               // 🆕 Référence au SceneManager pour les transitions
};

// Créer un lien de navigation vers une scène
UINode* ui_create_link(UITree* tree, const char* id, const char* text, 
                      const char* target_scene_id, SceneTransitionOption transition);

// Définir les propriétés du lien
void ui_link_set_target(UINode* link, const char* target_scene_id);
void ui_link_set_transition(UINode* link, SceneTransitionOption transition);
void ui_link_set_target_window(UINode* link, WindowType window_type);
void ui_link_set_click_handler(UINode* link, void (*on_click)(UINode* link));

// Connecter le lien au SceneManager
void ui_link_connect_to_manager(UINode* link, SceneManager* manager);

// 🆕 Nouvelle fonction pour récupérer les données de lien
UILinkData* ui_link_get_data(UINode* link);

// 🆕 Fonction helper pour éviter les dépendances circulaires
const char* ui_link_get_target_scene_id_from_data(void* data);

#endif // UI_LINK_H
