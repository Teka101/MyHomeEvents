#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include "Brain.h"
#include "DeviceDHT22.h"
#include "Domoticz.h"

class TestUpdate
{
public:
	TestUpdate(){}
	~TestUpdate(){}

	//void operator()(sDomoticzDevice *dev) const
	void hello(sDomoticzDevice *dev)
	{
		std::cout << "YOUHOU ! temp=" << dev->temperature << " hum=" << dev->humidity << std::endl;
	}
};

static void helloMe(sDomoticzDevice *dev)
{
	std::cout << "YOUHOU ! temp=" << dev->temperature << " hum=" << dev->humidity << std::endl;
}

int main(int ac, char **av)
{
	boost::program_options::options_description desc("ConfigFile");
	boost::program_options::variables_map vm;
	std::string domoURL, domoAuth, dht22Cmd;
	int domoPlan, domoDeviceIdxDHT22, domoDeviceIdxHeating, domoDeviceIdxHeater;
	int webPort;

	desc.add_options()
		("WebServer.port", boost::program_options::value<int>(&webPort)->default_value(8080))
	    ("Domoticz.url", boost::program_options::value<std::string>(&domoURL))
	    ("Domoticz.http_auth", boost::program_options::value<std::string>(&domoAuth))
	    ("Domoticz.plan", boost::program_options::value<int>(&domoPlan)->default_value(0))
	    ("Domoticz.device_idx_dht22", boost::program_options::value<int>(&domoDeviceIdxDHT22)->default_value(-1))
	    ("Domoticz.device_idx_heating", boost::program_options::value<int>(&domoDeviceIdxHeating)->default_value(-1))
	    ("Domoticz.device_idx_heater", boost::program_options::value<int>(&domoDeviceIdxHeater)->default_value(-1))
	    ("DHT22.command", boost::program_options::value<std::string>(&dht22Cmd))
	    ;
	std::ifstream settings_file("config.ini", std::ifstream::in);
	boost::program_options::store(boost::program_options::parse_config_file(settings_file, desc, true), vm);
	settings_file.close();
	boost::program_options::notify(vm);

	//
	Brain *brain = new Brain(webPort);
	TestUpdate *tu = new TestUpdate();
	Domoticz *d = new Domoticz(domoURL, domoAuth, domoPlan, domoDeviceIdxDHT22, domoDeviceIdxHeating, domoDeviceIdxHeater);
	//DeviceDHT22 *ddht22 = new DeviceDHT22(d, dht22Cmd);

	d->listenerDHT22.connect(boost::bind(&TestUpdate::hello, tu, _1));
	d->listenerDHT22.connect(&helloMe);

	brain->doMe(22, 10);
	//db->getGraphs();
	//d->setValuesDHT22(0, 1, 2);

	std::cout << "Press ENTER to quit..." << std::endl;
	getchar();

	//FIXME bugged delete dht22;
	delete d;
	delete tu;
	delete brain;
	return 0;
}
