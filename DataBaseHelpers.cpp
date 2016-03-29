#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cmath>
#include "DataBaseHelpers.h"

typedef std::pair<const std::string, boost::property_tree::ptree> ptreePair;

void readFromPTree(boost::property_tree::ptree &pTree, DBHarware &hard)
{
    hard.id = pTree.get<int>("id");
    hard.type = pTree.get<std::string>("type");
	hard.name = pTree.get<std::string>("name");
	hard.param1 = pTree.get<std::string>("param1");
}

void readFromPTree(boost::property_tree::ptree &pTree, DBDevice &dev)
{
    std::string devType = pTree.get<std::string>("type");

    dev.id = pTree.get<int>("id");
    dev.name = pTree.get<std::string>("name");
    if (devType == "devTempHum") dev.type = devTempHum;
    else if (devType == "devInterruptor") dev.type = devInterruptor;
    else if (devType == "devDimmer") dev.type = devDimmer;
    else if (devType == "devTV") dev.type = devTV;
    dev.hardwareId = pTree.get<int>("hardwareId");
    dev.cacheLifetime = pTree.get<int>("cacheLifetime");
	dev.param1 = pTree.get<std::string>("param1");
	dev.param2 = pTree.get<std::string>("param2");
	dev.cloneToDeviceId = pTree.get<int>("cloneToDeviceId");
	dev.hidden = pTree.get<bool>("hidden");
	dev.cacheRunning = pTree.get<bool>("cacheRunning");
}

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

void readFromPTree(boost::property_tree::ptree &pTree, DBCondition &cond)
{
	cond.id = pTree.get<int>("id");
	cond.name = pTree.get<std::string>("name");
	cond.deviceId = pTree.get<int>("deviceId", -1);
	cond.temperatureMin = pTree.get<float>("temperatureMin", NAN);
	cond.temperatureMax = pTree.get<float>("temperatureMax", NAN);
	cond.days = pTree.get<int>("days", -1);
	cond.useCalendar = pTree.get<bool>("useCalendar");
}

void readFromPTree(boost::property_tree::ptree &pTree, DBRoom &room)
{
	room.id = pTree.get<int>("id");
	room.name = pTree.get<std::string>("name");
	room.deviceTemperatureId = pTree.get<int>("deviceTemperatureId", -1);
	room.deviceHeatingId = pTree.get<int>("deviceHeatingId", -1);
	room.deviceHeaterId = pTree.get<int>("deviceHeaterId", -1);
}

void readFromPTree(boost::property_tree::ptree &pTree, DBRoomGraphCond &rgc)
{
    rgc.id = pTree.get<int>("id");
	rgc.roomId = pTree.get<int>("roomId", -1);
	rgc.graphId = pTree.get<int>("graphId", -1);
	rgc.conditionId = pTree.get<int>("conditionId", -1);
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
    case devDimmer: pTree.put("type", "devDimmer"); break;
    case devTV: pTree.put("type", "devTV"); break;
    default: pTree.put("type", "---unkown-type---"); break;
    }
	pTree.put("hardwareId", dev.hardwareId);
	pTree.put("cacheLifetime", dev.cacheLifetime);
	pTree.put("param1", dev.param1);
    pTree.put("param2", dev.param2);
	pTree.put("cloneToDeviceId", dev.cloneToDeviceId);
	pTree.put("hidden", dev.hidden ? "true" : "false");
	pTree.put("cacheRunning", dev.cacheRunning ? "true" : "false");
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
	if (std::isnormal(cond.temperatureMin))
        pTree.put("temperatureMin", cond.temperatureMin);
    else
        pTree.put("temperatureMin", "");
    if (std::isnormal(cond.temperatureMax))
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

void writeToPTree(boost::property_tree::ptree &pTree, MHEDevice &dev, bool useCache)
{
    pTree.put("id", dev.getId());
    switch (dev.getType())
    {
    case devTempHum: pTree.put("type", "devTempHum"); break;
    case devInterruptor: pTree.put("type", "devInterruptor"); break;
    case devDimmer: pTree.put("type", "devDimmer"); break;
    case devTV: pTree.put("type", "devTV"); break;
    default: pTree.put("type", "---unkown-type---"); break;
    }
    pTree.put("name", dev.getName());
    if (dev.getType() == devTempHum)
    {
        float temp = useCache ? dev.getCachedTemperature() : dev.getTemperature();
        float tempRaw = useCache ? dev.getCachedRawTemperature() : dev.getRawTemperature();

        pTree.put("humidity", useCache ? dev.getCachedHumidity() : dev.getHumidity());
        pTree.put("temperature", temp);
        if (std::isnormal(tempRaw) && temp != tempRaw)
            pTree.put("temperatureRaw", tempRaw);
    }
    else
        pTree.put("status", (useCache ? dev.isCachedActivated() : dev.isActivated()) ? "on" : "off");
    pTree.put("lastUpdate", dev.getLastUpdate());
    pTree.put("hidden", dev.isHidden() ? "true" : "false");
}

void writeToPTree(boost::property_tree::ptree &pTree, sMHEDeviceValue &value)
{
    pTree.put("date", value.date);
    if (std::isnormal(value.humdity))
        pTree.put("hum", value.humdity);
    if (std::isnormal(value.temperature))
        pTree.put("tem", value.temperature);
    if (value.status.size() > 0)
        pTree.put("status", value.status);
}

void writeToPTree(boost::property_tree::ptree &pTree, sMHEDeviceVolume &value)
{
    pTree.put("currentVol", value.currentVol);
    pTree.put("minVol", value.minVol);
    pTree.put("maxVol", value.maxVol);
}
