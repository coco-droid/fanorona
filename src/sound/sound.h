#ifndef SOUND_H
#define SOUND_H

#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

// === TYPES D'EFFETS SONORES ===
typedef enum SoundEffect {
    SOUND_CLICK,           // Clic de bouton
    SOUND_HOVER,           // Survol de bouton
    SOUND_PIECE_MOVE,      // Déplacement de pièce
    SOUND_PIECE_CAPTURE,   // Capture de pièce
    SOUND_VICTORY,         // Victoire
    SOUND_DEFEAT,          // Défaite
    SOUND_INVALID_MOVE,    // Coup invalide
    SOUND_TURN_CHANGE,     // Changement de tour
    SOUND_ERROR,           // Erreur
    SOUND_SUCCESS,         // Succès
    SOUND_EFFECT_COUNT     // Nombre total d'effets
} SoundEffect;

// === TYPES DE MUSIQUE ===
typedef enum MusicTrack {
    MUSIC_MENU,            // Musique du menu
    MUSIC_GAME,            // Musique de jeu
    MUSIC_VICTORY,         // Musique de victoire
    MUSIC_COUNT            // Nombre total de pistes
} MusicTrack;

// === INITIALISATION ET NETTOYAGE ===

// Initialiser le système de son (retourne false en cas d'échec)
bool sound_init(void);

// Nettoyer le système de son
void sound_cleanup(void);

// === CONTRÔLE DES VOLUMES ===

// Régler le volume de la musique (0-100)
void sound_set_music_volume(int volume);

// Régler le volume des effets sonores (0-100)
void sound_set_sfx_volume(int volume);

// Obtenir le volume actuel de la musique
int sound_get_music_volume(void);

// Obtenir le volume actuel des effets
int sound_get_sfx_volume(void);

// === GESTION DE LA MUSIQUE ===

// Charger une piste de musique
bool sound_load_music(MusicTrack track, const char* filepath);

// Jouer une piste de musique (loop: -1 pour infini, 0 pour une fois, >0 pour N fois)
bool sound_play_music(MusicTrack track, int loops);

// Arrêter la musique en cours
void sound_stop_music(void);

// Mettre en pause la musique
void sound_pause_music(void);

// Reprendre la musique
void sound_resume_music(void);

// Fade out de la musique (durée en millisecondes)
void sound_fadeout_music(int duration_ms);

// Vérifier si la musique est en cours de lecture
bool sound_is_music_playing(void);

// === GESTION DES EFFETS SONORES ===

// Charger un effet sonore (avec fallback automatique si fichier introuvable)
// Si le fichier n'existe pas, un son carré 16-bit sera généré automatiquement:
//  - SOUND_CLICK: 1000Hz, 50ms (clic court)
//  - SOUND_HOVER: 800Hz, 30ms (survol doux)
//  - SOUND_PIECE_MOVE: sweep 400-600Hz, 150ms (déplacement)
//  - SOUND_PIECE_CAPTURE: 300Hz+500Hz, 200ms (capture)
//  - SOUND_VICTORY: sweep 400-1200Hz, 500ms (victoire)
//  - SOUND_DEFEAT: sweep 800-200Hz, 600ms (défaite)
//  - SOUND_INVALID_MOVE: 200Hz, 100ms (invalide)
//  - SOUND_TURN_CHANGE: 600Hz+900Hz, 120ms (changement)
//  - SOUND_ERROR: 300Hz, 80ms (erreur)
//  - SOUND_SUCCESS: 1400Hz, 150ms (succès)
bool sound_load_effect(SoundEffect effect, const char* filepath);

// Jouer un effet sonore (retourne le canal utilisé, -1 en cas d'échec)
int sound_play_effect(SoundEffect effect);

// Jouer un effet sonore sur un canal spécifique
int sound_play_effect_on_channel(SoundEffect effect, int channel);

// Arrêter tous les effets sonores
void sound_stop_all_effects(void);

// Arrêter un canal spécifique
void sound_stop_channel(int channel);

// === HELPERS POUR L'INTÉGRATION UI ===

// Jouer le son de clic de bouton
void sound_play_button_click(void);

// Jouer le son de survol de bouton
void sound_play_button_hover(void);

// Jouer le son de déplacement de pièce
void sound_play_piece_move(void);

// Jouer le son de capture
void sound_play_piece_capture(void);

// Jouer le son de coup invalide
void sound_play_invalid_move(void);

// === CONTRÔLE GLOBAL ===

// Activer/désactiver tous les sons
void sound_set_enabled(bool enabled);

// Vérifier si les sons sont activés
bool sound_is_enabled(void);

// Activer/désactiver la musique uniquement
void sound_set_music_enabled(bool enabled);

// Activer/désactiver les effets uniquement
void sound_set_sfx_enabled(bool enabled);

// === DEBUG ===

// Afficher l'état du système de son
void sound_debug_print_status(void);

#endif // SOUND_H
