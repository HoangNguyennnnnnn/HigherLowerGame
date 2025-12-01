# Higher Lower Game 🎮

Game đoán giá trị cao/thấp theo thời gian thực, hỗ trợ multiplayer qua hệ thống phòng chơi.

## 📁 Cấu trúc dự án

```
HigherLowerGame/
├── client/                 # React + Vite frontend
│   ├── src/
│   │   ├── components/     # React components (NameInput, Lobby, GameScreen...)
│   │   ├── hooks/          # Custom hooks (useRoom, useGame)
│   │   ├── services/       # API services (axios)
│   │   └── constants/      # Config & constants
│   └── package.json
│
├── server/                 # C TCP server
│   ├── include/            # Header files
│   ├── src/                # Source files (modular)
│   │   ├── main.c          # Entry point
│   │   ├── router.c        # HTTP routing
│   │   ├── sse.c           # Server-Sent Events
│   │   ├── http.c          # HTTP utilities
│   │   ├── database.c      # Game database
│   │   ├── game_single.c   # Single player (legacy)
│   │   └── room_logic.c    # Room & multiplayer
│   ├── data/               # Game items data
│   └── Makefile
│
└── README.md
```

## 🚀 Quick Start

### Server (C)
```bash
cd server
make
./bin/game_server
# Server chạy tại http://localhost:8080
```

### Client (React)
```bash
cd client
npm install
npm run dev
# Client chạy tại http://localhost:5173
```

## 🎯 Tính năng

- **Multiplayer**: Tạo phòng, mời bạn bè, chơi cùng lúc
- **Real-time**: SSE (Server-Sent Events) cập nhật điểm số trực tiếp
- **Leaderboard**: Bảng xếp hạng real-time trong game
- **Timer**: Countdown 15 giây mỗi câu, tính điểm theo thời gian trả lời

## 🔧 Tech Stack

| Layer | Technology |
|-------|------------|
| Frontend | React + Vite |
| Backend | C (TCP sockets, pthreads) |
| Protocol | HTTP + SSE |
| Build | Makefile (server), Vite (client) |

## 📖 Documentation

- [Client README](./client/README.md) - Chi tiết về React frontend
- [Server README](./server/README.md) - Chi tiết về C backend

## 👥 Đóng góp

1. Fork project
2. Tạo branch (`git checkout -b feature/amazing-feature`)
3. Commit (`git commit -m 'Add amazing feature'`)
4. Push (`git push origin feature/amazing-feature`)
5. Tạo Pull Request
