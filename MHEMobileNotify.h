#ifndef MHEMOBILENOTIFY_H
#define MHEMOBILENOTIFY_H

#include <log4cplus/logger.h>
#include "MHEDatabase.h"
#include "MHEDevice.h"

enum MHEMobileNotifyType
{
    conditionEnter,
    conditionLeave
};

class MHEMobileNotify
{
private:
    log4cplus::Logger _log;
    std::string _gcmAppId;
    MHEDatabase *_db;

public:
    MHEMobileNotify(std::string gcmAppId, MHEDatabase *db);
    ~MHEMobileNotify();

    void notifyDevices(const std::string &event, const std::string &type, const std::string &msg);
    void notifyDevices(MHEMobileNotifyType type, DBCondition &condition);
    void notifyDevices(MHEMobileNotifyType type, MHEDevice &device);

private:
    void notifyDevices(std::vector<DBMobile> &devices, const std::string &type, const std::string &msg);
    void notifyDevice(DBMobile &device, const std::string &type, const std::string &msg);
};

#endif // MHEMOBILENOTIFY_H
