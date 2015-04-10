#ifndef DOMOTICZ_H_
#define DOMOTICZ_H_

#include <string>
#include <vector>

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
	time_t lastUpdateDevices;
	std::vector<sDomoticzDevice> devices;
	int idxDHT22, idxHeating, idxHeater;

	void refreshData();

public:
	Domoticz(std::string &domoURL, std::string &domoAuth, int deviceIdxDHT22, int deviceIdxHeating, int deviceIdxHeater);
	~Domoticz();

	void get();
};

#endif /* DOMOTICZ_H_ */
