#include <cfloat>
#include "MHEDevice.h"

MHEDevice::MHEDevice(int id, DBDeviceType type, std::string &name, int cacheLifetime) : _id(id), _type(type), _name(name), _cacheLifetime(cacheLifetime), _cloneTo(NULL), _lastUpdate(0)
{
}

MHEDevice::~MHEDevice()
{
}

int MHEDevice::getId()
{
    return _id;
}

DBDeviceType MHEDevice::getType()
{
    return _type;
}

std::string MHEDevice::getName()
{
    return _name;
}

time_t MHEDevice::getLastUpdate()
{
    return _lastUpdate;
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
