#ifndef DOMOTICZ_H_
#define DOMOTICZ_H_

#include <string>
#include <vector>

#include <boost/signals2/signal.hpp>

enum eDomoticzDeviceType
{
	none = -1,
	temperatureIn = 0,
	temperatureOut = 1,
	heater = 2
};

struct sDomoticzDevice
{
	int idx;
	std::string name;
	time_t lastUpdate;
	bool statusIsOn;
	float temperature;
	float humidity;

	sDomoticzDevice() : idx(-1), lastUpdate(0), statusIsOn(false), temperature(0), humidity(0)
	{
	}
};

class Domoticz
{
private:
	std::string serverUrl;
	std::string serverAuth;
	int plan;
	time_t lastUpdateDevices;
	std::vector<sDomoticzDevice*> devices;
	int idxDHT22, idxHeating, idxHeater, idxOutdoor;

	void removeDevices();
	void refreshData();

public:
	boost::signals2::signal<void(sDomoticzDevice *dev)> listenerDHT22;

	Domoticz(std::string &domoURL, std::string &domoAuth, int planIdx, int deviceIdxDHT22, int deviceIdxHeating, int deviceIdxHeater, int domoDeviceIdxOutdoor);
	~Domoticz();

	sDomoticzDevice *getDeviceDTH22();
	sDomoticzDevice *getDeviceOutdoor();
	bool setValuesDHT22(time_t timestamp, float temperature, float humidity);
	bool setValuesHeating(time_t timestamp, float temperature);
	bool setStatusHeater(bool activate);
};

#endif /* DOMOTICZ_H_ */
