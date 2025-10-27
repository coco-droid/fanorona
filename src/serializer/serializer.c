#include "serializer.h"
#include "../logic/logic.h"
#include "../utils/log_console.h"
#include <string.h>
#include <stdio.h>

// === JSON SERIALIZATION HELPERS ===

// === SÉRIALISATION JOUEURS (JSON) ===

bool serialize_player(const GamePlayer* player, SerializedPlayer* out) {
    if (!player || !out) return false;
    
    // Just copy data - JSON conversion happens in serialize_player_to_buffer
    memset(out, 0, sizeof(SerializedPlayer));
    strncpy(out->name, player->name, sizeof(out->name) - 1);
    out->logical_color = (uint8_t)player->logical_color;
    out->piece_color = (uint8_t)player->piece_color;
    out->player_type = (uint8_t)player->type;
    out->player_number = (uint8_t)player->player_number;
    out->avatar_id = (uint8_t)player->avatar;
    out->pieces_remaining = player->pieces_remaining;
    out->captures_made = player->captures_made;
    out->total_time = player->total_time;
    out->thinking_time = player->thinking_time;
    out->is_current_turn = player->is_current_turn ? 1 : 0;
    out->has_mandatory_capture = player->has_mandatory_capture ? 1 : 0;
    
    return true;
}

bool deserialize_player(const SerializedPlayer* data, GamePlayer* out) {
    if (!data || !out) return false;
    
    strncpy(out->name, data->name, sizeof(out->name) - 1);
    out->name[sizeof(out->name) - 1] = '\0';
    out->logical_color = (Player)data->logical_color;
    out->piece_color = (PieceColor)data->piece_color;
    out->type = (PlayerType)data->player_type;
    out->player_number = data->player_number;
    out->avatar = (AvatarID)data->avatar_id;
    out->pieces_remaining = data->pieces_remaining;
    out->captures_made = data->captures_made;
    out->total_time = data->total_time;
    out->thinking_time = data->thinking_time;
    out->is_current_turn = data->is_current_turn != 0;
    out->has_mandatory_capture = data->has_mandatory_capture != 0;
    
    return true;
}

// === SÉRIALISATION COUPS (JSON) ===

bool serialize_move(const Move* move, Player player, SerializedMove* out) {
    if (!move || !out) return false;
    
    memset(out, 0, sizeof(SerializedMove));
    out->from_id = move->from_id;
    out->to_id = move->to_id;
    out->player = (uint8_t)player;
    out->is_capture = move->is_capture ? 1 : 0;
    out->capture_count = (uint8_t)move->capture_count;
    
    for (int i = 0; i < move->capture_count && i < MAX_CAPTURE_LIST; i++) {
        out->captured_ids[i] = move->captured_ids[i];
    }
    
    out->timestamp = SDL_GetTicks();
    return true;
}

bool deserialize_move(const SerializedMove* data, Move* out, Player* player) {
    if (!data || !out || !player) return false;
    
    out->from_id = data->from_id;
    out->to_id = data->to_id;
    *player = (Player)data->player;
    out->is_capture = data->is_capture != 0;
    out->capture_count = data->capture_count;
    
    for (int i = 0; i < data->capture_count && i < MAX_CAPTURE_LIST; i++) {
        out->captured_ids[i] = data->captured_ids[i];
    }
    
    return true;
}

// === CONVERSION JSON (BUFFER) ===

size_t serialize_move_to_buffer(const Move* move, Player player, uint8_t* buffer, size_t buffer_size) {
    SerializedMove serialized;
    if (!serialize_move(move, player, &serialized)) return 0;
    
    // Generate compact JSON
    int len = snprintf((char*)buffer, buffer_size,
        "{\"from\":%d,\"to\":%d,\"player\":%d,\"capture\":%d,\"count\":%d,\"ids\":[",
        serialized.from_id, serialized.to_id, serialized.player, 
        serialized.is_capture, serialized.capture_count);
    
    if (len < 0 || (size_t)len >= buffer_size) return 0;
    
    for (int i = 0; i < serialized.capture_count && i < MAX_CAPTURE_LIST; i++) {
        int n = snprintf((char*)buffer + len, buffer_size - len, 
            "%s%d", i > 0 ? "," : "", serialized.captured_ids[i]);
        if (n < 0 || (size_t)(len + n) >= buffer_size) return 0;
        len += n;
    }
    
    int final = snprintf((char*)buffer + len, buffer_size - len, "],\"ts\":%u}", serialized.timestamp);
    if (final < 0 || (size_t)(len + final) >= buffer_size) return 0;
    
    return len + final;
}

bool deserialize_move_from_buffer(const uint8_t* buffer, size_t buffer_size, Move* out, Player* player) {
    if (!buffer || !out || !player || buffer_size == 0) return false;
    
    // Simple JSON parsing (minimal)
    SerializedMove serialized = {0};
    const char* json = (const char*)buffer;
    
    // Parse main fields
    if (sscanf(json, "{\"from\":%d,\"to\":%d,\"player\":%d,\"capture\":%d,\"count\":%d",
        &serialized.from_id, &serialized.to_id, (int*)&serialized.player,
        (int*)&serialized.is_capture, (int*)&serialized.capture_count) != 5) {
        return false;
    }
    
    // Parse captured IDs array (simple sequential search)
    const char* ids_start = strstr(json, "\"ids\":[");
    if (ids_start && serialized.capture_count > 0) {
        ids_start += 7; // Skip "\"ids\":["
        for (int i = 0; i < serialized.capture_count && i < MAX_CAPTURE_LIST; i++) {
            if (sscanf(ids_start, "%d", &serialized.captured_ids[i]) != 1) break;
            ids_start = strchr(ids_start, ',');
            if (!ids_start) break;
            ids_start++;
        }
    }
    
    return deserialize_move(&serialized, out, player);
}

// === DEBUG ===

void serializer_debug_print_move(const SerializedMove* move) {
    if (!move) return;
    
    printf("\n=== 📦 SERIALIZED MOVE (JSON) ===\n");
    printf("From: %d → To: %d\n", move->from_id, move->to_id);
    printf("Player: %d\n", move->player);
    printf("Capture: %s (%d pièces)\n", 
        move->is_capture ? "OUI" : "NON", move->capture_count);
    
    // Show JSON representation
    char buffer[1024];
    size_t len = serialize_move_to_buffer(
        &(Move){move->from_id, move->to_id, move->is_capture, move->capture_count, {0}},
        (Player)move->player, (uint8_t*)buffer, sizeof(buffer));
    
    if (len > 0) {
        printf("JSON: %.*s\n", (int)len, buffer);
    }
    printf("================================\n");
}
