#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
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
//
//void readFromPTree(boost::property_tree::ptree &pTree, sGraph &graph, bool readData)
//{
//	graph.id = pTree.get<int>("id");
//	graph.position = pTree.get<int>("position", -1);
//	graph.description = pTree.get<std::string>("description");
//	graph.conditionId = pTree.get<int>("conditionId", -1);
//	if (readData)
//	{
//		BOOST_FOREACH(ptreePair it, pTree.get_child("data"))
//		{
//			sGraphData data;
//
//			data.time = it.second.get<int>("time");
//			data.value = it.second.get<float>("value");
//			graph.data.push_back(data);
//		}
//	}
//}
//
//void writeToPTree(boost::property_tree::ptree &pTree, sCondition &cond)
//{
//	pTree.put("id", cond.id);
//	pTree.put("description", cond.description);
//	pTree.put("domoticzDeviceType", cond.domoticzDeviceType);
//	pTree.put("temperatureMin", cond.temperatureMin);
//	pTree.put("temperatureMax", cond.temperatureMax);
//	pTree.put("day", cond.day);
//	pTree.put("useCalendar", cond.useCalendar);
//	if (cond.dates.size() > 0)
//	{
//		boost::property_tree::ptree ptChildren;
//
//		BOOST_FOREACH(int date, cond.dates)
//		{
//			boost::property_tree::ptree ptChild;
//
//			ptChild.put("", date);
//			ptChildren.push_back(std::make_pair("", ptChild));
//		}
//		pTree.add_child("dates", ptChildren);
//	}
//}
//
//void writeToPTree(boost::property_tree::ptree &pTree, sGraph &graph, bool writeData)
//{
//	pTree.put("id", graph.id);
//	pTree.put("position", graph.position);
//	pTree.put("description", graph.description);
//	pTree.put("conditionId", graph.conditionId);
//	if (writeData)
//	{
//		boost::property_tree::ptree ptChildren;
//
//		BOOST_FOREACH(sGraphData data, graph.data)
//		{
//			boost::property_tree::ptree ptChild;
//
//			ptChild.put("time", data.time);
//			ptChild.put("value", data.value);
//			ptChildren.push_back(std::make_pair("", ptChild));
//		}
//		pTree.add_child("data", ptChildren);
//	}
//}
