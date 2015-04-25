#include <boost/property_tree/ptree.hpp>
#include "DataBase.h"
#include "DataBaseHelpers.h"

void readFromPTree(boost::property_tree::ptree &pTree, sGraph &graph, bool readData)
{
	graph.id = pTree.get<int>("id");
	graph.position = pTree.get<int>("position", -1);
	graph.description = pTree.get<std::string>("description");
	graph.conditionId = pTree.get<int>("conditionId", -1);
	if (readData)
		for (auto it : pTree.get_child("data"))
		{
			sGraphData data;

			data.time = it.second.get<int>("time");
			data.value = it.second.get<float>("value");
			graph.data.push_back(data);
		}
}

void writeToPTree(boost::property_tree::ptree &pTree, sCondition &cond)
{
	pTree.put("id", cond.id);
	pTree.put("description", cond.description);
	pTree.put("domoticzDeviceType", cond.domoticzDeviceType);
	pTree.put("temperatureMin", cond.temperatureMin);
	pTree.put("temperatureMax", cond.temperatureMax);
	pTree.put("day", cond.day);
	pTree.put("calendarId", cond.calendarId);
}

void writeToPTree(boost::property_tree::ptree &pTree, sGraph &graph)
{
	pTree.put("id", graph.id);
	pTree.put("position", graph.position);
	pTree.put("description", graph.description);
	pTree.put("conditionId", graph.conditionId);
}

void writeToPTree(boost::property_tree::ptree &pTree, sGraphData &data)
{
	pTree.put("time", data.time);
	pTree.put("value", data.value);
}
