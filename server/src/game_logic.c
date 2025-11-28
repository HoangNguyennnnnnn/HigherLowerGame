#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "../include/game.h"

// Mock database with 5 items
GameItem game_database[MAX_ITEMS];

// Initialize the game database with hardcoded values
void init_game_database() {
    // Bitcoin
    strcpy(game_database[0].name, "Bitcoin");
    game_database[0].value = 50000;
    strcpy(game_database[0].image_url, "https://cryptologos.cc/logos/bitcoin-btc-logo.png");
    
    // Ethereum
    strcpy(game_database[1].name, "Ethereum");
    game_database[1].value = 3000;
    strcpy(game_database[1].image_url, "https://cryptologos.cc/logos/ethereum-eth-logo.png");
    
    // Tesla
    strcpy(game_database[2].name, "Tesla Stock");
    game_database[2].value = 250;
    strcpy(game_database[2].image_url, "https://logo.clearbit.com/tesla.com");
    
    // Gold (per ounce)
    strcpy(game_database[3].name, "Gold (per oz)");
    game_database[3].value = 2000;
    strcpy(game_database[3].image_url, "https://cdn-icons-png.flaticon.com/512/3699/3699516.png");
    
    // Apple Stock
    strcpy(game_database[4].name, "Apple Stock");
    game_database[4].value = 180;
    strcpy(game_database[4].image_url, "https://logo.clearbit.com/apple.com");
    
    printf("Game database initialized with %d items\n", MAX_ITEMS);
}

// Get random index except a specific one
int get_random_index_except(int except) {
    int index;
    do {
        index = rand() % MAX_ITEMS;
    } while (index == except);
    return index;
}

// Handle game initialization
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
    player_states[player_idx].current_index_A = rand() % MAX_ITEMS;
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
    
    // Send response to the requesting client
    send_json_response(sock, json);
    
    printf("\n[GAME] 🎮 New game initialized (Session %d)\n", session_id);
    printf("       Item A: %s ($%d)\n", itemA->name, itemA->value);
    printf("       Item B: %s ($%d)\n", itemB->name, itemB->value);
    printf("==========================================\n");
}

// Handle player choice
void handle_player_choice(int sock, int session_id, char *json_body) {
    if (session_id == 0) {
        char error_json[] = "{\"error\":\"No session ID\"}";
        send_json_response(sock, error_json);
        return;
    }
    // Manual JSON parsing - extract "choice" field
    char *choice_ptr = strstr(json_body, "\"choice\"");
    if (!choice_ptr) {
        char error_json[] = "{\"error\":\"Invalid request\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Find the value after "choice":
    char *value_start = strchr(choice_ptr, ':');
    if (!value_start) {
        char error_json[] = "{\"error\":\"Invalid request\"}";
        send_json_response(sock, error_json);
        return;
    }
    
    // Skip whitespace and parse number
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
        // User chose A has higher value
        correct = (itemA->value >= itemB->value);
    } else if (choice == 2) {
        // User chose B has higher value
        correct = (itemB->value >= itemA->value);
    }
    
    if (correct) {
        player->score += 10;
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
        player->current_index_A = rand() % MAX_ITEMS;
        player->current_index_B = get_random_index_except(player->current_index_A);
    }
    
    // Get updated items
    itemA = &game_database[player->current_index_A];
    itemB = &game_database[player->current_index_B];
    
    // Build JSON response with new state
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
    
    // Send response to the requesting client
    send_json_response(sock, json);
    
    printf("\n[GAME] 🎯 Player choice: %s (Session %d)\n", correct ? "✅ CORRECT" : "❌ WRONG", session_id);
    printf("       Score: %d | Streak: %d\n", player->score, player->streak);
    printf("       Next: %s vs %s\n", itemA->name, itemB->name);
    
    // Broadcast the new state to the player's SSE connection
    broadcast_sse_to_session(session_id, json);
}
