#pragma once
class Ports;
class LabelSetting;

class Solution
{
	enum port_status
	{
		SINGLE_VISIT,
		NORTH_VISIT,
		SOUTH_VISIT
	};
public:
	class MainPort {
	public:
		int port;
		port_status status;
		double arrivalTime;
		double departureTime;
		int	 deliveredAmount;
		int  pickedUpAmount;
		MainPort() { port = arrivalTime = departureTime = deliveredAmount = pickedUpAmount = 0; };
		MainPort(int p, port_status status) { this->port = p; this->status = status; arrivalTime = departureTime = deliveredAmount = pickedUpAmount = 0; };
		MainPort(int p, port_status status, double arrivalTime, double departureTime, int deliveredAmount, int pickedUpAmount) {
			this->port = p; this->status = status; this->arrivalTime = arrivalTime; this->departureTime = departureTime;
			this->deliveredAmount = deliveredAmount; this->pickedUpAmount = pickedUpAmount;
		};
	};
	Ports *pbm;
	LabelSetting *lbl;
	//int indexMRoute;
	vector<int> indexMRoutes;
	vector<pair<int, int>> motherVessels;	//type of vessel, number of vessels
	vector<pair<int, int>> indexDRoutes;	//vessel, route index
	vector<MainPort> mainPorts;
	vector<double> motherArrivalTimes, motherDepartureTimes;	//including the CP port as departure and arrival ports.
	vector<double> waitDeliveryCargo;	//each port has a demand to be delivered
	vector<double> waitPickedCargo;	//each port has a demand to be picked up
private:
	void computeMotherRouteTimes(int indexMRoute);
	int  getVisitedVessel(int indexMRoute, int port);
	//void getVisitIndexMPort(int port, int &index1, int &index2);
public:
	Solution(LabelSetting *lbl);
	void setMotherRoute(int indexM, int type, int nb);
	void addDaughterRoute(int vessel, int indexD);
	void computeTransitTimes();
	int numberMotherRoute();
	int numberDaughterRoutes();
	bool includeButterfly();

	double avgTransitTimeDelivery();
	double avgTransitTimePickUp();
	double agTransitTimeCargoes();
	void printUsedVessels();
	void printMotherVessels();
	void printRoutes();
	~Solution();
};

