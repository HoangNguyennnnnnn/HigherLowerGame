/**
 * GameOver Component - Final game over screen with results
 */

import PropTypes from 'prop-types'

function GameOver({ room, sessionId, onReturnToLobby }) {
  const sortedPlayers = room?.players
    ?.slice()
    .sort((a, b) => b.score - a.score) || []
  const winner = sortedPlayers[0]

  const getRankBadge = (index) => {
    switch (index) {
      case 0: return '🥇'
      case 1: return '🥈'
      case 2: return '🥉'
      default: return `#${index + 1}`
    }
  }

  const getRankClass = (index) => {
    switch (index) {
      case 0: return 'gold'
      case 1: return 'silver'
      case 2: return 'bronze'
      default: return ''
    }
  }

  return (
    <div className="game-over-screen">
      
      {/* Wrapper chính cho nội dung (để căn giữa) */}
      <div className="game-over-content"> 
        
        <h1>Game Over!</h1>

        <div className="winner-section">
          <h2>Winner</h2>
          <div className="winner-card">
            <span className="winner-name">{winner?.name || 'Chưa rõ'}</span>
            <span className="winner-score">{winner?.score || 0} điểm</span>
          </div>
        </div>

        <div className="final-leaderboard">
          <h2>Ranking</h2>
          <div className="leaderboard"> {/* Thêm wrapper leaderboard để giới hạn max-width */}
            {sortedPlayers.map((player, index) => (
              <div 
                key={player.session_id} 
                className={`final-rank ${getRankClass(index)} ${player.session_id === sessionId ? 'is-me' : ''}`}
              >
                <span className="rank-badge">{getRankBadge(index)}</span>
                <span className="player-name">{player.name}</span>
                <span className="player-score">{player.score} scores</span>
                {player.session_id === sessionId && (
                    <span className="you-tag">(You)</span>
                )}
              </div>
            ))}
          </div>
        </div>

        <button className="return-lobby-btn" onClick={onReturnToLobby}>
          Back to the Lobby
        </button>
      </div> {/* Kết thúc game-over-content */}
    </div>
  )
}

GameOver.propTypes = {
  room: PropTypes.shape({
    players: PropTypes.arrayOf(PropTypes.shape({
      session_id: PropTypes.number.isRequired,
      name: PropTypes.string.isRequired,
      score: PropTypes.number.isRequired
    }))
  }),
  sessionId: PropTypes.number,
  onReturnToLobby: PropTypes.func.isRequired
}

export default GameOver
