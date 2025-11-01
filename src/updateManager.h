#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <Arduino.h>

class UpdateManager
{
public:
    UpdateManager();
    String checkForUpdates();
    void startUpdate();

private:
    bool performUpdate(const String &url, const String &sha256, int command);
};

#endif