/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

bool MHEDevice::isCachedDimmable()
{
    return false;
}

int MHEDevice::getCachedDimmableValue()
{
    return 0;
}

int MHEDevice::getCachedDimmableMax()
{
    return 0;
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

bool MHEDevice::isDimmable()
{
    return false;
}

int MHEDevice::getDimmableValue()
{
    return 0;
}

int MHEDevice::getDimmableMax()
{
    return 0;
}

bool MHEDevice::isActivated()
{
    return false;
}

bool MHEDevice::setLevel(int level)
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
