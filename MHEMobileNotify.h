#ifndef MHEMOBILENOTIFY_H
#define MHEMOBILENOTIFY_H

#include <log4cplus/logger.h>
#include "MHEDatabase.h"

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

public:
    MHEMobileNotify(std::string gcmAppId);
    ~MHEMobileNotify();

    void notifyDevices(std::vector<DBMobile> &devices, MHEMobileNotifyType type, DBCondition &condition);

private:
    void notityDevice(DBMobile &device, MHEMobileNotifyType type, DBCondition &condition);
};

#endif // MHEMOBILENOTIFY_H
