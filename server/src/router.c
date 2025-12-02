/*
 * ============================================================================
 *                    HIGHER LOWER GAME - HTTP ROUTER
 * ============================================================================
 * File: router.c
 * Description: HTTP request parsing và routing
 * 
 * Chức năng:
 *   1. Parse HTTP requests
 *   2. Route đến handlers phù hợp
 *   3. CORS preflight handling
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/game.h"

/* ============================================================================
 *                           HTTP HELPERS
 * ============================================================================ */

/**
 * Lấy session ID từ HTTP request header
 * 
 * Tìm header "X-Session-ID: {number}"
 * 
 * @param request HTTP request buffer
 * @return Session ID, hoặc 0 nếu không tìm thấy
 */
int get_session_from_request(char *request) {
    char *session_header = strstr(request, "X-Session-ID: ");
    if (session_header) {
        session_header += 14; // Skip "X-Session-ID: "
        return atoi(session_header);
    }
    return 0; // No session ID found
}

/* ============================================================================
 *                           HTTP REQUEST ROUTER
 * ============================================================================ */

/**
 * Parse HTTP request và route đến handler phù hợp
 * 
 * Supported routes:
 *   - GET  /subscribe      -> SSE connection
 *   - GET  /rooms          -> List all rooms
 *   - GET  /rooms/info     -> Get current room info
 *   - POST /rooms/create   -> Create new room
 *   - POST /rooms/join     -> Join a room
 *   - POST /rooms/leave    -> Leave room
 *   - POST /rooms/start    -> Start game (host only)
 *   - POST /rooms/choice   -> Make a choice
 */
void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    int bytes_read = read(client_sock, buffer, sizeof(buffer) - 1);
    
    if (bytes_read <= 0) {
        close(client_sock);
        return NULL;
    }
    
    buffer[bytes_read] = '\0';
    
    // Parse HTTP method and path
    char method[16], path[256];
    sscanf(buffer, "%s %s", method, path);
    
    /* ---------- OPTIONS (CORS Preflight) ---------- */
    
    if (strcmp(method, "OPTIONS") == 0) {
        char response[512];
        snprintf(response, sizeof(response),
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type, X-Session-ID\r\n"
            "Connection: close\r\n"
            "\r\n"
        );
        write(client_sock, response, strlen(response));
        close(client_sock);
        return NULL;
    }
    
    /* ---------- SSE ENDPOINT ---------- */
    
    // GET /subscribe - SSE Connection
    if (strcmp(method, "GET") == 0 && strcmp(path, "/subscribe") == 0) {
        handle_sse_subscribe(client_sock);
        return NULL;  // KHÔNG close socket - SSE connection giữ mở
    }
    
    /* ---------- ROOM ENDPOINTS ---------- */
    
    // GET /rooms - Lấy danh sách phòng
    if (strcmp(method, "GET") == 0 && strcmp(path, "/rooms") == 0) {
        handle_list_rooms(client_sock);
        close(client_sock);
        return NULL;
    }
    
    // POST /rooms/create - Tạo phòng mới
    if (strcmp(method, "POST") == 0 && strcmp(path, "/rooms/create") == 0) {
        char *body_start = strstr(buffer, "\r\n\r\n");
        int session_id = get_session_from_request(buffer);
        handle_create_room(client_sock, session_id, body_start ? body_start + 4 : "");
        close(client_sock);
        return NULL;
    }
    
    // POST /rooms/join - Vào phòng
    if (strcmp(method, "POST") == 0 && strcmp(path, "/rooms/join") == 0) {
        char *body_start = strstr(buffer, "\r\n\r\n");
        int session_id = get_session_from_request(buffer);
        handle_join_room(client_sock, session_id, body_start ? body_start + 4 : "");
        close(client_sock);
        return NULL;
    }
    
    // POST /rooms/leave - Rời phòng
    if (strcmp(method, "POST") == 0 && strcmp(path, "/rooms/leave") == 0) {
        int session_id = get_session_from_request(buffer);
        handle_leave_room(client_sock, session_id);
        close(client_sock);
        return NULL;
    }
    
    // POST /rooms/start - Bắt đầu game (host only)
    if (strcmp(method, "POST") == 0 && strcmp(path, "/rooms/start") == 0) {
        int session_id = get_session_from_request(buffer);
        handle_start_game(client_sock, session_id);
        close(client_sock);
        return NULL;
    }
    
    // POST /rooms/choice - Chọn đáp án trong game
    if (strcmp(method, "POST") == 0 && strcmp(path, "/rooms/choice") == 0) {
        char *body_start = strstr(buffer, "\r\n\r\n");
        int session_id = get_session_from_request(buffer);
        if (body_start) {
            handle_room_choice(client_sock, session_id, body_start + 4);
        } else {
            char error_json[] = "{\"error\":\"No body found\"}";
            send_json_response(client_sock, error_json);
        }
        close(client_sock);
        return NULL;
    }
    
    // GET /rooms/info - Lấy thông tin phòng hiện tại
    if (strcmp(method, "GET") == 0 && strcmp(path, "/rooms/info") == 0) {
        int session_id = get_session_from_request(buffer);
        handle_get_room_info(client_sock, session_id);
        close(client_sock);
        return NULL;
    }
    
    /* ---------- 404 NOT FOUND ---------- */
    
    char not_found[512];
    snprintf(not_found, sizeof(not_found),
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{\"error\":\"Route not found: %s %s\"}",
        method, path
    );
    write(client_sock, not_found, strlen(not_found));
    close(client_sock);
    
    return NULL;
}
