/**
 * @file main.c
 * @brief Entry point và khởi tạo server
 * 
 * File này chứa:
 * - Biến toàn cục (global variables)
 * - Hàm main() khởi tạo server và accept connections
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "../include/game.h"

/* =============================================================================
 * BIẾN TOÀN CỤC (GLOBAL VARIABLES)
 * ========================================================================== */

/**
 * @brief Mảng lưu trữ các SSE client connections
 */
SSE_Client sse_clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Mảng lưu trạng thái player (cho single player mode)
 */
PlayerGameState player_states[MAX_CLIENTS];
pthread_mutex_t game_state_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief ID tự tăng cho session mới
 */
int next_session_id = 1;

/* =============================================================================
 * ENTRY POINT
 * ========================================================================== */

/**
 * @brief Hàm main - khởi tạo server và chạy vòng lặp accept
 */
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Khởi tạo database từ file items.txt
    init_game_database();
    
    // Khởi tạo rooms cho multiplayer
    init_rooms();
    
    // Khởi tạo SSE clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sse_clients[i].active = 0;
        sse_clients[i].socket = -1;
    }
    
    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Cho phép reuse address
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    // Cấu hình address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen
    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("===========================================\n");
    printf("  Higher Lower Game Server\n");
    printf("  Port: %d\n", PORT);
    printf("  Loaded: %d items\n", item_count);
    printf("===========================================\n");
    
    // Vòng lặp chính - accept connections
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        // Tạo thread để xử lý client
        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;
        
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, client_socket);
        pthread_detach(thread_id);
    }
    
    close(server_fd);
    return 0;
}
