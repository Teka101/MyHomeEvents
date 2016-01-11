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
    boost::mt19937 _gen;
    iconv_t _ic;
    std::map<std::string,std::string> _words;
    std::map<std::string,std::vector<std::string>*> _responses;
    std::map<std::string,std::string> _plugins;

    void clearCache();
    void loadLanguage(const std::string &fileName);
    std::string parseWord(const std::string &word);

public:
    SpeechRecognize(const std::string &fileName);
    ~SpeechRecognize();

    std::string parse(const std::string &sentence);
    std::string getResponse(const std::string &responseCode);
    std::string getResponse(const std::string &responseCode1, const std::string &responseCode2);
    bool getPlugin(const std::string &order, std::string &pluginFile) const;
};

#endif // SPEECHRECOGNIZE_H
