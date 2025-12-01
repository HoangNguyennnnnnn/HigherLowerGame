/**
 * GameScreen Component - Split screen Higher Lower Game with Timer
 */

import { useState, useEffect, useRef } from 'react'
import PropTypes from 'prop-types'

const ROUND_TIME_SECONDS = 15 // 15 giây mỗi câu hỏi

function GameScreen({ 
  room, 
  sessionId, 
  gameState, 
  onChoice, 
  loading 
}) {
  // Timer state
  const [timeLeft, setTimeLeft] = useState(ROUND_TIME_SECONDS)
  const roundStartTime = useRef(Date.now())
  const timerRef = useRef(null)
  
  // Use local game state (from makeChoice response), NOT from room SSE data
  const { score, labelA, valueA, labelB, message, hasAnswered, round, roundResults } = gameState
  const maxRounds = room?.max_rounds || 10

  // Reset timer when new round starts
  useEffect(() => {
    // Reset timer
    setTimeLeft(ROUND_TIME_SECONDS)
    roundStartTime.current = Date.now()
    
    // Clear old timer
    if (timerRef.current) {
      clearInterval(timerRef.current)
    }
    
    // Start countdown
    timerRef.current = setInterval(() => {
      setTimeLeft(prev => {
        if (prev <= 1) {
          clearInterval(timerRef.current)
          return 0
        }
        return prev - 1
      })
    }, 1000)
    
    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current)
      }
    }
  }, [round, labelB]) // Reset when round or question changes
  
  // Stop timer when answered
  useEffect(() => {
    if (hasAnswered && timerRef.current) {
      clearInterval(timerRef.current)
    }
  }, [hasAnswered])
  
  // Handle choice with response time
  const handleChoice = (choice) => {
    const responseTime = Date.now() - roundStartTime.current
    onChoice(choice, responseTime)
  }
  
  // Auto answer when time runs out (optional - could be wrong answer)
  useEffect(() => {
    if (timeLeft === 0 && !hasAnswered && !loading) {
      // Time's up - auto submit as "higher" (or could be treated as wrong)
      const responseTime = ROUND_TIME_SECONDS * 1000
      onChoice(1, responseTime) // Default to higher
    }
  }, [timeLeft, hasAnswered, loading, onChoice])

  return (
    <div className="game-split-container">
      {/* Header */}
      <div className="game-header-bar">
        <span className="room-name">{room?.name}</span>
        <span className="round-info">Vòng {round}/{maxRounds}</span>
        <span className="score-info">Điểm: {score}</span>
        <span className={`timer ${timeLeft <= 5 ? 'timer-warning' : ''}`}>
          ⏱️ {timeLeft}s
        </span>
      </div>

      <div className="game-split-screen">
        {/* Left side - Item A (Known) */}
        <div className="split-panel panel-a"
          style={{ 
              backgroundImage: `url(${gameState.labelA})`, 
              backgroundSize: 'cover',
              backgroundPosition: 'center'
          }}
        >
          <div className="panel-content">
            <h2 className="item-name">{labelA}</h2>
            <p className="item-value">${valueA?.toLocaleString()}</p>
          </div>
        </div>

        {/* Right side - Item B (Unknown) */}
        <div className="split-panel panel-b"
          style={{ 
              backgroundImage: `url(${gameState.imageB})`, 
              backgroundSize: 'cover',
              backgroundPosition: 'center'
          }}
        >
          <div className="panel-content">
            <h2 className="item-name">{labelB}</h2>
            
            {hasAnswered ? (
              <div className="answered-state">
                <p className="answer-message">{message}</p>
                {!roundResults ? (
                  <p className="waiting-text">
                    ⏳ Đang chờ người chơi khác...
                  </p>
                ) : (
                  <div className="round-results">
                    <h4>Kết quả vòng {round}:</h4>
                    {roundResults.map((r, idx) => (
                      <div key={idx} className={`result-row ${r.correct ? 'correct' : 'wrong'}`}>
                        <span className="result-name">{r.name}</span>
                        <span className="result-icon">{r.correct ? '✅' : '❌'}</span>
                        <span className="result-time">{(r.response_time / 1000).toFixed(1)}s</span>
                        <span className="result-score">{r.score}đ</span>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            ) : (
              <div className="choice-area">
                <p className="question-text">Giá cao hơn hay thấp hơn?</p>
                <div className="choice-buttons-vertical">
                  <button 
                    onClick={() => handleChoice(1)} 
                    disabled={loading || timeLeft === 0}
                    className="btn-choice btn-higher"
                  >
                    ⬆️ CAO HƠN
                  </button>
                  <button 
                    onClick={() => handleChoice(2)} 
                    disabled={loading || timeLeft === 0}
                    className="btn-choice btn-lower"
                  >
                    ⬇️ THẤP HƠN
                  </button>
                </div>
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Players score bar - No answer status shown */}
      <div className="players-status-bar">
        {room?.players?.map(p => (
          <div 
            key={p.session_id} 
            className={`player-status ${p.session_id === sessionId ? 'is-me' : ''}`}
          >
            <span className="player-name">{p.name}</span>
            <span className="player-score">{p.score}đ</span>
            {p.streak > 0 && <span className="player-streak">🔥{p.streak}</span>}
          </div>
        ))}
      </div>
    </div>
  )
}

GameScreen.propTypes = {
  room: PropTypes.shape({
    name: PropTypes.string,
    max_rounds: PropTypes.number,
    players: PropTypes.array
  }),
  sessionId: PropTypes.number,
  gameState: PropTypes.shape({
    score: PropTypes.number,
    labelA: PropTypes.string,
    valueA: PropTypes.number,
    labelB: PropTypes.string,
    message: PropTypes.string,
    hasAnswered: PropTypes.bool,
    round: PropTypes.number,
    roundResults: PropTypes.array
  }).isRequired,
  onChoice: PropTypes.func.isRequired,
  loading: PropTypes.bool
}

export default GameScreen
