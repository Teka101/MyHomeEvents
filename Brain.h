/*
Copyright (c) 2016 Teka101

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BRAIN_H_
#define BRAIN_H_

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <log4cplus/logger.h>
#include "MHEDatabase.h"
#include "MHEHardDevContainer.h"
#include "MHEMobileNotify.h"

class Brain
{
private:
	log4cplus::Logger _log;
	MHEDatabase *_db;
	MHEHardDevContainer *_hardDevContainer;
	MHEMobileNotify *_notify;
	int _refreshInSeconds;
	boost::asio::io_service _io;
	boost::asio::deadline_timer _timer;
	boost::asio::signal_set _signals;
	std::map<int,int> _lastConditionIdByRoomId;
	float _hysteresisMin, _hysteresisMax;

	void computeNextLaunch();
	void launch();
	void updateDevices();

public:
	Brain(MHEDatabase *db, MHEHardDevContainer *hardDevContainer, int refreshInSeconds, MHEMobileNotify *notify);
	~Brain();

	void start();
	void stop();
	void setHysteresis(float hysteresisMin, float hysteresisMax);
};

#endif /* BRAIN_H_ */
