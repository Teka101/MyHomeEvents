#ifndef CURLHELPERS_H_
#define CURLHELPERS_H_

#include <sstream>

void curlInit();
void curlDestroy();
bool curlExecute(const std::string &url, std::stringstream *output = NULL);
bool curlExecute(const std::string &url, const std::string &serverAuth, std::stringstream *output = NULL);

#endif /* CURLHELPERS_H_ */
