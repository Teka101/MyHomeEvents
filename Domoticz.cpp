#include <iostream>
#include <sstream>
#include <time.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <curl/curl.h>
#include "Domoticz.h"

static size_t writeMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	std::stringstream *ss = (std::stringstream *)data;
	size_t length = (size * nmemb);

	ss->write((char *)ptr, length);
	return length;
}

Domoticz::Domoticz(std::string &domoURL, std::string &domoAuth, int deviceIdxDHT22, int deviceIdxHeating, int deviceIdxHeater)
	: serverUrl(domoURL), serverAuth(domoAuth), lastUpdateDevices(0), idxDHT22(deviceIdxDHT22), idxHeating(deviceIdxHeating), idxHeater(deviceIdxHeater)
{
	tzset();
}

Domoticz::~Domoticz()
{
}


void Domoticz::refreshData()
{
	time_t currentTime = time(nullptr);
	time_t diff = currentTime - lastUpdateDevices;
	CURL *curl;

	if (diff < 300)
		return;
	lastUpdateDevices = currentTime;

	curl = curl_easy_init();
	if (curl != nullptr)
	{
		std::stringstream ss;
		std::string url = serverUrl + "json.htm?type=devices&filter=all&used=true&order=Name&plan=1";
		CURLcode res;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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

			devices.erase(devices.begin(), devices.end());
			boost::property_tree::read_json(ss, pTree);

			for (auto it : pTree.get_child("result"))
				if (it.second.size() > 0)
				{
					sDomoticzDevice device;

					for (auto itChild : it.second)
					{
						if (itChild.first == "idx")
							device.idx = itChild.second.get_value<int>();
						else if (itChild.first == "Name")
							device.name = itChild.second.get_value<std::string>();
						else if (itChild.first == "LastUpdate")
						{
							std::string date = itChild.second.get_value<std::string>();
							struct tm tm;

							memset(&tm, 0, sizeof(tm));
							if (strptime(date.c_str(), "%Y-%m-%d %H:%M:%S", &tm) != nullptr)
								device.lastUpdate = mktime(&tm);
						}
						else if (itChild.first == "Status")
						{
							std::string status = itChild.second.get_value<std::string>();

							device.statusIsOn = boost::starts_with(status, "On");
						}
						else if (itChild.first == "Temp")
							device.temperature = itChild.second.get_value<float>();
						else if (itChild.first == "Humidity")
							device.humidity = itChild.second.get_value<float>();
					}
#ifdef DODEBUG
					std::cout << "sDomoticzDevice[idx=" << device.idx << " name=" << device.name << " lastUpdate=" << device.lastUpdate << " temperature="
							<< device.temperature << " humidity=" << device.humidity << " isOn=" << (device.statusIsOn ? "true" : "false") << "]" << std::endl;
#endif
					devices.push_back(device);
				}
		}
		curl_easy_cleanup(curl);
	}
}

void Domoticz::get()
{
	refreshData();
}
