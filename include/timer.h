#pragma once

#include "pre.h"

namespace Timer_ms
{

static int64 milliseconds_timer_started_at;

void timer_start() {
    using namespace std::chrono;
    milliseconds_timer_started_at = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
};

int64 timer_get_ms_since_start(){
    using namespace std::chrono;
    int64 now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return (now - milliseconds_timer_started_at);
};

}