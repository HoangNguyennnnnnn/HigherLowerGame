/**
 * useGame Hook - Manage game state and actions
 */

import { useState, useCallback } from 'react'
import * as gameService from '../services/gameService'

/**
 * Initial game state
 */
const initialGameState = {
  score: 0,
  streak: 0,
  labelA: '',
  valueA: 0,
  labelB: '',
  valueB: 0,
  message: '',
  gameOver: false,
  hasAnswered: false,
  round: 0,
  waitingFor: 0,
  loading: false,
  roundResults: null  // Kết quả của tất cả players sau khi round kết thúc
}

/**
 * Custom hook for game management
 * @param {number} sessionId - Current session ID
 * @returns {Object} Game state and actions
 */
export const useGame = (sessionId) => {
  const [gameState, setGameState] = useState(initialGameState)

  // Make a choice with response time
  const makeChoice = useCallback(async (choice, responseTime = 0) => {
    if (!sessionId || gameState.gameOver || gameState.hasAnswered) return null

    setGameState(prev => ({ ...prev, loading: true }))

    try {
      const data = await gameService.makeChoice(choice, responseTime)
      
      setGameState(prev => ({
        ...prev,
        score: data.score,
        streak: data.streak,
        gameOver: data.game_over,
        hasAnswered: true,
        waitingFor: data.waiting_for || 0,
        valueB: data.valueB || prev.valueB,
        message: data.message || '',
        loading: false
      }))
      
      return data
    } catch {
      setGameState(prev => ({ ...prev, loading: false }))
      return null
    }
  }, [sessionId, gameState.gameOver, gameState.hasAnswered])

  // Fetch game state from room info
  const fetchGameState = useCallback(async () => {
    if (!sessionId) return

    try {
      const data = await gameService.getGameState()
      
      if (data.in_room) {
        setGameState(prev => ({
          ...prev,
          score: data.my_score,
          streak: data.my_streak,
          labelA: data.labelA,
          valueA: data.valueA,
          labelB: data.labelB,
          gameOver: data.my_game_over,
          hasAnswered: data.has_answered,
          round: data.round,
          message: ''
        }))
      }
    } catch {
      // Silent fail
    }
  }, [sessionId])

  // Update game data from SSE
  const updateGameData = useCallback((data) => {
    setGameState(prev => ({
      ...prev,
      ...data
    }))
  }, [])

  // Reset game state
  const resetGame = useCallback(() => {
    setGameState(initialGameState)
  }, [])

  return {
    // State
    ...gameState,
    
    // Actions
    makeChoice,
    fetchGameState,
    updateGameData,
    resetGame
  }
}

export default useGame
