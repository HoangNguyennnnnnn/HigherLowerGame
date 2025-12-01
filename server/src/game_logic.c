#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "../include/game.h"

// Game database - loaded from file
GameItem game_database[MAX_ITEMS];
int item_count = 0;  // Actual number of items loaded

// Load items from file
void init_game_database() {
    FILE *file = fopen(ITEMS_FILE, "r");
    if (!file) {
        printf("⚠️  Warning: Could not open %s, using default items\n", ITEMS_FILE);
        
        // Fallback to default items
        strcpy(game_database[0].name, "iPhone 15 Pro");
        game_database[0].value = 1199;
        strcpy(game_database[0].image_url, "https://images.unsplash.com/photo-1695048133142-1a20484d2569?w=400");
        
        strcpy(game_database[1].name, "MacBook Pro");
        game_database[1].value = 2499;
        strcpy(game_database[1].image_url, "https://images.unsplash.com/photo-1517336714731-489689fd1ca8?w=400");
        
        strcpy(game_database[2].name, "PlayStation 5");
        game_database[2].value = 499;
        strcpy(game_database[2].image_url, "https://images.unsplash.com/photo-1606813907291-d86efa9b94db?w=400");
        
        strcpy(game_database[3].name, "Nike Air Jordan");
        game_database[3].value = 170;
        strcpy(game_database[3].image_url, "https://images.unsplash.com/photo-1542291026-7eec264c27ff?w=400");
        
        strcpy(game_database[4].name, "Tesla Model 3");
        game_database[4].value = 42990;
        strcpy(game_database[4].image_url, "https://images.unsplash.com/photo-1560958089-b8a1929cea89?w=400");
        
        item_count = 5;
        printf("📦 Game database initialized with %d default items\n", item_count);
        return;
    }
    
    char line[512];
    item_count = 0;
    
    while (fgets(line, sizeof(line), file) && item_count < MAX_ITEMS) {
        // Skip empty lines and comments
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        
        // Remove newline
        line[strcspn(line, "\n\r")] = 0;
        
        // Parse: name|value|image_url
        char *name = strtok(line, "|");
        char *value_str = strtok(NULL, "|");
        char *image_url = strtok(NULL, "|");
        
        if (name && value_str && image_url) {
            strncpy(game_database[item_count].name, name, ITEM_NAME_LEN - 1);
            game_database[item_count].name[ITEM_NAME_LEN - 1] = '\0';
            
            game_database[item_count].value = atoi(value_str);
            
            strncpy(game_database[item_count].image_url, image_url, IMAGE_URL_LEN - 1);
            game_database[item_count].image_url[IMAGE_URL_LEN - 1] = '\0';
            
            item_count++;
        }
    }
    
    fclose(file);
    
    if (item_count < 2) {
        printf("⚠️  Warning: Not enough items loaded, adding defaults\n");
        strcpy(game_database[0].name, "Item A");
        game_database[0].value = 100;
        strcpy(game_database[0].image_url, "https://via.placeholder.com/400");
        
        strcpy(game_database[1].name, "Item B");
        game_database[1].value = 200;
        strcpy(game_database[1].image_url, "https://via.placeholder.com/400");
        
        item_count = 2;
    }
    
    printf("📦 Game database loaded %d items from %s\n", item_count, ITEMS_FILE);
    for (int i = 0; i < item_count && i < 5; i++) {
        printf("   • %s: $%d\n", game_database[i].name, game_database[i].value);
    }
    if (item_count > 5) {
        printf("   ... and %d more items\n", item_count - 5);
    }
}

// Get random index except a specific one
int get_random_index_except(int except) {
    if (item_count <= 1) return 0;
    int index;
    do {
        index = rand() % item_count;
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
