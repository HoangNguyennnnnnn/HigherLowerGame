/*
 * ============================================================================
 *                    HIGHER LOWER GAME - DATABASE
 * ============================================================================
 * File: database.h
 * Description: Game database declarations
 * ============================================================================
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "types.h"

/* ============================================================================
 *                           GLOBAL VARIABLES
 * ============================================================================ */

// Database lưu trữ tất cả game items
extern GameItem game_database[MAX_ITEMS];

// Số lượng items trong database
extern int item_count;

/* ============================================================================
 *                           DATABASE FUNCTIONS
 * ============================================================================ */

/**
 * Khởi tạo game database
 * 
 * Load hoặc tạo danh sách items cho game
 */
void init_game_database(void);

/**
 * Lấy random index khác với exclude_index
 * 
 * @param exclude_index Index không muốn chọn
 * @return Random index khác exclude_index
 */
int get_random_index_except(int exclude_index);

#endif // DATABASE_H
