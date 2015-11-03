#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/locale.hpp>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

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
        BOOST_FOREACH(sSpeechExprOrder *sr, _orders)
        {
            boost::cmatch what;

            if (boost::regex_match(order.c_str(), what, sr->expression))
            {
                hasFoundOrder = true;
                ret.isSuccess = sr->method(web, &ret, what);
                break;
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
