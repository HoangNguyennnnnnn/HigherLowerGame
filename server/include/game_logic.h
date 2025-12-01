/*
 * ============================================================================
 *                    HIGHER LOWER GAME - SINGLE PLAYER LOGIC
 * ============================================================================
 * File: game_logic.h
 * Description: Single player game declarations
 * ============================================================================
 */

#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "types.h"
#include <pthread.h>

/* ============================================================================
 *                           GLOBAL VARIABLES
 * ============================================================================ */

// Database chứa các items trong game
extern GameItem game_database[MAX_ITEMS];

// Trạng thái game của các người chơi (single player)
extern PlayerGameState player_states[MAX_CLIENTS];

// Mutex bảo vệ player states
extern pthread_mutex_t game_state_mutex;

// Session ID tiếp theo
extern int next_session_id;

/* ============================================================================
 *                           INITIALIZATION
 * ============================================================================ */

/**
 * Khởi tạo database game
 * 
 * Load các items vào game_database
 */
void init_game_database(void);

/* ============================================================================
 *                           HTTP HANDLERS
 * ============================================================================ */

/**
 * GET /game - Bắt đầu/lấy thông tin game mới
 * 
 * @param sock Socket để trả response
 * @param session_id ID của session
 */
void handle_game_init(int sock, int session_id);

/**
 * POST /game/choice - Xử lý lựa chọn của người chơi
 * 
 * @param sock Socket để trả response
 * @param session_id ID của session
 * @param json_body Body của request chứa choice
 */
void handle_player_choice(int sock, int session_id, char *json_body);

/* ============================================================================
 *                           UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * Lấy index ngẫu nhiên khác với index cho trước
 * 
 * @param except Index cần tránh
 * @return Index ngẫu nhiên trong khoảng [0, MAX_ITEMS)
 */
int get_random_index_except(int except);

#endif // GAME_LOGIC_H
