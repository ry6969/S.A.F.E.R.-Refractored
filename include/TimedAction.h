#ifndef TIMEDACTIONS_H
#define TIMEDACTIONS_H

inline bool timedAction(unsigned long &prev, unsigned long interval, unsigned long now) {
    if (now - prev >= interval) {
        prev = now;
        return true;
    }
    return false;
}

#endif

