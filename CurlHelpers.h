#ifndef CURLHELPERS_H_
#define CURLHELPERS_H_

bool curlExecute(const std::string &url, const std::string &serverAuth, std::stringstream *output = NULL);

#endif /* CURLHELPERS_H_ */
