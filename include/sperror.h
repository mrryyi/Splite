#pragma once

#include "def.h"

constexpr int32 FR_OK = 0;
constexpr int32 FR_FAILURE = 1;

class FRESULT {
    int32 v;
public:
    FRESULT(int32 v) : v(v) {
    }
    ~FRESULT () {

    }

    // If value is equal to the parameter.v
    bool8 operator ==(const FRESULT &p) {
        return v == p.v;
    };

    // If value is equal to the parameter
    bool8 operator ==(const int32 &p) {
        return v == p;
    };

    void operator =(const FRESULT &p) {
        v = p.v;
    };

    void operator =(const int32 &p) {
        v = p;
    };
};