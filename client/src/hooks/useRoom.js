/**
 * useRoom Hook - Manage room state and actions
 */

import { useState, useCallback } from 'react'
import * as roomService from '../services/roomService'

/**
 * Initial room state
 */
const initialRoomState = {
  rooms: [],
  currentRoom: null,
  loading: false,
  error: null
}

/**
 * Custom hook for room management
 * @param {number} sessionId - Current session ID
 * @returns {Object} Room state and actions
 */
export const useRoom = (sessionId) => {
  const [state, setState] = useState(initialRoomState)

  // Set loading state
  const setLoading = (loading) => {
    setState(prev => ({ ...prev, loading, error: null }))
  }

  // Set error state
  const setError = (error) => {
    setState(prev => ({ ...prev, error, loading: false }))
  }

  // Fetch rooms list
  const fetchRooms = useCallback(async () => {
    try {
      const rooms = await roomService.getRooms()
      setState(prev => ({ ...prev, rooms }))
    } catch {
      // Silent fail for room list fetch
    }
  }, [])

  // Create room
  const createRoom = useCallback(async ({ roomName, playerName, maxPlayers, maxRounds }) => {
    if (!sessionId) {
      setError('Chưa kết nối đến server')
      return null
    }

    setLoading(true)
    try {
      const data = await roomService.createRoom({ roomName, playerName, maxPlayers, maxRounds })
      setState(prev => ({
        ...prev,
        currentRoom: data.room,
        loading: false
      }))
      return data
    } catch (error) {
      setError(error.response?.data?.error || 'Không thể tạo phòng')
      return null
    }
  }, [sessionId])

  // Join room
  const joinRoom = useCallback(async ({ roomId, playerName }) => {
    if (!sessionId) {
      setError('Chưa kết nối đến server')
      return null
    }

    setLoading(true)
    try {
      const data = await roomService.joinRoom({ roomId, playerName })
      setState(prev => ({
        ...prev,
        currentRoom: data.room,
        loading: false
      }))
      return data
    } catch (error) {
      setError(error.response?.data?.error || 'Không thể vào phòng')
      return null
    }
  }, [sessionId])

  // Leave room
  const leaveRoom = useCallback(async () => {
    if (!sessionId) return

    setLoading(true)
    try {
      await roomService.leaveRoom()
      setState(prev => ({
        ...prev,
        currentRoom: null,
        loading: false
      }))
      return true
    } catch (error) {
      setError(error.response?.data?.error || 'Không thể rời phòng')
      return false
    }
  }, [sessionId])

  // Start game
  const startGame = useCallback(async () => {
    if (!sessionId) {
      setError('Chưa kết nối đến server')
      return null
    }

    setLoading(true)
    try {
      const data = await roomService.startGame()
      setState(prev => ({
        ...prev,
        currentRoom: data.room,
        loading: false
      }))
      return data
    } catch (error) {
      setError(error.response?.data?.error || 'Không thể bắt đầu game')
      return null
    }
  }, [sessionId])

  // Update room from SSE
  const updateRoom = useCallback((room) => {
    setState(prev => ({
      ...prev,
      currentRoom: room
    }))
  }, [])

  return {
    // State
    rooms: state.rooms,
    currentRoom: state.currentRoom,
    loading: state.loading,
    error: state.error,
    
    // Actions
    fetchRooms,
    createRoom,
    joinRoom,
    leaveRoom,
    startGame,
    updateRoom
  }
}

export default useRoom
