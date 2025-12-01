/*
 * ============================================================================
 *                    HIGHER LOWER GAME - SINGLE PLAYER MODULE
 * ============================================================================
 * File: game_single.c
 * Description: Single player game logic (Legacy mode)
 * 
 * Chức năng:
 *   1. Khởi tạo game single player
 *   2. Xử lý lựa chọn của người chơi
 * 
 * Note: Đây là mode cũ, chủ yếu dùng cho testing.
 *       Mode chính là multiplayer rooms.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/game.h"

/* ============================================================================
 *                           EXTERNAL VARIABLES
 * ============================================================================ */

// Từ database.c
extern GameItem game_database[MAX_ITEMS];
extern int item_count;

/* ============================================================================
 *                           SINGLE PLAYER HANDLERS
 * ============================================================================ */

/**
 * Khởi tạo game mới cho single player
 * 
 * Route: POST /game
 * Response: { action: "update_game", score, streak, labelA, valueA, ... }
 */
void handle_game_init(int sock, int session_id) {
    if (session_id == 0) {
        char error_json[] = "{\"error\":\"No session ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    pthread_mutex_lock(&game_state_mutex);
    
    // Find or create player state
    int player_idx = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (player_states[i].active && player_states[i].session_id == session_id) {
            player_idx = i;
            break;
        }
        if (player_idx == -1 && !player_states[i].active) {
            player_idx = i;
        }
    }
    
    if (player_idx == -1) {
        pthread_mutex_unlock(&game_state_mutex);
        char error_json[] = "{\"error\":\"Server full\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Initialize/reset player state
    player_states[player_idx].active = 1;
    player_states[player_idx].session_id = session_id;
    player_states[player_idx].score = 0;
    player_states[player_idx].streak = 0;
    player_states[player_idx].current_index_A = rand() % item_count;
    player_states[player_idx].current_index_B = get_random_index_except(player_states[player_idx].current_index_A);
    
    GameItem *itemA = &game_database[player_states[player_idx].current_index_A];
    GameItem *itemB = &game_database[player_states[player_idx].current_index_B];
    
    // Build JSON response
    char json[BUFFER_SIZE];
    snprintf(json, sizeof(json),
        "{"
        "\"action\":\"update_game\","
        "\"score\":%d,"
        "\"streak\":%d,"
        "\"labelA\":\"%s\","
        "\"valueA\":%d,"
        "\"imageA\":\"%s\","
        "\"labelB\":\"%s\","
        "\"valueB\":%d,"
        "\"imageB\":\"%s\","
        "\"message\":\"Game initialized! Make your guess.\""
        "}",
        player_states[player_idx].score, player_states[player_idx].streak,
        itemA->name, itemA->value, itemA->image_url,
        itemB->name, itemB->value, itemB->image_url
    );
    
    pthread_mutex_unlock(&game_state_mutex);
    
    send_json_response(sock, json);
    
    printf("\n[GAME] 🎮 Single player game initialized (Session %d)\n", session_id);
    printf("       Item A: %s ($%d)\n", itemA->name, itemA->value);
    printf("       Item B: %s ($%d)\n", itemB->name, itemB->value);
    printf("==========================================\n");
}

/**
 * Xử lý lựa chọn của người chơi (single player)
 * 
 * Route: POST /game/choice
 * Body: { choice: 1|2 }  // 1 = A cao hơn, 2 = B cao hơn
 */
void handle_player_choice(int sock, int session_id, char *json_body) {
    if (session_id == 0) {
        char error_json[] = "{\"error\":\"No session ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Parse choice from JSON
    char *choice_ptr = strstr(json_body, "\"choice\"");
    if (!choice_ptr) {
        char error_json[] = "{\"error\":\"Invalid request\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    char *value_start = strchr(choice_ptr, ':');
    if (!value_start) {
        char error_json[] = "{\"error\":\"Invalid request\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    value_start++;
    while (*value_start == ' ' || *value_start == '\t') value_start++;
    int choice = atoi(value_start);
    
    pthread_mutex_lock(&game_state_mutex);
    
    // Find player state
    int player_idx = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (player_states[i].active && player_states[i].session_id == session_id) {
            player_idx = i;
            break;
        }
    }
    
    if (player_idx == -1) {
        pthread_mutex_unlock(&game_state_mutex);
        char error_json[] = "{\"error\":\"No game found. Please start a new game.\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    PlayerGameState *player = &player_states[player_idx];
    GameItem *itemA = &game_database[player->current_index_A];
    GameItem *itemB = &game_database[player->current_index_B];
    
    int correct = 0;
    char message[256];
    
    // Check if choice is correct
    if (choice == 1) {
        correct = (itemA->value >= itemB->value);
    } else if (choice == 2) {
        correct = (itemB->value >= itemA->value);
    }
    
    if (correct) {
        player->score += SCORE_PER_CORRECT;
        player->streak++;
        snprintf(message, sizeof(message), "Correct! %s ($%d) vs %s ($%d)", 
                 itemA->name, itemA->value, itemB->name, itemB->value);
        
        // Move B to A, pick new B
        player->current_index_A = player->current_index_B;
        player->current_index_B = get_random_index_except(player->current_index_A);
    } else {
        player->streak = 0;
        snprintf(message, sizeof(message), "Wrong! %s ($%d) vs %s ($%d). Streak reset!", 
                 itemA->name, itemA->value, itemB->name, itemB->value);
        
        // Pick two new random items
        player->current_index_A = rand() % item_count;
        player->current_index_B = get_random_index_except(player->current_index_A);
    }
    
    // Get updated items
    itemA = &game_database[player->current_index_A];
    itemB = &game_database[player->current_index_B];
    
    // Build JSON response
    char json[BUFFER_SIZE];
    snprintf(json, sizeof(json),
        "{"
        "\"action\":\"update_game\","
        "\"score\":%d,"
        "\"streak\":%d,"
        "\"labelA\":\"%s\","
        "\"valueA\":%d,"
        "\"imageA\":\"%s\","
        "\"labelB\":\"%s\","
        "\"valueB\":%d,"
        "\"imageB\":\"%s\","
        "\"message\":\"%s\""
        "}",
        player->score, player->streak,
        itemA->name, itemA->value, itemA->image_url,
        itemB->name, itemB->value, itemB->image_url,
        message
    );
    
    pthread_mutex_unlock(&game_state_mutex);
    
    send_json_response(sock, json);
    
    printf("[GAME] 🎯 Single player choice: %s (Session %d)\n", 
           correct ? "✅ CORRECT" : "❌ WRONG", session_id);
    printf("       Score: %d | Streak: %d\n", player->score, player->streak);
    
    // Broadcast update via SSE
    broadcast_sse_to_session(session_id, json);
}
