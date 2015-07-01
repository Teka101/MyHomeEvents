#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#include "Brain.h"
#include "CurlHelpers.h"
#include "MHEHardDevContainer.h"
#include "MHEDatabase.h"
#include "MHEHardware.h"
#include "MHEWeb.h"

static log4cplus::Logger logger;

int main(int ac, char **av)
{
    boost::program_options::options_description desc("ConfigFile");
	boost::program_options::variables_map vm;
	int webPort, brainRefresh;

	log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
	logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("Main"));
	desc.add_options()
		("WebServer.port", boost::program_options::value<int>(&webPort)->default_value(8080))
		("Brain.refresh", boost::program_options::value<int>(&brainRefresh)->default_value(300))
	    ;
    std::ifstream settings_file("config.ini", std::ifstream::in);
	boost::program_options::store(boost::program_options::parse_config_file(settings_file, desc, true), vm);
	settings_file.close();
	boost::program_options::notify(vm);
	curlInit();
	LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - WebServer.port=" << webPort));
	LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - Brain.refresh=" << brainRefresh));

    MHEDatabase *db = new MHEDatabase();
    MHEHardDevContainer *hardDev = new MHEHardDevContainer(*db);
    MHEWeb *ws = new MHEWeb(webPort, db, hardDev);

    std::vector<DBCondition> conds = db->getConditions();
    BOOST_FOREACH(DBCondition cond, conds)
    {
        std::cout << (std::string)cond << std::endl;
    }
    std::vector<DBGraph> graphs = db->getGraphs();
    BOOST_FOREACH(DBGraph graph, graphs)
    {
        std::cout << (std::string)graph << std::endl;
    }
    std::vector<DBRoom> rooms = db->getRooms();
    BOOST_FOREACH(DBRoom room, rooms)
    {
        std::cout << (std::string)room << std::endl;
    }
    std::vector<DBRoomGraphCond> roomGraphConds = db->getRoomGraphCondByActiveDaysAndCalendar();
    BOOST_FOREACH(DBRoomGraphCond rgc, roomGraphConds)
    {
        std::cout << (std::string)rgc << std::endl;
    }

    Brain b(db, hardDev, brainRefresh);
    b.start();

    delete ws;
    delete hardDev;
    delete db;
    curlDestroy();
}
