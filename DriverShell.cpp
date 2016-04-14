/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <log4cplus/loggingmacros.h>
#include <cmath>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "DriverShell.h"

HardwareShell::HardwareShell(DBHarware &hw) : MHEHardware(hw.id), _hw(hw)
{
}

HardwareShell::~HardwareShell()
{
}

MHEDevice *HardwareShell::buildObject(DBDevice &dev)
{
    return new DeviceShell(dev);
}

DeviceShell::DeviceShell(DBDevice &dev) : MHEDevice(dev.id, dev.type, dev.name, dev.cacheLifetime, dev.hidden, dev.cacheRunning), _shellCmd(dev.param1), _temperature(0.0), _humidity(0.0), _offsetTemperature(0.0)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("DeviceShell"));

    boost::regex expression("^offset_temperature=(-?[0-9\\.]+)$", boost::regex::perl);
    boost::cmatch what;

    if (boost::regex_match(dev.param2.c_str(), what, expression))
        _offsetTemperature = boost::lexical_cast<float>(what[1]);
}

DeviceShell::~DeviceShell()
{
}

float DeviceShell::getCachedTemperature()
{
    return _temperature;
}

float DeviceShell::getCachedHumidity()
{
    return _humidity;
}

float DeviceShell::getTemperature()
{
    refreshCache();
    return _temperature;
}

float DeviceShell::getHumidity()
{
    refreshCache();
    return _humidity;
}


void DeviceShell::refreshCache()
{
    time_t now = time(NULL) - _cacheLifetime;

    if (_lastUpdate < now)
    {
        FILE *fh;

        fh = popen(_shellCmd.c_str(), "r");
        if (fh == NULL)
            LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("DeviceShell::refreshCache - popen() failed: " << strerror(errno)));
        else
        {
            boost::iostreams::file_descriptor_source fdSource(fileno(fh), boost::iostreams::never_close_handle);
            boost::iostreams::stream<boost::iostreams::file_descriptor_source> stream(fdSource);
            boost::regex expression("^Humidity = (.+) % Temperature = (.+) \\*C.*$", boost::regex::perl);
            boost::cmatch what;
            std::string line;
            int execReturn;

            LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("DeviceShell::refreshCache - popen() for command: " << _shellCmd));
            while (std::getline(stream, line))
                if (boost::regex_match(line.c_str(), what, expression))
                {
                    float newHumidity = boost::lexical_cast<float>(what[1]);
                    float newTemperature = boost::lexical_cast<float>(what[2]) + _offsetTemperature;
                    bool isSucess = checkValidity(newHumidity, newTemperature);

                    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("DeviceShell::refreshCache - Update: humidity=" << _humidity << " temperature=" << _temperature << " isSucess=" << (isSucess ? "true" : "false")));
                }
            execReturn = pclose(fh);
            LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("DeviceShell::refreshCache - Exec return: " << execReturn));
        }
    }
}

bool DeviceShell::checkValidity(float newHumidty, float newTemperature)
{
    time_t now = time(NULL);
    time_t diffDate = std::abs(now - _lastUpdate);
    float diffTemperature = std::abs(newTemperature - _temperature);
    float diffHumidity = std::abs(newHumidty - _humidity);

    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("DeviceShell::checkValidity - values : temperature=" << newTemperature << " d=" << diffTemperature << " humdity=" << newHumidty << " d=" << diffHumidity << " dT=" << diffDate));
    if (diffDate >= 2400 || (diffHumidity <= 15.0 && diffTemperature <= 5.0))
    {
		_temperature = newTemperature;
		_humidity = newHumidty;
		_lastUpdate = now;
		if (_cloneTo != NULL)
            _cloneTo->setTempHum(newTemperature, newHumidty);
		return true;
    }
    return false;
}
