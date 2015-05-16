#include <cmath>
#include <iostream>
#include <stdio.h>
#include <time.h>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "DeviceDHT22.h"

DeviceDHT22::DeviceDHT22(Domoticz *domoticz, std::string &execCmd, int refreshInSeconds) : domo(domoticz), command(execCmd), refreshInSeconds(refreshInSeconds)
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
	currentMS = this->refreshInSeconds - (currentMS % this->refreshInSeconds);
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
	if (fh == NULL)
		std::cerr << "popen() failed" << std::endl;
	else
	{
		boost::regex expression("^Humidity = (.+) % Temperature = (.+) \\*C.*$", boost::regex::perl);
		boost::cmatch what;
		char line[1024];
		int execReturn;

		while (fgets(line, sizeof(line), fh) != NULL)
			if (boost::regex_match(line, what, expression))
			{
				float humidity = boost::lexical_cast<float>(what[1]);
				float temperature = boost::lexical_cast<float>(what[2]);

#ifdef DODEBUG
		std::cout << "DeviceDHT22::launch() - Update: humidity=" << humidity << " temperature=" << temperature << std::endl;
#endif
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
	time_t now = time(NULL);

	if (dev != NULL)
	{
		time_t diffDate = std::abs(now - dev->lastUpdate);
		float diffTemperature = std::abs(temperature - dev->temperature);
		float diffHumidity = std::abs(humidity - dev->humidity);

		if (diffDate >= 1200 || (diffHumidity <= 15.0 && diffTemperature <= 5.0))
			domo->setValuesDHT22(now, temperature, humidity);
		else
			std::cerr << "DeviceDHT22::updateData - invalid value : temperature=" << temperature << " delta:" << diffTemperature << " humdity=" << humidity << " delta:" << diffHumidity << " last delta-time=" << diffDate << std::endl;
	}
#ifdef DODEBUG
	else
		std::cout << "DeviceDHT22::updateData() - No device DHT22 founded !?!" << std::endl;
#endif
}
