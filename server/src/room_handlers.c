/*
 * ============================================================================
 *                    HIGHER LOWER GAME - ROOM HANDLERS
 * ============================================================================
 * File: room_handlers.c
 * Description: HTTP handlers cho room management (list, create, join, leave)
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/game.h"
#include "../include/room_helpers.h"

/* ============================================================================
 *                           EXTERNAL VARIABLES
 * ============================================================================ */

extern int item_count;

/* ============================================================================
 *                           ROOM MANAGEMENT HANDLERS
 * ============================================================================ */

/**
 * GET /rooms - Lấy danh sách phòng
 */
void handle_list_rooms(int sock) {
    pthread_mutex_lock(&rooms_mutex);
    
    char response[BUFFER_SIZE] = "{\"action\":\"room_list\",\"rooms\":[";
    int first = 1;
    
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status == ROOM_WAITING || rooms[i].status == ROOM_PLAYING) {
            char entry[512];
            snprintf(entry, sizeof(entry),
                "%s{\"id\":%d,\"name\":\"%s\",\"player_count\":%d,\"max_players\":%d,\"status\":\"%s\"}",
                first ? "" : ",",
                rooms[i].id, rooms[i].name, rooms[i].player_count, rooms[i].max_players,
                rooms[i].status == ROOM_WAITING ? "waiting" : "playing"
            );
            strcat(response, entry);
            first = 0;
        }
    }
    strcat(response, "]}");
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, response);
    printf("[ROOM] 📋 Room list requested\n");
}

/**
 * POST /rooms/create - Tạo phòng mới
 */
void handle_create_room(int sock, int session_id, char *json_body) {
    if (session_id == 0) {
        send_json_response(sock, "{\"error\":\"No session ID\"}");
        return;
    }
    
    // Parse request body
    char room_name[ROOM_NAME_LEN] = "Game Room";
    char player_name[PLAYER_NAME_LEN] = "";
    
    parse_json_string(json_body, "room_name", room_name, sizeof(room_name));
    parse_json_string(json_body, "player_name", player_name, sizeof(player_name));
    
    int max_rounds = parse_json_int(json_body, "max_rounds");
    if (max_rounds < 5) max_rounds = 10;
    if (max_rounds > 50) max_rounds = 50;
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Check if already in a room
    if (is_player_in_any_room(session_id)) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"You are already in a room\"}");
        return;
    }
    
    // Find empty slot
    int room_idx = find_empty_room_slot();
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"Server is full, no room slots available\"}");
        return;
    }
    
    // Initialize room
    GameRoom *room = &rooms[room_idx];
    room->id = next_room_id++;
    strncpy(room->name, room_name, ROOM_NAME_LEN - 1);
    room->name[ROOM_NAME_LEN - 1] = '\0';
    room->host_session_id = session_id;
    room->max_players = MAX_PLAYERS_PER_ROOM;
    room->max_rounds = max_rounds;
    room->status = ROOM_WAITING;
    room->player_count = 1;
    room->current_round = 0;
    
    // Add host as first player
    init_room_player(&room->players[0], session_id, player_name, 1);
    
    // Update SSE client
    update_sse_client_room(session_id, room->id, player_name);
    
    // Build response
    char room_json[BUFFER_SIZE];
    build_room_json(room, room_json, sizeof(room_json));
    
    char response[RESPONSE_SIZE];
    snprintf(response, sizeof(response), "{\"action\":\"room_created\",\"room\":%s}", room_json);
    
    printf("[ROOM] 🏠 Room created: \"%s\" (ID: %d) by session %d\n", room_name, room->id, session_id);
    
    pthread_mutex_unlock(&rooms_mutex);
    send_json_response(sock, response);
}

/**
 * POST /rooms/join - Vào phòng
 */
void handle_join_room(int sock, int session_id, char *json_body) {
    if (session_id == 0) {
        send_json_response(sock, "{\"error\":\"No session ID\"}");
        return;
    }
    
    // Parse request body
    int room_id = parse_json_int(json_body, "room_id");
    char player_name[PLAYER_NAME_LEN] = "";
    parse_json_string(json_body, "player_name", player_name, sizeof(player_name));
    
    if (room_id == 0) {
        send_json_response(sock, "{\"error\":\"Invalid room ID\"}");
        return;
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Check if already in a room
    if (is_player_in_any_room(session_id)) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"You are already in a room\"}");
        return;
    }
    
    // Find room
    int room_idx = find_room_index(room_id);
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"Room not found\"}");
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    
    // Validate room state
    if (room->status != ROOM_WAITING) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"Room is not accepting players (game in progress)\"}");
        return;
    }
    
    if (room->player_count >= MAX_PLAYERS_PER_ROOM) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"Room is full\"}");
        return;
    }
    
    // Add player
    init_room_player(&room->players[room->player_count], session_id, player_name, 0);
    room->player_count++;
    
    // Update SSE client
    update_sse_client_room(session_id, room->id, player_name);
    
    // Build response
    char room_json[BUFFER_SIZE];
    build_room_json(room, room_json, sizeof(room_json));
    
    char response[RESPONSE_SIZE];
    snprintf(response, sizeof(response), "{\"action\":\"room_joined\",\"room\":%s}", room_json);
    
    char notify_json[RESPONSE_SIZE];
    snprintf(notify_json, sizeof(notify_json), "{\"action\":\"player_joined\",\"room\":%s}", room_json);
    
    printf("[ROOM] 👤 Player %d joined room \"%s\" (ID: %d)\n", session_id, room->name, room_id);
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, response);
    broadcast_sse_to_room(room_id, notify_json);
}

/**
 * POST /rooms/leave - Rời phòng
 */
void handle_leave_room(int sock, int session_id) {
    if (session_id == 0) {
        send_json_response(sock, "{\"error\":\"No session ID\"}");
        return;
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Find room with player
    int player_idx;
    int room_idx = find_room_with_player(session_id, &player_idx);
    
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"You are not in any room\"}");
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    int room_id = room->id;
    int was_host = (room->host_session_id == session_id);
    
    // Remove player (shift remaining players)
    for (int i = player_idx; i < room->player_count - 1; i++) {
        room->players[i] = room->players[i + 1];
    }
    room->player_count--;
    
    // Update SSE client
    update_sse_client_room(session_id, -1, NULL);
    
    char response[256];
    char notify_json[RESPONSE_SIZE] = "";
    
    if (room->player_count == 0) {
        // Room empty - delete it
        room->status = ROOM_EMPTY;
        room->id = 0;
        snprintf(response, sizeof(response), "{\"action\":\"room_left\",\"message\":\"Room deleted (empty)\"}");
    } else {
        // Assign new host if needed
        if (was_host) {
            room->host_session_id = room->players[0].session_id;
            room->players[0].is_ready = 1;
        }
        
        char room_json[BUFFER_SIZE];
        build_room_json(room, room_json, sizeof(room_json));
        snprintf(notify_json, sizeof(notify_json), "{\"action\":\"player_left\",\"room\":%s}", room_json);
        snprintf(response, sizeof(response), "{\"action\":\"room_left\",\"message\":\"Left room successfully\"}");
    }
    
    printf("[ROOM] 🚪 Player %d left room ID: %d\n", session_id, room_id);
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, response);
    if (strlen(notify_json) > 0) {
        broadcast_sse_to_room(room_id, notify_json);
    }
}
