#include <iostream>
#include <sstream>

#include <curl/curl.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include "CurlHelpers.h"

static log4cplus::Logger log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("CurlHelpers"));

void curlInit()
{
	CURLcode res;

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

bool curlExecute(const std::string &url, const std::string *postData, std::stringstream *output)
{
	return curlExecute(url, NULL, postData, output);
}

bool curlExecute(const std::string &url, const std::string *serverAuth, const std::string *postData, std::stringstream *output)
{
	CURL *curl;

	LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("curlExecute - url=" << url));
	curl = curl_easy_init();
	if (curl != NULL)
	{
		CURLcode res;
		int httpCode = -1;

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1000L);
        curl_easy_setopt(curl, CURLOPT_ACCEPTTIMEOUT_MS, 3000L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		if (serverAuth != NULL && serverAuth->size() > 0)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
			curl_easy_setopt(curl, CURLOPT_USERPWD, serverAuth->c_str());
		}
		if (postData != NULL)
		{
            curl_easy_setopt(curl, CURLOPT_POST, true);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData->c_str());
        }
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
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
		return (httpCode == 200);
	}
	return false;
}
