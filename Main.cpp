#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include <microhttpd.h>
#include <sqlite3.h>
#include "Domoticz.h"

static int answer_to_connection(void *cls, struct MHD_Connection *connection,
		const char *url, const char *method, const char *version,
		const char *upload_data, size_t *upload_data_size, void **con_cls)
{
	const char *page = "<html><body>Hello, browser!</body></html>";
	struct MHD_Response *response;
	int ret;

	std::cout << "HTTP-request: url=" << url << " method=" << method << " version=" << version << std::endl;
	//MHD_create_response_from_fd
	//MHD_add_response_header
	response = MHD_create_response_from_buffer(strlen(page), (void *) page, MHD_RESPMEM_PERSISTENT);
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

int main(int ac, char **av)
{
	boost::program_options::options_description desc("Options");
	std::string domoURL, domoAuth, dht22Cmd;
	int domoDeviceIdxDHT22, domoDeviceIdxHeating, domoDeviceIdxHeater;

	desc.add_options()
	    ("Domoticz.url", boost::program_options::value<std::string>(&domoURL),  "URL of Domoticz")
	    ("Domoticz.http_auth", boost::program_options::value<std::string>(&domoAuth), "Auth of Domoticz (if needed)")
	    ("Domoticz.device_idx_dht22", boost::program_options::value<int>(&domoDeviceIdxDHT22)->default_value(-1), "Device Index in Domoticz")
	    ("Domoticz.device_idx_heating", boost::program_options::value<int>(&domoDeviceIdxHeating)->default_value(-1), "Device Index in Domoticz")
	    ("Domoticz.device_idx_heater", boost::program_options::value<int>(&domoDeviceIdxHeater)->default_value(-1), "Device Index in Domoticz")
	    ("DHT22.command", boost::program_options::value<std::string>(&dht22Cmd), "Command to read DTH22")
	    ;

	boost::program_options::variables_map vm;
	std::ifstream settings_file("config.ini", std::ifstream::in);
	boost::program_options::store(boost::program_options::parse_config_file(settings_file, desc, true), vm);
	settings_file.close();
	boost::program_options::notify(vm);

	//
	Domoticz *d = new Domoticz(domoURL, domoAuth, domoDeviceIdxDHT22, domoDeviceIdxHeating, domoDeviceIdxHeater);

	d->get();
	exit(0);

	//
	FILE *fh;

	fh = popen(dht22Cmd.c_str(), "r");
	if (fh == nullptr)
		std::cerr << "popen() failed" << std::endl;
	else
	{
		char line[1024];
		int execReturn;

		while (fgets(line, sizeof(line), fh) != nullptr)
			std::cout << "PIPE: " << line;
		execReturn = pclose(fh);
		std::cout << "Exec return: " << execReturn << std::endl;
	}

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


	struct MHD_Daemon *daemon;

	daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8080, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_END);
	if (daemon == nullptr)
		std::cerr << "MHD_start_daemon() error" << std::endl;
	else
		getchar();
	MHD_stop_daemon(daemon);
	return 0;
}
