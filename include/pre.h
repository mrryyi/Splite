#pragma once
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <thread>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#define SOCKET_BUFFER_SIZE 1024
#define INTERVAL_CHECK_CONNECTION_MS 2000
#define MAX_TIME_BEFORE_DISCONNECT_MS 5000
#define ever ;;