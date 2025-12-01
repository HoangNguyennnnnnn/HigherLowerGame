/**
 * App Component - Main application entry point
 */

import { useState, useEffect, useCallback, useRef } from 'react'
import './App.css'

// Constants
import { SCREENS, SERVER_URL, ENDPOINTS } from './constants'

// Hooks
import { useRoom, useGame } from './hooks'

// Components
import { NameInput, Lobby, WaitingRoom, GameScreen, GameOver } from './components'

function App() {
  // Player state
  const [playerName, setPlayerName] = useState('')
  const [isNameSet, setIsNameSet] = useState(false)
  
  // Screen state
  const [currentScreen, setCurrentScreen] = useState(SCREENS.LOBBY)
  
  // SSE state
  const [connected, setConnected] = useState(false)
  const [sessionId, setSessionId] = useState(null)

  // Custom hooks
  const room = useRoom(sessionId)
  const game = useGame(sessionId)
  
  // Ref to access latest room.updateRoom
  const roomRef = useRef(room)
  useEffect(() => {
    roomRef.current = room
  }, [room])

  // SSE Connection
  useEffect(() => {
    let eventSource = null
    let mounted = true

    const setupSSE = () => {
      const sseUrl = `${SERVER_URL}${ENDPOINTS.SUBSCRIBE}`
      eventSource = new EventSource(sseUrl)

      eventSource.onopen = () => {
        if (mounted) {
          setConnected(true)
        }
      }

      eventSource.onmessage = (event) => {
        if (!mounted) return

        try {
          const data = JSON.parse(event.data)

          // Handle session ID
          if (data.session_id) {
            setSessionId(data.session_id)
            localStorage.setItem('sessionId', data.session_id)
          }

          // Handle room events
          if (data.action === 'player_joined' || data.action === 'player_left') {
            roomRef.current.updateRoom(data.room)
          }

          // Handle game start
          if (data.action === 'game_started') {
            roomRef.current.updateRoom(data.room)
            // Update game state with initial question and reset state
            if (data.labelA) {
              game.updateGameData({
                labelA: data.labelA,
                valueA: data.valueA,
                labelB: data.labelB,
                round: data.round,
                hasAnswered: false,
                gameOver: false,
                score: 0,
                streak: 0,
                message: ''
              })
            }
            setCurrentScreen(SCREENS.PLAYING)
          }

          // Handle player answered - KHÔNG CÒN dùng nữa (server không gửi)
          if (data.action === 'player_answered') {
            // Không làm gì - mỗi player chỉ biết kết quả của mình
          }

          // Handle round results (khi TẤT CẢ đã trả lời)
          if (data.action === 'round_results') {
            game.updateGameData({
              roundResults: data.results
            })
          }

          // Handle new round
          if (data.action === 'new_round') {
            roomRef.current.updateRoom(data.room)
            game.updateGameData({
              labelA: data.labelA,
              valueA: data.valueA,
              labelB: data.labelB,
              round: data.round,
              hasAnswered: false,
              message: '',
              roundResults: null  // Reset round results
            })
          }

          // Handle player update during game
          if (data.action === 'player_update') {
            roomRef.current.updateRoom(data.room)
          }

          // Handle game finished
          if (data.action === 'game_finished') {
            roomRef.current.updateRoom(data.room)
            setCurrentScreen(SCREENS.GAME_OVER)
          }
        } catch {
          // Non-JSON message, ignore
        }
      }

      eventSource.onerror = () => {
        if (mounted && eventSource.readyState === EventSource.CLOSED) {
          setConnected(false)
        }
      }
    }

    setupSSE()

    return () => {
      mounted = false
      if (eventSource) {
        eventSource.close()
      }
    }
  }, [])

  // Fetch rooms only once when entering lobby
  useEffect(() => {
    if (connected && currentScreen === SCREENS.LOBBY) {
      room.fetchRooms()
    }
  }, [connected, currentScreen])

  // Fetch game state when entering game
  useEffect(() => {
    if (currentScreen === SCREENS.PLAYING && sessionId) {
      game.fetchGameState()
    }
  }, [currentScreen, sessionId, game.fetchGameState])

  // ==================== HANDLERS ====================

  // Set player name
  const handleSetName = () => {
    if (playerName.trim()) {
      setIsNameSet(true)
    }
  }

  // Create room
  const handleCreateRoom = async ({ roomName, maxRounds }) => {
    if (!playerName.trim()) {
      alert('Vui lòng nhập tên trước!')
      return false
    }

    const result = await room.createRoom({
      roomName,
      playerName,
      maxRounds
    })

    if (result) {
      setCurrentScreen(SCREENS.WAITING_ROOM)
      return true
    } else if (room.error) {
      alert(room.error)
    }
    return false
  }

  // Join room
  const handleJoinRoom = async (roomId) => {
    if (!playerName.trim()) {
      alert('Vui lòng nhập tên trước!')
      return
    }

    const result = await room.joinRoom({
      roomId,
      playerName
    })

    if (result) {
      setCurrentScreen(SCREENS.WAITING_ROOM)
    } else if (room.error) {
      alert(room.error)
    }
  }

  // Leave room
  const handleLeaveRoom = async () => {
    const success = await room.leaveRoom()
    if (success) {
      setCurrentScreen(SCREENS.LOBBY)
      room.fetchRooms()
    }
  }

  // Start game
  const handleStartGame = async () => {
    const result = await room.startGame()
    if (result) {
      setCurrentScreen(SCREENS.PLAYING)
    } else if (room.error) {
      alert(room.error)
    }
  }

  // Make choice with response time
  const handleChoice = async (choice, responseTime = 0) => {
    await game.makeChoice(choice, responseTime)
  }

  // Return to lobby
  const handleReturnToLobby = async () => {
    await handleLeaveRoom()
    game.resetGame()
  }

  // ==================== RENDER ====================

  if (!isNameSet) {
    return (
      <NameInput
        playerName={playerName}
        setPlayerName={setPlayerName}
        onSubmit={handleSetName}
        connected={connected}
      />
    )
  }

  switch (currentScreen) {
    case SCREENS.LOBBY:
      return (
        <Lobby
          playerName={playerName}
          rooms={room.rooms}
          onCreateRoom={handleCreateRoom}
          onJoinRoom={handleJoinRoom}
          onRefresh={room.fetchRooms}
          loading={room.loading}
          connected={connected}
        />
      )

    case SCREENS.WAITING_ROOM:
      return (
        <WaitingRoom
          room={room.currentRoom}
          sessionId={sessionId}
          onStartGame={handleStartGame}
          onLeaveRoom={handleLeaveRoom}
          loading={room.loading}
        />
      )

    case SCREENS.PLAYING:
      return (
        <GameScreen
          room={room.currentRoom}
          sessionId={sessionId}
          gameState={game}
          onChoice={handleChoice}
          loading={game.loading}
        />
      )

    case SCREENS.GAME_OVER:
      return (
        <GameOver
          room={room.currentRoom}
          sessionId={sessionId}
          onReturnToLobby={handleReturnToLobby}
        />
      )

    default:
      return (
        <Lobby
          playerName={playerName}
          rooms={room.rooms}
          onCreateRoom={handleCreateRoom}
          onJoinRoom={handleJoinRoom}
          onRefresh={room.fetchRooms}
          loading={room.loading}
          connected={connected}
        />
      )
  }
}

export default App
