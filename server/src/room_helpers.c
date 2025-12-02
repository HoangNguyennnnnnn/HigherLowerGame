/*
 * ============================================================================
 *                    HIGHER LOWER GAME - ROOM HELPERS
 * ============================================================================
 * File: room_helpers.c
 * Description: Helper functions và JSON builders cho room system
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/game.h"
#include "../include/room_helpers.h"

/* ============================================================================
 *                           ROOM FINDER FUNCTIONS
 * ============================================================================ */

int find_room_index(int room_id) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].id == room_id && rooms[i].status != ROOM_EMPTY) {
            return i;
        }
    }
    return -1;
}

int find_player_in_room(GameRoom *room, int session_id) {
    for (int i = 0; i < room->player_count; i++) {
        if (room->players[i].session_id == session_id) {
            return i;
        }
    }
    return -1;
}

int find_room_with_player(int session_id, int *player_idx) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status != ROOM_EMPTY) {
            int p_idx = find_player_in_room(&rooms[i], session_id);
            if (p_idx >= 0) {
                if (player_idx) *player_idx = p_idx;
                return i;
            }
        }
    }
    return -1;
}

int find_empty_room_slot(void) {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i].status == ROOM_EMPTY) {
            return i;
        }
    }
    return -1;
}

int is_player_in_any_room(int session_id) {
    return find_room_with_player(session_id, NULL) >= 0;
}

/* ============================================================================
 *                           SSE & PLAYER HELPERS
 * ============================================================================ */

void update_sse_client_room(int session_id, int room_id, const char *player_name) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sse_clients[i].active && sse_clients[i].session_id == session_id) {
            sse_clients[i].room_id = room_id;
            if (player_name && strlen(player_name) > 0) {
                strncpy(sse_clients[i].player_name, player_name, PLAYER_NAME_LEN - 1);
            }
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void init_room_player(RoomPlayer *player, int session_id, const char *name, int is_host) {
    player->session_id = session_id;
    if (name && strlen(name) > 0) {
        strncpy(player->name, name, PLAYER_NAME_LEN - 1);
        player->name[PLAYER_NAME_LEN - 1] = '\0';
    } else {
        snprintf(player->name, PLAYER_NAME_LEN, "Player_%d", session_id);
    }
    player->score = 0;
    player->streak = 0;
    player->is_ready = is_host ? 1 : 0;
    player->game_over = 0;
    player->has_answered = 0;
    player->last_answer_correct = 0;
    player->response_time_ms = 0;
}

int count_answered_players(GameRoom *room) {
    int count = 0;
    for (int i = 0; i < room->player_count; i++) {
        if (room->players[i].has_answered) {
            count++;
        }
    }
    return count;
}

void reset_round_state(GameRoom *room) {
    for (int i = 0; i < room->player_count; i++) {
        room->players[i].has_answered = 0;
    }
}

/* ============================================================================
 *                           JSON PARSE HELPERS
 * ============================================================================ */

void parse_json_string(const char *json, const char *key, char *out, size_t out_size) {
    char search_key[64];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    
    char *ptr = strstr(json, search_key);
    if (!ptr) return;
    
    char *value_start = strchr(ptr, ':');
    if (!value_start) return;
    
    value_start++;
    while (*value_start == ' ' || *value_start == '"') value_start++;
    
    char *value_end = strchr(value_start, '"');
    if (!value_end) return;
    
    int len = value_end - value_start;
    if (len > 0 && len < (int)out_size) {
        strncpy(out, value_start, len);
        out[len] = '\0';
    }
}

int parse_json_int(const char *json, const char *key) {
    char search_key[64];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    
    char *ptr = strstr(json, search_key);
    if (!ptr) return 0;
    
    char *value_start = strchr(ptr, ':');
    if (!value_start) return 0;
    
    return atoi(value_start + 1);
}

/* ============================================================================
 *                           JSON BUILDERS
 * ============================================================================ */

void build_players_json(GameRoom *room, char *json, size_t json_size) {
    strcpy(json, "[");
    
    for (int i = 0; i < room->player_count; i++) {
        RoomPlayer *p = &room->players[i];
        char entry[512];
        snprintf(entry, sizeof(entry),
            "%s{\"session_id\":%d,\"name\":\"%s\",\"score\":%d,\"streak\":%d,"
            "\"is_ready\":%d,\"game_over\":%d,\"has_answered\":%d,\"is_host\":%s}",
            i > 0 ? "," : "",
            p->session_id, p->name, p->score, p->streak,
            p->is_ready, p->game_over, p->has_answered,
            p->session_id == room->host_session_id ? "true" : "false"
        );
        
        if (strlen(json) + strlen(entry) < json_size - 2) {
            strcat(json, entry);
        }
    }
    strcat(json, "]");
}

void build_room_json(GameRoom *room, char *json, size_t json_size) {
    char players_json[4096];
    build_players_json(room, players_json, sizeof(players_json));
    
    const char *status_str;
    switch (room->status) {
        case ROOM_WAITING:  status_str = "waiting"; break;
        case ROOM_PLAYING:  status_str = "playing"; break;
        case ROOM_FINISHED: status_str = "finished"; break;
        default:            status_str = "empty"; break;
    }
    
    snprintf(json, json_size,
        "{\"id\":%d,\"name\":\"%s\",\"host_session_id\":%d,\"player_count\":%d,"
        "\"max_players\":%d,\"max_rounds\":%d,\"status\":\"%s\",\"current_round\":%d,\"players\":%s}",
        room->id, room->name, room->host_session_id, room->player_count,
        room->max_players, room->max_rounds, status_str, room->current_round, players_json
    );
}

void build_round_results_json(GameRoom *room, int round, int valueB, const char *labelB, 
                               char *json, size_t json_size) {
    char results[2048] = "[";
    
    for (int i = 0; i < room->player_count; i++) {
        RoomPlayer *p = &room->players[i];
        char entry[256];
        snprintf(entry, sizeof(entry),
            "%s{\"session_id\":%d,\"name\":\"%s\",\"correct\":%s,"
            "\"score\":%d,\"streak\":%d,\"response_time\":%d}",
            i > 0 ? "," : "",
            p->session_id, p->name,
            p->last_answer_correct ? "true" : "false",
            p->score, p->streak, p->response_time_ms
        );
        strcat(results, entry);
    }
    strcat(results, "]");
    
    snprintf(json, json_size,
        "{\"action\":\"round_results\",\"round\":%d,\"valueB\":%d,\"labelB\":\"%s\",\"results\":%s}",
        round, valueB, labelB, results
    );
}
