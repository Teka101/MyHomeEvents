#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <ctime>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <locale.h>
#include <sstream>
#include <string.h>
#include "SpeechRecognize.h"

SpeechRecognize::SpeechRecognize(const std::string &fileName) : _gen(std::time(NULL))
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("SpeechRecognize"));
    setlocale(LC_ALL, ""); //en_US.UTF-8
    _ic = iconv_open("ASCII//TRANSLIT", "UTF-8");
    if (_ic == reinterpret_cast<iconv_t>(-1))
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("SpeechRecognize::SpeechRecognize - iconv_open error: " << strerror(errno)));
    else
        loadLanguage(fileName);
}

SpeechRecognize::~SpeechRecognize()
{
    clearCache();
    if (_ic != reinterpret_cast<iconv_t>(-1))
        iconv_close(_ic);
}

void SpeechRecognize::clearCache()
{
    std::pair<std::string, std::vector<std::string>*> kv;

    _words.erase(_words.begin(), _words.end());
    BOOST_FOREACH(kv, _responses)
    {
        delete kv.second;
    }
    _responses.erase(_responses.begin(), _responses.end());
}

void SpeechRecognize::loadLanguage(const std::string &fileName)
{
    std::ifstream ifs;
    bool isWords = false;
    bool isResponses = false;

    LOG4CPLUS_INFO(_log, LOG4CPLUS_TEXT("loadLanguage - try to load file '" << fileName << "'"));
    ifs.open(fileName.c_str());
    if (ifs.is_open())
    {
        std::string tmpLine;

        clearCache();
        while (ifs.good())
        {
            std::vector<std::string> words;

            tmpLine.clear();
            std::getline(ifs, tmpLine);
            if (boost::equals(tmpLine, "[Words]"))
            {
                isWords = true;
                isResponses = false;
            }
            else if (boost::equals(tmpLine, "[Responses]"))
            {
                isWords = false;
                isResponses = true;
            }
            else if (isWords || isResponses)
            {
                boost::split(words, tmpLine, boost::is_any_of("=,"));

                if (words.size() > 1)
                {
                    std::string key = boost::trim_left_copy(words[0]);

                    words.erase(words.begin());
                    BOOST_FOREACH(std::string &value, words)
                    {
                        if (isWords)
                            _words[value] = key;
                        else if (isResponses)
                        {
                            std::vector<std::string> *l = _responses[key];

                            if (l == NULL)
                            {
                                l = new std::vector<std::string>();
                                _responses[key] = l;
                            }
                            l->push_back(value);
                        }
                    }
                }
            }
        }
        ifs.close();
        LOG4CPLUS_INFO(_log, LOG4CPLUS_TEXT("loadLanguage - loaded file '" << fileName << "' : words[" << _words.size() << "] responses[" << _responses.size() << "]"));
    }
}

std::vector<std::string> SpeechRecognize::parse(const std::string &sentence)
{
    std::vector<std::string> ret;
    std::vector<std::string> words;
    char *tmpIn, *tmpOut;
    char *wordOut;
    char *wordIn = strdup(sentence.c_str());
    size_t sizeIn = sentence.size();
    size_t sizeOut = sizeIn * 2;

    wordOut = static_cast<char*>(calloc(sizeof(char), sizeOut + 1));
    tmpIn = wordIn;
    tmpOut = wordOut;
    if (iconv(_ic, &tmpIn, &sizeIn, &tmpOut, &sizeOut) == static_cast<size_t>(-1))
    {
        free(wordIn);
        free(wordOut);
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("parse - iconv error: sentence=" << sentence << " error=" << strerror(errno)));
        return ret;
    }
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("parse - iconv: sentence='" << sentence << "' after='" << wordOut << "'"));
    boost::split(words, wordOut, boost::is_any_of(" ,.'?"));
    BOOST_FOREACH(std::string &word, words)
    {
        std::string w = _words[word];

        if (w.size() > 0)
            ret.push_back(w);
    }
    free(wordIn);
    free(wordOut);
    return ret;
}

std::string SpeechRecognize::getResponse(const std::string responseCode)
{
    std::vector<std::string> *responses = _responses[responseCode];

    if (responses != NULL)
    {
        boost::random::uniform_int_distribution<> dist(0, responses->size() - 1);
        int idx = dist(_gen);

        return responses->at(idx);
    }
    else
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("getResponse - no response for code=" << responseCode));
    return responseCode;
}

std::string SpeechRecognize::getResponse(const std::string responseCode1, const std::string responseCode2)
{
    std::string ret = getResponse(responseCode1);

    if (ret == responseCode1)
        ret = getResponse(responseCode2);
    return ret;
}
