/*
 * ============================================================================
 *                    HIGHER LOWER GAME - SSE MODULE
 * ============================================================================
 * File: sse.c
 * Description: Server-Sent Events handling
 * 
 * Chức năng:
 *   1. Xử lý SSE subscription
 *   2. Broadcast messages đến session/room
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "../include/game.h"

/* ============================================================================
 *                           SSE SUBSCRIPTION
 * ============================================================================ */

/**
 * Xử lý SSE subscription request
 * 
 * Flow:
 *   1. Gửi SSE headers
 *   2. Tạo session ID mới
 *   3. Thêm client vào sse_clients array
 *   4. Gửi connected message
 *   5. Giữ connection mở
 */
void handle_sse_subscribe(int client_sock) {
    // Send SSE headers
    char sse_headers[512];
    snprintf(sse_headers, sizeof(sse_headers),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/event-stream\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n"
    );
    
    write(client_sock, sse_headers, strlen(sse_headers));
    
    // Generate session ID and add client
    pthread_mutex_lock(&clients_mutex);
    
    int session_id = next_session_id++;
    int added = 0;
    int active_count = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sse_clients[i].active) active_count++;
        
        if (!added && !sse_clients[i].active) {
            sse_clients[i].socket = client_sock;
            sse_clients[i].active = 1;
            sse_clients[i].session_id = session_id;
            sse_clients[i].room_id = -1;  // Not in any room
            sse_clients[i].player_name[0] = '\0';  // No name yet
            added = 1;
            active_count++;
            printf("\n[SSE] ✅ New client connected\n");
            printf("      Socket: %d | Session: %d | Slot: %d\n", client_sock, session_id, i);
            printf("      Active SSE clients: %d/%d\n", active_count, MAX_CLIENTS);
            printf("==========================================\n");
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
    
    if (!added) {
        printf("Warning: SSE client limit reached\n");
        close(client_sock);
        return;
    }
    
    // Send initial connection message with session ID
    char init_message[512];
    snprintf(init_message, sizeof(init_message), 
             "data: {\"message\":\"Connected to SSE stream\",\"session_id\":%d}\n\n", session_id);
    write(client_sock, init_message, strlen(init_message));
    
    // QUAN TRỌNG: Giữ connection mở - KHÔNG close socket
    // Socket sẽ bị close khi client disconnect hoặc broadcast fail
}

/* ============================================================================
 *                           SSE BROADCAST FUNCTIONS
 * ============================================================================ */

/**
 * Gửi SSE message đến một session cụ thể
 * 
 * @param session_id Session ID cần gửi
 * @param json_data JSON data để gửi
 */
void broadcast_sse_to_session(int session_id, char *json_data) {
    char sse_message[BUFFER_SIZE];
    
    // Format as SSE: "data: {json}\n\n"
    snprintf(sse_message, sizeof(sse_message), "data: %s\n\n", json_data);
    
    pthread_mutex_lock(&clients_mutex);
    
    int sent = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sse_clients[i].active && sse_clients[i].session_id == session_id) {
            int bytes_sent = write(sse_clients[i].socket, sse_message, strlen(sse_message));
            
            // If write fails, mark client as inactive
            if (bytes_sent <= 0) {
                printf("\n[SSE] ❌ Client disconnected: socket %d (session %d)\n", sse_clients[i].socket, session_id);
                close(sse_clients[i].socket);
                sse_clients[i].active = 0;
            } else {
                sent = 1;
                printf("[SSE] 📡 Update sent to session %d\n", session_id);
            }
            break; // Only one client per session
        }
    }
    
    if (!sent) {
        printf("[SSE] ⚠️  No active client for session %d\n", session_id);
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

/**
 * Gửi SSE message đến tất cả players trong một phòng
 * 
 * @param room_id Room ID cần gửi
 * @param json_data JSON data để gửi
 */
void broadcast_sse_to_room(int room_id, char *json_data) {
    if (room_id <= 0) return;
    
    char sse_message[BUFFER_SIZE];
    snprintf(sse_message, sizeof(sse_message), "data: %s\n\n", json_data);
    
    pthread_mutex_lock(&clients_mutex);
    
    int sent_count = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sse_clients[i].active && sse_clients[i].room_id == room_id) {
            int bytes_sent = write(sse_clients[i].socket, sse_message, strlen(sse_message));
            
            if (bytes_sent <= 0) {
                printf("[SSE] ❌ Client disconnected: socket %d (session %d, room %d)\n", 
                       sse_clients[i].socket, sse_clients[i].session_id, room_id);
                close(sse_clients[i].socket);
                sse_clients[i].active = 0;
            } else {
                sent_count++;
            }
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
    
    if (sent_count > 0) {
        printf("[SSE] 📡 Broadcast to room %d: %d clients\n", room_id, sent_count);
    }
}
