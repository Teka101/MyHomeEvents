#ifndef SPEECHRECOGNIZE_H
#define SPEECHRECOGNIZE_H

#include <boost/random/mersenne_twister.hpp>
#include <iconv.h>
#include <log4cplus/logger.h>
#include <map>
#include <vector>

class SpeechRecognize
{
private:
    log4cplus::Logger _log;
    boost::random::mt19937 _gen;
    iconv_t _ic;
    std::map<std::string,std::string> _words;
    std::map<std::string,std::vector<std::string>*> _responses;

    void clearCache();
    void loadLanguage(const std::string &fileName);
    std::string parseWord(const std::string &word);

public:
    SpeechRecognize(const std::string &lang);
    ~SpeechRecognize();

    std::vector<std::string> parse(const std::string &sentence);
    std::string getResponse(const std::string responseCode);
};

#endif // SPEECHRECOGNIZE_H