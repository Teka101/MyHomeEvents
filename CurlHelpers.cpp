/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <boost/foreach.hpp>
#include <curl/curl.h>
#include <iostream>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include "CurlHelpers.h"

static log4cplus::Logger log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("CurlHelpers"));
static long _timeOut;

void curlInit(long timeOut)
{
	CURLcode res;

	_timeOut = timeOut;
	res = curl_global_init(CURL_GLOBAL_ALL);
	if (res != CURLE_OK)
		LOG4CPLUS_ERROR(log, LOG4CPLUS_TEXT("curlInit - curl_global_init() failed: " << curl_easy_strerror(res)));
}

void curlDestroy()
{
	curl_global_cleanup();
}

static size_t writeMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t length = (size * nmemb);

	if (data != NULL)
	{
		std::stringstream *ss = static_cast<std::stringstream*>(data);

		ss->write(static_cast<char*>(ptr), length);
	}
	return length;
}

bool curlExecute(const std::string &url, std::stringstream *output)
{
    return curlExecute(url, NULL, NULL, output);
}

bool curlExecute(const std::string &url, std::vector<std::string> *headers, const std::string *postData, std::stringstream *output)
{
	return curlExecute(url, headers, NULL, postData, output);
}

bool curlExecute(const std::string &url, std::vector<std::string> *headers, const std::string *serverAuth, const std::string *postData, std::stringstream *output)
{
	CURL *curl;

	LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("curlExecute - url=" << url));
	curl = curl_easy_init();
	if (curl != NULL)
	{
        struct curl_slist *chunk = NULL;
		CURLcode res;
		int httpCode = -1;

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, _timeOut);
        curl_easy_setopt(curl, CURLOPT_ACCEPTTIMEOUT_MS, _timeOut);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, _timeOut);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		if (headers != NULL)
		{
            BOOST_FOREACH(std::string &h, *headers)
            {
                LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("curlExecute - AddHeader=" << h));
                chunk = curl_slist_append(chunk, h.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		}
		if (serverAuth != NULL && serverAuth->size() > 0)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
			curl_easy_setopt(curl, CURLOPT_USERPWD, serverAuth->c_str());
		}
		if (postData != NULL)
		{
            LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("curlExecute - postData=" << *postData));
            curl_easy_setopt(curl, CURLOPT_POST, true);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData->c_str());
        }
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
#ifdef CURLOPT_PIPEWAIT
		curl_easy_setopt(curl, CURLOPT_PIPEWAIT, 0L);
#endif //CURLOPT_PIPEWAIT
#ifdef CURLOPT_FORBIT_REUSE
		curl_easy_setopt(curl, CURLOPT_FORBIT_REUSE, 1L);
#endif //CURLOPT_FORBIT_REUSE
		curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

		res = curl_easy_perform(curl);
		if (res == CURLE_OK)
		{
			if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode) == CURLE_OK)
				LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("curlExecute - HttpCode=" << httpCode));
		}
		else
			LOG4CPLUS_ERROR(log, LOG4CPLUS_TEXT("curlExecute - curl_easy_perform() failed: " << curl_easy_strerror(res)));
		curl_easy_cleanup(curl);
		if (chunk != NULL)
            curl_slist_free_all(chunk);
		return (httpCode == 200);
	}
	return false;
}
