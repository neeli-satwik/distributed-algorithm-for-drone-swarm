// Ankit's Part

#ifndef TIME_UTILIS_H
#define TIME_UTILIS_H

#include <Arduino.h>
#include <bits/stdc++.h>
#include <map>
using namespace std;

unsigned long getCurrentTimestamp();
bool isTimeout(unsigned long startTime, unsigned long timeoutMs);
unsigned long getTimeDiff(unsigned long startTime, unsigned long endTime);
void preciseDelay(unsigned long milliseconds);

class TimeoutManager
{
private:
    struct TimeoutInfo
    {
        unsigned long startTime;
        unsigned long timeoutDuration;
        bool isActive;
        String name;
    };
    std::map<int, TimeoutInfo> timeouts;
    int nextTimeoutId;

public:
    TimeoutManager();
    int addTimeout(unsigned long timeoutMs, String name = "");
    bool isTimeoutExpired(int timeoutId);
    void resetTimeout(int timeoutId);
    void removeTimeout(int timeoutId);
    std::vector<int> getExpiredTimeouts();
    std::vector<int> checkAllTimeouts();
    unsigned long getRemainingTime(int timeoutId);
    void printStatus();
};

#endif

// Ankit's Part