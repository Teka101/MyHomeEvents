#ifndef BRAIN_H_
#define BRAIN_H_

#include <vector>

#include "DataBase.h"
#include "Domoticz.h"
#include "WebServer.h"

class Brain
{
private:
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
