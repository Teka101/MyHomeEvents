#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/locale.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <errno.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include "MHESpeechService.h"

MHESpeechService::MHESpeechService(SpeechRecognize *sr, const std::string &speechLang) : _sr(sr), _locale(speechLang.c_str())
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("MHESpeechService"));
    _orders.push_back(new sSpeechExprOrder(boost::regex("^TO_LEAVE ([0-9]+) DURATION_DAY$", boost::regex::perl), boost::bind(&MHESpeechService::executeLeaveHouseInDay, this, _1, _2, _3)));
    _orders.push_back(new sSpeechExprOrder(boost::regex("^TO_LEAVE ([0-9]+) DURATION_WEEK$", boost::regex::perl), boost::bind(&MHESpeechService::executeLeaveHouseInWeek, this, _1, _2, _3)));
    _orders.push_back(new sSpeechExprOrder(boost::regex("^TO_LEAVE ([0-9]+) DURATION_MONTH$", boost::regex::perl), boost::bind(&MHESpeechService::executeLeaveHouseInMonth, this, _1, _2, _3)));
    _orders.push_back(new sSpeechExprOrder(boost::regex("^HOW HOUSE$", boost::regex::perl), boost::bind(&MHESpeechService::executeHowHouse, this, _1, _2, _3)));
    _orders.push_back(new sSpeechExprOrder(boost::regex("^TO_WATCH ([0-9]+)$", boost::regex::perl), boost::bind(&MHESpeechService::executeWatchTV, this, _1, _2, _3)));
    _orders.push_back(new sSpeechExprOrder(boost::regex("^GOOD NIGHT$", boost::regex::perl), boost::bind(&MHESpeechService::executeGoodNight, this, _1, _2, _3)));
    _orders.push_back(new sSpeechExprOrder(boost::regex("^DIAGNOSTIC$", boost::regex::perl), boost::bind(&MHESpeechService::executeDiagnostic, this, _1, _2, _3)));
}

MHESpeechService::~MHESpeechService()
{
    BOOST_FOREACH(sSpeechExprOrder *sr, _orders)
    {
        delete sr;
    }
}

sSpeechOrder MHESpeechService::parseAndExecute(const MHEWeb *web, const std::string &sentence)
{
    sSpeechOrder ret;

    ret.isSuccess = false;
    if (_sr == NULL)
    {
        ret.debug = sentence;
        ret.result = "No speech service installed.";
    }
    else
    {
        std::string order = _sr->parse(sentence);
        bool hasFoundOrder = false;

        ret.debug = order;
        BOOST_FOREACH(sSpeechExprOrder *ser, _orders)
        {
            boost::cmatch what;

            if (boost::regex_match(order.c_str(), what, ser->expression))
            {
                hasFoundOrder = true;
                ret.isSuccess = ser->method(web, &ret, what);
                break;
            }
        }
        if (!hasFoundOrder)
        {
            std::string pluginFile;

            if (_sr->getPlugin(order, pluginFile))
            {
                hasFoundOrder = true;
                ret.isSuccess = executePlugin(order, pluginFile, &ret);
            }
        }
        if (!hasFoundOrder)
            ret.result = _sr->getResponse("UNKNOWN_ORDER");
        else if (ret.result.length() == 0)
            ret.result = _sr->getResponse(ret.isSuccess ? "OK" : "KO");
    }
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("parseAndExecute: ret='" << (std::string)ret));
    return ret;
}

bool MHESpeechService::executeLeaveHouse(const MHEWeb *web, sSpeechOrder *result, int nbDays)
{
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    boost::gregorian::date_duration dd(nbDays);
	boost::gregorian::date currentDate = now.date();
	boost::posix_time::time_duration currentTime = now.time_of_day();
    long dateStartYYYYMMDDHHMM, dateEndYYYYMMDDHHMM;
    int conditionId = nbDays > 8 ? 6 : 5;

	dateStartYYYYMMDDHHMM = ((long)currentDate.year() * 10000L + (long)currentDate.month() * 100L + (long)currentDate.day()) * 10000L + (currentTime.hours() * 100L + currentTime.minutes());
    currentDate = currentDate + dd;
    dateEndYYYYMMDDHHMM = ((long)currentDate.year() * 10000L + (long)currentDate.month() * 100L + (long)currentDate.day()) * 10000L + (currentTime.hours() * 100L + currentTime.minutes());
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("executeLeaveHouse... nbDays=" << nbDays << " conditionId=" << conditionId << " dateStartYYYYMMDDHHMM=" << dateStartYYYYMMDDHHMM << " dateEndYYYYMMDDHHMM=" << dateEndYYYYMMDDHHMM));
    return web->getDataBase()->addConditionDate(conditionId, dateStartYYYYMMDDHHMM, dateEndYYYYMMDDHHMM);
}

bool MHESpeechService::executeLeaveHouseInDay(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m)
{
    return executeLeaveHouse(web, result, boost::lexical_cast<int>(m[1]));
}

bool MHESpeechService::executeLeaveHouseInWeek(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m)
{
    return executeLeaveHouse(web, result, boost::lexical_cast<int>(m[1]) * 7);
}

bool MHESpeechService::executeLeaveHouseInMonth(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m)
{
    return executeLeaveHouse(web, result, boost::lexical_cast<int>(m[1]) * 30);
}

bool MHESpeechService::executeHowHouse(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m)
{
    std::vector<MHEDevice*> devices = web->getHardDevContainer()->getDevices();
    std::stringstream ss;
    bool isFirst = true;

    BOOST_FOREACH(MHEDevice *dev, devices)
    {
        if (!dev->isHidden() && dev->getType() == devTempHum)
        {
            std::stringstream localizedTemperature;
            std::string msg = _sr->getResponse("howHouse_temperature");

            localizedTemperature.imbue(_locale);
            localizedTemperature << dev->getTemperature();
            boost::replace_all(msg, "%name%", dev->getName());
            boost::replace_all(msg, "%temperature%", localizedTemperature.str());
            if (!isFirst)
                ss << ' ';
            ss << msg;
            isFirst = false;
        }
    }
    result->result = ss.str();
    return true;
}

bool MHESpeechService::executeWatchTV(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m)
{
    std::vector<MHEDevice*> devices = web->getHardDevContainer()->getDevices();
    std::string channelId = m[1];
    bool status = false;

    BOOST_FOREACH(MHEDevice *dev, devices)
    {
        if (!dev->isHidden() && dev->getType() == devTV)
            status = dev->sendCommand("channel", channelId, NULL);
    }
    return status;
}

bool MHESpeechService::executeGoodNight(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m)
{
    std::vector<MHEDevice*> devices = web->getHardDevContainer()->getDevices();
    std::stringstream ss;
    bool isFirst = true;

    BOOST_FOREACH(MHEDevice *dev, devices)
    {
        if (!dev->isHidden() && dev->getType() == devTV)
            if (dev->setStatus(false))
            {
                if (!isFirst)
                    ss << ' ';
                ss << _sr->getResponse("STOP_TV");
                isFirst = false;
            }
    }
    if (!isFirst)
        ss << ' ';
    ss << _sr->getResponse("GOOD_NIGHT");
    result->result = ss.str();
    return true;
}

bool MHESpeechService::executeDiagnostic(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m)
{
    std::string msg = _sr->getResponse("diagnostic");
    int firstUsed, firstIdle;
    int memFree, memTotal;

    if (executeDiagnosticReadCPU(firstUsed, firstIdle))
    {
	int secondUsed, secondIdle;

        sleep(1);
        if (executeDiagnosticReadCPU(secondUsed, secondIdle))
        {
            int diffUsed = secondUsed - firstUsed;
            int diffIdle = secondIdle - firstIdle;
            std::stringstream localizedCPU;
            int cpuUsage = diffUsed * 100 / (diffUsed + diffIdle);

            localizedCPU.imbue(_locale);
            localizedCPU << cpuUsage;
            boost::replace_all(msg, "%cpu%", localizedCPU.str());
            LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("executeDiagnostic: cpu=" << cpuUsage << "% diffUsed=" << diffUsed << " diffIdle=" << diffIdle));
        }
    }
    if (executeDiagnosticReadMemory(memFree, memTotal))
    {
        std::stringstream localizedMEM;
        int memUsage = memFree * 100 / memTotal;

        localizedMEM.imbue(_locale);
        localizedMEM << memUsage;
        boost::replace_all(msg, "%mem%", localizedMEM.str());
        LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("executeDiagnostic: mem=" << memUsage << "% memFree=" << memFree << " memTotal=" << memTotal));
    }
    result->result = msg;
    return true;
}

bool MHESpeechService::executeDiagnosticReadCPU(int &totalUsed, int &totalIdle)
{
    std::ifstream fProc("/proc/stat");

    if (fProc.good())
    {
        boost::regex expression("^cpu[\\t ]+([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+).*$", boost::regex::perl);
        boost::cmatch what;
        std::string line;

        while(std::getline(fProc, line))
        {
            if (boost::regex_match(line.c_str(), what, expression))
            {
                int userMode = boost::lexical_cast<int>(what[1]);
                int userNicedMode = boost::lexical_cast<int>(what[2]);
                int systemMode = boost::lexical_cast<int>(what[3]);
                int idleTask = boost::lexical_cast<int>(what[4]);

                totalUsed = userMode + userNicedMode + systemMode;
                totalIdle = idleTask;
                LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("executeDiagnosticReadCPU: totalUsed=" << totalUsed << " totalIdle=" << totalIdle));
                return true;
            }
        }
    }
    else
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("executeDiagnosticReadCPU: unable to open /proc/stat"));
    return false;
}

bool MHESpeechService::executeDiagnosticReadMemory(int &memFree, int &memTotal)
{
    std::ifstream fProc("/proc/meminfo");

    memFree = 0;
    memTotal = 0;
    if (fProc.good())
    {
        boost::regex expressionMemFree("^MemFree:[\\t ]+([0-9]+) kB$", boost::regex::perl);
        boost::regex expressionMemTotal("^MemTotal:[\\t ]+([0-9]+) kB$", boost::regex::perl);
        boost::cmatch what;
        std::string line;

        while(std::getline(fProc, line))
        {
            if (boost::regex_match(line.c_str(), what, expressionMemFree))
                memFree = boost::lexical_cast<int>(what[1]);
            else if (boost::regex_match(line.c_str(), what, expressionMemTotal))
                memTotal = boost::lexical_cast<int>(what[1]);
        }
    }
    else
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("executeDiagnosticReadMemory: unable to open /proc/meminfo"));
    return (memFree > 0 && memTotal > 0);
}

bool MHESpeechService::executePlugin(const std::string &order, const std::string &pluginFile, sSpeechOrder *result)
{
    std::stringstream ssPlugins;
    FILE *fh;

    ssPlugins << "plugins/" << pluginFile;

    fh = popen(ssPlugins.str().c_str(), "r");
    if (fh == NULL)
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("executePlugin - popen(" << ssPlugins.str() << ") failed: " << strerror(errno)));
    else
    {
        std::stringstream ssRes;
        char line[1024];
        int execReturn;

        LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("executePlugin - popen() for command: " << ssPlugins.str()));
        while (fgets(line, sizeof(line), fh) != NULL)
            ssRes << line;
        execReturn = pclose(fh);
        LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("executePlugin - Exec return: " << execReturn));
        result->result = ssRes.str();
        return (execReturn == 0);
    }
    return false;
}
