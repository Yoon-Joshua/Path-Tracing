#include "simple_timer.h"

SimpleTimer::SimpleTimer() : start_time{Clock::now()},
                 previous_tick{Clock::now()}
{
}

void SimpleTimer::Start()
{
    if (!running)
    {
        running = true;
        start_time = Clock::now();
    }
}

void SimpleTimer::Lap()
{
    lapping = true;
    lap_time = Clock::now();
}

bool SimpleTimer::is_running() const
{
    return running;
}