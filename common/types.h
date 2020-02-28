//
// Message types
//
#define MSGTYPE_LEGACYPOSITION      0
#define MSGTYPE_CONNECTION          1

//https://stackoverflow.com/a/34321463
const char* MsgTypeName(int value) {
#define NAME(ERR) case ERR: return #ERR;
    switch (value) {
        NAME(MSGTYPE_LEGACYPOSITION)
        NAME(MSGTYPE_CONNECTION)
    }
    return "unknown";
#undef NAME
}