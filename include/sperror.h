#pragma once

#include "def.h"
constexpr int32 FRES = 10000;
constexpr int32 FR_OK = 0;
// FRESULT Error Start. Means we start errors at 10000 so we can identify if an FRESULT is error or not.
constexpr int32 FR_FAILURE = FRES + 1;

class FRESULT {
    // FRESULT value.
    int32 v;
public:
    FRESULT(const int32 v) : v(v) {
    }
    FRESULT() {
    }
    ~FRESULT () {
        // Possibility for an autotrace here.
        // Something that outputs to a file
        // or console or both, depending on defines.
        // Like output what error, or what success, etc etc.
    }
    
    // Returns true if error, that is to say, the FRESULT value
    // is above the FRESULT Error Start.
    explicit operator bool() const {
        return v > FRES;
    };

    // If value is equal to parameter FRESULT value
    bool8 operator ==(const FRESULT &p) {
        return v == p.v;
    };

    // If value is equal to parameter value
    bool8 operator ==(const int32 &p) {
        return v == p;
    };

    // Set value to parameter FRESULTs value.
    void operator =(const FRESULT &p) {
        v = p.v;
    };

    // Set value to parameter value.
    void operator =(const int32 &p) {
        v = p;
    };
};