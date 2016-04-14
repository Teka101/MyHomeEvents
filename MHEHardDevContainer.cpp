/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <boost/foreach.hpp>
#include <log4cplus/loggingmacros.h>
#include "MHEHardDevContainer.h"
#include "DriverDomoticz.h"
#include "DriverPhilipsTV.h"
#include "DriverShell.h"

MHEHardDevContainer::MHEHardDevContainer(MHEDatabase &db)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("MHEHardDevContainer"));
    buildHardwares(db);
    buildDevices(db);
}

MHEHardDevContainer::~MHEHardDevContainer()
{
    std::pair<int, MHEDevice*> kvDev;
    std::pair<int, MHEHardware*> kvHard;

    BOOST_FOREACH(kvDev, _devById)
    {
        delete kvDev.second;
    }
    BOOST_FOREACH(kvHard, _hwById)
    {
        delete kvHard.second;
    }
}

void MHEHardDevContainer::buildHardwares(MHEDatabase &db)
{
    std::vector<DBHarware> hws = db.getHardwares();

    BOOST_FOREACH(DBHarware &hw, hws)
    {
        LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("buildHardwares - try to build: " << (std::string)hw));
        if (hw.type == "domoticz")
            _hwById[hw.id] = new HardwareDomoticz(hw);
        else if (hw.type == "philipsTV")
            _hwById[hw.id] = new HardwarePhilipsTV(hw);
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
    BOOST_FOREACH(DBDevice &dev, devs)
    {
        if (dev.cloneToDeviceId != -1 && dev.id != dev.cloneToDeviceId)
        {
            MHEDevice *d = _devById[dev.id];
            MHEDevice *cloneTo = _devById[dev.cloneToDeviceId];

            if (d != NULL && cloneTo != NULL)
            {
                d->setCloneTo(cloneTo);
                LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("buildDevices - device: " << (std::string)*d) << " is cloned to: " << (std::string)*cloneTo);
            }
        }
    }
}

std::vector<MHEDevice*> MHEHardDevContainer::getDevices()
{
    std::pair<int, MHEDevice*> kv;
    std::vector<MHEDevice*> ret;

    BOOST_FOREACH(kv, _devById)
    {
        ret.push_back(kv.second);
    }
    return ret;
}

MHEDevice *MHEHardDevContainer::getDeviceById(int id)
{
    return _devById[id];
}
