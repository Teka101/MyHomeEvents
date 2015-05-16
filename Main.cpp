#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include "Brain.h"
#include "DeviceDHT22.h"
#include "Domoticz.h"

static void helloMe(sDomoticzDevice *dev)
{
	std::cout << "YOUHOU ! temp=" << dev->temperature << " hum=" << dev->humidity << std::endl;
}

int main(int ac, char **av)
{
	boost::program_options::options_description desc("ConfigFile");
	boost::program_options::variables_map vm;
	std::string domoURL, domoAuth, dht22Cmd;
	int dht22Speed, domoPlan, domoDeviceIdxDHT22, domoDeviceIdxHeating, domoDeviceIdxHeater, domoDeviceIdxOutdoor;
	int webPort;

	desc.add_options()
		("WebServer.port", boost::program_options::value<int>(&webPort)->default_value(8080))
	    ("Domoticz.url", boost::program_options::value<std::string>(&domoURL))
	    ("Domoticz.http_auth", boost::program_options::value<std::string>(&domoAuth))
	    ("Domoticz.plan", boost::program_options::value<int>(&domoPlan)->default_value(0))
	    ("Domoticz.device_idx_dht22", boost::program_options::value<int>(&domoDeviceIdxDHT22)->default_value(-1))
	    ("Domoticz.device_idx_heating", boost::program_options::value<int>(&domoDeviceIdxHeating)->default_value(-1))
	    ("Domoticz.device_idx_heater", boost::program_options::value<int>(&domoDeviceIdxHeater)->default_value(-1))
	    ("Domoticz.device_idx_outdoor", boost::program_options::value<int>(&domoDeviceIdxOutdoor)->default_value(-1))
	    ("DHT22.command", boost::program_options::value<std::string>(&dht22Cmd))
	    ("DHT22.refresh", boost::program_options::value<int>(&dht22Speed)->default_value(300))
	    ;
	std::ifstream settings_file("config.ini", std::ifstream::in);
	boost::program_options::store(boost::program_options::parse_config_file(settings_file, desc, true), vm);
	settings_file.close();
	boost::program_options::notify(vm);

	//
	Domoticz *d = new Domoticz(domoURL, domoAuth, domoPlan, domoDeviceIdxDHT22, domoDeviceIdxHeating, domoDeviceIdxHeater, domoDeviceIdxOutdoor);
	Brain *brain = new Brain(webPort, d);
	DeviceDHT22 *dht22 = new DeviceDHT22(d, dht22Cmd, dht22Speed);

	d->listenerDHT22.connect(boost::bind(&Brain::update, brain, _1));
	d->listenerDHT22.connect(&helloMe);

	//db->getGraphs();
	//d->setValuesDHT22(0, 1, 2);

	std::cout << "Press ENTER to quit..." << std::endl;
	getchar();

	delete dht22;//FIXME bugged delete
	delete d;
	delete brain;
	return 0;
}
