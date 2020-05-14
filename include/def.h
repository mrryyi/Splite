#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define ever ;;
#define _DEBUG_EVERY_MESSAGE

#define NO_ID_GIVEN -1
#define assert(x)

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef long long int64;
typedef int int32;
typedef short int16;
typedef char int8;
typedef int32 bool32;
typedef uint8 bool8;
typedef float float32;
typedef double float64;

// If client has not been heard from for this long, then kick them.
constexpr uint64 MAX_TIME_UNHEARD_FROM_MS = 5000;
// Send requests every this many milliseconds, if unheard from for this many milliseconds.
constexpr uint64 INTERVAL_CHECK_CLIENT_MS = 500;
// How many milliseconds we wait before going into the function that checks the above.
constexpr uint64 INTERVAL_CHECK_CLIENTS_MS = 50;

constexpr int32 SOCKET_BUFFER_SIZE = 1024;
constexpr int32 MAX_CLIENTS_CONNECTED = 32;

constexpr int32 SERVER_TICK_RATE = 30;
constexpr int32 CLIENT_TICK_RATE = 60;

constexpr float32 server_milliseconds_per_tick = 1000 / SERVER_TICK_RATE;

constexpr float32 acceleration_per_millisecond = 0.5;
constexpr float32 gravitation_y_per_millisecond = 0.003;

constexpr float32 player_width = 10.0;
constexpr float32 player_height = 10.0;

constexpr float32 player_eat_cube_distance = 10.0f;

constexpr float32 window_coord_width = 1000;
constexpr float32 window_coord_height = 1000;

