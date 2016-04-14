/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CURLHELPERS_H_
#define CURLHELPERS_H_

#include <sstream>
#include <vector>

void curlInit(long timeOut);
void curlDestroy();
bool curlExecute(const std::string &url, std::stringstream *output);
bool curlExecute(const std::string &url, std::vector<std::string> *headers = NULL, const std::string *postData = NULL, std::stringstream *output = NULL);
bool curlExecute(const std::string &url, std::vector<std::string> *headers, const std::string *serverAuth, const std::string *postData, std::stringstream *output);

#endif /* CURLHELPERS_H_ */
