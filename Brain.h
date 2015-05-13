#ifndef BRAIN_H_
#define BRAIN_H_

#include <vector>

#include "DataBase.h"
#include "WebServer.h"

class Brain
{
private:
	DataBase *db;
	WebServer *web;

public:
	Brain(int webPort);
	~Brain();

	void doMe(float tempIn, float tempOut);
};

#endif /* BRAIN_H_ */
