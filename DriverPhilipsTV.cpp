#include <time.h>
#include "CurlHelpers.h"
#include "DriverPhilipsTV.h"

HardwarePhilipsTV::HardwarePhilipsTV(DBHarware &hw) : MHEHardware(hw.id), _hw(hw)
{
}

HardwarePhilipsTV::~HardwarePhilipsTV()
{
}

MHEDevice *HardwarePhilipsTV::buildObject(DBDevice &dev)
{
    return new DevicePhilipsTV(dev);
}

DevicePhilipsTV::DevicePhilipsTV(DBDevice &dev) : MHEDevice(dev.id, dev.type, dev.name, dev.cacheLifetime), _ip(dev.param1)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("DevicePhilipsTV"));
}

DevicePhilipsTV::~DevicePhilipsTV()
{
}

bool DevicePhilipsTV::isActivated()
{
    std::stringstream ssUrl;
    bool status = false;

    ssUrl << "http://" << _ip << ":1925/1/channels/current";
    if (curlExecute(ssUrl.str()))
        status = true;
    _lastUpdate = time(NULL);
    return status;
}

bool DevicePhilipsTV::setStatus(bool activate)
{
    if (!activate)
        return sendKey("Standby");
    return false;
}

bool DevicePhilipsTV::sendKey(const std::string &key)
{
    std::stringstream ssUrl, ssPost;
    std::string postData;

    ssUrl << "http://" << _ip << ":1925/1/input/key";
    ssPost << "{\"key\": \""<< key << "\"}";
    postData = ssPost.str();
    if (curlExecute(ssUrl.str(), &postData))
    {
        time_t now = time(NULL);

        _lastUpdate = now;
        return true;
    }
    return false;
}
