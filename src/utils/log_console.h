#ifndef LOG_CONSOLE_H
#define LOG_CONSOLE_H

#include <stdbool.h>
#include <SDL2/SDL.h>  // 🔧 FIX: Ajout pour les types SDL comme Uint32

// === CONSOLE DE LOGS SÉPARÉE ===

// Initialiser la console de logs (ouvre un terminal séparé)
bool log_console_init(void);

// Fermer la console de logs
void log_console_cleanup(void);

// Envoyer un log vers la console séparée
void log_console_write(const char* source, const char* event_type, const char* element_id, const char* message);

// 🆕 Log avec code d'événement et nom correspondant
void log_console_write_event(const char* source, const char* event_type, const char* element_id, const char* message, int event_code);

// Activer/désactiver la console de logs
void log_console_set_enabled(bool enabled);
bool log_console_is_enabled(void);

// Activer/désactiver les logs d'événements souris
void log_console_set_mouse_tracking(bool enabled);
bool log_console_is_mouse_tracking_enabled(void);

// 🆕 Contrôler les logs d'itération
void log_console_set_iteration_logging(bool enabled);
bool log_console_is_iteration_logging_enabled(void);

// Logs spécialisés pour différents types d'événements
void log_console_mouse_event(const char* event_type, int x, int y, const char* element_id);
void log_console_keyboard_event(const char* event_type, const char* key_name);
void log_console_ui_event(const char* source, const char* event_type, const char* element_id, const char* message);

// 🆕 Log spécialisé pour les itérations de boucle
void log_console_iteration_event(int iteration_number, const char* details);

// 🆕 Log spécialisé pour la fermeture de fenêtre
void log_console_window_close_event(const char* window_name, Uint32 window_id);

// 🆕 Debug function pour EventManager
void log_console_debug_event_manager(void* manager_ptr);

#endif // LOG_CONSOLE_H
