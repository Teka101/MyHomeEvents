#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <curl/curl.h>
#include <microhttpd.h>
#include <sqlite3.h>

static void dumpTree(boost::property_tree::ptree &pTree, int level)
{
	for (auto it : pTree)
	{
		for (int i = 0; i < level; i++)
			std::cout << "\t";
		std::cout << it.first << "[" << it.second.size() << "]="
				<< it.second.get_value<std::string>() << std::endl;
		if (it.second.size() > 0)
			dumpTree(it.second, level++);
	}
}

static size_t writeMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	std::stringstream *ss = (std::stringstream *)data;
	size_t length = (size * nmemb);

	ss->write((char *)ptr, length);
	return length;
}

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
	std::string domoURL, domoAuth;

	desc.add_options()
	    ("Domoticz.url", boost::program_options::value<std::string>(&domoURL), "URL of Domoticz" )
	    ("Domoticz.http_auth", boost::program_options::value<std::string>(&domoAuth),"Auth of Domoticz (if needed)" );


	boost::program_options::variables_map vm;
	std::ifstream settings_file("config.ini", std::ifstream::in);
	boost::program_options::store(boost::program_options::parse_config_file(settings_file, desc, true), vm);
	settings_file.close();
	boost::program_options::notify(vm);

	CURL *curl;
	CURLcode res;
	std::stringstream ss;

	curl = curl_easy_init();
	if (curl != nullptr)
	{
		domoURL += "json.htm?type=devices&filter=all&used=true&order=Name&plan=0";
		curl_easy_setopt(curl, CURLOPT_URL, domoURL.c_str());
		if (domoAuth.size() > 0)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
			curl_easy_setopt(curl, CURLOPT_USERPWD, domoAuth.c_str());
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

			//cout << "===CONTENT===" << endl << ss.str() << endl;
			std::cout << "===DUMP-JSON===" << std::endl;
			boost::property_tree::read_json(ss, pTree);
			dumpTree(pTree, 0);
		}
		curl_easy_cleanup(curl);
	}

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
