/**
 * Lobby Component - Main lobby screen
 */

import { useState } from 'react'
import PropTypes from 'prop-types'
import CreateRoomForm from './CreateRoomForm'
import RoomCard from './RoomCard'

function Lobby({ 
  playerName, 
  rooms, 
  onCreateRoom, 
  onJoinRoom, 
  onRefresh, 
  loading, 
  connected 
}) {
  const [roomName, setRoomName] = useState('')
  const [maxRounds, setMaxRounds] = useState(10)

  const handleCreateRoom = async () => {
    const success = await onCreateRoom({ 
      roomName: roomName || `Phòng của ${playerName}`, 
      maxRounds
    })
    if (success) {
      setRoomName('')
    }
  }

  return (
    <div className="lobby-screen">
      <div className="lobby-header">
        <h1>🏠 Sảnh Chờ</h1>
        <p>Xin chào, <strong>{playerName}</strong>!</p>
      </div>

      <CreateRoomForm
        roomName={roomName}
        setRoomName={setRoomName}
        maxRounds={maxRounds}
        setMaxRounds={setMaxRounds}
        onCreate={handleCreateRoom}
        loading={loading}
        disabled={!connected}
      />

      <div className="rooms-list-section">
        <h2>Danh Sách Phòng ({rooms.length})</h2>
        <button className="refresh-btn" onClick={onRefresh}>
          🔄 Làm mới
        </button>

        {rooms.length === 0 ? (
          <p className="no-rooms">Chưa có phòng nào. Hãy tạo phòng mới!</p>
        ) : (
          <div className="rooms-grid">
            {rooms.map(room => (
              <RoomCard
                key={room.id}
                room={room}
                onJoin={onJoinRoom}
                loading={loading}
              />
            ))}
          </div>
        )}
      </div>
    </div>
  )
}

Lobby.propTypes = {
  playerName: PropTypes.string.isRequired,
  rooms: PropTypes.array.isRequired,
  onCreateRoom: PropTypes.func.isRequired,
  onJoinRoom: PropTypes.func.isRequired,
  onRefresh: PropTypes.func.isRequired,
  loading: PropTypes.bool,
  connected: PropTypes.bool
}

Lobby.defaultProps = {
  loading: false,
  connected: false
}

export default Lobby
