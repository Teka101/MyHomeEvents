/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MHEMOBILENOTIFY_H
#define MHEMOBILENOTIFY_H

#include "MHEDevice.h"
#include "SpeechRecognize.h"

enum MHEMobileNotifyType
{
    conditionEnter,
    conditionLeave
};

class MHEMobileNotify
{
private:
    log4cplus::Logger _log;
    std::string _fcmServerKey;
    std::string _gcmAppId;
    MHEDatabase *_db;
    SpeechRecognize *_sr;

public:
    MHEMobileNotify(std::string fcmServerKey, std::string gcmAppId, MHEDatabase *db, SpeechRecognize *sr);
    ~MHEMobileNotify();

    void notifyDevices(const std::string &event, const std::string &type, const std::string &msg);
    void notifyDevices(MHEMobileNotifyType type, DBCondition &condition);
    void notifyDevices(MHEMobileNotifyType type, MHEDevice &device);

private:
    void notifyDevices(std::vector<DBMobile> &devices, const std::string &type, const std::string &msg);
    void notifyDevice(DBMobile &device, const std::string &type, const std::string &msg);
    void notifyDeviceFCM(DBMobile &device, const std::string &type, const std::string &msg);
    void notifyDeviceGCM(DBMobile &device, const std::string &type, const std::string &msg);
};

#endif // MHEMOBILENOTIFY_H
