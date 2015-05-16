#include <iostream>
#include <map>
#include <math.h>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>
#include "Brain.h"

Brain::Brain(int webPort, Domoticz *domoticz)
{
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
	std::cout << "YOUHOU ! temp=" << dev->temperature << " hum=" << dev->humidity << std::endl;
	this->doMe(domo->getDeviceDTH22()->temperature, domo->getDeviceOutdoor()->temperature);
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

#ifdef DODEBUG
	std::cout << "Brain::doMe(" << tempIn << ", " << tempOut << ")" << std::endl;
#endif
	BOOST_FOREACH(sCondition cond, conds)
		condById[cond.id] = cond;
	BOOST_FOREACH(sGraph graph, graphs)
		if (graph.conditionId == -1)
			applyMe = &graph;
		else if (condById.find(graph.conditionId) != condById.end())
		{
			sCondition cond = condById[graph.conditionId];
			float temp = (cond.domoticzDeviceType == temperatureIn ? tempIn : tempOut);

			std::cout << "graph[" << graph.id << "][condition:"  << graph.conditionId << "]";
			std::cout << "cond.day=" << cond.day << " day.validity=" << ((cond.day & dayMask) == dayMask) << " deviceType=" << cond.domoticzDeviceType << " deviceTypeIsNone=" << (cond.domoticzDeviceType == none) << std::endl;
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
		{
#ifdef DODEBUG
			std::cerr << "doMe()-no graph data found !!! currentMS=" << currentMS << std::endl;
#endif
		}
		else
		{
			time_t now = time(NULL);

			this->domo->setValuesHeating(now, applyTemperature);
		}
	}
#ifdef DODEBUG
	else
		std::cerr << "doMe()-no valid graph found !!!" << std::endl;
#endif
}
