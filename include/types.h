#pragma once
//
// Message types
//
#define MSGTYPE_LEGACYPOSITION      0
#define MSGTYPE_LECACYDIRECTION     1
#define MSGTYPE_CONNECTION          2
#define MSGTYPE_REGISTERREQUEST     3
#define MSGTYPE_REGISTERSYN         4
#define MSGTYPE_REGISTERACK         5
#define MSGTYPE_REGISTERACCEPT      6

//https://stackoverflow.com/a/34321463
const char* MsgTypeName(int value) {
#define NAME(TYPE) case TYPE: return #TYPE;
    switch (value) {
        NAME(MSGTYPE_LEGACYPOSITION)
        NAME(MSGTYPE_LECACYDIRECTION)
        NAME(MSGTYPE_CONNECTION)
        NAME(MSGTYPE_REGISTERREQUEST)
        NAME(MSGTYPE_REGISTERSYN)
        NAME(MSGTYPE_REGISTERACK)
        NAME(MSGTYPE_REGISTERACCEPT)
    }
    return "unknown";
#undef NAME
}