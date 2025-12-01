import { useState, useEffect } from 'react'
import axios from 'axios'
import './App.css'
import logo from './assets/logo.png'
import GameScreen from './components/GameScreen'

const SERVER_URL = 'http://172.24.18.63:8080'

function App() {
  const [gameState, setGameState] = useState({
    score: 0,
    streak: 0,
    labelA: '',
    valueA: 0,
    imageA: '',
    labelB: '',
    valueB: 0,
    imageB: '',
    message: 'Click "Start Game" to begin'
  })
  const [connected, setConnected] = useState(false)
  const [loading, setLoading] = useState(false)
  const [sessionId, setSessionId] = useState(null)

  // SSE Connection
  useEffect(() => {
    let eventSource = null
    let mounted = true
    let currentSessionId = null

    const setupSSE = () => {
      console.log('Setting up SSE connection to:', `${SERVER_URL}/subscribe`)
      eventSource = new EventSource(`${SERVER_URL}/subscribe`)

      eventSource.onopen = () => {
        console.log('✅ SSE Connected successfully')
        if (mounted) {
          setConnected(true)
        }
      }

      eventSource.onmessage = (event) => {
        console.log('📩 SSE Received:', event.data)
        if (!mounted) return
        
        try {
          const data = JSON.parse(event.data)
          
          if (data.session_id && !currentSessionId) {
            currentSessionId = data.session_id
            console.log('🔑 Session ID:', currentSessionId)
            setSessionId(currentSessionId)
          }
          
          if (data.action === 'update_game') {
            console.log('🎮 Game state updated:', data)
            setGameState(data)
          }
        } catch (error) {
          console.log('ℹ️ SSE message (not JSON):', event.data)
        }
      }

      eventSource.onerror = (error) => {
        console.error('❌ SSE Error:', error)
        if (mounted) {
          if (eventSource.readyState === EventSource.CLOSED) {
            setConnected(false)
          }
        }
      }
    }

    setupSSE()

    return () => {
      console.log('Cleanup: Closing SSE connection')
      mounted = false
      if (eventSource) {
        eventSource.close()
      }
    }
  }, [])

  // Start new game (Gửi POST /game)
  const startGame = async () => {
    if (!sessionId) {
      alert('Not connected to server yet. Please wait...')
      return
    }
    
    setLoading(true)
    try {
      const response = await axios.post(`${SERVER_URL}/game`, {}, {
        headers: {
          'X-Session-ID': sessionId
        }
      })
      setGameState(response.data)
    } catch (error) {
      console.error('Error starting game:', error)
      alert('Failed to start game. Is the server running?')
    }
    setLoading(false)
  }

  // Send player choice (Gửi POST /game/choice)
  const makeChoice = async (choice) => {
    if (!sessionId) {
      alert('Not connected to server yet. Please wait...')
      return
    }
    
    setLoading(true)
    try {
      const response = await axios.post(`${SERVER_URL}/game/choice`, 
        { choice }, // Gửi 1 (Chọn A) hoặc 2 (Chọn B)
        {
          headers: {
            'X-Session-ID': sessionId
          }
        }
      )
      setGameState(response.data)
    } catch (error) {
      console.error('Error making choice:', error)
      alert('Failed to submit choice')
    }
    setLoading(false)
  }

  // ------------------------------------------------------------------
  // RENDER LOGIC: Hiện Start Screen hay Game Screen?
  // ------------------------------------------------------------------

  const renderGameContent = () => {
    // 1. HIỂN THỊ START SCREEN
    if (!gameState.labelA) {
      return (
        <div className="start-screen-bg"> 
          <div className="start-content">
            <img src={logo} alt="Higher Lower Game Logo" className="game-logo" />
            
            <p className="rule-explanation">
              Which of the two topics below do you think has a higher monthly search volume?
            </p>
            
            <button 
              className="start-button" 
              onClick={startGame}
              disabled={!connected || loading}
            >
              {loading ? 'Đang tải...' : 'CHƠI NGAY'}
            </button>
            
            <div className="connection-status">
              Trạng thái: {connected ? '🟢 Đã kết nối' : '🔴 Chờ kết nối...'}
            </div>
          </div>
        </div>
      )
    }

    // 2. HIỂN THỊ GAME SCREEN (Sử dụng Component mới)
    return (
      <GameScreen 
        gameState={gameState} 
        connected={connected} 
        loading={loading} 
        makeChoice={makeChoice} 
        startGame={startGame}
      />
    )
  }

  return renderGameContent();
}

export default App
