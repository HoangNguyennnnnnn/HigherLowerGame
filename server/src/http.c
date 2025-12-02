/*
 * ============================================================================
 *                    HIGHER LOWER GAME - HTTP MODULE
 * ============================================================================
 * File: http.c
 * Description: HTTP response utilities
 * 
 * Chức năng:
 *   1. Gửi CORS headers
 *   2. Gửi JSON responses
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/game.h"

/* ============================================================================
 *                           HTTP RESPONSE FUNCTIONS
 * ============================================================================ */

/**
 * Gửi CORS headers
 * 
 * @param sock Socket để gửi
 */
void send_cors_headers(int sock) {
    char headers[512];
    snprintf(headers, sizeof(headers),
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, X-Session-ID\r\n"
    );
    write(sock, headers, strlen(headers));
}

/**
 * Gửi JSON response với đầy đủ HTTP headers
 * 
 * Response format:
 *   HTTP/1.1 200 OK
 *   Content-Type: application/json
 *   Content-Length: {length}
 *   Access-Control-Allow-Origin: *
 *   Connection: close
 *   
 *   {json_body}
 * 
 * @param sock Socket để gửi
 * @param body JSON string để gửi
 */
void send_json_response(int sock, char *body) {
    char response[RESPONSE_SIZE];
    int body_len = strlen(body);
    
    int header_len = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        body_len, body
    );
    
    write(sock, response, header_len);
}
