#include "../include/utilities/time_utils.h"

unsigned long getCurrentTimestamp()
{
    return millis();
}

bool isTimeout(unsigned long startTime, unsigned long timeoutMs)
{
    unsigned long currentTime = getCurrentTimestamp();
    return (currentTime - startTime) >= timeoutMs;
}
unsigned long getTimeDiff(unsigned long startTime, unsigned long endTime)
{
    if (endTime >= startTime)
    {
        return endTime - startTime;
    }
    else
    {
        return (ULONG_MAX - startTime) + endTime + 1;
    }
}
void preciseDelay(unsigned long millisecond)
{
    unsigned long startTime = getCurrentTimestamp();
    while (!isTimeout(startTime, millisecond))
    {
        yield();
    }
}
TimeoutManager::TimeoutManager()
{
    nextTimeoutId = 1;
}

int TimeoutManager::addTimeout(unsigned long timeoutMs, String name)
{
    TimeoutInfo info;
    info.startTime = getCurrentTimestamp();
    info.timeoutDuration = timeoutMs;
    info.isActive = true;
    info.name = name.length() > 0 ? name : "Timeout_" + String(nextTimeoutId);
    timeouts[nextTimeoutId] = info;
    Serial.println("Added timeout ID " + String(nextTimeoutId) + " (" + info.name + ") for " + String(timeoutMs) + "ms");
    return nextTimeoutId++;
}
bool TimeoutManager::isTimeoutExpired(int timeoutId)
{
    if (timeouts.find(timeoutId) == timeouts.end())
    {
        return false;
    }
    TimeoutInfo &info = timeouts[timeoutId];
    if (!info.isActive)
    {
        return false;
    }
    return isTimeout(info.startTime, info.timeoutDuration);
}

void TimeoutManager::resetTimeout(int timeoutId)
{
    if (timeouts.find(timeoutId) != timeouts.end())
    {
        timeouts[timeoutId].startTime = getCurrentTimestamp();
        timeouts[timeoutId].isActive = true;
        Serial.println("Reset timeout ID " + String(timeoutId) + " (" + timeouts[timeoutId].name + ")");
    }
}

void TimeoutManager::removeTimeout(int timeoutId)
{
    if (timeouts.find(timeoutId) != timeouts.end())
    {
        Serial.println("Removed timeout ID " + String(timeoutId) + " (" + timeouts[timeoutId].name + ")");
        timeouts.erase(timeoutId);
    }
}
std::vector<int> TimeoutManager::checkAllTimeouts()
{
    std::vector<int> expiredIds = getExpiredTimeouts();
    for (int id : expiredIds)
    {
        timeouts[id].isActive = false;
        Serial.println("TIMEOUT EXPIRED: ID " + String(id) + " (" + timeouts[id].name + ")");
    }
    return expiredIds;
}

unsigned long TimeoutManager::getRemainingTime(int timeoutId)
{
    if (timeouts.find(timeoutId) == timeouts.end())
    {
        return 0;
    }
    TimeoutInfo &info = timeouts[timeoutId];
    if (!info.isActive)
    {
        return 0;
    }
    unsigned long elapsed = getTimeDiff(info.startTime, getCurrentTimestamp());
    if (elapsed >= info.timeoutDuration)
    {
        return 0;
    }
    return info.timeoutDuration - elapsed;
}

void TimeoutManager::printStatus()
{
    Serial.println("::: TimeoutManager Status :::");
    Serial.println("Active timeouts: " + String(timeouts.size()));
    for (auto &pair : timeouts)
    {
        TimeoutInfo &info = pair.second;
        unsigned long remaining = getRemainingTime(pair.first);
        Serial.println("ID " + String(pair.first) + " (" + info.name + "): " + (info.isActive ? "Active" : "Expired") + ", Remaining: " + String(remaining) + "ms");
    }
    Serial.println("::::::::::::::::::::::");
}