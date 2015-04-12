#include <cmath>
#include <iostream>
#include <stdio.h>
#include <time.h>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "DeviceDHT22.h"

#define DHT2_REFRESH_EVERY 300

DeviceDHT22::DeviceDHT22(Domoticz *domoticz, std::string &execCmd) : domo(domoticz), command(execCmd)
{
	tzset();
	this->timer = new boost::asio::deadline_timer(io);
	this->computeNextLaunch();
	this->threads.create_thread(boost::bind(&boost::asio::io_service::run, &this->io));
}

DeviceDHT22::~DeviceDHT22()
{
	this->threads.interrupt_all();
	delete this->timer;
}

void DeviceDHT22::computeNextLaunch()
{
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	long currentMS = now.time_of_day().minutes() * 60L + now.time_of_day().seconds();

#ifdef DODEBUG
	std::cout << "DeviceDHT22::computeNextLaunch() - currentTime=" << to_simple_string(now.time_of_day());
#endif
	currentMS = DHT2_REFRESH_EVERY - (currentMS % DHT2_REFRESH_EVERY);
#ifdef DODEBUG
	std::cout << " waiting=" << currentMS << std::endl;
#endif
	this->timer->expires_from_now(boost::posix_time::seconds(currentMS));
	this->timer->async_wait(boost::bind(&DeviceDHT22::launch, this));
}

void DeviceDHT22::launch()
{
	FILE *fh;

	fh = popen(this->command.c_str(), "r");
	if (fh == nullptr)
		std::cerr << "popen() failed" << std::endl;
	else
	{
		boost::regex expression("^Humidity = (.+) % Temperature = (.+) \\*C.*$", boost::regex::perl);
		boost::cmatch what;
		char line[1024];
		int execReturn;

		while (fgets(line, sizeof(line), fh) != nullptr)
			if (boost::regex_match(line, what, expression))
			{
				float humidity = boost::lexical_cast<float>(what[1]);
				float temperature = boost::lexical_cast<float>(what[2]);

				this->updateData(temperature, humidity);
			}
		execReturn = pclose(fh);
#ifdef DODEBUG
		std::cout << "DeviceDHT22::launch() - Exec return: " << execReturn << std::endl;
#endif
	}
	this->computeNextLaunch();
}

void DeviceDHT22::updateData(float temperature, float humidity)
{
	sDomoticzDevice *dev = domo->getDeviceDTH22();
	time_t now = time(nullptr);

	if (dev != nullptr)
	{
		time_t diffDate = std::abs(now - dev->lastUpdate);
		float diffTemperature = std::abs(temperature - dev->temperature);
		float diffHumidity = std::abs(humidity - dev->humidity);

		if (diffDate >= 1200 || (diffHumidity <= 15.0 && diffTemperature <= 5.0))
			domo->setValuesDHT22(now, temperature, humidity);
		else
			std::cerr << "DeviceDHT22::updateData - invalid value : temperature=" << temperature << " d" << diffTemperature << " humdity=" << humidity << " d" << diffHumidity << std::endl;
	}
}
