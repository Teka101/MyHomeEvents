#ifndef MHESPEECHSERVICE_H
#define MHESPEECHSERVICE_H

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/regex.hpp>
#include <list>
#include <locale>
#include <log4cplus/logger.h>
#include "MHEWeb.h"

class MHEWeb;

struct sSpeechOrder
{
    std::string debug;
    bool isSuccess;
    std::string result;

    operator std::string() const
    {
        std::stringstream ss;

        ss << "sSpeechOrder["
            << "debug=" << debug
            << " isSuccess=" << (isSuccess ? "true" : "false")
            << " result=" << result
            << "]";
        return ss.str();
    }
};

typedef boost::function<bool(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m)> seoMethod;
struct sSpeechExprOrder
{
    boost::regex expression;
    seoMethod method;

    sSpeechExprOrder(boost::regex expr, seoMethod m) : expression(expr), method(m)
    {
    }
};

class MHESpeechService
{
private:
    log4cplus::Logger _log;
    SpeechRecognize *_sr;
    std::locale _locale;
    std::list<sSpeechExprOrder*> _orders;

public:
    MHESpeechService(SpeechRecognize *sr, const std::string &speechLang);
    ~MHESpeechService();

    sSpeechOrder parseAndExecute(const MHEWeb *web, const std::string &sentence);

private:
    bool executeLeaveHouse(const MHEWeb *web, sSpeechOrder *result, int nbDays);
    bool executeLeaveHouseInDay(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m);
    bool executeLeaveHouseInWeek(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m);
    bool executeLeaveHouseInMonth(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m);
    bool executeHowHouse(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m);
    bool executeWatchTV(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m);
    bool executeGoodNight(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m);
    bool executeDiagnostic(const MHEWeb *web, sSpeechOrder *result, boost::cmatch &m);
    bool executeDiagnosticReadCPU(int &totalUsed, int &totalIdle);
    bool executeDiagnosticReadMemory(int &totalUsed, int &totalFree);
    bool executePlugin(const std::string &sentence, const std::string &pluginFile, sSpeechOrder *result);
};

#endif // MHESPEECHSERVICE_H
