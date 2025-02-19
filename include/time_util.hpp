#pragma once

#include <stdint.h>

typedef ::int8_t int8;
typedef ::int16_t int16;
typedef ::int32_t int32;
typedef ::int64_t int64;

typedef ::uint8_t uint8;
typedef ::uint16_t uint16;
typedef ::uint32_t uint32;
typedef ::uint64_t uint64;

#define DISALLOW_COPY_AND_ASSIGN(Type) \
    Type(const Type&);                 \
    void operator=(const Type&)

template <typename To, typename From>
inline To implicit_cast(From f) {
    return (To)f;
}

// Sleep for msec
void SleepInMs(uint32 ms);

// Sleep for usec
void SleepInUs(uint32 us);

inline void SleepInSeconds(uint32 sec) { SleepInMs(sec * 1000); }

// Monotonic timestamp in usec
uint64 NowInUs();

// Monotonic timestamp in msec
inline uint64 NowInMs() { return NowInUs() / 1000; }

class WallTimer {
   public:
    WallTimer() { _start = NowInUs(); }
    ~WallTimer() {}

    void Restart() { _start = NowInUs(); }

    uint64 ElapseInUs() const { return NowInUs() - _start; }

    uint64 ElapseInMs() const { return this->ElapseInUs() / 1000; }

   private:
    uint64 _start;

    DISALLOW_COPY_AND_ASSIGN(WallTimer);
};
