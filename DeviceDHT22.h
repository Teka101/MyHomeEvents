#ifndef DEVICEDHT22_H_
#define DEVICEDHT22_H_

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include "Domoticz.h"

class DeviceDHT22
{
private:
	Domoticz *domo;
	boost::asio::io_service io;
	boost::asio::deadline_timer *timer;
	boost::thread_group threads;
	std::string command;
	int refreshInSeconds;

	void computeNextLaunch();
	void launch();
	void updateData(float temperature, float humidity);

public:
	DeviceDHT22(Domoticz *domoticz, std::string &execCmd, int refreshInSeconds);
	~DeviceDHT22();
};

#endif /* DEVICEDHT22_H_ */
