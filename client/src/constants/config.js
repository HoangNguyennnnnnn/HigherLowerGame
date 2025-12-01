/**
 * Application Configuration
 */

// Server URL - Change this based on environment
export const SERVER_URL = 'http://172.20.127.157:8080'

// API Endpoints
export const ENDPOINTS = {
  // SSE
  SUBSCRIBE: '/subscribe',
  
  // Rooms
  ROOMS: '/rooms',
  ROOMS_CREATE: '/rooms/create',
  ROOMS_JOIN: '/rooms/join',
  ROOMS_LEAVE: '/rooms/leave',
  ROOMS_START: '/rooms/start',
  ROOMS_CHOICE: '/rooms/choice',
  ROOMS_INFO: '/rooms/info',
  
  // Game (legacy single player)
  GAME: '/game',
  GAME_CHOICE: '/game/choice'
}


