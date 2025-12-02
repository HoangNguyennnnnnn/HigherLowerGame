/*
 * ============================================================================
 *                    HIGHER LOWER GAME - GAME HANDLERS
 * ============================================================================
 * File: game_handlers.c
 * Description: HTTP handlers cho game flow (start, choice, info)
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/game.h"
#include "../include/room_helpers.h"

/* ============================================================================
 *                           EXTERNAL VARIABLES
 * ============================================================================ */

extern int item_count;

/* ============================================================================
 *                           GAME FLOW HANDLERS
 * ============================================================================ */

/**
 * POST /rooms/start - Bắt đầu game (host only)
 */
void handle_start_game(int sock, int session_id) {
    if (session_id == 0) {
        send_json_response(sock, "{\"error\":\"No session ID\"}");
        return;
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Find room with player
    int room_idx = find_room_with_player(session_id, NULL);
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"You are not in any room\"}");
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    
    // Validate permissions
    if (room->host_session_id != session_id) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"Only the host can start the game\"}");
        return;
    }
    
    if (room->status != ROOM_WAITING) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"Game has already started\"}");
        return;
    }
    
    // Initialize game state
    room->status = ROOM_PLAYING;
    room->current_round = 1;
    room->current_index_A = rand() % item_count;
    room->current_index_B = get_random_index_except(room->current_index_A);
    
    // Reset all players
    for (int i = 0; i < room->player_count; i++) {
        room->players[i].score = 0;
        room->players[i].streak = 0;
        room->players[i].game_over = 0;
        room->players[i].has_answered = 0;
        room->players[i].last_answer_correct = 0;
    }
    
    int room_id = room->id;
    GameItem *itemA = &game_database[room->current_index_A];
    GameItem *itemB = &game_database[room->current_index_B];
    
    // Build response
    char room_json[BUFFER_SIZE];
    build_room_json(room, room_json, sizeof(room_json));
    
    char response[RESPONSE_SIZE];
    snprintf(response, sizeof(response), 
        "{\"action\":\"game_started\",\"room\":%s,\"round\":%d,"
        "\"labelA\":\"%s\",\"valueA\":%d,\"labelB\":\"%s\"}",
        room_json, room->current_round,
        itemA->name, itemA->value, itemB->name);
    
    printf("[ROOM] 🎮 Game started in room ID: %d by host %d\n", room_id, session_id);
    printf("       Round 1: %s ($%d) vs %s (?)\n", itemA->name, itemA->value, itemB->name);
    
    pthread_mutex_unlock(&rooms_mutex);
    
    send_json_response(sock, response);
    broadcast_sse_to_room(room_id, response);
}

/**
 * POST /rooms/choice - Player chọn đáp án
 */
void handle_room_choice(int sock, int session_id, char *json_body) {
    if (session_id == 0) {
        send_json_response(sock, "{\"error\":\"No session ID\"}");
        return;
    }
    
    // Parse request
    int choice = parse_json_int(json_body, "choice");
    int response_time_ms = parse_json_int(json_body, "response_time");
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Find room and player
    int player_idx;
    int room_idx = -1;
    
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status == ROOM_PLAYING) {
            int p_idx = find_player_in_room(&rooms[i], session_id);
            if (p_idx >= 0) {
                room_idx = i;
                player_idx = p_idx;
                break;
            }
        }
    }
    
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"No active game found\"}");
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    RoomPlayer *player = &room->players[player_idx];
    
    // Validate player state
    if (player->has_answered) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"Already answered. Waiting for other players.\"}");
        return;
    }
    
    if (player->game_over) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"error\":\"Your game is over. Wait for others to finish.\"}");
        return;
    }
    
    // Get current items
    GameItem *itemA = &game_database[room->current_index_A];
    GameItem *itemB = &game_database[room->current_index_B];
    
    // Check answer: choice=1 means B>=A (higher), choice=2 means B<=A (lower)
    int correct = 0;
    if (choice == 1) {
        correct = (itemB->value >= itemA->value);
    } else if (choice == 2) {
        correct = (itemB->value <= itemA->value);
    }
    
    // Update player state
    player->has_answered = 1;
    player->last_answer_correct = correct;
    player->response_time_ms = response_time_ms;
    
    char message[256];
    if (correct) {
        player->score += SCORE_PER_CORRECT;
        player->streak++;
        snprintf(message, sizeof(message), "%s: $%d - Đúng rồi!", itemB->name, itemB->value);
    } else {
        player->streak = 0;
        snprintf(message, sizeof(message), "%s: $%d - Sai rồi!", itemB->name, itemB->value);
    }
    
    int room_id = room->id;
    int total_players = room->player_count;
    int answered_players = count_answered_players(room);
    
    // Send immediate response to this player
    char response[RESPONSE_SIZE];
    snprintf(response, sizeof(response),
        "{\"action\":\"choice_result\",\"correct\":%s,\"score\":%d,\"streak\":%d,"
        "\"message\":\"%s\",\"valueB\":%d,\"waiting_for\":%d,\"response_time\":%d}",
        correct ? "true" : "false",
        player->score, player->streak, message,
        itemB->value, total_players - answered_players, response_time_ms
    );
    send_json_response(sock, response);
    
    printf("[ROOM] 🎯 Player %d answered: %s (Score: %d, Time: %dms) - %d/%d answered\n", 
           session_id, correct ? "✅" : "❌", player->score, response_time_ms, 
           answered_players, total_players);
    
    // Check if all players answered
    if (answered_players >= total_players) {
        // Broadcast round results
        char results_json[RESPONSE_SIZE];
        build_round_results_json(room, room->current_round, itemB->value, itemB->name,
                                  results_json, sizeof(results_json));
        broadcast_sse_to_room(room_id, results_json);
        printf("[ROOM] 📊 Round %d results broadcasted\n", room->current_round);
        
        // Check if game finished
        if (room->max_rounds > 0 && room->current_round >= room->max_rounds) {
            room->status = ROOM_FINISHED;
            
            char room_json[BUFFER_SIZE];
            build_room_json(room, room_json, sizeof(room_json));
            
            char finish_json[RESPONSE_SIZE];
            snprintf(finish_json, sizeof(finish_json),
                "{\"action\":\"game_finished\",\"room\":%s}", room_json);
            
            pthread_mutex_unlock(&rooms_mutex);
            broadcast_sse_to_room(room_id, finish_json);
            
            printf("[ROOM] 🏆 Game finished in room ID: %d (reached %d rounds)\n", 
                   room_id, room->max_rounds);
            return;
        }
        
        // Move to next round
        room->current_round++;
        room->current_index_A = room->current_index_B;
        room->current_index_B = get_random_index_except(room->current_index_A);
        reset_round_state(room);
        
        // Get new items
        itemA = &game_database[room->current_index_A];
        itemB = &game_database[room->current_index_B];
        
        char room_json[BUFFER_SIZE];
        build_room_json(room, room_json, sizeof(room_json));
        
        char new_round_json[RESPONSE_SIZE];
        snprintf(new_round_json, sizeof(new_round_json),
            "{\"action\":\"new_round\",\"round\":%d,\"room\":%s,"
            "\"labelA\":\"%s\",\"valueA\":%d,\"labelB\":\"%s\"}",
            room->current_round, room_json,
            itemA->name, itemA->value, itemB->name);
        
        pthread_mutex_unlock(&rooms_mutex);
        broadcast_sse_to_room(room_id, new_round_json);
        
        printf("[ROOM] ➡️  Round %d/%d: %s ($%d) vs %s (?)\n", 
               room->current_round, room->max_rounds, 
               itemA->name, itemA->value, itemB->name);
    } else {
        pthread_mutex_unlock(&rooms_mutex);
    }
}

/**
 * GET /rooms/info - Lấy thông tin phòng hiện tại
 */
void handle_get_room_info(int sock, int session_id) {
    if (session_id == 0) {
        send_json_response(sock, "{\"error\":\"No session ID\"}");
        return;
    }
    
    pthread_mutex_lock(&rooms_mutex);
    
    // Find room with player
    int player_idx;
    int room_idx = find_room_with_player(session_id, &player_idx);
    
    if (room_idx == -1) {
        pthread_mutex_unlock(&rooms_mutex);
        send_json_response(sock, "{\"action\":\"room_info\",\"in_room\":false}");
        return;
    }
    
    GameRoom *room = &rooms[room_idx];
    RoomPlayer *player = &room->players[player_idx];
    
    GameItem *itemA = &game_database[room->current_index_A];
    GameItem *itemB = &game_database[room->current_index_B];
    
    char room_json[BUFFER_SIZE];
    build_room_json(room, room_json, sizeof(room_json));
    
    char response[RESPONSE_SIZE];
    snprintf(response, sizeof(response),
        "{\"action\":\"room_info\",\"in_room\":true,\"is_host\":%s,"
        "\"room\":%s,\"round\":%d,\"my_score\":%d,\"my_streak\":%d,"
        "\"my_game_over\":%s,\"has_answered\":%s,"
        "\"labelA\":\"%s\",\"valueA\":%d,\"labelB\":\"%s\"}",
        room->host_session_id == session_id ? "true" : "false",
        room_json, room->current_round,
        player->score, player->streak,
        player->game_over ? "true" : "false",
        player->has_answered ? "true" : "false",
        itemA->name, itemA->value, itemB->name
    );
    
    pthread_mutex_unlock(&rooms_mutex);
    send_json_response(sock, response);
}
