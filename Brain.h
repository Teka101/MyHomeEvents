#ifndef BRAIN_H_
#define BRAIN_H_

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <log4cplus/logger.h>
#include "MHEDatabase.h"
#include "MHEHardDevContainer.h"

class Brain
{
private:
	log4cplus::Logger _log;
	MHEDatabase *_db;
	MHEHardDevContainer *_hardDevContainer;
	int _refreshInSeconds;
	boost::asio::io_service _io;
	boost::asio::deadline_timer *_timer;

	void computeNextLaunch();
	void launch();

public:
	Brain(MHEDatabase *db, MHEHardDevContainer *hardDevContainer);
	~Brain();

	void start();
};

#endif /* BRAIN_H_ */
