#include <boost/foreach.hpp>
#include "MHEHardDevContainer.h"
#include "DriverDomoticz.h"
#include "DriverShell.h"

MHEHardDevContainer::MHEHardDevContainer(MHEDatabase &db)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("MHEHardDevContainer"));
    buildHardwares(db);
    buildDevices(db);
}

MHEHardDevContainer::~MHEHardDevContainer()
{
}

void MHEHardDevContainer::buildHardwares(MHEDatabase &db)
{
    std::vector<DBHarware> hws = db.getHardwares();

    BOOST_FOREACH(DBHarware &hw, hws)
    {
        LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("buildHardwares - try to build: " << (std::string)hw));
        if (hw.type == "domoticz")
            _hwById[hw.id] = new HardwareDomoticz(hw);
        else if (hw.type == "shell")
            _hwById[hw.id] = new HardwareShell(hw);
        else
            LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("buildHardwares - unknown: " << (std::string)hw));
    }
}

void MHEHardDevContainer::buildDevices(MHEDatabase &db)
{
    std::vector<DBDevice> devs = db.getDevices();

    BOOST_FOREACH(DBDevice &dev, devs)
    {
        MHEHardware *hw = _hwById[dev.hardwareId];

        if (hw != NULL)
        {
            MHEDevice *d = hw->buildObject(dev);

            if (d != NULL)
            {
                LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("buildDevices - builded: " << (std::string)*d));
                _devById[dev.id] = d;
            }
            else
                LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("buildDevices - unable to build: " << (std::string)dev << " from hardware: " << (std::string)*hw));
        }
        else
            LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("buildDevices - unable to build: " << (std::string)dev << " unknown hardware with id=" << dev.hardwareId));
    }
}

MHEDevice *MHEHardDevContainer::getDeviceById(int id)
{
    return _devById[id];
}
