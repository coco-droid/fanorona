#define _POSIX_C_SOURCE 200809L
// 🔧 FIX: Supprimer la redéfinition pour éviter le warning
// #define _GNU_SOURCE est déjà défini par les flags de compilation
#include "log_console.h"
#include <SDL2/SDL.h>  // 🔧 FIX: Ajout pour Uint32 et autres types SDL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

// === VARIABLES GLOBALES ===

// 🔧 RÉACTIVER: Variables pour la console séparée
static FILE* log_pipe = NULL;
static pid_t log_console_pid = -1;
static bool log_console_enabled = false;
static bool mouse_tracking_enabled = false;
static bool iteration_logging_enabled = true; // 🆕 Contrôler les logs d'itération

// === FONCTIONS UTILITAIRES ===

// Obtenir l'horodatage actuel
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "%H:%M:%S", tm_info);
}

// 🆕 Fonction utilitaire pour convertir le code d'événement en nom lisible
static const char* get_event_name(int event_type) {
    switch (event_type) {
        case 1024: return "SDL_MOUSEBUTTONDOWN";
        case 1025: return "SDL_MOUSEBUTTONUP";
        case 1026: return "SDL_MOUSEMOTION";
        case 1027: return "SDL_MOUSEWHEEL";
        case 768: return "SDL_KEYDOWN";
        case 769: return "SDL_KEYUP";
        case 512: return "SDL_WINDOWEVENT";
        case 256: return "SDL_QUIT";
        case 32768: return "SDL_USEREVENT";
        default: {
            static char unknown_event[64];
            snprintf(unknown_event, sizeof(unknown_event), "UNKNOWN_EVENT_%d", event_type);
            return unknown_event;
        }
    }
}

// === FONCTIONS PUBLIQUES ===

bool log_console_init(void) {
    if (log_console_enabled) {
        return true; // Déjà initialisé
    }
    
    printf("🖥️ Initialisation de la console d'événements dédiée...\n");
    
    // Ignorer SIGPIPE pour éviter que le processus parent crash si la console se ferme
    signal(SIGPIPE, SIG_IGN);
    
    // Créer un pipe pour communiquer avec la console
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        printf("❌ Erreur: Impossible de créer le pipe pour la console d'événements\n");
        return false;
    }
    
    // Rendre le pipe non-bloquant côté écriture
    int flags = fcntl(pipefd[1], F_GETFL, 0);
    fcntl(pipefd[1], F_SETFL, flags | O_NONBLOCK);
    
    // Fork pour créer un processus séparé pour la console
    log_console_pid = fork();
    
    if (log_console_pid == -1) {
        printf("❌ Erreur: Impossible de créer le processus de console d'événements\n");
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    }
    
    if (log_console_pid == 0) {
        // Processus enfant - console d'événements
        close(pipefd[1]); // Fermer l'écriture
        
        // Rediriger stdin vers le pipe
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        
        // Détacher le processus enfant du processus parent
        setsid();
        
        // Essayer différents terminaux disponibles
        // D'abord gnome-terminal avec titre spécifique
        execlp("gnome-terminal", "gnome-terminal", 
               "--title=🎯 Fanorona - Console d'Événements UI", 
               "--geometry=100x40+50+100",  // Plus large pour les logs d'événements
               "--", "bash", "-c", 
               "echo '=== 🎯 CONSOLE D\\'ÉVÉNEMENTS FANORONA ==='; "
               "echo '🖱️  Événements souris, clavier et UI en temps réel'; "
               "echo '🔍 Debugging interactif des interactions utilisateur'; "
               "echo '🎮 Cette console est indépendante de la fenêtre de jeu'; "
               "echo '====================================='; "
               "echo ''; "
               "while IFS= read -r line; do "
               "  if [ -n \"$line\" ]; then "
               "    echo \"$line\"; "
               "  fi; "
               "done; "
               "echo '🔚 Console d\\'événements fermée'; "
               "sleep 3", 
               NULL);
        
        // Si gnome-terminal échoue, essayer xterm
        execlp("xterm", "xterm", 
               "-title", "🎯 Fanorona - Events Console",
               "-geometry", "100x40+50+100",
               "-fg", "white", "-bg", "black",  // Couleurs pour bien voir les événements
               "-e", "bash", "-c",
               "echo '=== 🎯 CONSOLE D\\'ÉVÉNEMENTS FANORONA ==='; "
               "echo '🖱️ Événements UI en temps réel'; "
               "echo '🔍 Debugging des interactions'; "
               "echo '====================================='; "
               "echo ''; "
               "while IFS= read -r line; do "
               "  if [ -n \"$line\" ]; then "
               "    echo \"$line\"; "
               "  fi; "
               "done; "
               "echo '🔚 Console fermée'; "
               "sleep 3",
               NULL);
        
        // Fallback: x-terminal-emulator
        execlp("x-terminal-emulator", "x-terminal-emulator",
               "-T", "🎯 Fanorona - Events Console",
               "-geometry", "100x40",
               "-e", "bash", "-c",
               "echo '=== 🎯 CONSOLE D\\'ÉVÉNEMENTS ==='; "
               "echo '🖱️ Événements UI en temps réel'; "
               "echo '====================================='; "
               "echo ''; "
               "while IFS= read -r line; do echo \"$line\"; done; sleep 2",
               NULL);
        
        // Dernière option : xfce4-terminal
        execlp("xfce4-terminal", "xfce4-terminal",
               "--title=🎯 Fanorona - Events Console",
               "--geometry=100x40",
               "-e", "bash -c 'echo \"=== 🎯 CONSOLE D\\'ÉVÉNEMENTS ===\"; while read line; do echo \"$line\"; done; sleep 2'",
               NULL);
        
        // Si tout échoue, utiliser konsole (KDE)
        execlp("konsole", "konsole",
               "--title", "🎯 Fanorona - Events Console",
               "-e", "bash", "-c", "echo '=== CONSOLE D\\'ÉVÉNEMENTS ==='; while read line; do echo \"$line\"; done",
               NULL);
        
        // Fallback final : cat
        execlp("cat", "cat", NULL);
        
        // Si même cat échoue, sortir proprement
        printf("❌ Impossible de lancer un terminal pour les événements\n");
        exit(1);
    }
    
    // Processus parent
    close(pipefd[0]); // Fermer la lecture
    log_pipe = fdopen(pipefd[1], "w");
    
    if (!log_pipe) {
        printf("❌ Erreur: Impossible d'ouvrir le pipe en écriture\n");
        kill(log_console_pid, SIGTERM);
        waitpid(log_console_pid, NULL, WNOHANG);
        return false;
    }
    
    // Rendre le pipe avec flush automatique mais non-bloquant
    setvbuf(log_pipe, NULL, _IOLBF, 0);
    
    log_console_enabled = true;
    mouse_tracking_enabled = true; // Activer par défaut
    
    // Attendre que la console soit prête
    usleep(300000); // 300ms
    
    // Envoyer un message de bienvenue spécialisé
    log_console_write("EventConsole", "Init", "system", "🎯 Console d'événements initialisée - Prête pour le debugging UI");
    log_console_write("EventConsole", "Info", "system", "🖱️ Tracking souris activé - bougez la souris dans la fenêtre de jeu");
    log_console_write("EventConsole", "Info", "system", "🎮 Cliquez sur les boutons pour voir les événements en temps réel");
    log_console_write("EventConsole", "Separator", "system", "==========================================");
    
    printf("✅ Console d'événements créée (PID: %d)\n", log_console_pid);
    printf("🎯 Une fenêtre séparée s'est ouverte pour les événements UI\n");
    printf("🖱️ Les interactions seront loggées en temps réel dans cette console\n");
    
    return true;
}

void log_console_cleanup(void) {
    if (!log_console_enabled) return;
    
    printf("🧹 Fermeture de la console d'événements...\n");
    
    if (log_pipe) {
        log_console_write("EventConsole", "Cleanup", "system", "🔚 Fermeture de la console d'événements - Merci d'avoir debuggé !");
        fflush(log_pipe);
        fclose(log_pipe);
        log_pipe = NULL;
    }
    
    if (log_console_pid > 0) {
        // Envoyer SIGTERM au processus de la console
        kill(log_console_pid, SIGTERM);
        
        // Attendre un peu puis forcer la fermeture si nécessaire
        int status;
        pid_t result = waitpid(log_console_pid, &status, WNOHANG);
        if (result == 0) {
            // Le processus ne s'est pas terminé, attendre encore un peu
            usleep(500000); // 500ms
            result = waitpid(log_console_pid, &status, WNOHANG);
            if (result == 0) {
                // Forcer la fermeture
                kill(log_console_pid, SIGKILL);
                waitpid(log_console_pid, &status, 0);
            }
        }
        log_console_pid = -1;
    }
    
    log_console_enabled = false;
    mouse_tracking_enabled = false;
    
    printf("✅ Console d'événements fermée\n");
}

void log_console_write(const char* source, const char* event_type, const char* element_id, const char* message) {
    if (!log_console_enabled || !log_pipe) {
        // Fallback : afficher dans la console principale si la console dédiée n'est pas disponible
        char timestamp[32];
        get_timestamp(timestamp, sizeof(timestamp));
        printf("🔍 [%s] [%s] [%s] [%s] : %s\n", 
               timestamp, source ? source : "Unknown", event_type ? event_type : "Unknown", 
               element_id ? element_id : "NoID", message ? message : "No message");
        fflush(stdout);
        return;
    }
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    // 🎯 FORMAT COLORÉ POUR LA CONSOLE D'ÉVÉNEMENTS
    if (fprintf(log_pipe, "🔍 [%s] [%s] [%s] [%s] : %s\n", 
                timestamp,
                source ? source : "Unknown",
                event_type ? event_type : "Unknown", 
                element_id ? element_id : "NoID",
                message ? message : "No message") < 0) {
        // Si l'écriture échoue, la console a peut-être été fermée
        // Fallback vers printf
        printf("🔍 [%s] [%s] [%s] [%s] : %s\n", 
               timestamp, source ? source : "Unknown", event_type ? event_type : "Unknown", 
               element_id ? element_id : "NoID", message ? message : "No message");
    }
    
    fflush(log_pipe);
}

// 🆕 Nouvelle fonction spécialisée pour les événements avec code
void log_console_write_event(const char* source, const char* event_type, const char* element_id, const char* message, int event_code) {
    if (!log_console_enabled || !log_pipe) return;
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    
    const char* event_name = get_event_name(event_code);
    
    // 🎯 FORMAT SPÉCIAL pour les événements SDL avec codes
    if (fprintf(log_pipe, "⚡ [%s] [%s] [%s] [%s] : %s (code=%d=%s)\n", 
                timestamp,
                source ? source : "Unknown",
                event_type ? event_type : "Unknown", 
                element_id ? element_id : "NoID",
                message ? message : "No message",
                event_code,
                event_name) < 0) {
        // Fallback si échec
        printf("⚡ [%s] [%s] [%s] [%s] : %s (code=%d=%s)\n", 
               timestamp, source ? source : "Unknown", event_type ? event_type : "Unknown", 
               element_id ? element_id : "NoID", message ? message : "No message",
               event_code, event_name);
    }
    
    fflush(log_pipe);
}

void log_console_set_enabled(bool enabled) {
    if (enabled && !log_console_enabled) {
        log_console_init();
    } else if (!enabled && log_console_enabled) {
        log_console_cleanup();
    }
}

bool log_console_is_enabled(void) {
    return log_console_enabled;
}

void log_console_set_mouse_tracking(bool enabled) {
    mouse_tracking_enabled = enabled;
    
    printf("🖱️ [LOG_CONSOLE] Tracking souris %s\n", enabled ? "ACTIVÉ" : "DÉSACTIVÉ");
    
    if (log_console_enabled) {
        log_console_write("LogConsole", "Config", "mouse", 
                         enabled ? "Tracking souris activé" : "Tracking souris désactivé");
    }
}

bool log_console_is_mouse_tracking_enabled(void) {
    return mouse_tracking_enabled && log_console_enabled;
}

// 🆕 Fonction pour contrôler les logs d'itération
void log_console_set_iteration_logging(bool enabled) {
    iteration_logging_enabled = enabled;
    
    printf("🔄 [LOG_CONSOLE] Iteration logging %s\n", enabled ? "ACTIVÉ" : "DÉSACTIVÉ");
    
    if (log_console_enabled) {
        log_console_write("LogConsole", "Config", "iteration", 
                         enabled ? "Iteration logging enabled" : "Iteration logging disabled");
    }
}

bool log_console_is_iteration_logging_enabled(void) {
    return iteration_logging_enabled && log_console_enabled;
}

// === LOGS SPÉCIALISÉS ===

void log_console_mouse_event(const char* event_type, int x, int y, const char* element_id) {
    if (!mouse_tracking_enabled || !log_console_enabled) return;
    
    char message[256];
    snprintf(message, sizeof(message), "Mouse at (%d, %d)", x, y);
    
    log_console_write("MouseEvent", event_type, element_id ? element_id : "window", message);
}

void log_console_keyboard_event(const char* event_type, const char* key_name) {
    if (!log_console_enabled) return;
    
    char message[128];
    snprintf(message, sizeof(message), "Key: %s", key_name ? key_name : "unknown");
    
    log_console_write("KeyboardEvent", event_type, "keyboard", message);
}

void log_console_ui_event(const char* source, const char* event_type, const char* element_id, const char* message) {
    if (!log_console_enabled) return;
    
    log_console_write(source, event_type, element_id, message);
}

// 🆕 Log spécialisé pour les itérations de boucle
void log_console_iteration_event(int iteration_number, const char* details) {
    if (!iteration_logging_enabled || !log_console_enabled) return;
    
    char message[256];
    snprintf(message, sizeof(message), "Iteration #%d - %s", iteration_number, details ? details : "processing");
    
    log_console_write("EventLoop", "Iteration", "event_loop", message);
}

// 🆕 Log spécialisé pour la fermeture de fenêtre
void log_console_window_close_event(const char* window_name, Uint32 window_id) {
    if (!log_console_enabled) return;
    
    char message[256];
    snprintf(message, sizeof(message), "🚪 Window '%s' (ID=%u) close requested", 
             window_name ? window_name : "unknown", window_id);
    
    log_console_write("WindowClose", "CloseRequested", "window", message);
}

// 🆕 Fonction de debug pour compter les éléments enregistrés
void log_console_debug_event_manager(void* manager_ptr) {
    if (!log_console_enabled) return;
    
    // Cast vers EventManager (défini dans event.h)
    typedef struct EventElement {
        int x, y, width, height, z_index;
        bool display;
        void (*callback)(void*, void*);
        void* user_data;
        struct EventElement* next;
    } EventElement;
    
    typedef struct EventManager {
        EventElement* elements;
        bool running;
    } EventManager;
    
    EventManager* manager = (EventManager*)manager_ptr;
    if (!manager) {
        log_console_write("Debug", "EventManagerDebug", "log_console.c", 
                         "[DEBUG] EventManager is NULL!");
        return;
    }
    
    int count = 0;
    EventElement* current = manager->elements;
    while (current) {
        count++;
        char message[256];
        snprintf(message, sizeof(message), 
                "[DEBUG] Element #%d: pos(%d,%d) size(%dx%d) z=%d display=%s", 
                count, current->x, current->y, current->width, current->height,
                current->z_index, current->display ? "true" : "false");
        log_console_write("Debug", "RegisteredElement", "log_console.c", message);
        current = current->next;
    }
    
    char summary[128];
    snprintf(summary, sizeof(summary), "[DEBUG] Total registered elements: %d", count);
    log_console_write("Debug", "ElementCount", "log_console.c", summary);
}
