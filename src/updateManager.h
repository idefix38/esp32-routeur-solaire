#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <Arduino.h>

class UpdateManager {
public:
    UpdateManager();
    String checkForUpdates();
    void startUpdate(const String& firmwareUrl, const String& firmwareMd5Url, const String& spiffsUrl, const String& spiffsMd5Url);

private:
    bool performUpdate(const String& url, const String& md5Url, int command);
};

#endif
