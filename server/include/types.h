/*
 * ============================================================================
 *                    HIGHER LOWER GAME - TYPES
 * ============================================================================
 * File: types.h
 * Description: All data structures and enumerations
 * ============================================================================
 */

#ifndef TYPES_H
#define TYPES_H

#include "config.h"

/* ============================================================================
 *                           ENUMERATIONS
 * ============================================================================ */

/**
 * Room Status - Các trạng thái của phòng chơi
 */
typedef enum {
    ROOM_EMPTY = 0,     // Phòng trống, chưa được tạo
    ROOM_WAITING,       // Đang chờ người chơi
    ROOM_PLAYING,       // Đang trong game
    ROOM_FINISHED       // Game đã kết thúc
} RoomStatus;

/* ============================================================================
 *                           GAME STRUCTURES
 * ============================================================================ */

/**
 * GameItem - Một item trong game (coin, stock, etc.)
 * 
 * Ví dụ: Bitcoin với giá $50000
 */
typedef struct {
    char name[ITEM_NAME_LEN];           // Tên item: "Bitcoin"
    int value;                          // Giá trị: 50000
    char image_url[IMAGE_URL_LEN];      // URL hình ảnh
} GameItem;

/* ============================================================================
 *                           PLAYER STRUCTURES
 * ============================================================================ */

/**
 * RoomPlayer - Thông tin người chơi trong một phòng
 * 
 * Chứa cả game state của người chơi đó
 */
typedef struct {
    int session_id;                     // ID session của người chơi
    char name[PLAYER_NAME_LEN];         // Tên hiển thị
    
    // Game state
    int score;                          // Điểm hiện tại
    int streak;                         // Chuỗi đúng liên tiếp
    
    // Status flags
    int is_ready;                       // Đã sẵn sàng chưa
    int game_over;                      // Đã thua chưa
    int has_answered;                   // Đã trả lời câu hiện tại chưa
    int last_answer_correct;            // Câu trả lời cuối có đúng không
    int response_time_ms;               // Thời gian trả lời (milliseconds)
} RoomPlayer;

/**
 * PlayerGameState - Game state cho chế độ chơi đơn (legacy)
 */
typedef struct {
    int active;                         // Có đang hoạt động không
    int session_id;                     // ID session
    int score;                          // Điểm
    int streak;                         // Chuỗi đúng
    int current_index_A;                // Index item A
    int current_index_B;                // Index item B
} PlayerGameState;

/* ============================================================================
 *                           ROOM STRUCTURES
 * ============================================================================ */

/**
 * GameRoom - Một phòng chơi
 * 
 * Chứa thông tin phòng và danh sách người chơi
 */
typedef struct {
    int id;                                     // ID phòng
    char name[ROOM_NAME_LEN];                   // Tên phòng
    int host_session_id;                        // Session ID của chủ phòng
    
    // Players
    RoomPlayer players[MAX_PLAYERS_PER_ROOM];   // Danh sách người chơi
    int player_count;                           // Số người chơi hiện tại
    int max_players;                            // Số người chơi tối đa
    
    // Game settings
    int max_rounds;                             // Số câu hỏi tối đa (0 = unlimited)
    
    // Current question (shared by all players)
    int current_index_A;                        // Index của item A
    int current_index_B;                        // Index của item B
    int current_round;                          // Vòng hiện tại
    
    // Status
    RoomStatus status;                          // Trạng thái phòng
} GameRoom;

/* ============================================================================
 *                           SSE CLIENT STRUCTURES
 * ============================================================================ */

/**
 * SSE_Client - Thông tin client kết nối SSE
 * 
 * Mỗi client có một SSE connection để nhận real-time updates
 */
typedef struct {
    int socket;                         // Socket descriptor
    int active;                         // Có đang hoạt động không
    int session_id;                     // ID session
    char player_name[PLAYER_NAME_LEN];  // Tên người chơi
    int room_id;                        // ID phòng đang ở (-1 nếu không ở phòng nào)
} SSE_Client;

#endif // TYPES_H
