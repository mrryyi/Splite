#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define ever ;;


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

constexpr int64 MAX_TIME_UNHEARD_FROM_MS = 5000;
constexpr int64 INTERVAL_CHECK_CLIENT_MS = 500;

constexpr int32 SOCKET_BUFFER_SIZE = 1024;

constexpr int32 MAX_CLIENTS_CONNECTED = 32;

constexpr int32 SERVER_TICK_RATE = 120;