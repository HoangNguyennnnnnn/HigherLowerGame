/*
 * ============================================================================
 *                    HIGHER LOWER GAME - SSE
 * ============================================================================
 * File: sse.h
 * Description: Server-Sent Events handling
 * ============================================================================
 */

#ifndef SSE_H
#define SSE_H

#include "types.h"
#include <pthread.h>

/* ============================================================================
 *                           GLOBAL VARIABLES
 * ============================================================================ */

// Danh sách tất cả SSE clients
extern SSE_Client sse_clients[MAX_CLIENTS];

// Mutex để bảo vệ danh sách SSE clients
extern pthread_mutex_t clients_mutex;

/* ============================================================================
 *                           SSE CONNECTION FUNCTIONS
 * ============================================================================ */

/**
 * Xử lý request subscribe SSE
 * 
 * - Gửi SSE headers
 * - Tạo session ID mới
 * - Thêm client vào danh sách
 * - Gửi message connected
 * 
 * @param client_sock Socket của client
 */
void handle_sse_subscribe(int client_sock);

/* ============================================================================
 *                           BROADCAST FUNCTIONS
 * ============================================================================ */

/**
 * Gửi SSE message đến một session cụ thể
 * 
 * @param session_id Session ID cần gửi
 * @param json_data JSON data để gửi
 */
void broadcast_sse_to_session(int session_id, char *json_data);

/**
 * Gửi SSE message đến tất cả người chơi trong một phòng
 * 
 * @param room_id Room ID cần gửi
 * @param json_data JSON data để gửi
 */
void broadcast_sse_to_room(int room_id, char *json_data);

#endif // SSE_H
