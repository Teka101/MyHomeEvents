/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
    MHEDatabase *db = NULL;
    SpeechRecognize *sr = NULL;
    MHEMobileNotify *notify = NULL;
    MHEHardDevContainer *hardDev = NULL;
    MHESpeechService *ss = NULL;
    MHEWeb *ws = NULL;
    boost::program_options::options_description desc("ConfigFile");
    boost::program_options::variables_map vm;
    log4cplus::Logger logger;
    std::string gcmAppId, speechFile, speechLang;
    long curlTimeout;
    float hysteresisMin, hysteresisMax;
    int webPort, brainRefresh;

    log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
    logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("Main"));
    desc.add_options()
        ("WebServer.port", boost::program_options::value<int>(&webPort)->default_value(8080))
        ("Brain.refresh", boost::program_options::value<int>(&brainRefresh)->default_value(300))
        ("Notify.gcmAppId", boost::program_options::value<std::string>(&gcmAppId))
        ("General.curlTimeout", boost::program_options::value<long>(&curlTimeout)->default_value(1000L))
        ("General.speechFile", boost::program_options::value<std::string>(&speechFile))
        ("General.speechLang", boost::program_options::value<std::string>(&speechLang))
        ("Heating.hysteresisMin", boost::program_options::value<float>(&hysteresisMin)->default_value(0))
        ("Heating.hysteresisMax", boost::program_options::value<float>(&hysteresisMax)->default_value(0))
        ;
    try
    {
        std::ifstream settings_file("config.ini", std::ifstream::in);
        boost::program_options::store(boost::program_options::parse_config_file(settings_file, desc, true), vm);
        settings_file.close();
    }
    catch (const boost::program_options::unknown_option &e)
    {
        LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("Unkown option: " << e.get_option_name()));
        exit(1);
    }
    boost::program_options::notify(vm);
    LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("Application starting..."));
    LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - General.curlTimeout=" << curlTimeout));
    LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - WebServer.port=" << webPort));
    LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - Brain.refresh=" << brainRefresh));
    LOG4CPLUS_DEBUG(logger, LOG4CPLUS_TEXT("main - Notify.gcmAppId=" << gcmAppId));
    curlInit(curlTimeout);
    try
    {
        db = new MHEDatabase();
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

        sr = (speechFile.size() == 0 ? NULL : new SpeechRecognize(speechFile));
        notify = (gcmAppId.size() == 0 ? NULL : new MHEMobileNotify(gcmAppId, db, sr));
        hardDev = new MHEHardDevContainer(*db);
        ss = new MHESpeechService(sr, speechLang);
        ws = new MHEWeb(webPort, db, hardDev, notify, ss);

        if (!ws->isStarted())
            LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("Unable to start web server !"));
        else
        {
            Brain b(db, hardDev, brainRefresh, notify);

            if (hysteresisMin > 0 && hysteresisMax > 0)
		b.setHysteresis(hysteresisMin, hysteresisMax);
            b.start();
        }
        LOG4CPLUS_INFO(logger, LOG4CPLUS_TEXT("Application stopping..."));
    }
    catch (const boost::exception &e)
    {
        LOG4CPLUS_ERROR(logger, LOG4CPLUS_TEXT("Error " << boost::diagnostic_information(e)));
    }
    delete ws;
    delete notify;
    delete sr;
    delete ss;
    delete hardDev;
    delete db;
    curlDestroy();
    return 0;
}
