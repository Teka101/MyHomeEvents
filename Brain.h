#ifndef BRAIN_H_
#define BRAIN_H_

#include <vector>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include "DataBase.h"
#include "Domoticz.h"
#include "WebServer.h"

class Brain
{
private:
	log4cplus::Logger log;
	DataBase *db;
	Domoticz *domo;
	WebServer *web;

	void doMe(float tempIn, float tempOut);

public:
	Brain(int webPort, Domoticz *domoticz);
	~Brain();

	void update(sDomoticzDevice *dev);
};

#endif /* BRAIN_H_ */
