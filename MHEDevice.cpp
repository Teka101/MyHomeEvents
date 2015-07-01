#include <cfloat>
#include "MHEDevice.h"

MHEDevice::MHEDevice(int id, DBDeviceType type, int cacheLifetime) : _id(id), _type(type), _cacheLifetime(cacheLifetime), _cloneTo(NULL)
{
}

MHEDevice::~MHEDevice()
{
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
