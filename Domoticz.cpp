#include <iostream>
#include <sstream>
#include <time.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <curl/curl.h>
#include "Domoticz.h"

#define DOMOTICZ_REFRESH_EVERY 300

static size_t writeMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	std::stringstream *ss = (std::stringstream *)data;
	size_t length = (size * nmemb);

	ss->write((char *)ptr, length);
	return length;
}

Domoticz::Domoticz(std::string &domoURL, std::string &domoAuth, int planIdx, int deviceIdxDHT22, int deviceIdxHeating, int deviceIdxHeater)
	: serverUrl(domoURL), serverAuth(domoAuth), plan(planIdx), lastUpdateDevices(0), idxDHT22(deviceIdxDHT22), idxHeating(deviceIdxHeating), idxHeater(deviceIdxHeater)
{
	tzset();
}

Domoticz::~Domoticz()
{
	this->removeDevices();
}

void Domoticz::removeDevices()
{
#ifdef DODEBUG
	std::cout << "Domoticz::removeDevices() - clean devices []:" << this->devices.size() << std::endl;
#endif
	for (sDomoticzDevice *d : this->devices)
		delete d;
	devices.erase(devices.begin(), devices.end());
}


void Domoticz::refreshData()
{
	time_t currentTime = time(nullptr);
	time_t diff = currentTime - lastUpdateDevices;
	CURL *curl;

	if (diff < DOMOTICZ_REFRESH_EVERY)
		return;
	lastUpdateDevices = currentTime;

	curl = curl_easy_init();
	if (curl != nullptr)
	{
		std::stringstream ss, ssUrl;
		CURLcode res;

		ssUrl << serverUrl << "json.htm?type=devices&filter=all&used=true&order=Name&plan=" << plan;
		curl_easy_setopt(curl, CURLOPT_URL, ssUrl.str().c_str());
		if (serverAuth.size() > 0)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
			curl_easy_setopt(curl, CURLOPT_USERPWD, serverAuth.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&ss);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		else
		{
			boost::property_tree::ptree pTree;

			boost::property_tree::read_json(ss, pTree);
			this->removeDevices();
			for (auto it : pTree.get_child("result"))
				if (it.second.size() > 0)
				{
					sDomoticzDevice *device = new sDomoticzDevice();

					for (auto itChild : it.second)
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
							if (strptime(date.c_str(), "%Y-%m-%d %H:%M:%S", &tm) != nullptr)
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
#ifdef DODEBUG
					std::cout << "Domoticz::refreshData() - sDomoticzDevice[idx=" << device->idx << " name=" << device->name << " lastUpdate=" << device->lastUpdate << " temperature="
							<< device->temperature << " humidity=" << device->humidity << " isOn=" << (device->statusIsOn ? "true" : "false") << "]" << std::endl;
#endif
					devices.push_back(device);
				}
		}
		curl_easy_cleanup(curl);
	}
}

sDomoticzDevice *Domoticz::getDeviceDTH22()
{
	refreshData();
	for (sDomoticzDevice *d : this->devices)
		if (d->idx == this->idxDHT22)
			return d;
	return nullptr;
}

bool Domoticz::setValuesDHT22(time_t timestamp, float temperature, float humidity)
{
	std::stringstream ss, ssURL;
	CURL *curl;

	ssURL << this->serverUrl << "json.htm?type=command&param=udevice&idx=" << this->idxDHT22 << "&nvalue=0&svalue=" << temperature << ';' << humidity << ";2";
	curl = curl_easy_init();
	if (curl != nullptr)
	{
		CURLcode res;

		curl_easy_setopt(curl, CURLOPT_URL, ssURL.str().c_str());
		if (serverAuth.size() > 0)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
			curl_easy_setopt(curl, CURLOPT_USERPWD, serverAuth.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&ss);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		else
		{
			long httpCode;

#ifdef DODEBUG
			std::cout << "Domoticz::setValuesDHT22 - result:" << std::endl << "[[" << ss.str() << "]]" << std::endl;
#endif
			res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
			if (res == CURLE_OK)
				if (httpCode == 200)
				{
					sDomoticzDevice *dev = this->getDeviceDTH22();

					if (dev != nullptr)
					{
						dev->lastUpdate = timestamp;
						dev->humidity = humidity;
						dev->temperature = temperature;
						this->listenerDHT22(dev);
					}
					return true;
				}
		}
	}
	return false;
}

bool Domoticz::setStatusHeater(bool activate)
{
	std::stringstream ss, ssURL;
	CURL *curl;

	ssURL << this->serverUrl << "json.htm?type=command&param=switchlight&idx=" << this->idxHeater << "&switchcmd=" << (activate ? "On" : "Off") << "&level=0";
	curl = curl_easy_init();
	if (curl != nullptr)
	{
		CURLcode res;

		curl_easy_setopt(curl, CURLOPT_URL, ssURL.str().c_str());
		if (serverAuth.size() > 0)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
			curl_easy_setopt(curl, CURLOPT_USERPWD, serverAuth.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&ss);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		else
		{
			long httpCode;

	#ifdef DODEBUG
			std::cout << "Domoticz::setStatusHeater - result:" << std::endl << "[[" << ss.str() << "]]" << std::endl;
	#endif
			res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
			if (res == CURLE_OK)
				return (httpCode == 200);
		}
	}
	return false;
}
