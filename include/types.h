#pragma once

//
// Message types
//

#define MSGTYPE_LEGACYPOSITION      0
#define MSGTYPE_CONNECTION          1
#define MSGTYPE_REGISTERCLIENT      2
#define MSGTYPE_REGISTERREQUEST     3
#define MSGTYPE_REGISTERACCEPTED    4

//https://stackoverflow.com/a/34321463
const char* MsgTypeName(int value) {
#define NAME(TYPE) case TYPE: return #TYPE;
    switch (value) {
        NAME(MSGTYPE_LEGACYPOSITION)
        NAME(MSGTYPE_CONNECTION)
    }
    return "unknown";
#undef NAME
}