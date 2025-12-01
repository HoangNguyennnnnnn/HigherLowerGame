/*
 * ============================================================================
 *                    HIGHER LOWER GAME - SERVER
 * ============================================================================
 * File: server.h
 * Description: TCP Server and HTTP routing declarations
 * ============================================================================
 */

#ifndef SERVER_H
#define SERVER_H

/* ============================================================================
 *                           SERVER FUNCTIONS
 * ============================================================================ */

/**
 * Xử lý request từ client (trong thread riêng)
 * 
 * - Parse HTTP request
 * - Route đến handler phù hợp
 * 
 * @param arg Pointer đến socket descriptor
 * @return NULL
 */
void* handle_client(void* arg);

/**
 * Lấy session ID từ HTTP request header
 * 
 * Tìm header "X-Session-ID: {number}"
 * 
 * @param request HTTP request buffer
 * @return Session ID, hoặc 0 nếu không tìm thấy
 */
int get_session_from_request(char *request);

/**
 * Cleanup khi player disconnect
 * 
 * Xóa player khỏi tất cả rooms
 * 
 * @param session_id Session ID của player
 */
void cleanup_player_from_rooms(int session_id);

#endif // SERVER_H
