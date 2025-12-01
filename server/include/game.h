/*
 * ============================================================================
 *                    HIGHER LOWER GAME - MAIN INCLUDE
 * ============================================================================
 * File: game.h
 * Description: Master header file - includes all other headers
 *              Giữ backward compatibility với code cũ
 * ============================================================================
 */

#ifndef GAME_H
#define GAME_H

/* ============================================================================
 *                           MODULE HEADERS
 * ============================================================================ */

// Cấu hình và constants
#include "config.h"

// Data types và structures
#include "types.h"

// HTTP utilities
#include "http.h"

// SSE (Server-Sent Events)
#include "sse.h"

// Server core
#include "server.h"

// Room/Lobby system
#include "room.h"

// Legacy single player game
#include "game_logic.h"

#endif // GAME_H
