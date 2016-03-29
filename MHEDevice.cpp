#include <cmath>
#include "MHEDevice.h"

MHEDevice::MHEDevice(int id, DBDeviceType type, std::string &name, int cacheLifetime, bool hidden, bool cacheRunning) : _id(id), _type(type), _name(name), _cacheLifetime(cacheLifetime), _hidden(hidden), _cacheRunning(cacheRunning), _cloneTo(NULL), _lastUpdate(0)
{
}

MHEDevice::~MHEDevice()
{
}

int MHEDevice::getId() const
{
    return _id;
}

DBDeviceType MHEDevice::getType() const
{
    return _type;
}

std::string MHEDevice::getName() const
{
    return _name;
}

time_t MHEDevice::getLastUpdate() const
{
    return _lastUpdate;
}

bool MHEDevice::isHidden() const
{
    return _hidden;
}

bool MHEDevice::isCacheRunning() const
{
    return _cacheRunning;
}

void MHEDevice::setCloneTo(MHEDevice *cloneTo)
{
    _cloneTo = cloneTo;
}

float MHEDevice::getCachedRawTemperature()
{
    return NAN;
}

float MHEDevice::getCachedTemperature()
{
    return NAN;
}

float MHEDevice::getCachedHumidity()
{
    return NAN;
}

bool MHEDevice::isCachedActivated()
{
    return false;
}

float MHEDevice::getRawTemperature()
{
    return NAN;
}

float MHEDevice::getTemperature()
{
    return NAN;
}

float MHEDevice::getHumidity()
{
    return NAN;
}

bool MHEDevice::setTempHum(float temperature, float humidity)
{
    return false;
}

bool MHEDevice::isActivated()
{
    return false;
}

bool MHEDevice::setStatus(bool activate)
{
    return false;
}

bool MHEDevice::sendCommand(const std::string &command, const std::string &value, void *output)
{
    return false;
}
