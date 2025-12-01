/**
 * RoomCard Component - Display a single room in the list
 */

import PropTypes from 'prop-types'

function RoomCard({ room, onJoin, loading }) {
  const isPlaying = room.status !== 'waiting'
  const canJoin = !isPlaying

  const getButtonText = () => {
    if (isPlaying) return 'Đang chơi'
    return 'Vào Phòng'
  }

  return (
    <div className={`room-card ${room.status}`}>
      <h3>{room.name}</h3>
      <p>👥 {room.player_count} người chơi</p>
      <p className="room-status">
        {room.status === 'waiting' ? '⏳ Đang chờ' : '🎮 Đang chơi'}
      </p>
      <button
        onClick={() => onJoin(room.id)}
        disabled={loading || !canJoin}
      >
        {getButtonText()}
      </button>
    </div>
  )
}

RoomCard.propTypes = {
  room: PropTypes.shape({
    id: PropTypes.number.isRequired,
    name: PropTypes.string.isRequired,
    player_count: PropTypes.number.isRequired,
    status: PropTypes.string.isRequired
  }).isRequired,
  onJoin: PropTypes.func.isRequired,
  loading: PropTypes.bool
}

RoomCard.defaultProps = {
  loading: false
}

export default RoomCard
