/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef SPEECHRECOGNIZE_H
#define SPEECHRECOGNIZE_H

#include <boost/random/mersenne_twister.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <iconv.h>
#include <log4cplus/logger.h>
#include <map>
#include <vector>

class SpeechRecognize
{
private:
    log4cplus::Logger _log;
    std::string _fileName;
    boost::mutex _mutex;
    boost::thread *_thread;
    bool _run;
    boost::mt19937 _gen;
    iconv_t _ic;
    std::map<std::string,std::string> _words;
    std::map<std::string,std::vector<std::string>*> _responses;
    std::map<std::string,std::string> _plugins;

    void watchModifications();
    void clearCache();
    void loadLanguage();
    std::string parseWord(const std::string &word);

public:
    SpeechRecognize(const std::string &fileName);
    ~SpeechRecognize();

    std::string parse(const std::string &sentence);
    std::string getResponse(const std::string &responseCode);
    std::string getResponse(const std::string &responseCode1, const std::string &responseCode2);
    bool getPlugin(const std::string &order, std::string &pluginFile);
};

#endif // SPEECHRECOGNIZE_H
