#include <iostream>
#include <sstream>
#include <time.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "CurlHelpers.h"
#include "Domoticz.h"

#define DOMOTICZ_HYSTERESIS 0.5
#define DOMOTICZ_REFRESH_EVERY 300
typedef std::pair<const std::string, boost::property_tree::ptree> ptreePair;

Domoticz::Domoticz(std::string &domoURL, std::string &domoAuth, int planIdx, int deviceIdxDHT22, int deviceIdxHeating, int deviceIdxHeater, int domoDeviceIdxOutdoor)
	: serverUrl(domoURL), serverAuth(domoAuth), plan(planIdx), lastUpdateDevices(0),
	  idxDHT22(deviceIdxDHT22), idxHeating(deviceIdxHeating), idxHeater(deviceIdxHeater), idxOutdoor(domoDeviceIdxOutdoor)
{
	this->log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("Domoticz"));
	tzset();
}

Domoticz::~Domoticz()
{
	this->removeDevices();
}

void Domoticz::removeDevices()
{
	LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("Domoticz::removeDevices - clean devices []:" << this->devices.size()));
	BOOST_FOREACH(sDomoticzDevice *d, this->devices)
		delete d;
	devices.erase(devices.begin(), devices.end());
}


void Domoticz::refreshData()
{
	std::stringstream ss, ssUrl;
	time_t currentTime = time(NULL);
	time_t diff = currentTime - lastUpdateDevices;

	if (diff < DOMOTICZ_REFRESH_EVERY)
	{
		LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("Domoticz::refreshData : data is already up-to-date: " << this->devices.size() << " items"));
		return;
	}
	lastUpdateDevices = currentTime;
	ssUrl << serverUrl << "json.htm?type=devices&filter=all&used=true&order=Name&plan=" << plan;
	if (curlExecute(ssUrl.str(), this->serverAuth, &ss))
	{
		boost::property_tree::ptree pTree;

		boost::property_tree::read_json(ss, pTree);
		this->removeDevices();
		BOOST_FOREACH(ptreePair it, pTree.get_child("result"))
			if (it.second.size() > 0)
			{
				sDomoticzDevice *device = new sDomoticzDevice();

				BOOST_FOREACH(ptreePair itChild, it.second)
				{
					if (itChild.first == "idx")
						device->idx = itChild.second.get_value<int>();
					else if (itChild.first == "Name")
						device->name = itChild.second.get_value<std::string>();
					else if (itChild.first == "LastUpdate")
					{
						std::string date = itChild.second.get_value<std::string>();
						struct tm tm;

						memset(&tm, 0, sizeof(tm));
						tm.tm_zone = *tzname;
						tm.tm_isdst = daylight;
						if (strptime(date.c_str(), "%Y-%m-%d %H:%M:%S", &tm) != NULL)
							device->lastUpdate = mktime(&tm);
					}
					else if (itChild.first == "Status")
					{
						std::string status = itChild.second.get_value<std::string>();

						device->statusIsOn = boost::starts_with(status, "On");
					}
					else if (itChild.first == "Temp")
						device->temperature = itChild.second.get_value<float>();
					else if (itChild.first == "Humidity")
						device->humidity = itChild.second.get_value<float>();
				}
				LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("Domoticz::refreshData - sDomoticzDevice[idx=" << device->idx << " name=" << device->name
									<< " lastUpdate=" << device->lastUpdate << " temperature=" << device->temperature
									<< " humidity=" << device->humidity << " isOn=" << (device->statusIsOn ? "true" : "false") << "]"));
				this->devices.push_back(device);
			}
	}
}

sDomoticzDevice *Domoticz::getDeviceDTH22()
{
	refreshData();
	BOOST_FOREACH(sDomoticzDevice *d, this->devices)
		if (d->idx == this->idxDHT22)
			return d;
	return NULL;
}

sDomoticzDevice *Domoticz::getDeviceOutdoor()
{
	refreshData();
	BOOST_FOREACH(sDomoticzDevice *d, this->devices)
		if (d->idx == this->idxOutdoor)
			return d;
	return NULL;
}

bool Domoticz::setValuesDHT22(time_t timestamp, float temperature, float humidity)
{
	std::stringstream ssUrl;

	ssUrl << this->serverUrl << "json.htm?type=command&param=udevice&idx=" << this->idxDHT22 << "&nvalue=0&svalue=" << temperature << ';' << humidity << ";2";
	if (curlExecute(ssUrl.str(), this->serverAuth))
	{
		sDomoticzDevice *dev = this->getDeviceDTH22();

		if (dev != NULL)
		{
			dev->lastUpdate = timestamp;
			dev->humidity = humidity;
			dev->temperature = temperature;
			this->listenerDHT22(dev);
		}
		return true;
	}
	return false;
}

bool Domoticz::setValuesHeating(time_t timestamp, float temperature)
{
	std::stringstream ssUrl;

	ssUrl << this->serverUrl << "json.htm?type=command&param=udevice&idx=" << this->idxHeating << "&nvalue=0&svalue=" << temperature;
	if (curlExecute(ssUrl.str(), this->serverAuth))
	{
		sDomoticzDevice *dev = this->getDeviceDTH22();

		if (dev != NULL)
		{
			dev->lastUpdate = timestamp;
			dev->temperature = temperature;
		}
		return true;
	}
	return false;
}

bool Domoticz::setStatusHeater(float temperatureMeasured, float temperatureOrder)
{
	if (temperatureMeasured <= (temperatureOrder - DOMOTICZ_HYSTERESIS))
		return this->setStatusHeater(true);
	else if (temperatureMeasured >= (temperatureOrder + DOMOTICZ_HYSTERESIS))
		return this->setStatusHeater(false);
	return false;
}

bool Domoticz::setStatusHeater(bool activate)
{
	std::stringstream ssUrl;

	ssUrl << this->serverUrl << "json.htm?type=command&param=switchlight&idx=" << this->idxHeater << "&switchcmd=" << (activate ? "On" : "Off") << "&level=0";
	return curlExecute(ssUrl.str(), this->serverAuth);
}
