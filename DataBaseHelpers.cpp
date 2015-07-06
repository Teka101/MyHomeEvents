#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cfloat>
#include "DataBaseHelpers.h"

typedef std::pair<const std::string, boost::property_tree::ptree> ptreePair;

//void readFromPTree(boost::property_tree::ptree &pTree, sCondition &cond)
//{
//	cond.id = pTree.get<int>("id");
//	cond.description = pTree.get<std::string>("description");
//	cond.domoticzDeviceType = static_cast<eDomoticzDeviceType>(pTree.get<int>("domoticzDeviceType", -1));
//	cond.temperatureMin = pTree.get<int>("temperatureMin", -1);
//	cond.temperatureMax = pTree.get<int>("temperatureMax", -1);
//	cond.day = pTree.get<int>("day", -1);
//	cond.useCalendar = pTree.get<int>("useCalendar", 0);
//	if (pTree.count("dates") > 0)
//	{
//		BOOST_FOREACH(ptreePair it, pTree.get_child("dates"))
//		{
//			int date = it.second.get<int>("");
//
//			cond.dates.push_back(date);
//		}
//    }
//}

void readFromPTree(boost::property_tree::ptree &pTree, DBGraph &graph)
{
	graph.id = pTree.get<int>("id");
	graph.position = pTree.get<int>("position", -1);
	graph.description = pTree.get<std::string>("description");
	if (pTree.count("data") > 0)
	{
		BOOST_FOREACH(ptreePair it, pTree.get_child("data"))
		{
			DBGraphData data;

			data.time = it.second.get<int>("time");
			data.value = it.second.get<float>("value");
			graph.data.push_back(data);
		}
	}
}

void writeToPTree(boost::property_tree::ptree &pTree, DBHarware &hard)
{
	pTree.put("id", hard.id);
	pTree.put("type", hard.type);
	pTree.put("name", hard.name);
	pTree.put("param1", hard.param1);
}

void writeToPTree(boost::property_tree::ptree &pTree, DBDevice &dev)
{
    pTree.put("id", dev.id);
    pTree.put("name", dev.name);
    switch (dev.type)
    {
    case devTempHum: pTree.put("type", "devTempHum"); break;
    case devInterruptor: pTree.put("type", "devInterruptor"); break;
    default: pTree.put("type", "---unkown-type---"); break;
    }
	pTree.put("hardwareId", dev.hardwareId);
	pTree.put("cacheLifetime", dev.cacheLifetime);
	pTree.put("param1", dev.param1);
    pTree.put("param2", dev.param2);
	pTree.put("cloneToDeviceId", dev.cloneToDeviceId);
}

void writeToPTree(boost::property_tree::ptree &pTree, DBGraph &graph)
{
	pTree.put("id", graph.id);
	pTree.put("position", graph.position);
	pTree.put("description", graph.description);
	if (graph.data.size() > 0)
	{
		boost::property_tree::ptree ptChildren;

		BOOST_FOREACH(DBGraphData data, graph.data)
		{
			boost::property_tree::ptree ptChild;

			ptChild.put("time", data.time);
			ptChild.put("value", data.value);
			ptChildren.push_back(std::make_pair("", ptChild));
		}
		pTree.add_child("data", ptChildren);
	}
}

void writeToPTree(boost::property_tree::ptree &pTree, DBCondition &cond)
{
    pTree.put("id", cond.id);
	pTree.put("name", cond.name);
	pTree.put("deviceId", cond.deviceId);
	if (cond.temperatureMin > FLT_MIN)
        pTree.put("temperatureMin", cond.temperatureMin);
    else
        pTree.put("temperatureMin", "");
    if (cond.temperatureMax < FLT_MAX)
        pTree.put("temperatureMax", cond.temperatureMax);
    else
        pTree.put("temperatureMax", "");
    if (cond.days > 0)
        pTree.put("days", cond.days);
    else
        pTree.put("days", "");
	pTree.put("useCalendar", cond.useCalendar ? "true" : "false");
}

void writeToPTree(boost::property_tree::ptree &pTree, DBRoom &room)
{
    pTree.put("id", room.id);
	pTree.put("name", room.name);
	pTree.put("deviceTemperatureId", room.deviceTemperatureId);
	pTree.put("deviceHeatingId", room.deviceHeatingId);
	pTree.put("deviceHeaterId", room.deviceHeaterId);
}

void writeToPTree(boost::property_tree::ptree &pTree, DBRoomGraphCond &rgc)
{
    pTree.put("id", rgc.id);
	pTree.put("roomId", rgc.roomId);
	pTree.put("graphId", rgc.graphId);
	pTree.put("conditionId", rgc.conditionId);
}
