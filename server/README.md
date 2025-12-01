# Higher Lower Game - Server

Game server viết bằng C sử dụng TCP sockets, hỗ trợ SSE (Server-Sent Events) và hệ thống phòng chơi multiplayer.

## 📁 Cấu trúc thư mục

```
server/
├── include/                    # Header files
│   ├── game.h                 # Master header (include all)
│   ├── config.h               # Cấu hình và constants
│   ├── types.h                # Data structures và enums
│   ├── http.h                 # HTTP response utilities
│   ├── sse.h                  # Server-Sent Events
│   ├── server.h               # Server core functions
│   ├── room.h                 # Room/Lobby system
│   ├── database.h             # Game database declarations
│   └── room_helpers.h         # Room helper functions
│
├── src/                        # Source files (modular)
│   ├── main.c                 # Entry point, globals, server loop
│   ├── router.c               # HTTP request parsing & routing
│   ├── sse.c                  # SSE connection handling
│   ├── http.c                 # HTTP response utilities
│   ├── database.c             # Game database (items.txt loading)
│   ├── room_init.c            # Room globals & initialization
│   ├── room_helpers.c         # Room finder & JSON builders
│   ├── room_handlers.c        # Room CRUD handlers
│   └── game_handlers.c        # Game flow handlers
│
├── data/                       # Data files
│   └── items.txt              # Game items (name, value, image_url)
│
├── obj/                        # Object files (generated)
├── bin/                        # Executable (generated)
└── Makefile                    # Build configuration
```

## 🧩 Source Modules

| File | Chức năng |
|------|-----------|
| `main.c` | Entry point, khởi tạo server, accept loop |
| `router.c` | Parse HTTP requests, route đến handlers |
| `sse.c` | SSE subscribe, broadcast to session/room |
| `http.c` | send_cors_headers(), send_json_response() |
| `database.c` | Load items.txt, get_random_index_except() |
| `room_init.c` | Global vars (rooms, mutex) + init_rooms() |
| `room_helpers.c` | find_room_*, JSON parse/build functions |
| `room_handlers.c` | Room CRUD handlers (list, create, join, leave) |
| `game_handlers.c` | Game flow handlers (start, choice, info) |

## 📋 Header Files

### `config.h`
Chứa tất cả constants và macros:
- `PORT` (8080) - Port server
- `MAX_CLIENTS` (100) - Số client tối đa
- `BUFFER_SIZE` (8192) - Kích thước buffer
- `RESPONSE_SIZE` (16384) - Kích thước response buffer
- `MAX_ROOMS` (20) - Số phòng tối đa
- `MAX_PLAYERS_PER_ROOM` (50) - Số người chơi mỗi phòng

### `types.h`
Chứa tất cả data structures:
- `RoomStatus` - Enum trạng thái phòng (EMPTY, WAITING, PLAYING, FINISHED)
- `GameItem` - Struct cho item trong game (name, value, image_url)
- `RoomPlayer` - Struct cho người chơi trong phòng
- `GameRoom` - Struct cho phòng chơi
- `SSE_Client` - Struct cho SSE connection

### `http.h`
HTTP response utilities:
- `send_cors_headers()` - Gửi CORS headers
- `send_json_response()` - Gửi JSON response

### `sse.h`
Server-Sent Events:
- `handle_sse_subscribe()` - Xử lý subscribe SSE
- `broadcast_sse_to_session()` - Gửi message đến session
- `broadcast_sse_to_room()` - Gửi message đến tất cả người trong phòng

### `database.h`
Game database:
- `game_database[]` - Mảng chứa tất cả items
- `item_count` - Số lượng items
- `init_game_database()` - Khởi tạo database
- `get_random_index_except()` - Lấy random index

### `room.h`
Room/Lobby system declarations:
- `init_rooms()` - Khởi tạo hệ thống phòng
- `handle_list_rooms()` - GET /rooms
- `handle_create_room()` - POST /rooms/create
- `handle_join_room()` - POST /rooms/join
- `handle_leave_room()` - POST /rooms/leave
- `handle_start_game()` - POST /rooms/start
- `handle_room_choice()` - POST /rooms/choice
- `handle_get_room_info()` - GET /rooms/info

### `room_helpers.h`
Room helper functions:
- `find_room_index()` - Tìm room theo ID
- `find_player_in_room()` - Tìm player trong room
- `find_room_with_player()` - Tìm room chứa player
- `build_room_json()` - Build JSON cho room
- `parse_json_string/int()` - Parse JSON primitives

### `game.h` (Master Header)
Include tất cả các header khác, giữ backward compatibility.

## 🔨 Build

```bash
# Build
make

# Clean và rebuild
make rebuild

# Chạy server
make run

# Xem help
make help
```

## 🚀 API Endpoints

### SSE Connection
```
GET /subscribe
Response: text/event-stream
```

### Room APIs
```
GET /rooms                     # Danh sách phòng
POST /rooms/create             # Tạo phòng mới
POST /rooms/join               # Vào phòng
POST /rooms/leave              # Rời phòng
POST /rooms/start              # Bắt đầu game (chỉ host)
POST /rooms/choice             # Chọn đáp án
GET /rooms/info                # Thông tin phòng hiện tại
```

## 📊 Luồng dữ liệu

```
Client          Server
  |                |
  |--- SSE ------->| (kết nối, nhận session_id)
  |                |
  |--- /rooms --->| (lấy danh sách phòng)
  |<-- JSON -------|
  |                |
  |--- /rooms/create -->| (tạo phòng)
  |<-- JSON ------------|
  |<-- SSE (room_created)
  |                |
  |--- /rooms/join ---->| (vào phòng)
  |<-- SSE (player_joined) to all players
  |                |
  |--- /rooms/start -->| (chủ phòng bắt đầu)
  |<-- SSE (game_started) to all players
  |                |
  |--- /rooms/choice -->| (chơi game)
  |<-- SSE (player_update) to all players
  |                |
  |<-- SSE (game_finished) when all done
```

## 🔧 Threading Model

- Main thread: Accept connections
- Worker threads: Xử lý mỗi HTTP request
- SSE connections: Giữ socket mở để gửi events

## 📝 Notes

- Session ID được tạo khi client subscribe SSE
- Mỗi request cần gửi `X-Session-ID` header
- Rooms mutex bảo vệ danh sách phòng
- Clients mutex bảo vệ danh sách SSE clients
- Host là người tạo phòng, có quyền bắt đầu game
