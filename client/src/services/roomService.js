/**
 * Room Service - API calls for room management
 */

import api from './api'
import { ENDPOINTS } from '../constants'

/**
 * Get list of all rooms
 * @returns {Promise<Array>} List of rooms
 */
export const getRooms = async () => {
  const response = await api.get(ENDPOINTS.ROOMS)
  return response.data.rooms || []
}

/**
 * Create a new room
 * @param {Object} params
 * @param {string} params.roomName - Room name
 * @param {string} params.playerName - Player name
 * @param {number} params.maxPlayers - Max players (2-10)
 * @param {number} params.maxRounds - Max rounds (5-50)
 * @returns {Promise<Object>} Created room data
 */
export const createRoom = async ({ roomName, playerName, maxPlayers, maxRounds }) => {
  const response = await api.post(ENDPOINTS.ROOMS_CREATE, {
    room_name: roomName,
    player_name: playerName,
    max_players: maxPlayers,
    max_rounds: maxRounds || 10
  })
  return response.data
}

/**
 * Join an existing room
 * @param {Object} params
 * @param {number} params.roomId - Room ID to join
 * @param {string} params.playerName - Player name
 * @returns {Promise<Object>} Joined room data
 */
export const joinRoom = async ({ roomId, playerName }) => {
  const response = await api.post(ENDPOINTS.ROOMS_JOIN, {
    room_id: roomId,
    player_name: playerName
  })
  return response.data
}

/**
 * Leave current room
 * @returns {Promise<Object>} Response data
 */
export const leaveRoom = async () => {
  const response = await api.post(ENDPOINTS.ROOMS_LEAVE, {})
  return response.data
}

/**
 * Start game (host only)
 * @returns {Promise<Object>} Game start data
 */
export const startGame = async () => {
  const response = await api.post(ENDPOINTS.ROOMS_START, {})
  return response.data
}

/**
 * Get current room info
 * @returns {Promise<Object>} Room info
 */
export const getRoomInfo = async () => {
  const response = await api.get(ENDPOINTS.ROOMS_INFO)
  return response.data
}

export default {
  getRooms,
  createRoom,
  joinRoom,
  leaveRoom,
  startGame,
  getRoomInfo
}
