#include <iostream>
#include <map>
#include <math.h>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include "Brain.h"

Brain::Brain(int webPort)
{
	this->db = new DataBase();
	this->web = new WebServer(webPort, db);
}

Brain::~Brain()
{
	if (this->db != nullptr)
		delete this->db;
	if (this->web != nullptr)
		delete this->web;
}

void Brain::doMe(float tempIn, float tempOut)
{
	std::vector<sCondition> conds = this->db->getConditions();
	std::vector<sGraph> graphs = this->db->getGraphs();
	std::map<int,sCondition> condById;
	sGraph *applyMe = nullptr;
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	int weekDayStartMonday = ((int)now.date().day_of_week() - 1 + 7) % 7;
	int dayMask = pow(2, weekDayStartMonday);

	for (sCondition cond : conds)
		condById[cond.id] = cond;
	for (sGraph graph : graphs)
		if (graph.conditionId == -1)
			applyMe = &graph;
		else if (condById.find(graph.conditionId) != condById.end())
		{
			sCondition cond = condById[graph.conditionId];
			float temp = (cond.domoticzDeviceType == temperatureIn ? tempIn : tempOut);

			if (
				(cond.day == -1 || (cond.day & dayMask) == dayMask)
				&& (cond.domoticzDeviceType == none
					|| (cond.domoticzDeviceType == temperatureIn && cond.temperatureMin != -1 && cond.temperatureMax == -1 && cond.temperatureMin <= temp)
					|| (cond.domoticzDeviceType == temperatureIn && cond.temperatureMin == -1 && cond.temperatureMax != -1 && temp <= cond.temperatureMax)
					|| (cond.domoticzDeviceType == temperatureIn && cond.temperatureMin != -1 && cond.temperatureMax != -1 && cond.temperatureMin <= temp && temp <= cond.temperatureMax)
					)
				//TODO domoticzDeviceType temperatureMin|temperatureMax
				//TODO calendarId
				)
				applyMe = &graph;
		}
	if (applyMe != nullptr)
	{
		sGraph fullGraph = this->db->getGraph(applyMe->id);
		long currentMS = now.time_of_day().hours() * 100L + now.time_of_day().minutes();
		float applyTemperature = NAN;

		for (sGraphData data : fullGraph.data)
			if (currentMS >= data.time)
				applyTemperature = data.value;
		if (isnan(applyTemperature))
			std::cerr << "doMe()-no graph data found !!! currentMS=" << currentMS << std::endl;
		else
			std::cout << "apply temperature:" << applyTemperature << std::endl;
	}
	else
		std::cerr << "doMe()-no valid graph found !!!" << std::endl;
}
