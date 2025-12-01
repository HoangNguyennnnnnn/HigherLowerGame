/*
 * ============================================================================
 *                    HIGHER LOWER GAME - ROOM MANAGEMENT
 * ============================================================================
 * File: room.h
 * Description: Room/Lobby system declarations
 * ============================================================================
 */

#ifndef ROOM_H
#define ROOM_H

#include "types.h"
#include <pthread.h>

/* ============================================================================
 *                           GLOBAL VARIABLES
 * ============================================================================ */

// Danh sách tất cả phòng
extern GameRoom rooms[MAX_ROOMS];

// Mutex để bảo vệ danh sách phòng
extern pthread_mutex_t rooms_mutex;

// ID phòng tiếp theo
extern int next_room_id;

/* ============================================================================
 *                           INITIALIZATION
 * ============================================================================ */

/**
 * Khởi tạo hệ thống phòng
 * 
 * Reset tất cả rooms về trạng thái ROOM_EMPTY
 */
void init_rooms(void);

/* ============================================================================
 *                           HTTP HANDLERS
 * ============================================================================ */

/**
 * GET /rooms - Lấy danh sách phòng
 * 
 * Response: { "action": "room_list", "rooms": [...] }
 */
void handle_list_rooms(int sock);

/**
 * POST /rooms/create - Tạo phòng mới
 * 
 * Request body: { "room_name": "...", "player_name": "...", "max_players": N }
 * Response: { "action": "room_created", "room": {...} }
 */
void handle_create_room(int sock, int session_id, char *json_body);

/**
 * POST /rooms/join - Vào phòng
 * 
 * Request body: { "room_id": N, "player_name": "..." }
 * Response: { "action": "room_joined", "room": {...} }
 */
void handle_join_room(int sock, int session_id, char *json_body);

/**
 * POST /rooms/leave - Rời phòng
 * 
 * Response: { "action": "room_left", "message": "..." }
 */
void handle_leave_room(int sock, int session_id);

/**
 * POST /rooms/start - Bắt đầu game (chỉ host)
 * 
 * Response: { "action": "game_started", "room": {...} }
 */
void handle_start_game(int sock, int session_id);

/**
 * POST /rooms/choice - Chọn đáp án trong game
 * 
 * Request body: { "choice": 1 hoặc 2 }
 * Response: { "action": "choice_result", ... }
 */
void handle_room_choice(int sock, int session_id, char *json_body);

/**
 * GET /rooms/info - Lấy thông tin phòng hiện tại
 * 
 * Response: { "action": "room_info", "in_room": true/false, ... }
 */
void handle_get_room_info(int sock, int session_id);

#endif // ROOM_H
