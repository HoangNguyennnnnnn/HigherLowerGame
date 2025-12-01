/**
 * WaitingRoom Component - Room waiting screen before game starts
 */

import PropTypes from 'prop-types'
import PlayerList from './PlayerList'

function WaitingRoom({ 
  room, 
  sessionId, 
  onStartGame, 
  onLeaveRoom, 
  loading 
}) {
  // Calculate isHost directly from room data (ensure number comparison)
  const isHost = Number(room?.host_session_id) === Number(sessionId)
  const maxRounds = room?.max_rounds || 10

  return (
    <div className="waiting-room-screen">
      <h1>🚪 {room?.name || 'Phòng Chờ'}</h1>

      <div className="room-info">
        <p>Mã phòng: <strong>#{room?.id}</strong></p>
        <p>Số câu hỏi: <strong>{maxRounds}</strong></p>
        <p>
          Trạng thái: 
          <strong>
            {room?.status === 'waiting' ? ' ⏳ Đang chờ' : ' 🎮 Đang chơi'}
          </strong>
        </p>
      </div>

      <PlayerList
        players={room?.players}
        currentSessionId={sessionId}
        maxPlayers={room?.max_players}
      />

      <div className="waiting-room-actions">
        {isHost ? (
          <>
            <button
              className="start-game-btn"
              onClick={onStartGame}
              disabled={loading}
            >
              {loading ? 'Đang bắt đầu...' : '🎮 Bắt Đầu Game'}
            </button>
            <p className="host-note">
              Bạn là chủ phòng. Nhấn để bắt đầu khi sẵn sàng!
            </p>
          </>
        ) : (
          <p className="waiting-note">
            ⏳ Đang chờ chủ phòng bắt đầu game...
          </p>
        )}

        <button 
          className="leave-room-btn" 
          onClick={onLeaveRoom} 
          disabled={loading}
        >
          🚪 Rời Phòng
        </button>
      </div>
    </div>
  )
}

WaitingRoom.propTypes = {
  room: PropTypes.shape({
    id: PropTypes.number,
    name: PropTypes.string,
    status: PropTypes.string,
    player_count: PropTypes.number,
    max_players: PropTypes.number,
    max_rounds: PropTypes.number,
    host_session_id: PropTypes.number,
    players: PropTypes.array
  }),
  sessionId: PropTypes.number,
  onStartGame: PropTypes.func.isRequired,
  onLeaveRoom: PropTypes.func.isRequired,
  loading: PropTypes.bool
}

WaitingRoom.defaultProps = {
  loading: false
}

export default WaitingRoom
