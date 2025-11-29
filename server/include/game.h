#ifndef GAME_H
#define GAME_H

#include <pthread.h>

// Constants
#define MAX_CLIENTS 100
#define BUFFER_SIZE 8192
#define PORT 8080
#define MAX_ITEMS 5

// SSE Client structure with session
typedef struct {
    int socket;
    int active;
    int session_id;
} SSE_Client;

// Game Item structure
typedef struct {
    char name[64];
    int value;
    char image_url[256];
} GameItem;

// Player game state
typedef struct {
    int active;
    int session_id;
    int score;
    int streak;
    int current_index_A;
    int current_index_B;
} PlayerGameState;

// Global variables for SSE clients
extern SSE_Client sse_clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;

// Global variables for player game states
extern PlayerGameState player_states[MAX_CLIENTS];
extern pthread_mutex_t game_state_mutex;
extern int next_session_id;

// Mock database
extern GameItem game_database[MAX_ITEMS];

// Function prototypes - HTTP utilities
void send_cors_headers(int sock);
void send_json_response(int sock, char *body);
void broadcast_sse_to_session(int session_id, char *json_data);

// Function prototypes - Game logic
void init_game_database();
void handle_game_init(int sock, int session_id);
void handle_player_choice(int sock, int session_id, char *json_body);
int get_random_index_except(int except);
int get_session_from_request(char *request);

// Function prototypes - Main server
void *handle_client(void *arg);
void handle_sse_subscribe(int client_sock);

#endif // GAME_H
