/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <log4cplus/loggingmacros.h>
#include <ctime>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <locale.h>
#include <sstream>
#include <string.h>
#include <sys/inotify.h>
#include "SpeechRecognize.h"

#define EVENT_SIZE      (sizeof(struct inotify_event))
#define BUF_LEN         (1024 * (EVENT_SIZE + 16))

SpeechRecognize::SpeechRecognize(const std::string &fileName) : _fileName(fileName), _thread(NULL), _run(true), _gen(std::time(NULL))
{
    _log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("SpeechRecognize"));
    setlocale(LC_ALL, ""); //en_US.UTF-8
    _ic = iconv_open("ASCII//TRANSLIT", "UTF-8");
    if (_ic == reinterpret_cast<iconv_t>(-1))
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("SpeechRecognize::SpeechRecognize - iconv_open error: " << strerror(errno)));
    else
    {
        loadLanguage();
        _thread = new boost::thread(&SpeechRecognize::watchModifications, this);
    }
}

SpeechRecognize::~SpeechRecognize()
{
    clearCache();
    if (_ic != reinterpret_cast<iconv_t>(-1))
        iconv_close(_ic);
    _run = false;
    if (_thread != NULL)
    {
        _thread->join();
        delete _thread;
    }
}

void SpeechRecognize::watchModifications()
{
    int fd = inotify_init();

    if (fd == -1)
    {
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("Unable to init INOTIFY : " << strerror(errno)));
        return;
    }
    if (inotify_add_watch(fd, ".", IN_CLOSE_WRITE | IN_MOVE_SELF | IN_MOVED_TO) == -1)
    {
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("Unable to add watch INOTIFY : " << strerror(errno)));
        return;
    }
    LOG4CPLUS_INFO(_log, LOG4CPLUS_TEXT("Start thread..."));
    while (_run)
    {
        struct timeval  tm;
        fd_set          fds;
        char            buf[BUF_LEN];
        int             len;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tm.tv_sec = 1;
        tm.tv_usec = 0;
        if (select(fd + 1, &fds, NULL, NULL, &tm) <= 0)
            continue;
        len = read(fd, buf, BUF_LEN);
        if (len > 0)
        {
            int i = 0;

            while (i < len)
            {
                struct inotify_event    *event;

                event = reinterpret_cast<struct inotify_event *>(&buf[i]);
                LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("event: wd=" << event->wd << " mask=" << event->mask << " cookie=" << event->cookie << " len=" << event->len << " name=" << event->name));
                if (boost::equals(static_cast<char *>(event->name), _fileName))
                    loadLanguage();
                i += EVENT_SIZE + event->len;
            }
        }
    }
    close(fd);
    LOG4CPLUS_WARN(_log, LOG4CPLUS_TEXT("End thread !!!"));
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
    _plugins.erase(_plugins.begin(), _plugins.end());
}

void SpeechRecognize::loadLanguage()
{
    std::ifstream ifs;

    LOG4CPLUS_INFO(_log, LOG4CPLUS_TEXT("loadLanguage - try to load file '" << _fileName << "'"));
    ifs.open(_fileName.c_str());
    if (ifs.is_open())
    {
        std::string tmpLine;
        bool isWords = false;
        bool isResponses = false;
        bool isPlugins = false;

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
                isPlugins = false;
            }
            else if (boost::equals(tmpLine, "[Responses]"))
            {
                isWords = false;
                isResponses = true;
                isPlugins = false;
            }
            else if (boost::equals(tmpLine, "[Plugins]"))
            {
                isWords = false;
                isResponses = false;
                isPlugins = true;
            }
            else if (isWords || isResponses || isPlugins)
            {
                boost::split(words, tmpLine, isWords ? boost::is_any_of("=,") : boost::is_any_of("="));

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
                        else if (isPlugins)
                            _plugins[value] = key;
                    }
                }
            }
        }
        ifs.close();
        LOG4CPLUS_INFO(_log, LOG4CPLUS_TEXT("loadLanguage - loaded file '" << _fileName << "' : words[" << _words.size() << "] responses[" << _responses.size() << "] plugins[" << _plugins.size() << "]"));
    }
}

std::string SpeechRecognize::parse(const std::string &sentence)
{
    std::stringstream result;
    std::vector<std::string> words;
    std::string sentenceLC = boost::algorithm::to_lower_copy(sentence);
    char *tmpIn, *tmpOut;
    char *wordOut;
    char *wordIn = strdup(sentenceLC.c_str());
    size_t sizeIn = sentenceLC.size();
    size_t sizeOut = sizeIn * 2;
    bool isFirstWord = true;

    wordOut = static_cast<char*>(calloc(sizeof(char), sizeOut + 1));
    tmpIn = wordIn;
    tmpOut = wordOut;
    if (iconv(_ic, &tmpIn, &sizeIn, &tmpOut, &sizeOut) == static_cast<size_t>(-1))
    {
        free(wordIn);
        free(wordOut);
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("parse - iconv error: sentenceLC=" << sentenceLC << " error=" << strerror(errno)));
        return result.str();
    }
    LOG4CPLUS_DEBUG(_log, LOG4CPLUS_TEXT("parse - iconv: sentenceLC='" << sentenceLC << "' after='" << wordOut << "'"));
    boost::split(words, wordOut, boost::is_any_of(" ,.'?"));
    _mutex.lock();
    BOOST_FOREACH(std::string &word, words)
    {
        std::string w = _words[word];

        if (w.size() > 0)
        {
            if (!isFirstWord)
                result << ' ';
            result << w;
            isFirstWord = false;
        }
    }
    _mutex.unlock();
    free(wordIn);
    free(wordOut);
    return result.str();
}

std::string SpeechRecognize::getResponse(const std::string &responseCode)
{
    _mutex.lock();
    std::vector<std::string> *responses = _responses[responseCode];
    std::string response;

    if (responses != NULL)
    {
        boost::random::uniform_int_distribution<> dist(0, responses->size() - 1);
        int idx = dist(_gen);

        response = responses->at(idx);
    }
    else
    {
        LOG4CPLUS_ERROR(_log, LOG4CPLUS_TEXT("getResponse - no response for code=" << responseCode));
        response = responseCode;
    }
    _mutex.unlock();
    return response;
}

std::string SpeechRecognize::getResponse(const std::string &responseCode1, const std::string &responseCode2)
{
    std::string ret = getResponse(responseCode1);

    if (ret == responseCode1)
        ret = getResponse(responseCode2);
    return ret;
}

bool SpeechRecognize::getPlugin(const std::string &sentence, std::string &pluginFile)
{
    _mutex.lock();
    std::map<std::string,std::string>::const_iterator it = _plugins.find(sentence);
    bool hasFound = false;

    if (it != _plugins.end())
    {
        pluginFile = it->second;
        hasFound = true;
    }
    _mutex.unlock();
    return hasFound;
}
