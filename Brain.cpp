#include <iostream>
#include <map>
#include <math.h>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include "Brain.h"

Brain::Brain(int webPort, Domoticz *domoticz)
{
	this->log = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("Brain"));
	this->db = new DataBase();
	this->domo = domoticz;
	this->web = new WebServer(webPort, db);
}

Brain::~Brain()
{
	if (this->db != NULL)
		delete this->db;
	if (this->web != NULL)
		delete this->web;
}

void Brain::update(sDomoticzDevice *dev)
{
	sDomoticzDevice *devDTH22, *devOutdoor;

	LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("Brain::update - invoke : temp=" << dev->temperature << " hum=" << dev->humidity));
	devDTH22 = domo->getDeviceDTH22();
	devOutdoor = domo->getDeviceOutdoor();
	if (devDTH22 != NULL && devOutdoor != NULL)
		this->doMe(devDTH22->temperature, devOutdoor->temperature);
}

void Brain::doMe(float tempIn, float tempOut)
{
	std::vector<sCondition> conds = this->db->getConditions();
	std::vector<sGraph> graphs = this->db->getGraphs();
	std::map<int,sCondition> condById;
	sGraph *applyMe = NULL;
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	int weekDayStartMonday = ((int)now.date().day_of_week() - 1 + 7) % 7;
	int dayMask = pow(2, weekDayStartMonday);

	LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("Brain::doMe - invoke(" << tempIn << ", " << tempOut << ")"));
	BOOST_FOREACH(sCondition cond, conds)
		condById[cond.id] = cond;
	BOOST_FOREACH(sGraph graph, graphs)
		if (graph.conditionId == -1)
			applyMe = &graph;
		else if (condById.find(graph.conditionId) != condById.end())
		{
			sCondition cond = condById[graph.conditionId];
			float temp = (cond.domoticzDeviceType == temperatureIn ? tempIn : tempOut);

			LOG4CPLUS_DEBUG(log, LOG4CPLUS_TEXT("Brain::doMe - graph[" << graph.id << "][condition:"  << graph.conditionId << "]cond.day=" << cond.day << " day.validity=" << ((cond.day & dayMask) == dayMask) << " deviceType=" << cond.domoticzDeviceType << " deviceTypeIsNone=" << (cond.domoticzDeviceType == none)));
			if (
				(cond.day == -1 || (cond.day & dayMask) == dayMask)
				&& (cond.domoticzDeviceType == none
					|| (cond.domoticzDeviceType != none && cond.temperatureMin != -1 && cond.temperatureMax == -1 && cond.temperatureMin <= temp)
					|| (cond.domoticzDeviceType != none && cond.temperatureMin == -1 && cond.temperatureMax != -1 && temp <= cond.temperatureMax)
					|| (cond.domoticzDeviceType != none && cond.temperatureMin != -1 && cond.temperatureMax != -1 && cond.temperatureMin <= temp && temp <= cond.temperatureMax)
					)
				//TODO calendarId
				)
				applyMe = &graph;
		}
	if (applyMe != NULL)
	{
		sGraph fullGraph = this->db->getGraph(applyMe->id);
		long currentMS = now.time_of_day().hours() * 100L + now.time_of_day().minutes();
		float applyTemperature = NAN;

		BOOST_FOREACH(sGraphData data, fullGraph.data)
			if (currentMS >= data.time)
				applyTemperature = data.value;
		if (isnan(applyTemperature))
			LOG4CPLUS_ERROR(log, LOG4CPLUS_TEXT("Brain::doMe - no graph data found !!! currentMS=" << currentMS));
		else
		{
			time_t now = time(NULL);

			this->domo->setValuesHeating(now, applyTemperature);
			this->domo->setStatusHeater(tempIn, applyTemperature);
		}
	}
	else
		LOG4CPLUS_ERROR(log, LOG4CPLUS_TEXT("Brain::doMe - no valid graph found !!!"));
}
