/**
 * Game Service - API calls for game actions
 */

import api from './api'
import { ENDPOINTS } from '../constants'

/**
 * Make a choice in the game
 * @param {number} choice - Choice (1 for Higher, 2 for Lower)
 * @param {number} responseTime - Time taken to answer (ms)
 * @returns {Promise<Object>} Game state after choice
 */
export const makeChoice = async (choice, responseTime = 0) => {
  const response = await api.post(ENDPOINTS.ROOMS_CHOICE, { 
    choice,
    response_time: responseTime
  })
  return response.data
}

/**
 * Get room info with game state
 * @returns {Promise<Object>} Room and game info
 */
export const getGameState = async () => {
  const response = await api.get(ENDPOINTS.ROOMS_INFO)
  return response.data
}

export default {
  makeChoice,
  getGameState
}
