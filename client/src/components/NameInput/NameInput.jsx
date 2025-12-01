/**
 * NameInput Component - Player name input screen
 */

import PropTypes from 'prop-types'

function NameInput({ playerName, setPlayerName, onSubmit, connected }) {
  const handleKeyPress = (e) => {
    if (e.key === 'Enter' && playerName.trim()) {
      onSubmit()
    }
  }

  return (
    <div className="name-input-screen">
      <h1>🎮 Higher Lower Game</h1>
      <p>Nhập tên của bạn để bắt đầu</p>
      
      <input
        type="text"
        placeholder="Tên của bạn..."
        value={playerName}
        onChange={(e) => setPlayerName(e.target.value)}
        onKeyPress={handleKeyPress}
        maxLength={20}
        autoFocus
      />
      
      <button onClick={onSubmit} disabled={!playerName.trim()}>
        Tiếp tục
      </button>
      
      <div className="connection-status">
        {connected ? '🟢 Đã kết nối' : '🔴 Đang kết nối...'}
      </div>
    </div>
  )
}

NameInput.propTypes = {
  playerName: PropTypes.string.isRequired,
  setPlayerName: PropTypes.func.isRequired,
  onSubmit: PropTypes.func.isRequired,
  connected: PropTypes.bool.isRequired
}

export default NameInput
