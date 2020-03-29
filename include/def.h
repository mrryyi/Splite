#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define ever ;;
#define _DEBUG_EVERY_MESSAGE

#define NO_ID_GIVEN -1

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
constexpr int64 MAX_TIME_UNHEARD_FROM_MS = 5000;
// Send requests every this many milliseconds, if unheard from for this many milliseconds.
constexpr int64 INTERVAL_CHECK_CLIENT_MS = 500;
// How many milliseconds we wait before going into the function that checks the above.
constexpr int64 INTERVAL_CHECK_CLIENTS_MS = 50;

constexpr int32 SOCKET_BUFFER_SIZE = 1024;
constexpr int32 MAX_CLIENTS_CONNECTED = 32;

constexpr int32 SERVER_TICK_RATE = 30;
constexpr int32 CLIENT_TICK_RATE = 120;

constexpr float32 window_coord_width = 1000;
constexpr float32 window_coord_height = 1000;