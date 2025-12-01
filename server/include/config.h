/*
 * ============================================================================
 *                    HIGHER LOWER GAME - CONFIG
 * ============================================================================
 * File: config.h
 * Description: All configuration constants and macros
 * ============================================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

/* ============================================================================
 *                           SERVER CONFIG
 * ============================================================================ */
#define SERVER_PORT         8080
#define PORT                SERVER_PORT     // Alias for backward compatibility
#define MAX_CLIENTS         100
#define BUFFER_SIZE         8192
#define RESPONSE_SIZE       16384       // Larger buffer for JSON responses
#define BACKLOG             10          // Max pending connections

/* ============================================================================
 *                           ROOM CONFIG
 * ============================================================================ */
#define MAX_ROOMS           20
#define MAX_PLAYERS_PER_ROOM 50
#define ROOM_NAME_LEN       64
#define PLAYER_NAME_LEN     32

/* ============================================================================
 *                           GAME CONFIG
 * ============================================================================ */
#define MAX_ITEMS           100         // Max number of items in database
#define SCORE_PER_CORRECT   10          // Points for correct answer
#define ITEMS_FILE          "data/items.txt"  // Path to items data file

/* ============================================================================
 *                           STRING LIMITS
 * ============================================================================ */
#define ITEM_NAME_LEN       64
#define IMAGE_URL_LEN       256

#endif // CONFIG_H
