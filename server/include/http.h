/*
 * ============================================================================
 *                    HIGHER LOWER GAME - HTTP UTILITIES
 * ============================================================================
 * File: http.h
 * Description: HTTP response helpers
 * ============================================================================
 */

#ifndef HTTP_H
#define HTTP_H

/* ============================================================================
 *                           HTTP RESPONSE FUNCTIONS
 * ============================================================================ */

/**
 * Gửi CORS headers
 * 
 * Headers:
 *   Access-Control-Allow-Origin: *
 *   Access-Control-Allow-Methods: GET, POST, OPTIONS
 *   Access-Control-Allow-Headers: Content-Type
 * 
 * @param sock Socket để gửi
 */
void send_cors_headers(int sock);

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
void send_json_response(int sock, char *body);

#endif // HTTP_H
