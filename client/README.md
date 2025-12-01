# Higher Lower Game - Client

React client cho game Higher Lower với Vite, hỗ trợ multiplayer qua hệ thống phòng.

## 📁 Cấu trúc thư mục

```
client/src/
├── components/                 # React components
│   ├── NameInput/             # Màn hình nhập tên
│   │   ├── NameInput.jsx
│   │   └── index.js
│   ├── Lobby/                 # Sảnh chờ
│   │   ├── Lobby.jsx
│   │   ├── RoomCard.jsx
│   │   ├── CreateRoomForm.jsx
│   │   └── index.js
│   ├── WaitingRoom/           # Phòng chờ game
│   │   ├── WaitingRoom.jsx
│   │   ├── PlayerList.jsx
│   │   └── index.js
│   ├── GameScreen/            # Màn hình chơi game
│   │   ├── GameScreen.jsx
│   │   ├── GameItem.jsx
│   │   ├── Leaderboard.jsx
│   │   └── index.js
│   ├── GameOver/              # Màn hình kết thúc
│   │   ├── GameOver.jsx
│   │   └── index.js
│   └── index.js               # Barrel export
│
├── hooks/                      # Custom React hooks
│   ├── useRoom.js             # Room management hook
│   ├── useGame.js             # Game state hook
│   └── index.js               # Barrel export
│
├── services/                   # API services
│   ├── api.js                 # Axios instance với interceptors
│   ├── roomService.js         # Room API calls
│   ├── gameService.js         # Game API calls
│   └── index.js               # Barrel export
│
├── constants/                  # Constants và config
│   ├── config.js              # SERVER_URL, ENDPOINTS
│   ├── screens.js             # Screen states enum
│   └── index.js               # Barrel export
│
├── App.jsx                     # Main App component (SSE connection)
├── App.css                     # Styles
├── main.jsx                    # Entry point
└── index.css                   # Global styles
```

## 🧩 Components

| Component | Mô tả |
|-----------|-------|
| `NameInput` | Màn hình nhập tên người chơi |
| `Lobby` | Sảnh chờ với danh sách phòng |
| `RoomCard` | Card hiển thị thông tin phòng |
| `CreateRoomForm` | Form tạo phòng mới |
| `WaitingRoom` | Phòng chờ trước khi game bắt đầu |
| `PlayerList` | Danh sách người chơi trong phòng |
| `GameScreen` | Màn hình chơi game chính |
| `GameItem` | Hiển thị item A hoặc B |
| `Leaderboard` | Bảng xếp hạng real-time |
| `GameOver` | Màn hình kết quả cuối game |

## 🪝 Custom Hooks

### `useRoom(sessionId)`
Quản lý state và actions cho rooms.
```js
const { 
  rooms, currentRoom, loading, error,
  fetchRooms, createRoom, joinRoom, leaveRoom, startGame, updateRoom 
} = useRoom(sessionId)
```

### `useGame(sessionId)`
Quản lý game state và actions.
```js
const { 
  score, streak, labelA, valueA, labelB, valueB, gameOver, loading,
  makeChoice, fetchGameState, resetGame 
} = useGame(sessionId)
```

## 🔌 Services

### `api.js`
Axios instance với:
- Base URL từ config
- Request interceptor: thêm `X-Session-ID` header từ localStorage
- Response interceptor: error handling

### `roomService.js`
- `getRooms()` - Lấy danh sách phòng
- `createRoom({ roomName, playerName, maxPlayers })` - Tạo phòng
- `joinRoom({ roomId, playerName })` - Vào phòng
- `leaveRoom()` - Rời phòng
- `startGame()` - Bắt đầu game

### `gameService.js`
- `makeChoice(choice)` - Chọn đáp án (1 hoặc 2)
- `getGameState()` - Lấy game state từ /rooms/info

## ⚙️ Constants

### `config.js`
```js
// Đọc từ .env hoặc dùng default
SERVER_URL = import.meta.env.VITE_SERVER_URL || 'http://localhost:8080'

ENDPOINTS = {
  SUBSCRIBE: '/subscribe',
  ROOMS: '/rooms',
  ROOMS_CREATE: '/rooms/create',
  ROOMS_JOIN: '/rooms/join',
  ROOMS_LEAVE: '/rooms/leave',
  ROOMS_START: '/rooms/start',
  ROOMS_CHOICE: '/rooms/choice',
  ROOMS_INFO: '/rooms/info'
}
```

### `screens.js`
```js
SCREENS = { LOBBY, WAITING_ROOM, PLAYING, GAME_OVER }
```

## 🔄 SSE Events

App.jsx xử lý các SSE events:
- `session_id` - Nhận session ID khi kết nối
- `player_joined` - Người chơi vào phòng
- `player_left` - Người chơi rời phòng
- `game_started` - Game bắt đầu
- `player_update` - Cập nhật điểm người chơi
- `game_finished` - Game kết thúc

## 🚀 Development

```bash
# Install dependencies
npm install

# Run dev server
npm run dev

# Build for production
npm run build

# Preview production build
npm run preview
```

## 📋 Dependencies

- `react` - UI library
- `axios` - HTTP client
- `prop-types` - Runtime type checking
- `vite` - Build tool

## 🌐 Environment

Tạo file `.env` trong thư mục client:

```bash
# Copy từ template
cp .env.example .env
```

Nội dung `.env`:
```env
VITE_SERVER_URL=http://localhost:8080
```

Hoặc để trống sẽ dùng default `http://localhost:8080`.

## 📊 Game Flow

```
NameInput → Lobby → WaitingRoom → GameScreen → GameOver
    ↑                                              |
    └──────────────────────────────────────────────┘
```

1. **NameInput**: Nhập tên người chơi
2. **Lobby**: Xem danh sách phòng, tạo phòng hoặc vào phòng
3. **WaitingRoom**: Chờ host bắt đầu game
4. **GameScreen**: Chơi game, xem leaderboard real-time
5. **GameOver**: Xem kết quả, quay lại lobby
