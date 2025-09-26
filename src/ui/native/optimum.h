#ifndef OPTIMUM_H
#define OPTIMUM_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Forward declarations
typedef struct AtomicElement AtomicElement;
typedef struct UITree UITree;

// === OPTIMUM RENDER ENGINE ===
// Moteur de rendu dédié pour l'UI atomique

/**
 * Rendre un élément atomique et tous ses enfants
 * @param element L'élément à rendre
 * @param renderer Le renderer SDL2
 */
void optimum_render_element(AtomicElement* element, SDL_Renderer* renderer);

/**
 * Rendre un arbre UI complet avec le moteur Optimum
 * @param tree L'arbre UI à rendre
 * @param renderer Le renderer SDL2
 */
void optimum_render_ui_tree(UITree* tree, SDL_Renderer* renderer);

/**
 * Nettoyer les ressources du moteur Optimum
 */
void optimum_cleanup(void);

// === FONCTIONS DE DEBUG ===

/**
 * Rendre les limites de débug d'un élément
 * @param element L'élément à débugger
 * @param renderer Le renderer SDL2
 * @param show_content_rect Afficher aussi le rectangle de contenu
 */
void optimum_debug_render_bounds(AtomicElement* element, SDL_Renderer* renderer, bool show_content_rect);

/**
 * Afficher les informations de performance
 * @param renderer Le renderer SDL2
 * @param elements_rendered Nombre d'éléments rendus
 * @param render_time_ms Temps de rendu en millisecondes
 */
void optimum_render_performance_info(SDL_Renderer* renderer, int elements_rendered, float render_time_ms);

#endif // OPTIMUM_H
