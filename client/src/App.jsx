import { useState, useEffect } from 'react'
import axios from 'axios'
import './App.css'
import logo from './assets/logo.png' 

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
  }, []) // useEffect KHÔNG return JSX

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

  // ------------------------------------------------------------------
  // RENDER LOGIC: Hiện Start Screen hay Game Screen?
  // ------------------------------------------------------------------

  const renderGameContent = () => {
    // Nếu chưa có labelA (hoặc là màn hình khởi động) -> HIỂN THỊ START SCREEN
    if (!gameState.labelA) {
      return (
        // Áp dụng class background và style căn giữa
        <div className="app start-screen-bg"> 
          <div className="start-content">
            {/* DÙNG LOGO ĐÃ IMPORT */}
            <img src={logo} alt="Higher Lower Game Logo" className="game-logo" />
            
            <p className="rule-explanation">
              Bạn nghĩ chủ đề nào trong hai chủ đề sau đây có lượt tìm kiếm hàng tháng "CAO HƠN"?
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

    // Nếu đã có labelA và labelB -> HIỂN THỊ GAME SCREEN
    return (
      <div className="app">
        {/* Header/Status Bar */}
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
          {/* Item A */}
          <div className="item" onClick={() => !loading && makeChoice(1)}>
            <img src={gameState.imageA} alt={gameState.labelA} />
            <h2>{gameState.labelA}</h2>
            <p className="value">${gameState.valueA.toLocaleString()}</p>
            <button disabled={loading}>
              Chọn A Lớn Hơn
            </button>
          </div>

          <div className="vs">VS</div>

          {/* Item B */}
          <div className="item" onClick={() => !loading && makeChoice(2)}>
            <img src={gameState.imageB} alt={gameState.labelB} />
            <h2>{gameState.labelB}</h2>
            <p className="value">CAO HƠN hay THẤP HƠN?</p> {/* Sửa lại để ẩn giá trị B */}
            <button disabled={loading}>
              Chọn B Lớn Hơn
            </button>
          </div>
        </div>

        {gameState.labelA && (
          <button className="new-game-btn" onClick={startGame} disabled={loading}>
            New Game
          </button>
        )}
      </div>
    )
  }

  // 4. MAIN RETURN CALL
  return renderGameContent();
}

export default App
