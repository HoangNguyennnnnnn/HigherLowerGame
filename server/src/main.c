#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include "../include/game.h"

// Global variables definition
SSE_Client sse_clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

PlayerGameState player_states[MAX_CLIENTS];
pthread_mutex_t game_state_mutex = PTHREAD_MUTEX_INITIALIZER;
int next_session_id = 1;

// Forward declaration for room cleanup
void cleanup_player_from_rooms(int session_id);

// Handle SSE subscription
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
    
    // Keep connection open - do NOT close the socket
    // The socket will be closed when the client disconnects or when broadcast fails
}

// Extract session ID from HTTP request headers
int get_session_from_request(char *request) {
    char *session_header = strstr(request, "X-Session-ID: ");
    if (session_header) {
        session_header += 14; // Skip "X-Session-ID: "
        return atoi(session_header);
    }
    return 0; // No session ID found
}

// Parse HTTP request and route to appropriate handler
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
    
    // Handle OPTIONS (CORS preflight)
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
    
    // Handle GET /subscribe (SSE)
    if (strcmp(method, "GET") == 0 && strcmp(path, "/subscribe") == 0) {
        handle_sse_subscribe(client_sock);
        // DO NOT close socket - SSE connection stays open
        return NULL;
    }
    
    // Handle POST /game (Initialize game)
    if (strcmp(method, "POST") == 0 && strcmp(path, "/game") == 0) {
        int session_id = get_session_from_request(buffer);
        handle_game_init(client_sock, session_id);
        close(client_sock);
        return NULL;
    }
    
    // Handle POST /game/choice (Player choice - single player mode)
    if (strcmp(method, "POST") == 0 && strcmp(path, "/game/choice") == 0) {
        // Extract JSON body from request
        char *body_start = strstr(buffer, "\r\n\r\n");
        if (body_start) {
            body_start += 4; // Skip past "\r\n\r\n"
            int session_id = get_session_from_request(buffer);
            handle_player_choice(client_sock, session_id, body_start);
        } else {
            char error_json[] = "{\"error\":\"No body found\"}";
            send_json_response(client_sock, error_json);
        }
        close(client_sock);
        return NULL;
    }
    
    // ==================== ROOM ENDPOINTS ====================
    
    // Handle GET /rooms (List all rooms)
    if (strcmp(method, "GET") == 0 && strcmp(path, "/rooms") == 0) {
        handle_list_rooms(client_sock);
        close(client_sock);
        return NULL;
    }
    
    // Handle POST /rooms/create (Create a new room)
    if (strcmp(method, "POST") == 0 && strcmp(path, "/rooms/create") == 0) {
        char *body_start = strstr(buffer, "\r\n\r\n");
        int session_id = get_session_from_request(buffer);
        handle_create_room(client_sock, session_id, body_start ? body_start + 4 : "");
        close(client_sock);
        return NULL;
    }
    
    // Handle POST /rooms/join (Join an existing room)
    if (strcmp(method, "POST") == 0 && strcmp(path, "/rooms/join") == 0) {
        char *body_start = strstr(buffer, "\r\n\r\n");
        int session_id = get_session_from_request(buffer);
        handle_join_room(client_sock, session_id, body_start ? body_start + 4 : "");
        close(client_sock);
        return NULL;
    }
    
    // Handle POST /rooms/leave (Leave current room)
    if (strcmp(method, "POST") == 0 && strcmp(path, "/rooms/leave") == 0) {
        int session_id = get_session_from_request(buffer);
        handle_leave_room(client_sock, session_id);
        close(client_sock);
        return NULL;
    }
    
    // Handle POST /rooms/start (Start game - host only)
    if (strcmp(method, "POST") == 0 && strcmp(path, "/rooms/start") == 0) {
        int session_id = get_session_from_request(buffer);
        handle_start_game(client_sock, session_id);
        close(client_sock);
        return NULL;
    }
    
    // Handle POST /rooms/choice (Player choice in room game)
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
    
    // Handle GET /rooms/info (Get current room info)
    if (strcmp(method, "GET") == 0 && strcmp(path, "/rooms/info") == 0) {
        int session_id = get_session_from_request(buffer);
        handle_get_room_info(client_sock, session_id);
        close(client_sock);
        return NULL;
    }
    
    // ==================== END ROOM ENDPOINTS ====================
    
    // Handle unknown routes
    char not_found[256];
    snprintf(not_found, sizeof(not_found),
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{\"error\":\"Route not found\"}"
    );
    write(client_sock, not_found, strlen(not_found));
    close(client_sock);
    
    return NULL;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize game database
    init_game_database();
    
    // Initialize rooms
    init_rooms();
    
    // Initialize SSE clients and player states arrays
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sse_clients[i].active = 0;
        sse_clients[i].room_id = -1;
        sse_clients[i].player_name[0] = '\0';
        player_states[i].active = 0;
    }
    
    // Create TCP socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(1);
    }
    
    // Listen for connections
    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(1);
    }
    
    printf("\n");
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║   Higher Lower Game Server - C Edition   ║\n");
    printf("╠═══════════════════════════════════════════╣\n");
    printf("║  Server URL: http://localhost:%d        ║\n", PORT);
    printf("║  WSL IP:     http://172.20.127.157:%d   ║\n", PORT);
    printf("║  Max Clients: %-3d | Max Rooms: %-3d       ║\n", MAX_CLIENTS, MAX_ROOMS);
    printf("╠═══════════════════════════════════════════╣\n");
    printf("║  Features:                                ║\n");
    printf("║  • SSE (Server-Sent Events)               ║\n");
    printf("║  • Room/Lobby system                      ║\n");
    printf("║  • Multi-player support                   ║\n");
    printf("║  • CORS enabled                           ║\n");
    printf("╚═══════════════════════════════════════════╝\n");
    printf("\nWaiting for connections...\n\n");
    
    // Main server loop
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("[CONN] 🔌 New connection: socket %d\n", client_sock);
        
        // Create thread to handle client
        pthread_t thread_id;
        int *sock_ptr = malloc(sizeof(int));
        *sock_ptr = client_sock;
        
        if (pthread_create(&thread_id, NULL, handle_client, sock_ptr) != 0) {
            perror("Thread creation failed");
            free(sock_ptr);
            close(client_sock);
            continue;
        }
        
        // Detach thread so resources are freed automatically
        pthread_detach(thread_id);
    }
    
    close(server_sock);
    return 0;
}
