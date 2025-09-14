#ifndef UI_TREE_H
#define UI_TREE_H

#include "native/atomic.h"
#include <stdbool.h>

// Forward declarations
typedef struct UITree UITree;
typedef struct UINode UINode;

// Structure pour un nœud de l'arbre UI
struct UINode {
    char* id;                    // Identifiant unique
    char* class_name;            // Classe CSS-like
    char* tag_name;              // Type de composant (button, div, etc.)
    
    AtomicElement* element;      // Élément atomique associé
    
    UINode* parent;              // Nœud parent
    UINode** children;           // Tableau des enfants
    int children_count;
    int children_capacity;
    
    UITree* tree;                // Référence vers l'arbre
    
    // Données spécifiques au composant
    void* component_data;
    void (*component_destroy)(void* data);
};

// Structure pour l'arbre UI
struct UITree {
    UINode* root;                // Nœud racine
    UINode** id_map;             // Map des IDs pour recherche rapide
    char** id_keys;
    int id_count;
    int id_capacity;
    
    EventManager* event_manager; // Gestionnaire d'événements
};

// === FONCTIONS DE L'ARBRE ===

// Création et destruction
UITree* ui_tree_create(void);
void ui_tree_destroy(UITree* tree);

// Gestion des nœuds
UINode* ui_tree_create_node(UITree* tree, const char* id, const char* tag_name);
void ui_tree_destroy_node(UINode* node);

// Navigation dans l'arbre (syntaxe DOM-like)
UINode* ui_tree_get_by_id(UITree* tree, const char* id);
UINode* ui_tree_query_selector(UITree* tree, const char* selector);
UINode** ui_tree_query_selector_all(UITree* tree, const char* selector, int* count);

// Manipulation de l'arbre
void ui_tree_append_child(UINode* parent, UINode* child);
void ui_tree_remove_child(UINode* parent, UINode* child);
void ui_tree_insert_before(UINode* parent, UINode* new_node, UINode* reference);

// === FONCTIONS DU NŒUD ===

// Propriétés et attributs
void ui_node_set_attribute(UINode* node, const char* name, const char* value);
const char* ui_node_get_attribute(UINode* node, const char* name);

// Style (syntaxe CSS-like)
void ui_node_set_style(UINode* node, const char* property, const char* value);
void ui_node_add_class(UINode* node, const char* class_name);
void ui_node_remove_class(UINode* node, const char* class_name);
bool ui_node_has_class(UINode* node, const char* class_name);

// Contenu
void ui_node_set_text(UINode* node, const char* text);
void ui_node_set_inner_html(UINode* node, const char* html); // Pour plus tard

// Événements (syntaxe DOM-like)
void ui_node_add_event_listener(UINode* node, const char* event, void (*callback)(UINode*, void*), void* user_data);
void ui_node_remove_event_listener(UINode* node, const char* event, void (*callback)(UINode*, void*));

// === FONCTIONS DE RENDU ===

void ui_tree_update(UITree* tree, float delta_time);
void ui_tree_render(UITree* tree, SDL_Renderer* renderer);

// === HELPERS POUR SYNTAXE SIMPLIFIÉE ===

// Création fluide (chaînable)
UINode* ui_create_element(UITree* tree, const char* tag_name);
UINode* ui_create_element_with_id(UITree* tree, const char* tag_name, const char* id);

// Fonctions globales pour syntaxe simplifiée
UINode* $(const char* selector);  // Équivalent à document.querySelector
UINode** $$(const char* selector, int* count); // Équivalent à document.querySelectorAll

// Setters de l'arbre global pour les fonctions $
void ui_set_global_tree(UITree* tree);
UITree* ui_get_global_tree(void);

#endif // UI_TREE_H