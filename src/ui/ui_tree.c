#define _POSIX_C_SOURCE 200809L
#include "ui_tree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Arbre global pour les fonctions de raccourci
static UITree* g_global_tree = NULL;

// === FONCTIONS UTILITAIRES INTERNES ===

static void ui_tree_add_to_id_map(UITree* tree, UINode* node) {
    if (!tree || !node || !node->id) return;
    
    // Vérifier si on a besoin de redimensionner
    if (tree->id_count >= tree->id_capacity) {
        tree->id_capacity = tree->id_capacity == 0 ? 16 : tree->id_capacity * 2;
        tree->id_map = (UINode**)realloc(tree->id_map, tree->id_capacity * sizeof(UINode*));
        tree->id_keys = (char**)realloc(tree->id_keys, tree->id_capacity * sizeof(char*));
    }
    
    // Ajouter à la map
    tree->id_keys[tree->id_count] = strdup(node->id);
    tree->id_map[tree->id_count] = node;
    tree->id_count++;
}

static void ui_tree_remove_from_id_map(UITree* tree, const char* id) {
    if (!tree || !id) return;
    
    for (int i = 0; i < tree->id_count; i++) {
        if (strcmp(tree->id_keys[i], id) == 0) {
            free(tree->id_keys[i]);
            
            // Décaler les éléments suivants
            for (int j = i; j < tree->id_count - 1; j++) {
                tree->id_keys[j] = tree->id_keys[j + 1];
                tree->id_map[j] = tree->id_map[j + 1];
            }
            tree->id_count--;
            break;
        }
    }
}

// === FONCTIONS DE L'ARBRE ===

UITree* ui_tree_create(void) {
    UITree* tree = (UITree*)calloc(1, sizeof(UITree));
    if (!tree) {
        printf("Erreur: Impossible d'allouer la mémoire pour l'arbre UI\n");
        return NULL;
    }
    
    // Créer le nœud racine
    tree->root = ui_tree_create_node(tree, "root", "document");
    if (!tree->root) {
        free(tree);
        return NULL;
    }
    
    // Créer l'event manager
    tree->event_manager = event_manager_create();
    
    return tree;
}

void ui_tree_destroy(UITree* tree) {
    if (!tree) return;
    
    // Détruire l'arbre récursivement
    ui_tree_destroy_node(tree->root);
    
    // Nettoyer la map des IDs
    for (int i = 0; i < tree->id_count; i++) {
        free(tree->id_keys[i]);
    }
    free(tree->id_keys);
    free(tree->id_map);
    
    // Détruire l'event manager
    if (tree->event_manager) {
        event_manager_destroy(tree->event_manager);
    }
    
    free(tree);
}

// === GESTION DES NŒUDS ===

UINode* ui_tree_create_node(UITree* tree, const char* id, const char* tag_name) {
    UINode* node = (UINode*)calloc(1, sizeof(UINode));
    if (!node) {
        printf("Erreur: Impossible d'allouer la mémoire pour le nœud UI\n");
        return NULL;
    }
    
    // Initialiser les propriétés avec l'ordre correct
    node->tag_name = tag_name ? strdup(tag_name) : NULL;
    node->id = id ? strdup(id) : NULL;
    node->tree = tree;
    
    // Initialiser le tableau des enfants
    node->children_capacity = 4;
    node->children = (UINode**)calloc(node->children_capacity, sizeof(UINode*));
    
    // Créer l'élément atomique associé
    node->element = atomic_create(id);
    if (node->element) {
        node->element->user_data = node; // Liaison inverse
    }
    
    // Ajouter à la map des IDs
    if (tree && id) {
        ui_tree_add_to_id_map(tree, node);
    }
    
    return node;
}

void ui_tree_destroy_node(UINode* node) {
    if (!node) return;
    
    // Détruire les enfants récursivement
    for (int i = 0; i < node->children_count; i++) {
        ui_tree_destroy_node(node->children[i]);
    }
    
    // Supprimer de la map des IDs
    if (node->tree && node->id) {
        ui_tree_remove_from_id_map(node->tree, node->id);
    }
    
    // Détruire les données du composant
    if (node->component_data && node->component_destroy) {
        node->component_destroy(node->component_data);
    }
    
    // Libérer les ressources
    free(node->tag_name);
    free(node->id);
    free(node->class_name);
    free(node->children);
    
    if (node->element) {
        atomic_destroy(node->element);
    }
    
    free(node);
}

// === NAVIGATION DANS L'ARBRE ===

UINode* ui_tree_get_by_id(UITree* tree, const char* id) {
    if (!tree || !id) return NULL;
    
    for (int i = 0; i < tree->id_count; i++) {
        if (strcmp(tree->id_keys[i], id) == 0) {
            return tree->id_map[i];
        }
    }
    
    return NULL;
}

UINode* ui_tree_query_selector(UITree* tree, const char* selector) {
    if (!tree || !selector) return NULL;
    
    // Implémentation simple pour #id et .class
    if (selector[0] == '#') {
        return ui_tree_get_by_id(tree, selector + 1);
    }
    
    // TODO: Implémenter la recherche par classe et tag
    return NULL;
}

UINode** ui_tree_query_selector_all(UITree* tree, const char* selector, int* count) {
    if (!tree || !selector || !count) return NULL;
    
    *count = 0;
    // TODO: Implémenter la recherche multiple
    return NULL;
}

// === MANIPULATION DE L'ARBRE ===

void ui_tree_append_child(UINode* parent, UINode* child) {
    if (!parent || !child) return;
    
    // Vérifier si on a besoin de redimensionner
    if (parent->children_count >= parent->children_capacity) {
        parent->children_capacity *= 2;
        parent->children = (UINode**)realloc(parent->children, 
                                           parent->children_capacity * sizeof(UINode*));
    }
    
    // Ajouter l'enfant
    parent->children[parent->children_count] = child;
    parent->children_count++;
    child->parent = parent;
    
    // Gérer la hiérarchie atomique
    if (parent->element && child->element) {
        atomic_add_child(parent->element, child->element);
    }
}

void ui_tree_remove_child(UINode* parent, UINode* child) {
    if (!parent || !child) return;
    
    for (int i = 0; i < parent->children_count; i++) {
        if (parent->children[i] == child) {
            // Décaler les éléments suivants
            for (int j = i; j < parent->children_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->children_count--;
            child->parent = NULL;
            
            // Gérer la hiérarchie atomique
            if (parent->element && child->element) {
                atomic_remove_child(parent->element, child->element);
            }
            break;
        }
    }
}

void ui_tree_insert_before(UINode* parent, UINode* new_node, UINode* reference) {
    if (!parent || !new_node) return;
    
    if (!reference) {
        ui_tree_append_child(parent, new_node);
        return;
    }
    
    // Trouver la position de référence
    int ref_index = -1;
    for (int i = 0; i < parent->children_count; i++) {
        if (parent->children[i] == reference) {
            ref_index = i;
            break;
        }
    }
    
    if (ref_index == -1) {
        ui_tree_append_child(parent, new_node);
        return;
    }
    
    // Vérifier la capacité
    if (parent->children_count >= parent->children_capacity) {
        parent->children_capacity *= 2;
        parent->children = (UINode**)realloc(parent->children, 
                                           parent->children_capacity * sizeof(UINode*));
    }
    
    // Décaler les éléments
    for (int i = parent->children_count; i > ref_index; i--) {
        parent->children[i] = parent->children[i - 1];
    }
    
    // Insérer le nouveau nœud
    parent->children[ref_index] = new_node;
    parent->children_count++;
    new_node->parent = parent;
}

// === PROPRIÉTÉS ET STYLE ===

void ui_node_set_attribute(UINode* node, const char* name, const char* value) {
    if (!node || !name) return;
    
    // Mapper les attributs vers les propriétés atomiques
    if (strcmp(name, "width") == 0) {
        atomic_set_size(node->element, atoi(value), node->element->style.height);
    } else if (strcmp(name, "height") == 0) {
        atomic_set_size(node->element, node->element->style.width, atoi(value));
    } else if (strcmp(name, "x") == 0) {
        atomic_set_position(node->element, atoi(value), node->element->style.y);
    } else if (strcmp(name, "y") == 0) {
        atomic_set_position(node->element, node->element->style.x, atoi(value));
    }
    // TODO: Ajouter plus d'attributs
}

const char* ui_node_get_attribute(UINode* node, const char* name) {
    if (!node || !name) return NULL;
    
    static char buffer[32];
    
    if (strcmp(name, "width") == 0) {
        snprintf(buffer, sizeof(buffer), "%d", node->element->style.width);
        return buffer;
    } else if (strcmp(name, "height") == 0) {
        snprintf(buffer, sizeof(buffer), "%d", node->element->style.height);
        return buffer;
    }
    
    return NULL;
}

void ui_node_set_style(UINode* node, const char* property, const char* value) {
    if (!node || !property || !value) return;
    
    // Mapper les propriétés CSS vers les propriétés atomiques
    if (strcmp(property, "width") == 0) {
        atomic_set_size(node->element, atoi(value), node->element->style.height);
    } else if (strcmp(property, "height") == 0) {
        atomic_set_size(node->element, node->element->style.width, atoi(value));
    } else if (strcmp(property, "left") == 0 || strcmp(property, "x") == 0) {
        atomic_set_position(node->element, atoi(value), node->element->style.y);
    } else if (strcmp(property, "top") == 0 || strcmp(property, "y") == 0) {
        atomic_set_position(node->element, node->element->style.x, atoi(value));
    } else if (strcmp(property, "background-color") == 0) {
        // Parser la couleur (format simple rgb(r,g,b))
        int r, g, b;
        if (sscanf(value, "rgb(%d,%d,%d)", &r, &g, &b) == 3) {
            atomic_set_background_color(node->element, r, g, b, 255);
        }
    } else if (strcmp(property, "display") == 0) {
        if (strcmp(value, "none") == 0) {
            atomic_set_display(node->element, DISPLAY_NONE);
        } else if (strcmp(value, "block") == 0) {
            atomic_set_display(node->element, DISPLAY_BLOCK);
        }
    }
}

void ui_node_add_class(UINode* node, const char* class_name) {
    if (!node || !class_name) return;
    
    if (!node->class_name) {
        node->class_name = strdup(class_name);
    } else {
        // Ajouter à la liste des classes existantes
        size_t old_len = strlen(node->class_name);
        size_t new_len = old_len + strlen(class_name) + 2; // +2 pour " " et "\0"
        char* new_classes = (char*)malloc(new_len);
        snprintf(new_classes, new_len, "%s %s", node->class_name, class_name);
        free(node->class_name);
        node->class_name = new_classes;
    }
}

void ui_node_remove_class(UINode* node, const char* class_name) {
    if (!node || !class_name || !node->class_name) return;
    
    // TODO: Implémenter la suppression de classe
}

bool ui_node_has_class(UINode* node, const char* class_name) {
    if (!node || !class_name || !node->class_name) return false;
    
    return strstr(node->class_name, class_name) != NULL;
}

void ui_node_set_text(UINode* node, const char* text) {
    if (!node) return;
    atomic_set_text(node->element, text);
}

// === ÉVÉNEMENTS ===

void ui_node_add_event_listener(UINode* node, const char* event, void (*callback)(UINode*, void*), void* user_data) {
    if (!node || !event || !callback) return;
    
    // Mapper les événements vers les callbacks atomiques
    if (strcmp(event, "click") == 0) {
        // Wrapper pour adapter la signature
        atomic_set_click_handler(node->element, (void(*)(void*, SDL_Event*))callback);
    } else if (strcmp(event, "hover") == 0 || strcmp(event, "mouseenter") == 0) {
        atomic_set_hover_handler(node->element, (void(*)(void*, SDL_Event*))callback);
    }
    
    (void)user_data; // TODO: Gérer user_data
}

void ui_node_remove_event_listener(UINode* node, const char* event, void (*callback)(UINode*, void*)) {
    if (!node || !event || !callback) return;
    
    // TODO: Implémenter la suppression d'événements
}

// === RENDU ===

void ui_tree_update(UITree* tree, float delta_time) {
    if (!tree || !tree->root) return;
    
    atomic_update(tree->root->element, delta_time);
}

void ui_tree_render(UITree* tree, SDL_Renderer* renderer) {
    if (!tree || !tree->root || !renderer) return;
    
    atomic_render(tree->root->element, renderer);
}

// === HELPERS POUR SYNTAXE SIMPLIFIÉE ===

UINode* ui_create_element(UITree* tree, const char* tag_name) {
    return ui_tree_create_node(tree, tag_name, NULL);
}

UINode* ui_create_element_with_id(UITree* tree, const char* tag_name, const char* id) {
    return ui_tree_create_node(tree, tag_name, id);
}

// === FONCTIONS GLOBALES ===

void ui_set_global_tree(UITree* tree) {
    g_global_tree = tree;
}

UITree* ui_get_global_tree(void) {
    return g_global_tree;
}

UINode* $(const char* selector) {
    if (!g_global_tree) return NULL;
    return ui_tree_query_selector(g_global_tree, selector);
}

UINode** $$(const char* selector, int* count) {
    if (!g_global_tree) return NULL;
    return ui_tree_query_selector_all(g_global_tree, selector, count);
}