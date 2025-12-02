/*
 * ============================================================================
 *                    HIGHER LOWER GAME - ROOM INITIALIZATION
 * ============================================================================
 * File: room_init.c
 * Description: Global variables và khởi tạo room system
 * ============================================================================
 */

#include <stdio.h>
#include <pthread.h>
#include "../include/game.h"

/* ============================================================================
 *                           GLOBAL VARIABLES
 * ============================================================================ */

GameRoom rooms[MAX_ROOMS];                              // Mảng tất cả phòng chơi
pthread_mutex_t rooms_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex bảo vệ rooms
int next_room_id = 1;                                   // ID phòng tiếp theo

/* ============================================================================
 *                           INITIALIZATION
 * ============================================================================ */

void init_rooms(void) {
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
