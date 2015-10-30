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

int main(int ac, char **av)
{
    boost::program_options::options_description desc("ConfigFile");
	boost::program_options::variables_map vm;
	log4cplus::Logger logger;
	std::string gcmAppId, speechFile;
	long curlTimeout;
	int webPort, brainRefresh;

	log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
	logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("Main"));
	desc.add_options()
		("WebServer.port", boost::program_options::value<int>(&webPort)->default_value(8080))
		("Brain.refresh", boost::program_options::value<int>(&brainRefresh)->default_value(300))
		("Notify.gcmAppId", boost::program_options::value<std::string>(&gcmAppId))
		("General.curlTimeout", boost::program_options::value<long>(&curlTimeout)->default_value(1000L))
		("General.speechFile", boost::program_options::value<std::string>(&speechFile))
	    ;
    std::ifstream settings_file("config.ini", std::ifstream::in);
	boost::program_options::store(boost::program_options::parse_config_file(settings_file, desc, true), vm);
	settings_file.close();
	boost::program_options::notify(vm);
	LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("Application starting..."));
	LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - General.curlTimeout=" << curlTimeout));
	LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - WebServer.port=" << webPort));
	LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - Brain.refresh=" << brainRefresh));
	LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - Notify.gcmAppId=" << gcmAppId));
	curlInit(curlTimeout);

    MHEDatabase *db = new MHEDatabase();

    if (logger.isEnabledFor(log4cplus::DEBUG_LOG_LEVEL))
    {
        std::vector<DBCondition> conds = db->getConditions();
        BOOST_FOREACH(DBCondition cond, conds)
        {
            LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - " << (std::string)cond));
        }
        std::vector<DBGraph> graphs = db->getGraphs();
        BOOST_FOREACH(DBGraph graph, graphs)
        {
            LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - " << (std::string)graph));
        }
        std::vector<DBRoom> rooms = db->getRooms();
        BOOST_FOREACH(DBRoom room, rooms)
        {
            LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - " << (std::string)room));
        }
        std::vector<DBRoomGraphCond> roomGraphConds = db->getRoomGraphCondByActiveDaysAndCalendar();
        BOOST_FOREACH(DBRoomGraphCond rgc, roomGraphConds)
        {
            LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - " << (std::string)rgc));
        }
    }

    SpeechRecognize *sr = (speechFile.size() == 0 ? NULL : new SpeechRecognize(speechFile));
    MHEMobileNotify *notify = (gcmAppId.size() == 0 ? NULL : new MHEMobileNotify(gcmAppId, db, sr));
    MHEHardDevContainer *hardDev = new MHEHardDevContainer(*db);
    MHEWeb *ws = new MHEWeb(webPort, db, hardDev, notify);

    if (!ws->isStarted())
        LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("Unable to start web server !"));
    else
    {
        Brain b(db, hardDev, brainRefresh, notify);

        b.start();
    }
    LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("Application stopping..."));
    delete ws;
    if (notify != NULL)
        delete notify;
    delete hardDev;
    delete db;
    curlDestroy();
    return 0;
}
