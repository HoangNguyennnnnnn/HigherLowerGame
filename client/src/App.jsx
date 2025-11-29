import { useState, useEffect } from 'react'
import axios from 'axios'
import './App.css'

const SERVER_URL = 'http://172.20.127.157:8080'

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
          
          // Store session ID from initial connection
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
        console.log('ReadyState:', eventSource.readyState)
        
        if (mounted) {
          // 0 = connecting, 1 = open, 2 = closed
          if (eventSource.readyState === EventSource.CLOSED) {
            console.log('SSE Connection closed')
            setConnected(false)
          } else if (eventSource.readyState === EventSource.CONNECTING) {
            console.log('SSE Reconnecting...')
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

  // Start new game
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

  // Send player choice
  const makeChoice = async (choice) => {
    if (!sessionId) {
      alert('Not connected to server yet. Please wait...')
      return
    }
    
    setLoading(true)
    try {
      const response = await axios.post(`${SERVER_URL}/game/choice`, 
        { choice },
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

  return (
    <div className="app">
      <div className="header">
        <h1>🎮 Higher Lower Game</h1>
        <div className="status">
          <span className={connected ? 'connected' : 'disconnected'}>
            {connected ? '🟢 SSE Connected' : '🔴 SSE Disconnected'}
          </span>
        </div>
      </div>

      <div className="stats">
        <div className="stat">
          <span className="stat-label">Score:</span>
          <span className="stat-value">{gameState.score}</span>
        </div>
        <div className="stat">
          <span className="stat-label">Streak:</span>
          <span className="stat-value">{gameState.streak}</span>
        </div>
      </div>

      <div className="message">{gameState.message}</div>

      <div className="game-area">
        {gameState.labelA && gameState.labelB ? (
          <>
            <div className="item" onClick={() => !loading && makeChoice(1)}>
              <img src={gameState.imageA} alt={gameState.labelA} />
              <h2>{gameState.labelA}</h2>
              <p className="value">${gameState.valueA.toLocaleString()}</p>
              <button disabled={loading}>
                Choose Higher
              </button>
            </div>

            <div className="vs">VS</div>

            <div className="item" onClick={() => !loading && makeChoice(2)}>
              <img src={gameState.imageB} alt={gameState.labelB} />
              <h2>{gameState.labelB}</h2>
              <p className="value">${gameState.valueB.toLocaleString()}</p>
              <button disabled={loading}>
                Choose Higher
              </button>
            </div>
          </>
        ) : (
          <button className="start-btn" onClick={startGame} disabled={loading}>
            {loading ? 'Loading...' : 'Start Game'}
          </button>
        )}
      </div>

      {gameState.labelA && (
        <button className="new-game-btn" onClick={startGame} disabled={loading}>
          New Game
        </button>
      )}
    </div>
  )
}

export default App
