/**
 * PlayerList Component - Display players in waiting room
 */

import PropTypes from 'prop-types'

function PlayerList({ players, currentSessionId }) {
  return (
    <div className="players-list">
      <h2>👥 Người Chơi ({players?.length || 0})</h2>
      <div className="players-grid">
        {players?.map((player) => (
          <div 
            key={player.session_id} 
            className={`player-card ${player.is_host ? 'host' : ''}`}
          >
            <span className="player-icon">
              {player.is_host ? '👑' : '👤'}
            </span>
            <span className="player-name">{player.name}</span>
            {player.session_id === currentSessionId && (
              <span className="you-tag">(Bạn)</span>
            )}
          </div>
        ))}
      </div>
    </div>
  )
}

PlayerList.propTypes = {
  players: PropTypes.arrayOf(PropTypes.shape({
    session_id: PropTypes.number.isRequired,
    name: PropTypes.string.isRequired,
    is_host: PropTypes.bool
  })),
  currentSessionId: PropTypes.number
}

PlayerList.defaultProps = {
  players: []
}

export default PlayerList
