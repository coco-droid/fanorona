#define _POSIX_C_SOURCE 200809L
#include "sound.h"
#include "../utils/asset_manager.h" // 🆕 AJOUT: Pour la gestion des chemins
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h> // 🆕 FIX: Include SDL_mixer explicitly
#include <stdio.h>
#include <string.h>

// === CONSTANTES ===
#define MAX_SFX_CHANNELS 16
#define DEFAULT_MUSIC_VOLUME 50
#define DEFAULT_SFX_VOLUME 70

// 🆕 CONSTANTES POUR GÉNÉRATION DE SONS
#define SAMPLE_RATE 44100
#define AUDIO_FORMAT AUDIO_S16LSB
#define CHANNELS 1
#define BYTES_PER_SAMPLE 2

// === VARIABLES GLOBALES ===
static bool sound_system_initialized = false;
static bool sound_enabled = true;
static bool music_enabled = true;
static bool sfx_enabled = true;

static int music_volume = DEFAULT_MUSIC_VOLUME;
static int sfx_volume = DEFAULT_SFX_VOLUME;

static Mix_Music* music_tracks[MUSIC_COUNT] = {NULL};
static Mix_Chunk* sound_effects[SOUND_EFFECT_COUNT] = {NULL};

static MusicTrack current_music_track = MUSIC_COUNT; // Aucune piste

// === INITIALISATION ===

bool sound_init(void) {
    if (sound_system_initialized) {
        printf("⚠️ Système de son déjà initialisé\n");
        return true;
    }
    
    printf("🔊 Initialisation du système de son...\n");
    
    // 🆕 FIX: Initialiser le sous-système audio SDL explicitement
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        printf("❌ Erreur SDL_InitSubSystem(AUDIO): %s\n", SDL_GetError());
        return false;
    }
    
    // Initialiser SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("❌ Erreur Mix_OpenAudio: %s\n", Mix_GetError());
        return false;
    }
    
    // Allouer les canaux pour les effets sonores
    Mix_AllocateChannels(MAX_SFX_CHANNELS);
    
    // Configurer les volumes initiaux
    Mix_VolumeMusic(MIX_MAX_VOLUME * music_volume / 100);
    Mix_Volume(-1, MIX_MAX_VOLUME * sfx_volume / 100);
    
    sound_system_initialized = true;
    printf("✅ Système de son initialisé (Musique: %d%%, Effets: %d%%)\n", 
           music_volume, sfx_volume);
    
    // 🆕 CHARGEMENT AUTOMATIQUE DES SONS PAR DÉFAUT
    // Utilisation de noms de fichiers simples, asset_manager gère le chemin
    sound_load_effect(SOUND_CLICK, "click.wav");
    sound_load_effect(SOUND_PIECE_CAPTURE, "capture.wav");
    sound_load_effect(SOUND_PIECE_MOVE, "deplacement.wav");
    
    return true;
}

void sound_cleanup(void) {
    if (!sound_system_initialized) return;
    
    printf("🧹 Nettoyage du système de son...\n");
    
    // Arrêter toute la musique
    sound_stop_music();
    
    // Arrêter tous les effets
    sound_stop_all_effects();
    
    // Libérer les pistes de musique
    for (int i = 0; i < MUSIC_COUNT; i++) {
        if (music_tracks[i]) {
            Mix_FreeMusic(music_tracks[i]);
            music_tracks[i] = NULL;
        }
    }
    
    // Libérer les effets sonores
    for (int i = 0; i < SOUND_EFFECT_COUNT; i++) {
        if (sound_effects[i]) {
            Mix_FreeChunk(sound_effects[i]);
            sound_effects[i] = NULL;
        }
    }
    
    Mix_CloseAudio();
    sound_system_initialized = false;
    
    printf("✅ Système de son nettoyé\n");
}

// === CONTRÔLE DES VOLUMES ===

void sound_set_music_volume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    music_volume = volume;
    Mix_VolumeMusic(MIX_MAX_VOLUME * music_volume / 100);
    
    printf("🎵 Volume musique: %d%%\n", music_volume);
}

void sound_set_sfx_volume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    sfx_volume = volume;
    Mix_Volume(-1, MIX_MAX_VOLUME * sfx_volume / 100);
    
    printf("🔊 Volume effets: %d%%\n", sfx_volume);
}

int sound_get_music_volume(void) {
    return music_volume;
}

int sound_get_sfx_volume(void) {
    return sfx_volume;
}

// === GESTION DE LA MUSIQUE ===

bool sound_load_music(MusicTrack track, const char* filepath) {
    if (!sound_system_initialized) {
        printf("❌ Système de son non initialisé\n");
        return false;
    }
    
    if (track >= MUSIC_COUNT) {
        printf("❌ Track invalide: %d\n", track);
        return false;
    }
    
    // Libérer l'ancienne piste si elle existe
    if (music_tracks[track]) {
        Mix_FreeMusic(music_tracks[track]);
        music_tracks[track] = NULL;
    }
    
    // Charger la nouvelle piste
    music_tracks[track] = Mix_LoadMUS(filepath);
    if (!music_tracks[track]) {
        printf("❌ Erreur chargement musique '%s': %s\n", filepath, Mix_GetError());
        return false;
    }
    
    printf("✅ Musique chargée: %s (track %d)\n", filepath, track);
    return true;
}

bool sound_play_music(MusicTrack track, int loops) {
    if (!sound_system_initialized || !music_enabled || !sound_enabled) {
        return false;
    }
    
    if (track >= MUSIC_COUNT || !music_tracks[track]) {
        printf("❌ Track invalide ou non chargée: %d\n", track);
        return false;
    }
    
    if (Mix_PlayMusic(music_tracks[track], loops) == -1) {
        printf("❌ Erreur lecture musique: %s\n", Mix_GetError());
        return false;
    }
    
    current_music_track = track;
    printf("🎵 Lecture musique (track %d, loops %d)\n", track, loops);
    return true;
}

void sound_stop_music(void) {
    Mix_HaltMusic();
    current_music_track = MUSIC_COUNT;
}

void sound_pause_music(void) {
    Mix_PauseMusic();
}

void sound_resume_music(void) {
    Mix_ResumeMusic();
}

void sound_fadeout_music(int duration_ms) {
    Mix_FadeOutMusic(duration_ms);
}

bool sound_is_music_playing(void) {
    return Mix_PlayingMusic() != 0;
}

// === GESTION DES EFFETS SONORES ===

static Mix_Chunk* generate_square_wave(int frequency, int duration_ms, float amplitude) {
    // 🔧 FIX: Générer pour STEREO (2 canaux) car Mix_OpenAudio utilise channels=2
    int frames = (SAMPLE_RATE * duration_ms) / 1000;
    int sample_count = frames * 2; // L + R
    int buffer_size = sample_count * BYTES_PER_SAMPLE;
    
    Sint16* buffer = (Sint16*)malloc(buffer_size);
    if (!buffer) {
        printf("❌ Erreur allocation mémoire pour son généré\n");
        return NULL;
    }
    
    // Générer une onde carrée
    int half_period = SAMPLE_RATE / (2 * frequency);
    Sint16 high_value = (Sint16)(32767 * amplitude);
    Sint16 low_value = (Sint16)(-32767 * amplitude);
    
    for (int i = 0; i < frames; i++) {
        Sint16 value = ((i / half_period) % 2 == 0) ? high_value : low_value;
        buffer[2*i] = value;     // Left
        buffer[2*i+1] = value;   // Right
    }
    
    // Créer le Mix_Chunk
    Mix_Chunk* chunk = (Mix_Chunk*)malloc(sizeof(Mix_Chunk));
    if (!chunk) {
        free(buffer);
        printf("❌ Erreur allocation Mix_Chunk\n");
        return NULL;
    }
    
    chunk->allocated = 1;
    chunk->abuf = (Uint8*)buffer;
    chunk->alen = buffer_size;
    chunk->volume = MIX_MAX_VOLUME;
    
    return chunk;
}

// 🆕 GÉNÉRATEUR DE DOUBLE TONALITÉ (pour effets plus complexes)
static Mix_Chunk* generate_dual_tone(int freq1, int freq2, int duration_ms, float amplitude) {
    // 🔧 FIX: Générer pour STEREO
    int frames = (SAMPLE_RATE * duration_ms) / 1000;
    int sample_count = frames * 2;
    int buffer_size = sample_count * BYTES_PER_SAMPLE;
    
    Sint16* buffer = (Sint16*)malloc(buffer_size);
    if (!buffer) return NULL;
    
    int half_period1 = SAMPLE_RATE / (2 * freq1);
    int half_period2 = SAMPLE_RATE / (2 * freq2);
    Sint16 high = (Sint16)(32767 * amplitude);
    Sint16 low = (Sint16)(-32767 * amplitude);
    
    for (int i = 0; i < frames; i++) {
        Sint16 wave1 = ((i / half_period1) % 2 == 0) ? high : low;
        Sint16 wave2 = ((i / half_period2) % 2 == 0) ? high : low;
        Sint16 value = (Sint16)((wave1 + wave2) / 2);
        buffer[2*i] = value;     // Left
        buffer[2*i+1] = value;   // Right
    }
    
    Mix_Chunk* chunk = (Mix_Chunk*)malloc(sizeof(Mix_Chunk));
    if (!chunk) {
        free(buffer);
        return NULL;
    }
    
    chunk->allocated = 1;
    chunk->abuf = (Uint8*)buffer;
    chunk->alen = buffer_size;
    chunk->volume = MIX_MAX_VOLUME;
    
    return chunk;
}

// 🆕 GÉNÉRATEUR DE SWEEP (balayage de fréquence)
static Mix_Chunk* generate_sweep(int start_freq, int end_freq, int duration_ms, float amplitude) {
    // 🔧 FIX: Générer pour STEREO
    int frames = (SAMPLE_RATE * duration_ms) / 1000;
    int sample_count = frames * 2;
    int buffer_size = sample_count * BYTES_PER_SAMPLE;
    
    Sint16* buffer = (Sint16*)malloc(buffer_size);
    if (!buffer) return NULL;
    
    Sint16 high = (Sint16)(32767 * amplitude);
    Sint16 low = (Sint16)(-32767 * amplitude);
    
    for (int i = 0; i < frames; i++) {
        float progress = (float)i / frames;
        int current_freq = start_freq + (int)((end_freq - start_freq) * progress);
        int half_period = SAMPLE_RATE / (2 * current_freq);
        Sint16 value = ((i / half_period) % 2 == 0) ? high : low;
        buffer[2*i] = value;     // Left
        buffer[2*i+1] = value;   // Right
    }
    
    Mix_Chunk* chunk = (Mix_Chunk*)malloc(sizeof(Mix_Chunk));
    if (!chunk) {
        free(buffer);
        return NULL;
    }
    
    chunk->allocated = 1;
    chunk->abuf = (Uint8*)buffer;
    chunk->alen = buffer_size;
    chunk->volume = MIX_MAX_VOLUME;
    
    return chunk;
}

// 🆕 CRÉER SONS DE FALLBACK POUR CHAQUE EFFET
static Mix_Chunk* create_fallback_sound(SoundEffect effect) {
    switch (effect) {
        case SOUND_CLICK:
            // Clic court: 1000Hz, 50ms
            printf("🔊 Génération son fallback: CLICK (1000Hz, 50ms)\n");
            return generate_square_wave(1000, 50, 0.3f);
            
        case SOUND_HOVER:
            // Survol doux: 800Hz, 30ms
            printf("🔊 Génération son fallback: HOVER (800Hz, 30ms)\n");
            return generate_square_wave(800, 30, 0.2f);
            
        case SOUND_PIECE_MOVE:
            // Déplacement: sweep 400Hz -> 600Hz, 150ms
            printf("🔊 Génération son fallback: PIECE_MOVE (sweep 400-600Hz, 150ms)\n");
            return generate_sweep(400, 600, 150, 0.4f);
            
        case SOUND_PIECE_CAPTURE:
            // Capture: double tonalité 300Hz + 500Hz, 200ms
            printf("🔊 Génération son fallback: PIECE_CAPTURE (300Hz+500Hz, 200ms)\n");
            return generate_dual_tone(300, 500, 200, 0.5f);
            
        case SOUND_VICTORY:
            // Victoire: sweep ascendant 400Hz -> 1200Hz, 500ms
            printf("🔊 Génération son fallback: VICTORY (sweep 400-1200Hz, 500ms)\n");
            return generate_sweep(400, 1200, 500, 0.6f);
            
        case SOUND_DEFEAT:
            // Défaite: sweep descendant 800Hz -> 200Hz, 600ms
            printf("🔊 Génération son fallback: DEFEAT (sweep 800-200Hz, 600ms)\n");
            return generate_sweep(800, 200, 600, 0.5f);
            
        case SOUND_INVALID_MOVE:
            // Coup invalide: tonalité basse 200Hz, 100ms
            printf("🔊 Génération son fallback: INVALID_MOVE (200Hz, 100ms)\n");
            return generate_square_wave(200, 100, 0.6f);
            
        case SOUND_TURN_CHANGE:
            // Changement tour: double tonalité 600Hz + 900Hz, 120ms
            printf("🔊 Génération son fallback: TURN_CHANGE (600Hz+900Hz, 120ms)\n");
            return generate_dual_tone(600, 900, 120, 0.4f);
            
        case SOUND_ERROR:
            // Erreur: triple bip 300Hz, 3x80ms
            printf("🔊 Génération son fallback: ERROR (300Hz, triple)\n");
            return generate_square_wave(300, 80, 0.7f);
            
        case SOUND_SUCCESS:
            // Succès: tonalité aiguë 1400Hz, 150ms
            printf("🔊 Génération son fallback: SUCCESS (1400Hz, 150ms)\n");
            return generate_square_wave(1400, 150, 0.5f);
            
        default:
            // Son générique: 440Hz (La), 100ms
            printf("🔊 Génération son fallback: GENERIC (440Hz, 100ms)\n");
            return generate_square_wave(440, 100, 0.4f);
    }
}

bool sound_load_effect(SoundEffect effect, const char* filename) {
    if (!sound_system_initialized) {
        printf("❌ Système de son non initialisé\n");
        return false;
    }
    
    if (effect >= SOUND_EFFECT_COUNT) {
        printf("❌ Effet invalide: %d\n", effect);
        return false;
    }
    
    // Libérer l'ancien effet si il existe
    if (sound_effects[effect]) {
        Mix_FreeChunk(sound_effects[effect]);
        sound_effects[effect] = NULL;
    }
    
    // 🔧 MODIFICATION: Utiliser asset_manager pour obtenir le chemin complet
    char* full_path = asset_get_full_path(filename);
    
    // Essayer de charger avec le chemin résolu, sinon fallback sur le nom brut
    sound_effects[effect] = Mix_LoadWAV(full_path ? full_path : filename);
    
    if (!sound_effects[effect]) {
        printf("⚠️ Échec chargement effet '%s': %s\n", filename, Mix_GetError());
        printf("🔄 Utilisation du son généré en fallback...\n");
        
        // 🆕 FALLBACK: Générer le son
        sound_effects[effect] = create_fallback_sound(effect);
        
        if (!sound_effects[effect]) {
            printf("❌ Échec génération son fallback pour effet %d\n", effect);
            return false;
        }
        
        printf("✅ Son fallback généré pour effet %d\n", effect);
        return true;
    }
    
    printf("✅ Effet sonore chargé: %s (effect %d)\n", filename, effect);
    return true;
}

int sound_play_effect(SoundEffect effect) {
    return sound_play_effect_on_channel(effect, -1);
}

int sound_play_effect_on_channel(SoundEffect effect, int channel) {
    if (!sound_system_initialized || !sfx_enabled || !sound_enabled) {
        return -1;
    }
    
    if (effect >= SOUND_EFFECT_COUNT || !sound_effects[effect]) {
        printf("❌ Effet invalide ou non chargé: %d\n", effect);
        return -1;
    }
    
    int result = Mix_PlayChannel(channel, sound_effects[effect], 0);
    if (result == -1) {
        printf("❌ Erreur lecture effet: %s\n", Mix_GetError());
    }
    
    return result;
}

void sound_stop_all_effects(void) {
    Mix_HaltChannel(-1);
}

void sound_stop_channel(int channel) {
    Mix_HaltChannel(channel);
}

// === HELPERS UI ===

void sound_play_button_click(void) {
    sound_play_effect(SOUND_CLICK);
}

void sound_play_button_hover(void) {
    sound_play_effect(SOUND_HOVER);
}

void sound_play_piece_move(void) {
    sound_play_effect(SOUND_PIECE_MOVE);
}

void sound_play_piece_capture(void) {
    sound_play_effect(SOUND_PIECE_CAPTURE);
}

void sound_play_invalid_move(void) {
    sound_play_effect(SOUND_INVALID_MOVE);
}

// === CONTRÔLE GLOBAL ===

void sound_set_enabled(bool enabled) {
    sound_enabled = enabled;
    
    if (!enabled) {
        sound_stop_music();
        sound_stop_all_effects();
    }
    
    printf("🔊 Sons %s\n", enabled ? "ACTIVÉS" : "DÉSACTIVÉS");
}

bool sound_is_enabled(void) {
    return sound_enabled;
}

void sound_set_music_enabled(bool enabled) {
    music_enabled = enabled;
    
    if (!enabled) {
        sound_stop_music();
    }
    
    printf("🎵 Musique %s\n", enabled ? "ACTIVÉE" : "DÉSACTIVÉE");
}

void sound_set_sfx_enabled(bool enabled) {
    sfx_enabled = enabled;
    
    if (!enabled) {
        sound_stop_all_effects();
    }
    
    printf("🔊 Effets sonores %s\n", enabled ? "ACTIVÉS" : "DÉSACTIVÉS");
}

// === DEBUG ===

void sound_debug_print_status(void) {
    printf("\n=== 🔊 SYSTÈME DE SON ===\n");
    printf("Initialisé: %s\n", sound_system_initialized ? "OUI" : "NON");
    printf("Sons activés: %s\n", sound_enabled ? "OUI" : "NON");
    printf("Musique activée: %s\n", music_enabled ? "OUI" : "NON");
    printf("Effets activés: %s\n", sfx_enabled ? "OUI" : "NON");
    printf("Volume musique: %d%%\n", music_volume);
    printf("Volume effets: %d%%\n", sfx_volume);
    printf("Musique en cours: %s\n", sound_is_music_playing() ? "OUI" : "NON");
    
    // Statistiques de chargement
    int music_loaded = 0;
    for (int i = 0; i < MUSIC_COUNT; i++) {
        if (music_tracks[i]) music_loaded++;
    }
    
    int effects_loaded = 0;
    for (int i = 0; i < SOUND_EFFECT_COUNT; i++) {
        if (sound_effects[i]) effects_loaded++;
    }
    
    printf("Pistes chargées: %d/%d\n", music_loaded, MUSIC_COUNT);
    printf("Effets chargés: %d/%d\n", effects_loaded, SOUND_EFFECT_COUNT);
    printf("=========================\n\n");
}
