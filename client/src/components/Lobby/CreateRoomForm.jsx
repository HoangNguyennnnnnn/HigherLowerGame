/**
 * CreateRoomForm Component - Form to create a new room
 */

import PropTypes from 'prop-types'

function CreateRoomForm({ 
  roomName, 
  setRoomName, 
  maxRounds,
  setMaxRounds,
  onCreate, 
  loading, 
  disabled 
}) {
  return (
    <div className="create-room-section">
      <h2>Tạo Phòng Mới</h2>
      <div className="create-room-form">
        <input
          type="text"
          placeholder="Tên phòng (để trống = mặc định)"
          value={roomName}
          onChange={(e) => setRoomName(e.target.value)}
        />
        <select 
          value={maxRounds} 
          onChange={(e) => setMaxRounds(Number(e.target.value))}
        >
          <option value={5}>5 câu hỏi</option>
          <option value={10}>10 câu hỏi</option>
          <option value={15}>15 câu hỏi</option>
          <option value={20}>20 câu hỏi</option>
        </select>
        <button onClick={onCreate} disabled={loading || disabled}>
          {loading ? 'Đang tạo...' : '➕ Tạo Phòng'}
        </button>
      </div>
    </div>
  )
}

CreateRoomForm.propTypes = {
  roomName: PropTypes.string.isRequired,
  setRoomName: PropTypes.func.isRequired,
  maxRounds: PropTypes.number.isRequired,
  setMaxRounds: PropTypes.func.isRequired,
  onCreate: PropTypes.func.isRequired,
  loading: PropTypes.bool,
  disabled: PropTypes.bool
}

CreateRoomForm.defaultProps = {
  loading: false,
  disabled: false
}

export default CreateRoomForm
