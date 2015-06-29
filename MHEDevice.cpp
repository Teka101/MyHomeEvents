#include <cfloat>
#include "MHEDevice.h"

MHEDevice::MHEDevice(int id, DBDeviceType type, int cacheLifetime) : _id(id), _type(type), _cacheLifetime(cacheLifetime)
{
}

MHEDevice::~MHEDevice()
{
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
