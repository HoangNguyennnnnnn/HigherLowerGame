/*
 * ============================================================================
 *                    HIGHER LOWER GAME - ROOM HELPERS
 * ============================================================================
 * File: room_helpers.h
 * Description: Helper functions và JSON builders cho room system
 * ============================================================================
 */

#ifndef ROOM_HELPERS_H
#define ROOM_HELPERS_H

#include "types.h"

/* ============================================================================
 *                           ROOM FINDER FUNCTIONS
 * ============================================================================ */

/**
 * Tìm room index theo room_id
 * @return Index trong mảng rooms, hoặc -1 nếu không tìm thấy
 */
int find_room_index(int room_id);

/**
 * Tìm player trong room theo session_id
 * @return Index của player trong room->players, hoặc -1 nếu không tìm thấy
 */
int find_player_in_room(GameRoom *room, int session_id);

/**
 * Tìm room chứa player theo session_id
 * @param player_idx Output: index của player trong room (có thể NULL)
 * @return Index của room trong mảng rooms, hoặc -1 nếu không tìm thấy
 */
int find_room_with_player(int session_id, int *player_idx);

/**
 * Tìm slot trống trong mảng rooms
 * @return Index của slot trống, hoặc -1 nếu đầy
 */
int find_empty_room_slot(void);

/**
 * Kiểm tra player đã ở trong phòng nào chưa
 * @return 1 nếu đã ở trong phòng, 0 nếu chưa
 */
int is_player_in_any_room(int session_id);

/* ============================================================================
 *                           SSE & PLAYER HELPERS
 * ============================================================================ */

/**
 * Cập nhật room_id và player_name cho SSE client
 */
void update_sse_client_room(int session_id, int room_id, const char *player_name);

/**
 * Khởi tạo RoomPlayer với giá trị mặc định
 */
void init_room_player(RoomPlayer *player, int session_id, const char *name, int is_host);

/**
 * Đếm số player đã trả lời trong round hiện tại
 */
int count_answered_players(GameRoom *room);

/**
 * Reset trạng thái round cho tất cả players
 */
void reset_round_state(GameRoom *room);

/* ============================================================================
 *                           JSON PARSE HELPERS
 * ============================================================================ */

/**
 * Parse chuỗi JSON để lấy giá trị string theo key
 */
void parse_json_string(const char *json, const char *key, char *out, size_t out_size);

/**
 * Parse chuỗi JSON để lấy giá trị int theo key
 * @return Giá trị int, hoặc 0 nếu không tìm thấy
 */
int parse_json_int(const char *json, const char *key);

/* ============================================================================
 *                           JSON BUILDERS
 * ============================================================================ */

/**
 * Build JSON array các players trong room
 */
void build_players_json(GameRoom *room, char *json, size_t json_size);

/**
 * Build JSON object cho room info
 */
void build_room_json(GameRoom *room, char *json, size_t json_size);

/**
 * Build JSON object cho kết quả round
 */
void build_round_results_json(GameRoom *room, int round, int valueB, const char *labelB, 
                               char *json, size_t json_size);

#endif // ROOM_HELPERS_H
