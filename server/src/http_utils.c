#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "../include/game.h"

// Send CORS headers with credentials support
void send_cors_headers(int sock) {
    char headers[512];
    snprintf(headers, sizeof(headers),
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
    );
    write(sock, headers, strlen(headers));
}

// Send JSON response with full HTTP headers
void send_json_response(int sock, char *body) {
    char response[BUFFER_SIZE];
    int body_len = strlen(body);
    
    int header_len = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        body_len, body
    );
    
    write(sock, response, header_len);
}

// Broadcast SSE message to specific session
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
