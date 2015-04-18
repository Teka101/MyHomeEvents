#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include <sqlite3.h>
#include "DeviceDHT22.h"
#include "Domoticz.h"
#include "WebServer.h"

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
	TestUpdate *tu = new TestUpdate();
	Domoticz *d = new Domoticz(domoURL, domoAuth, domoPlan, domoDeviceIdxDHT22, domoDeviceIdxHeating, domoDeviceIdxHeater);
	DeviceDHT22 *ddht22 = new DeviceDHT22(d, dht22Cmd);
	WebServer *web = new WebServer(webPort);

	d->listenerDHT22.connect(boost::bind(&TestUpdate::hello, tu, _1));
	d->listenerDHT22.connect(&helloMe);
	//d->setValuesDHT22(0, 1, 2);

	std::cout << "Press ENTER to quit..." << std::endl;
	getchar();

	delete web;
	//FIXME bugged delete ddht22;
	delete d;
	delete tu;
	exit(0);

	//
	sqlite3 *db = nullptr;
	int rc;

	rc = sqlite3_open("test.db", &db);
	if (rc != 0)
		std::cerr << "sqlite3_open() failed: " << sqlite3_errmsg(db) << std::endl;
	else
	{
		char *zErrMsg = nullptr;
		char *sql = nullptr;

		sql = "CREATE TABLE COMPANY("  \
		         "ID INT PRIMARY KEY     NOT NULL," \
		         "NAME           TEXT    NOT NULL," \
		         "AGE            INT     NOT NULL," \
		         "ADDRESS        CHAR(50)," \
		         "SALARY         REAL );";

		   /* Execute SQL statement */
		   rc = sqlite3_exec(db, sql, nullptr, nullptr, &zErrMsg);
		   if (rc != SQLITE_OK)
		   {
			   std::cerr << "SQL error: " << zErrMsg << std::endl;
			   sqlite3_free(zErrMsg);
		   }
		   else
			   std::cout << "Table created successfully" << std::endl;
	}
	sqlite3_close(db);
	return 0;
}
