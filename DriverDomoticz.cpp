#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
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
    return new DeviceDomoticz(_hw.param1, dev);
}

DeviceDomoticz::DeviceDomoticz(std::string &urlDomoticz, DBDevice &dev) : MHEDevice(dev.id, dev.type, dev.cacheLifetime), _urlDomoticz(urlDomoticz), _deviceIdx(dev.param1)
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("DeviceDomoticz"));
}

DeviceDomoticz::~DeviceDomoticz()
{
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

bool DeviceDomoticz::setTempHum(float temperature, float humidity)
{
    std::stringstream ssUrl;

	ssUrl << _urlDomoticz << "json.htm?type=command&param=udevice&idx=" << _deviceIdx << "&nvalue=0&svalue=" << temperature << ';' << humidity << ";2";
	if (curlExecute(ssUrl.str()))
	{
        time_t now = time(NULL);

        _cache.lastUpdate = now;
		_cache.humidity = humidity;
		_cache.temperature = temperature;
		return true;
	}
	return false;
}

bool DeviceDomoticz::isActivated()
{
    refreshCache();
    return _cache.statusIsOn;
}

bool DeviceDomoticz::setStatus(bool activate)
{
    std::stringstream ssUrl;

	ssUrl << _urlDomoticz << "json.htm?type=command&param=switchlight&idx=" << _deviceIdx << "&switchcmd=" << (activate ? "On" : "Off") << "&level=0";
	if (curlExecute(ssUrl.str()))
	{
        time_t now = time(NULL);

        _cache.lastUpdate = now;
        _cache.statusIsOn = activate;
        return true;
	}
    return false;
}


void DeviceDomoticz::refreshCache()
{
    time_t now = time(NULL) - _cacheLifetime;

    if (_cache.lastUpdate < now)
    {
        std::stringstream ssUrl;
        std::stringstream ssOut;

        ssUrl << _urlDomoticz << "json.htm?type=devices&rid=" << _deviceIdx;
        if (curlExecute(ssUrl.str(), &ssOut))
        {
            boost::property_tree::ptree pTree;

            boost::property_tree::read_json(ssOut, pTree);
            if (pTree.count("result") == 0)
                return;
            BOOST_FOREACH(ptreePair &it, pTree.get_child("result"))
            {
                if (it.second.size() > 0)
                {
                    BOOST_FOREACH(ptreePair &itChild, it.second)
                    {
                        if (itChild.first == "Name")
                            _cache.name = itChild.second.get_value<std::string>();
                        else if (itChild.first == "LastUpdate")
                        {
                            std::string date = itChild.second.get_value<std::string>();
                            struct tm tm;

                            memset(&tm, 0, sizeof(tm));
                            tm.tm_zone = *tzname;
                            tm.tm_isdst = daylight;
                            if (strptime(date.c_str(), "%Y-%m-%d %H:%M:%S", &tm) != NULL)
                                _cache.lastUpdate = mktime(&tm);
                        }
                        else if (itChild.first == "Status")
                        {
                            std::string status = itChild.second.get_value<std::string>();

                            _cache.statusIsOn = boost::starts_with(status, "On");
                        }
                        else if (itChild.first == "Temp")
                            _cache.temperature = itChild.second.get_value<float>();
                        else if (itChild.first == "Humidity")
                            _cache.humidity = itChild.second.get_value<float>();
                    }
                    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("DeviceDomoticz::refreshCache - Refresh(" << (std::string)*this << ") = " << (std::string)_cache));
                }
            }
        }
    }
}
