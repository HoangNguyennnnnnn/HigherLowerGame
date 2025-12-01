#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "../include/game.h"

// External variable from game_logic.c
extern int item_count;

// Global variables for rooms
GameRoom rooms[MAX_ROOMS];
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER;
int next_room_id = 1;

// Initialize rooms array
void init_rooms() {
    pthread_mutex_lock(&rooms_mutex);
    for (int i = 0; i < MAX_ROOMS; i++) {
        rooms[i].id = 0;
        rooms[i].status = ROOM_EMPTY;
        rooms[i].player_count = 0;
        rooms[i].host_session_id = 0;
    }
    pthread_mutex_unlock(&rooms_mutex);
    printf("[ROOM] 🏠 Room system initialized (max %d rooms)\n", MAX_ROOMS);
}

// Helper: Find room by ID
static int find_room_index(int room_id) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].id == room_id && rooms[i].status != ROOM_EMPTY) {
            return i;
        }
    }
    return -1;
}

// Helper: Find player in room
static int find_player_in_room(GameRoom *room, int session_id) {
    for (int i = 0; i < room->player_count; i++) {
        if (room->players[i].session_id == session_id) {
            return i;
        }
    }
    return -1;
}

// Helper: Build room JSON
static void build_room_json(GameRoom *room, char *json, size_t json_size) {
    char players_json[4096] = "[";
    
    for (int i = 0; i < room->player_count; i++) {
        char player_entry[512];
        snprintf(player_entry, sizeof(player_entry),
            "%s{\"session_id\":%d,\"name\":\"%s\",\"score\":%d,\"streak\":%d,\"is_ready\":%d,\"game_over\":%d,\"has_answered\":%d,\"is_host\":%s}",
            i > 0 ? "," : "",
            room->players[i].session_id,
            room->players[i].name,
            room->players[i].score,
            room->players[i].streak,
            room->players[i].is_ready,
            room->players[i].game_over,
            room->players[i].has_answered,
            room->players[i].session_id == room->host_session_id ? "true" : "false"
        );
        strcat(players_json, player_entry);
    }
    strcat(players_json, "]");
    
    const char *status_str;
    switch (room->status) {
        case ROOM_WAITING: status_str = "waiting"; break;
        case ROOM_PLAYING: status_str = "playing"; break;
        case ROOM_FINISHED: status_str = "finished"; break;
        default: status_str = "empty"; break;
    }
    
    snprintf(json, json_size,
        "{\"id\":%d,\"name\":\"%s\",\"host_session_id\":%d,\"player_count\":%d,\"max_players\":%d,\"max_rounds\":%d,\"status\":\"%s\",\"current_round\":%d,\"players\":%s}",
        room->id, room->name, room->host_session_id, room->player_count, room->max_players, room->max_rounds, status_str, room->current_round, players_json
    );
}

// Handle GET /rooms - List all available rooms
void handle_list_rooms(int sock) {
    pthread_mutex_lock(&rooms_mutex);
    
    char rooms_json[BUFFER_SIZE] = "{\"action\":\"room_list\",\"rooms\":[";
    int first = 1;
    
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status == ROOM_WAITING || rooms[i].status == ROOM_PLAYING) {
            char room_entry[512];
            snprintf(room_entry, sizeof(room_entry),
                "%s{\"id\":%d,\"name\":\"%s\",\"player_count\":%d,\"max_players\":%d,\"status\":\"%s\"}",
                first ? "" : ",",
                rooms[i].id,
                rooms[i].name,
                rooms[i].player_count,
                rooms[i].max_players,
                rooms[i].status == ROOM_WAITING ? "waiting" : "playing"
            );
            strcat(rooms_json, room_entry);
            first = 0;
        }
    }
    
    strcat(rooms_json, "]}");
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, rooms_json);
    printf("[ROOM] 📋 Room list requested\n");
}

// Handle POST /rooms/create - Create a new room
void handle_create_room(int sock, int session_id, char *json_body) {
    if (session_id == 0) {
        char error_json[] = "{\"error\":\"No session ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Parse room name from JSON
    char room_name[ROOM_NAME_LEN] = "Game Room";
    char player_name[PLAYER_NAME_LEN] = "";
    int max_players = 4;
    int max_rounds = 10;  // Default 10 rounds
    
    char *name_ptr = strstr(json_body, "\"room_name\"");
    if (name_ptr) {
        char *value_start = strchr(name_ptr, ':');
        if (value_start) {
            value_start++;
            while (*value_start == ' ' || *value_start == '"') value_start++;
            char *value_end = strchr(value_start, '"');
            if (value_end) {
                int len = value_end - value_start;
                if (len > 0 && len < ROOM_NAME_LEN) {
                    strncpy(room_name, value_start, len);
                    room_name[len] = '\0';
                }
            }
        }
    }
    
    char *player_name_ptr = strstr(json_body, "\"player_name\"");
    if (player_name_ptr) {
        char *value_start = strchr(player_name_ptr, ':');
        if (value_start) {
            value_start++;
            while (*value_start == ' ' || *value_start == '"') value_start++;
            char *value_end = strchr(value_start, '"');
            if (value_end) {
                int len = value_end - value_start;
                if (len > 0 && len < PLAYER_NAME_LEN) {
                    strncpy(player_name, value_start, len);
                    player_name[len] = '\0';
                }
            }
        }
    }
    
    char *max_ptr = strstr(json_body, "\"max_players\"");
    if (max_ptr) {
        char *value_start = strchr(max_ptr, ':');
        if (value_start) {
            max_players = atoi(value_start + 1);
            if (max_players < 2) max_players = 2;
            if (max_players > MAX_PLAYERS_PER_ROOM) max_players = MAX_PLAYERS_PER_ROOM;
        }
    }
    
    char *rounds_ptr = strstr(json_body, "\"max_rounds\"");
    if (rounds_ptr) {
        char *value_start = strchr(rounds_ptr, ':');
        if (value_start) {
            max_rounds = atoi(value_start + 1);
            if (max_rounds < 5) max_rounds = 5;
            if (max_rounds > 50) max_rounds = 50;
        }
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Check if player is already in a room
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status != ROOM_EMPTY) {
            if (find_player_in_room(&rooms[i], session_id) >= 0) {
                pthread_mutex_unlock(&rooms_mutex);
                char error_json[] = "{\"error\":\"You are already in a room\"}";
                send_json_response(sock, error_json);
                return;
            }
        }
    }
    
    // Find empty slot
    int room_idx = -1;
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status == ROOM_EMPTY) {
            room_idx = i;
            break;
        }
    }
    
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"Server is full, no room slots available\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Create room
    GameRoom *room = &rooms[room_idx];
    room->id = next_room_id++;
    strncpy(room->name, room_name, ROOM_NAME_LEN - 1);
    room->name[ROOM_NAME_LEN - 1] = '\0';
    room->host_session_id = session_id;
    room->max_players = max_players;
    room->max_rounds = max_rounds;
    room->status = ROOM_WAITING;
    room->player_count = 1;
    
    // Add host as first player
    room->players[0].session_id = session_id;
    if (strlen(player_name) > 0) {
        strncpy(room->players[0].name, player_name, PLAYER_NAME_LEN - 1);
    } else {
        snprintf(room->players[0].name, PLAYER_NAME_LEN, "Player_%d", session_id);
    }
    room->players[0].score = 0;
    room->players[0].streak = 0;
    room->players[0].is_ready = 1; // Host is always ready
    room->players[0].game_over = 0;
    
    // Update SSE client info
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sse_clients[i].active && sse_clients[i].session_id == session_id) {
            sse_clients[i].room_id = room->id;
            if (strlen(player_name) > 0) {
                strncpy(sse_clients[i].player_name, player_name, PLAYER_NAME_LEN - 1);
            }
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    // Build response
    char room_json[BUFFER_SIZE];
    build_room_json(room, room_json, sizeof(room_json));
    
    char response[RESPONSE_SIZE];
    snprintf(response, sizeof(response), "{\"action\":\"room_created\",\"room\":%s}", room_json);
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, response);
    
    printf("[ROOM] 🏠 Room created: \"%s\" (ID: %d) by session %d\n", room_name, room->id, session_id);
}

// Handle POST /rooms/join - Join an existing room
void handle_join_room(int sock, int session_id, char *json_body) {
    if (session_id == 0) {
        char error_json[] = "{\"error\":\"No session ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Parse room_id and player_name from JSON
    int room_id = 0;
    char player_name[PLAYER_NAME_LEN] = "";
    
    char *room_id_ptr = strstr(json_body, "\"room_id\"");
    if (room_id_ptr) {
        char *value_start = strchr(room_id_ptr, ':');
        if (value_start) {
            room_id = atoi(value_start + 1);
        }
    }
    
    char *player_name_ptr = strstr(json_body, "\"player_name\"");
    if (player_name_ptr) {
        char *value_start = strchr(player_name_ptr, ':');
        if (value_start) {
            value_start++;
            while (*value_start == ' ' || *value_start == '"') value_start++;
            char *value_end = strchr(value_start, '"');
            if (value_end) {
                int len = value_end - value_start;
                if (len > 0 && len < PLAYER_NAME_LEN) {
                    strncpy(player_name, value_start, len);
                    player_name[len] = '\0';
                }
            }
        }
    }
    
    if (room_id == 0) {
        char error_json[] = "{\"error\":\"Invalid room ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Check if player is already in a room
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status != ROOM_EMPTY) {
            if (find_player_in_room(&rooms[i], session_id) >= 0) {
                pthread_mutex_unlock(&rooms_mutex);
                char error_json[] = "{\"error\":\"You are already in a room\"}";
                send_json_response(sock, error_json);
                return;
            }
        }
    }
    
    int room_idx = find_room_index(room_id);
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"Room not found\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    
    if (room->status != ROOM_WAITING) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"Room is not accepting players (game in progress)\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    if (room->player_count >= room->max_players) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"Room is full\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Add player to room
    int player_idx = room->player_count;
    room->players[player_idx].session_id = session_id;
    if (strlen(player_name) > 0) {
        strncpy(room->players[player_idx].name, player_name, PLAYER_NAME_LEN - 1);
    } else {
        snprintf(room->players[player_idx].name, PLAYER_NAME_LEN, "Player_%d", session_id);
    }
    room->players[player_idx].score = 0;
    room->players[player_idx].streak = 0;
    room->players[player_idx].is_ready = 0;
    room->players[player_idx].game_over = 0;
    room->player_count++;
    
    // Update SSE client info
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sse_clients[i].active && sse_clients[i].session_id == session_id) {
            sse_clients[i].room_id = room->id;
            if (strlen(player_name) > 0) {
                strncpy(sse_clients[i].player_name, player_name, PLAYER_NAME_LEN - 1);
            }
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    // Build response
    char room_json[BUFFER_SIZE];
    build_room_json(room, room_json, sizeof(room_json));
    
    char response[RESPONSE_SIZE];
    snprintf(response, sizeof(response), "{\"action\":\"room_joined\",\"room\":%s}", room_json);
    
    // Notify other players in room via SSE
    char notify_json[RESPONSE_SIZE];
    snprintf(notify_json, sizeof(notify_json), "{\"action\":\"player_joined\",\"room\":%s}", room_json);
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, response);
    broadcast_sse_to_room(room_id, notify_json);
    
    printf("[ROOM] 👤 Player %d joined room \"%s\" (ID: %d)\n", session_id, room->name, room_id);
}

// Handle POST /rooms/leave - Leave current room
void handle_leave_room(int sock, int session_id) {
    if (session_id == 0) {
        char error_json[] = "{\"error\":\"No session ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Find the room this player is in
    int room_idx = -1;
    int player_idx = -1;
    
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status != ROOM_EMPTY) {
            int p_idx = find_player_in_room(&rooms[i], session_id);
            if (p_idx >= 0) {
                room_idx = i;
                player_idx = p_idx;
                break;
            }
        }
    }
    
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"You are not in any room\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    int room_id = room->id;
    int was_host = (room->host_session_id == session_id);
    
    // Remove player from room
    for (int i = player_idx; i < room->player_count - 1; i++) {
        room->players[i] = room->players[i + 1];
    }
    room->player_count--;
    
    // Update SSE client info
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sse_clients[i].active && sse_clients[i].session_id == session_id) {
            sse_clients[i].room_id = -1;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    char response[256];
    char notify_json[RESPONSE_SIZE] = "";
    
    if (room->player_count == 0) {
        // Room is now empty, delete it
        room->status = ROOM_EMPTY;
        room->id = 0;
        snprintf(response, sizeof(response), "{\"action\":\"room_left\",\"message\":\"Room deleted (empty)\"}");
    } else {
        // If host left, assign new host
        if (was_host) {
            room->host_session_id = room->players[0].session_id;
            room->players[0].is_ready = 1; // New host is automatically ready
        }
        
        // Build room JSON for notification
        char room_json[BUFFER_SIZE];
        build_room_json(room, room_json, sizeof(room_json));
        snprintf(notify_json, sizeof(notify_json), "{\"action\":\"player_left\",\"room\":%s}", room_json);
        snprintf(response, sizeof(response), "{\"action\":\"room_left\",\"message\":\"Left room successfully\"}");
    }
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, response);
    
    if (strlen(notify_json) > 0) {
        broadcast_sse_to_room(room_id, notify_json);
    }
    
    printf("[ROOM] 🚪 Player %d left room ID: %d\n", session_id, room_id);
}

// Handle POST /rooms/start - Start game (host only)
void handle_start_game(int sock, int session_id) {
    if (session_id == 0) {
        char error_json[] = "{\"error\":\"No session ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Find the room this player is in
    int room_idx = -1;
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status != ROOM_EMPTY) {
            if (find_player_in_room(&rooms[i], session_id) >= 0) {
                room_idx = i;
                break;
            }
        }
    }
    
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"You are not in any room\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    
    // Check if player is host
    if (room->host_session_id != session_id) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"Only the host can start the game\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    if (room->status != ROOM_WAITING) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"Game has already started\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Start the game - initialize room and all players
    room->status = ROOM_PLAYING;
    room->current_round = 1;
    room->current_index_A = rand() % item_count;
    room->current_index_B = get_random_index_except(room->current_index_A);
    
    for (int i = 0; i < room->player_count; i++) {
        room->players[i].score = 0;
        room->players[i].streak = 0;
        room->players[i].game_over = 0;
        room->players[i].has_answered = 0;
        room->players[i].last_answer_correct = 0;
    }
    
    int room_id = room->id;
    
    // Get current items
    GameItem *itemA = &game_database[room->current_index_A];
    GameItem *itemB = &game_database[room->current_index_B];
    
    // Build room JSON
    char room_json[BUFFER_SIZE];
    build_room_json(room, room_json, sizeof(room_json));
    
    // Send game start notification with question to all players
    char start_json[RESPONSE_SIZE];
    snprintf(start_json, sizeof(start_json), 
        "{\"action\":\"game_started\",\"room\":%s,\"round\":%d,\"labelA\":\"%s\",\"valueA\":%d,\"labelB\":\"%s\"}",
        room_json, room->current_round,
        itemA->name, itemA->value,
        itemB->name);
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, start_json);
    broadcast_sse_to_room(room_id, start_json);
    
    printf("[ROOM] 🎮 Game started in room ID: %d by host %d\n", room_id, session_id);
    printf("       Round 1: %s ($%d) vs %s (?)\n", itemA->name, itemA->value, itemB->name);
}

// Handle POST /rooms/choice - Player makes a choice in room game
void handle_room_choice(int sock, int session_id, char *json_body) {
    if (session_id == 0) {
        char error_json[] = "{\"error\":\"No session ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Parse choice and response_time from JSON
    int choice = 0;
    int response_time_ms = 0;
    
    char *choice_ptr = strstr(json_body, "\"choice\"");
    if (choice_ptr) {
        char *value_start = strchr(choice_ptr, ':');
        if (value_start) {
            choice = atoi(value_start + 1);
        }
    }
    
    char *time_ptr = strstr(json_body, "\"response_time\"");
    if (time_ptr) {
        char *value_start = strchr(time_ptr, ':');
        if (value_start) {
            response_time_ms = atoi(value_start + 1);
        }
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Find room and player
    int room_idx = -1;
    int player_idx = -1;
    
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status == ROOM_PLAYING) {
            int p_idx = find_player_in_room(&rooms[i], session_id);
            if (p_idx >= 0) {
                room_idx = i;
                player_idx = p_idx;
                break;
            }
        }
    }
    
    if (room_idx == -1 || player_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"No active game found\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    RoomPlayer *player = &room->players[player_idx];
    
    // Check if player already answered this round
    if (player->has_answered) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"Already answered. Waiting for other players.\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    if (player->game_over) {
        pthread_mutex_unlock(&rooms_mutex);
        char error_json[] = "{\"error\":\"Your game is over. Wait for others to finish.\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Use room's shared question
    GameItem *itemA = &game_database[room->current_index_A];
    GameItem *itemB = &game_database[room->current_index_B];
    
    int correct = 0;
    char message[256];
    
    // Check if choice is correct
    // choice = 1: Player thinks B is HIGHER than A
    // choice = 2: Player thinks B is LOWER than A
    if (choice == 1) {
        correct = (itemB->value >= itemA->value);  // B >= A means B is higher or equal
    } else if (choice == 2) {
        correct = (itemB->value <= itemA->value);  // B <= A means B is lower or equal
    }
    
    // Mark player as answered
    player->has_answered = 1;
    player->last_answer_correct = correct;
    player->response_time_ms = response_time_ms;
    
    if (correct) {
        player->score += SCORE_PER_CORRECT;
        player->streak++;
        snprintf(message, sizeof(message), "%s: $%d - Đúng rồi!", itemB->name, itemB->value);
    } else {
        player->streak = 0;
        // Không game over, chỉ mất streak
        snprintf(message, sizeof(message), "%s: $%d - Sai rồi!", itemB->name, itemB->value);
    }
    
    int room_id = room->id;
    
    // Count how many players have answered this round
    int total_players = room->player_count;
    int answered_players = 0;
    
    for (int i = 0; i < room->player_count; i++) {
        if (room->players[i].has_answered) {
            answered_players++;
        }
    }
    
    // Build immediate response for this player ONLY
    char response[RESPONSE_SIZE];
    snprintf(response, sizeof(response),
        "{\"action\":\"choice_result\",\"correct\":%s,\"score\":%d,\"streak\":%d,\"message\":\"%s\",\"valueB\":%d,\"waiting_for\":%d,\"response_time\":%d}",
        correct ? "true" : "false",
        player->score, player->streak,
        message,
        itemB->value,
        total_players - answered_players,
        response_time_ms
    );
    
    // Send immediate response to this player
    send_json_response(sock, response);
    
    // KHÔNG broadcast khi chưa đủ người trả lời
    // Chỉ log thông tin
    printf("[ROOM] 🎯 Player %d answered: %s (Score: %d, Time: %dms) - %d/%d answered\n", 
           session_id, correct ? "✅" : "❌", player->score, response_time_ms, answered_players, total_players);
    
    char room_json[BUFFER_SIZE];
    char broadcast_json[RESPONSE_SIZE];
    
    if (answered_players >= total_players) {
        // TẤT CẢ đã trả lời - broadcast kết quả cho mọi người
        // Build players results JSON
        char results_json[2048] = "[";
        for (int i = 0; i < room->player_count; i++) {
            char player_result[256];
            snprintf(player_result, sizeof(player_result),
                "%s{\"session_id\":%d,\"name\":\"%s\",\"correct\":%s,\"score\":%d,\"streak\":%d,\"response_time\":%d}",
                i > 0 ? "," : "",
                room->players[i].session_id,
                room->players[i].name,
                room->players[i].last_answer_correct ? "true" : "false",
                room->players[i].score,
                room->players[i].streak,
                room->players[i].response_time_ms
            );
            strcat(results_json, player_result);
        }
        strcat(results_json, "]");
        
        // Broadcast round results để hiển thị kết quả của tất cả players
        snprintf(broadcast_json, sizeof(broadcast_json),
            "{\"action\":\"round_results\",\"round\":%d,\"valueB\":%d,\"labelB\":\"%s\",\"results\":%s}",
            room->current_round, itemB->value, itemB->name, results_json);
        broadcast_sse_to_room(room_id, broadcast_json);
        
        printf("[ROOM] 📊 Round %d results broadcasted\n", room->current_round);
        
        // Check if reached max rounds
        if (room->max_rounds > 0 && room->current_round >= room->max_rounds) {
            // Game finished - reached max rounds
            room->status = ROOM_FINISHED;
            
            build_room_json(room, room_json, sizeof(room_json));
            snprintf(broadcast_json, sizeof(broadcast_json),
                "{\"action\":\"game_finished\",\"room\":%s}", room_json);
            
            pthread_mutex_unlock(&rooms_mutex);
            broadcast_sse_to_room(room_id, broadcast_json);
            
            printf("[ROOM] 🏆 Game finished in room ID: %d (reached %d rounds)\n", room_id, room->max_rounds);
            return;
        }
        
        // All active players answered - move to next round
        room->current_round++;
        
        // Pick new question: B becomes A, pick new B
        room->current_index_A = room->current_index_B;
        room->current_index_B = get_random_index_except(room->current_index_A);
        
        // Reset answered flags for all players
        for (int i = 0; i < room->player_count; i++) {
            room->players[i].has_answered = 0;
        }
        
        // Get new items
        itemA = &game_database[room->current_index_A];
        itemB = &game_database[room->current_index_B];
        
        build_room_json(room, room_json, sizeof(room_json));
        
        // Broadcast new round
        snprintf(broadcast_json, sizeof(broadcast_json),
            "{\"action\":\"new_round\",\"round\":%d,\"room\":%s,\"labelA\":\"%s\",\"valueA\":%d,\"labelB\":\"%s\"}",
            room->current_round, room_json,
            itemA->name, itemA->value,
            itemB->name);
        
        pthread_mutex_unlock(&rooms_mutex);
        broadcast_sse_to_room(room_id, broadcast_json);
        
        printf("[ROOM] ➡️  Round %d/%d: %s ($%d) vs %s (?)\n", 
               room->current_round, room->max_rounds, itemA->name, itemA->value, itemB->name);
    } else {
        pthread_mutex_unlock(&rooms_mutex);
    }
}

// Handle GET /rooms/info - Get current room info for a player
void handle_get_room_info(int sock, int session_id) {
    if (session_id == 0) {
        char error_json[] = "{\"error\":\"No session ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Find the room this player is in
    int room_idx = -1;
    int player_idx = -1;
    
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status != ROOM_EMPTY) {
            int p_idx = find_player_in_room(&rooms[i], session_id);
            if (p_idx >= 0) {
                room_idx = i;
                player_idx = p_idx;
                break;
            }
        }
    }
    
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        char response[] = "{\"action\":\"room_info\",\"in_room\":false}";
        send_json_response(sock, response);
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    RoomPlayer *player = &room->players[player_idx];
    
    // Use room's shared question
    GameItem *itemA = &game_database[room->current_index_A];
    GameItem *itemB = &game_database[room->current_index_B];
    
    char room_json[BUFFER_SIZE];
    build_room_json(room, room_json, sizeof(room_json));
    
    char response[RESPONSE_SIZE];
    snprintf(response, sizeof(response),
        "{"
        "\"action\":\"room_info\","
        "\"in_room\":true,"
        "\"is_host\":%s,"
        "\"room\":%s,"
        "\"round\":%d,"
        "\"my_score\":%d,"
        "\"my_streak\":%d,"
        "\"my_game_over\":%s,"
        "\"has_answered\":%s,"
        "\"labelA\":\"%s\","
        "\"valueA\":%d,"
        "\"labelB\":\"%s\""
        "}",
        room->host_session_id == session_id ? "true" : "false",
        room_json,
        room->current_round,
        player->score, player->streak,
        player->game_over ? "true" : "false",
        player->has_answered ? "true" : "false",
        itemA->name, itemA->value,
        itemB->name
    );
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, response);
}
