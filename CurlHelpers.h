#ifndef CURLHELPERS_H_
#define CURLHELPERS_H_

#include <sstream>

void curlInit(long timeOut);
void curlDestroy();
bool curlExecute(const std::string &url, std::stringstream *output);
bool curlExecute(const std::string &url, const std::string *postData = NULL, std::stringstream *output = NULL);
bool curlExecute(const std::string &url, const std::string *serverAuth, const std::string *postData, std::stringstream *output);

#endif /* CURLHELPERS_H_ */
