#ifndef MHEHARDDEVCONTAINER_H
#define MHEHARDDEVCONTAINER_H

#include <map>
#include "MHEHardware.h"

class MHEHardDevContainer
{
    private:
        log4cplus::Logger _log;
        std::map<int,MHEHardware*> _hwById;
        std::map<int,MHEDevice*> _devById;

        void buildHardwares(MHEDatabase &db);
        void buildDevices(MHEDatabase &db);

    public:
        MHEHardDevContainer(MHEDatabase &db);
        ~MHEHardDevContainer();

        std::vector<MHEDevice*> getDevices();
        MHEDevice *getDeviceById(int id);
};

#endif // MHEHARDDEVCONTAINER_H
