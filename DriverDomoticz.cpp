/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <log4cplus/loggingmacros.h>
#include <cmath>
#include <math.h>
#include <time.h>
#include "CurlHelpers.h"
#include "DriverDomoticz.h"

typedef std::pair<const std::string, boost::property_tree::ptree> ptreePair;

HardwareDomoticz::HardwareDomoticz(DBHarware &hw) : MHEHardware(hw.id), _hw(hw)
{
}

HardwareDomoticz::~HardwareDomoticz()
{
}

MHEDevice *HardwareDomoticz::buildObject(DBDevice &dev)
{
    return new DeviceDomoticz(_hw.param1, (dev.param2 == "24havg"), dev);
}

DeviceDomoticz::DeviceDomoticz(std::string &urlDomoticz, bool isDayAverage, DBDevice &dev) : MHEDevice(dev.id, dev.type, dev.name, dev.cacheLifetime, dev.hidden, dev.cacheRunning), _urlDomoticz(urlDomoticz), _deviceIdx(dev.param1), _isDayAverage(isDayAverage)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("DeviceDomoticz"));
}

DeviceDomoticz::~DeviceDomoticz()
{
}

float DeviceDomoticz::getCachedRawTemperature()
{
    return _cache.temperatureRaw;
}

float DeviceDomoticz::getCachedTemperature()
{
    return _cache.temperature;
}

float DeviceDomoticz::getCachedHumidity()
{
    return _cache.humidity;
}

bool DeviceDomoticz::isCachedDimmable()
{
    return _cache.dimmable;
}

int DeviceDomoticz::getCachedDimmableValue()
{
    return _cache.dimmableValue;
}

int DeviceDomoticz::getCachedDimmableMax()
{
    return _cache.dimmableMax;
}

bool DeviceDomoticz::isCachedActivated()
{
    return _cache.statusIsOn;
}

float DeviceDomoticz::getRawTemperature()
{
    refreshCache();
    return _cache.temperatureRaw;
}

float DeviceDomoticz::getTemperature()
{
    refreshCache();
    return _cache.temperature;
}

float DeviceDomoticz::getHumidity()
{
    refreshCache();
    return _cache.humidity;
}

bool DeviceDomoticz::isDimmable()
{
    refreshCache();
    return _cache.dimmable;
}

int DeviceDomoticz::getDimmableValue()
{
    refreshCache();
    return _cache.dimmableValue;
}

int DeviceDomoticz::getDimmableMax()
{
    refreshCache();
    return _cache.dimmableMax;
}

bool DeviceDomoticz::setTempHum(float temperature, float humidity)
{
    std::stringstream ssUrl;

    ssUrl << _urlDomoticz << "json.htm?type=command&param=udevice&idx=" << _deviceIdx << "&nvalue=0&svalue=" << temperature << ';' << humidity << ";2";
    if (curlExecute(ssUrl.str()))
    {
        time_t now = time(NULL);

        _lastUpdate = now;
        _cache.humidity = humidity;
        _cache.temperature = temperature;
        _cache.temperatureRaw = temperature;
        if (_cloneTo != NULL)
            _cloneTo->setTempHum(temperature, humidity);
        return true;
    }
    return false;
}

bool DeviceDomoticz::isActivated()
{
    refreshCache();
    return _cache.statusIsOn;
}

bool DeviceDomoticz::setLevel(int level)
{
    std::stringstream ssUrl;

	ssUrl << _urlDomoticz << "json.htm?type=command&param=switchlight&idx=" << _deviceIdx << "&switchcmd=Set%20Level&level=" << level;
	if (curlExecute(ssUrl.str()))
	{
        time_t now = time(NULL);

        _lastUpdate = now;
        _cache.statusIsOn = level > 0;
        _cache.dimmableValue = level;
        if (_cloneTo != NULL)
            _cloneTo->setLevel(level);
        return true;
	}
    return false;
}

bool DeviceDomoticz::setStatus(bool activate)
{
    std::stringstream ssUrl;

	ssUrl << _urlDomoticz << "json.htm?type=command&param=switchlight&idx=" << _deviceIdx << "&switchcmd=" << (activate ? "On" : "Off") << "&level=0";
	if (curlExecute(ssUrl.str()))
	{
        time_t now = time(NULL);

        _lastUpdate = now;
        _cache.statusIsOn = activate;
        if (_cloneTo != NULL)
            _cloneTo->setStatus(activate);
        return true;
	}
    return false;
}

bool DeviceDomoticz::sendCommand(const std::string &command, const std::string &value, void *output)
{
    tMHEDeviceValues *values = static_cast<tMHEDeviceValues*>(output);
    std::stringstream ssUrl;

    if (command == "values" && value == "last24h" && output != NULL)
    {
        std::string fieldDate;
        std::string dateFormat;

        if (_type == devTempHum)
        {
            ssUrl << _urlDomoticz << "json.htm?type=graph&sensor=temp&idx=" << _deviceIdx << "&range=day";
            fieldDate = "d";
            dateFormat = "%Y-%m-%d %H:%M";
        }
        else
        {
            ssUrl << _urlDomoticz << "json.htm?type=lightlog&idx=" << _deviceIdx;
            fieldDate = "Date";
            dateFormat = "%Y-%m-%d %H:%M:%S";
        }
        return parseGraphData(ssUrl.str(), values, fieldDate, dateFormat);
    }
    else if (command == "values" && value == "lastMonth" && output != NULL && _type == devTempHum)
    {
        ssUrl << _urlDomoticz << "json.htm?type=graph&sensor=temp&idx=" << _deviceIdx << "&range=month";
        return parseGraphData(ssUrl.str(), values, "d", "%Y-%m-%d");
    }
    else if (command == "values" && value == "lastYear" && output != NULL && _type == devTempHum)
    {
        ssUrl << _urlDomoticz << "json.htm?type=graph&sensor=temp&idx=" << _deviceIdx << "&range=year";
        return parseGraphData(ssUrl.str(), values, "d", "%Y-%m-%d");
    }
    return false;
}

bool DeviceDomoticz::parseGraphData(const std::string &url, tMHEDeviceValues *values, const std::string &fieldDate, const std::string &dateFormat)
{
    std::stringstream ssOut;

    if (curlExecute(url, &ssOut))
    {
        boost::property_tree::ptree pTree;

        boost::property_tree::read_json(ssOut, pTree);
        if (pTree.count("result") == 0)
            return false;
        BOOST_FOREACH(ptreePair it, pTree.get_child("result"))
        {
            std::string dateStr = it.second.get(fieldDate, "");
            float temperature = it.second.get<float>("te", NAN);
            float humidity = it.second.get<float>("hu", NAN);
            std::string status = it.second.get("Status", "");
            struct tm tm;

            memset(&tm, 0, sizeof(tm));
            tm.tm_zone = *tzname;
            tm.tm_isdst = daylight;
            if (strptime(dateStr.c_str(), dateFormat.c_str(), &tm) != NULL)
            {
                sMHEDeviceValue v;

                v.date = mktime(&tm);
                v.temperature = temperature;
                v.humdity = humidity;
                if (status.size() > 0)
                {
                    if (boost::starts_with(status, "On"))
                        v.status = "on";
                    else
                        v.status = "off";
                }
                values->push_back(v);
            }
        }
        return true;
    }
    return false;
}

void DeviceDomoticz::refreshCache()
{
    time_t now = time(NULL) - _cacheLifetime;

    if (_lastUpdate < now)
    {
        bool isSuccess = false;

        if (_isDayAverage)
            isSuccess = refreshDayAverage();
        else
            isSuccess = refreshNormal();
        if (isSuccess)
        {
            _lastUpdate = time(NULL);
            LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("DeviceDomoticz::refreshCache - Refresh(" << (std::string)*this << ") = " << (std::string)_cache));
            if (_cloneTo != NULL)
                _cloneTo->setTempHum(_cache.temperature, _cache.humidity);
        }
    }
}

bool DeviceDomoticz::refreshNormal()
{
    std::stringstream ssOut, ssUrl;

    ssUrl << _urlDomoticz << "json.htm?type=devices&rid=" << _deviceIdx;
    if (curlExecute(ssUrl.str(), &ssOut))
    {
        boost::property_tree::ptree pTree;

        boost::property_tree::read_json(ssOut, pTree);
        if (pTree.count("result") == 0)
            return false;
        BOOST_FOREACH(ptreePair &it, pTree.get_child("result"))
        {
            if (it.second.size() > 0)
            {
                BOOST_FOREACH(ptreePair &itChild, it.second)
                {
                    if (itChild.first == "Name")
                        _cache.name = itChild.second.get_value<std::string>();
                    else if (itChild.first == "Status")
                    {
                        std::string status = itChild.second.get_value<std::string>();

                        _cache.statusIsOn = boost::starts_with(status, "On");
                    }
                    else if (itChild.first == "Temp")
                    {
                        _cache.temperature = itChild.second.get_value<float>();
                        _cache.temperatureRaw = _cache.temperature;
                    }
                    else if (itChild.first == "Humidity")
                        _cache.humidity = itChild.second.get_value<float>();
                    else if (itChild.first == "LevelInt")
                        _cache.dimmableValue = itChild.second.get_value<int>();
                    else if (itChild.first == "MaxDimLevel")
                        _cache.dimmableMax = itChild.second.get_value<int>();
                    else if (itChild.first == "SwitchType")
                    {
                        std::string status = itChild.second.get_value<std::string>();

                        _cache.dimmable = boost::starts_with(status, "Dimmer");
                    }
                }
            }
        }
        if (!_cache.statusIsOn)
            _cache.dimmableValue = 0;
        return true;
    }
    return false;
}

bool DeviceDomoticz::refreshDayAverage()
{
    std::stringstream ssOut, ssUrl;

    ssUrl << _urlDomoticz << "json.htm?type=graph&sensor=temp&idx=" << _deviceIdx << "&range=day";
    if (curlExecute(ssUrl.str(), &ssOut))
    {
        boost::property_tree::ptree pTree;
        double tempTotal = 0.0, tempNumber = 0.0;
        float lastTemperature = 0.0;

        boost::property_tree::read_json(ssOut, pTree);
        if (pTree.count("result") == 0)
            return false;
        BOOST_FOREACH(ptreePair it, pTree.get_child("result"))
        {
            float currentTemp = it.second.get<float>("te", NAN);

            if (std::isnormal(currentTemp))
            {
                tempTotal += currentTemp;
                tempNumber++;
                lastTemperature = currentTemp;
            }
        }
        LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("DeviceDomoticz::refreshDayAverage - tempReturn=" << (tempTotal / tempNumber) << " tempTotal=" << tempTotal << " tempNumber=" << tempNumber << "]"));
        if (tempNumber > 0)
        {
            _cache.temperature = trunc((tempTotal / tempNumber) * 100.0) / 100;
            _cache.temperatureRaw = lastTemperature;
            return true;
        }
    }
    return false;
}
