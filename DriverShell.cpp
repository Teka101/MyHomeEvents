#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
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

DeviceShell::DeviceShell(DBDevice &dev) : MHEDevice(dev.id, dev.type, dev.cacheLifetime), _shellCmd(dev.param1), _lastUpdate(0), _temperature(0.0), _humidity(0.0)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("DeviceShell"));
}

DeviceShell::~DeviceShell()
{
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
            boost::regex expression("^Humidity = (.+) % Temperature = (.+) \\*C.*$", boost::regex::perl);
            boost::cmatch what;
            char line[1024];
            int execReturn;

            while (fgets(line, sizeof(line), fh) != NULL)
                if (boost::regex_match(line, what, expression))
                {
                    _humidity = boost::lexical_cast<float>(what[1]);
                    _temperature = boost::lexical_cast<float>(what[2]);
                    now = time(NULL);
                    _lastUpdate = now;
                    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("DeviceShell::refreshCache - Update: humidity=" << _humidity << " temperature=" << _temperature));
                }
            execReturn = pclose(fh);
            LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("DeviceShell::refreshCache - Exec return: " << execReturn));
        }
    }
}
