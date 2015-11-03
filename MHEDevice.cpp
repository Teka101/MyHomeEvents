#include <cfloat>
#include "MHEDevice.h"

MHEDevice::MHEDevice(int id, DBDeviceType type, std::string &name, int cacheLifetime, bool hidden) : _id(id), _type(type), _name(name), _cacheLifetime(cacheLifetime), _hidden(hidden), _cloneTo(NULL), _lastUpdate(0)
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

void MHEDevice::setCloneTo(MHEDevice *cloneTo)
{
    _cloneTo = cloneTo;
}

float MHEDevice::getTemperature()
{
    return FLT_MAX;
}

float MHEDevice::getHumidity()
{
    return FLT_MAX;
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
