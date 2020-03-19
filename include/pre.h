#pragma once
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <thread>
// Primitive data types.
#include "def.h"
#include "sperror.h"
#include "player.h"
#include "game.h"

uint64 timeSinceEpochMillisec() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void PrintAddress(SOCKADDR_IN address) {
    printf( "%d.%d.%d.%d:%d", 
    address.sin_addr.S_un.S_un_b.s_b1, 
    address.sin_addr.S_un.S_un_b.s_b2, 
    address.sin_addr.S_un.S_un_b.s_b3, 
    address.sin_addr.S_un.S_un_b.s_b4, 
    address.sin_port);
};
