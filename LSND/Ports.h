#pragma once
#include <string.h>
class Ports
{
public:
	enum port_type
	{
		CP_port,	// continental port
		S_port,		//small port
		M_port,		//main port single visit
		M_port_double_visit	//main port double visitS
	};
	class SpeedRatio
	{
	public:
		double speed;
		double ratio;
	};
	class Port {
	public:
		int index;
		string name;
		string type;
		//string name;
		int toDeliver;
		int toPickUp;
	};
	class Vessel {
	public:
		int		capacity;
		int		charterCost;
		double	fuelConsumption;
		vector<SpeedRatio>	speed_ratio;
	};
	int nbPorts, nbPortsM, nbPortsS;
	vector<Port> portsM, portsS, ports;
	vector<Vessel> vessels, motherVessels;
	int nbVessels;
	vector<int> indexPorts;
	vector<vector<double>> distance_matrix, time_matrix;
	vector<vector<int>> neighborPorts;
	int totalPickUp, totalDelivery;
	static const int vesselSpeed = 12;
	static const int maxTimeFromHub = 150;
	static const double CARGO_HANDLING_TIME_PORT;
	static const double CARGO_HANDLING_TIME_M_PORT;
	static const double CARGO_HANDLING_TIME_CP;
	static const double CARGO_HANDLING_COST_PORT;
	static const double CARGO_HANDLING_COST_CP;
	static const double FIXED_PORT_COST_MOTHER;
	static  double FIXED_PORT_COST_DAUGHTER;
	//static const double COST_CONTAINER_OH;
	//static const double COST_CONTAINER_REST;
	static double FUEL_CONSUMPTION;
	static const double COST_BUNKER;
	static double TIME_CHARTER_MOTHER;
	static const double REVENUE_CONTAINER;
	static const int MAX_TIME = 168;
	static const int MAX_VISIT_MAIN_PORTS = 50;
public:
	Ports();
	~Ports();

	void readData(string fileName, route_type route_t, cycle_type cycle_t);
	void readDataWithIndex(string fileName, route_type route_t, cycle_type cycle_t);	//required a file with all ports
	void setDefautMotherVessel(int capacity);
	void setAllPortsToMain();
	int getPickUp(int indexPort);
	int getDeliver(int indexPort);
	port_type getPortType(int indexPort);
	void randomDemand();
	void displayDemand();
	void writeInstance(int IDfile);
};
