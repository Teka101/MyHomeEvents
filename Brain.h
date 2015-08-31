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

	void computeNextLaunch();
	void launch();

public:
	Brain(MHEDatabase *db, MHEHardDevContainer *hardDevContainer, int refreshInSeconds, MHEMobileNotify *notify);
	~Brain();

	void start();
	void stop();
};

#endif /* BRAIN_H_ */
