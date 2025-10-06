#pragma once
class Ports;

class Label
{
public:
	Ports *pbm;
	int		index;
	int		hub;
	double	time = 0;
	double	timeLoop1 = 0;
	double	timeLoop2 = 0;
	double	sailTime = 0;	//Sailing time from hub and ports included in "route"
	double	returnSailTime = 0;	//additional sailing time to go back to the hub port
	double	cargoTimeMPort = 0;	//time to pick up and delivery all cargoes of the route at the HUB
	double	cost = 0;
	double	bunkerCost = 0;
	double  cargoCost = 0;
	double	costPorts = 0;
	int		qPickUp = 0;
	int		qDelivery = 0;
	int		qNeed = 0;
	int		vessel;			//vessel that has enough capacity to ensure this label (route)
	int		nbVessels = 1;
	double	speed;
	//int		northestPost;
	//double	estimatedDoubleOH;
	//double	estimatedSingleOH;
	bool	isLoop2 = false;
	bool	toDelete = false;
	unsigned int	mod;
	int		nextPosition;	//next position to insert a new port -- for HEURISTIC only
	vector<int>		route;
	vector<double>		departureTimes;
	vector<double>		arrivalTimes;
	Label*		loop1;		//for loop2 we need a pointer for loop1
	vector<Label> setLoop2;

	Label()	{};
	Label(Ports *PBM)
	{
		pbm = PBM; mod = qPickUp = qDelivery = qNeed = 0;
		time = timeLoop1 = timeLoop2 = sailTime = returnSailTime = cargoTimeMPort = cost = costPorts = bunkerCost = cargoCost = 0;
		nbVessels = 1; nextPosition = -1;
		toDelete = false;
		loop1 = NULL;
	};
	Label(Ports *PBM, Label *loop1)
	{
		pbm = PBM; mod = qPickUp = qDelivery = qNeed = 0;
		time = timeLoop1 = timeLoop2 = sailTime = returnSailTime = cargoTimeMPort = cost = costPorts = bunkerCost = cargoCost = 0;
		vessel = loop1->vessel;
		nbVessels = 1; nextPosition = -1;
		toDelete = false;
		this->loop1 = loop1;
	};
	Label(const Label &lbl)
	{
		pbm = lbl.pbm; index = lbl.index; hub = lbl.hub; time = lbl.time; timeLoop1 = lbl.timeLoop1; timeLoop2 = lbl.timeLoop2;
		sailTime = lbl.sailTime; returnSailTime = lbl.returnSailTime; cargoTimeMPort = lbl.cargoTimeMPort; cost = lbl.cost;
		costPorts = lbl.costPorts; bunkerCost = lbl.bunkerCost; cargoCost = lbl.cargoCost;
		qPickUp = lbl.qPickUp; qDelivery = lbl.qDelivery; qNeed = lbl.qNeed; isLoop2 = lbl.isLoop2; toDelete = lbl.toDelete; mod = lbl.mod; nbVessels = lbl.nbVessels;
		speed = lbl.speed; vessel = lbl.vessel; nbVessels = lbl.nbVessels; nextPosition = lbl.nextPosition;
		//std::copy(lbl.route.begin(), lbl.route.end(), route.begin());
		route = lbl.route;
		arrivalTimes = lbl.arrivalTimes;
		departureTimes = lbl.departureTimes;
		setLoop2 = lbl.setLoop2;
		loop1 = lbl.loop1;
		//if (lbl.setLoop2.size() > 0)	
		//	std::copy(lbl.setLoop2.begin(), lbl.setLoop2.end(), setLoop2.begin());
	};
	~Label() {};
	void copyFrom(const Label &lbl)
	{
		pbm = lbl.pbm; index = lbl.index; hub = lbl.hub; time = lbl.time; timeLoop1 = lbl.timeLoop1; timeLoop2 = lbl.timeLoop2;
		sailTime = lbl.sailTime; returnSailTime = lbl.returnSailTime; cargoTimeMPort = lbl.cargoTimeMPort; cost = lbl.cost;
		costPorts = lbl.costPorts; bunkerCost = lbl.bunkerCost; cargoCost = lbl.cargoCost;
		qPickUp = lbl.qPickUp; qDelivery = lbl.qDelivery; qNeed = lbl.qNeed; isLoop2 = lbl.isLoop2; toDelete = lbl.toDelete; mod = lbl.mod;
		speed = lbl.speed; vessel = lbl.vessel; nbVessels = lbl.nbVessels; nextPosition = lbl.nextPosition;
		//std::copy(lbl.route.begin(), lbl.route.end(), route.begin());
		route = lbl.route;
		loop1 = lbl.loop1;
		arrivalTimes = lbl.arrivalTimes;
		departureTimes = lbl.departureTimes;
	};
	void setHubPort(int hub);
	void updateLabel(int port);
	void addPortHeuristic(int port);
	void updateLabelHeuristic();
	void copyToNewLabelHeuristic(Label &lbl);
	void updateLabelLoop2(Label &loop1, int port);
	void updateLabelLoop2Heuristic(Label &loop1);
	bool feasibleRoute();
	bool feasibleMotherRoute();
	bool feasibleRoute(int vesselCapacity);
	int  getDelivery();
	int  getPickUp();
	double getArrivalTime(int port);
	double getDepartureTime(int port);
	double getWholeTime();
	bool isPortVisited(int port);
	bool isVistedInLoop1(int port);
	bool hasHub();
	bool differentPorts(const Label &compare);
};

class LabelMother
{
public:
	enum route_type
	{
		PARTIAL_ROUTE,
		COMPLETE_ROUTE
	};

	Ports *pbm;
	static visit_frequency frequency;
	int		index;
	int		indexVesselType;
	int		CM;
	int		nbVessel;
	double	time = 0, timeCM = 0;
	double	sailTime = 0;	//Sailing time from hub and ports included in "route"
	int		northestPort, indexNorthestPort;
	bool	isLoop2 = false;
	unsigned int	mod;
	vector<int>		route, routeCM;
	vector<int>		mainPortsToVisit;
	int		_arrivalTimeCM;
	double	charterCost;
	double	bunkerCost = 0;
	double	bunkerCostCM = 0;
	double  cargoCost = 0;
	double	costPorts = 0;
	double	maxCargoHandled = 0, totalCargo = 0;
	double	costCM = 0;
	double	revenue = 0;
	bool	goingUP = true;
	cargo_type c_cargo;

	LabelMother(Ports *PBM, cargo_type ctype) { pbm = PBM; goingUP = true; c_cargo = ctype; };
	~LabelMother() {};
	static void setFrequency(visit_frequency freq);
	void addPort(int port);
	void computeSailingTime();
	void computeTimesWithCM_Port();	//link the current route with the CM port (close the loop) and compute the arrival times.
	void computeCostsWithCM_Port(); //compute the costs with CM port
	void updateMotherLabel(int port);
	bool feasibleRoute(route_type r_type);
	void updateDirection();
};

class LabelSetting
{
public:

	Ports *pbm;
	vector<vector<int>> portsToMain;	//ports that can be visited from a main port
	vector<int> vesselCapacity;			//set of vessels' capacity
	int nbVesselType;
	int nbReplicates;
	int K_M;
	
	cycle_type typeGeneratedRoutes;
	route_type typeRoute;
	cargo_type cargoType;
	visit_frequency frequency;
	control_speed speed_mode;

	vector<vector<int>> routes;
	vector<Label> setPossibleRoutes, setLabelsLoop2;
	vector<LabelMother> setPossibleMotherRoutes, setMotherLabels;

	vector<vector<Label>> setDaughterLabels;		//set of daughter Labels for each vessel type
	vector<vector<vector<Label*>>> setCombinedDSets;	//Indixes: vessel-index, main-port, daughter route d. Content: daughter route that can be combined to d
	vector<Label>		setDaughterLabelsHeuristic;	// for the heuristic
	vector<Label>		setMotherLabelsHeuristic;	// for the heuristic, and for non south-bound journey mother routes

	vector<int> levelSetPossibleRoutes;	//bookmark for the end of each level of the set of possible routes
	vector<int> levelSetLabels;			//bookmark for the end of each level of the set of labels
public:
	LabelSetting(Ports *pbm);
	~LabelSetting();

private:
	int isNonDominated(vector<Label> &set, Label &label);
	int isNonDominatedM(vector<Label> &set, Label &label);
	int checkDominancePossibleRoutes(Label &label);// , vector<int> &indexDeleted);
	int checkDominancePossibleRoutesM(Label &label);// , vector<int> &indexDeleted);
	void generateDaughterLabelsForVesselType(int main_port);
	void generateSecondLoopForVesselType(int m_port);
	void generateSecondLoopForVesselTypeHeuristic();
	//void generateSecondLoop(int main_port);
	void indexLabels();
	void indexLabelsHeuristic(vector<Label>& setLabels);
	double getLowestVesselSpeed(int indexVessel, double sailTimeWithStandardSpeed, double timeLimit);
	double computeVesselBunkerCost(int indexVessel, double sailTimeWithStandardSpeed, float speed);
public:
	void portAllocation();
	void portAllocationHeuristic(int K);	//K is the number of neighbors, going south direction
	void setRouteTypes(route_type type);
	void setCargoTypes(cargo_type c_type);
	void setVisitFrequency(visit_frequency freq);
	void setSpeedMode(control_speed speed);
	bool routesWithDaughterRoutes();
	void setMotherNeighbors(int K);
	void generateDaughterLabels(cycle_type cycle);
	void generateDaughterLabelsHeuristic(cycle_type cycle);
	//void generateDaughterLabelsForVesselTypeHeuristic(int vesselIndex);
	void generateDaughterLabelsForVesselTypeHeuristic();
	void computeDaughterCosts();
	void computeDaughterCostsHeuristic(vector<Label>& setLabels, bool mother_vessels=false);
	void generateMotherRoutes();
	void generateMotherRoutesV2();	//generates routes like daughter route way; no southbound journey
	void constructCombinedDaughterRoutes();
	int  getNbDaughterRoutes();
	int  getNbMotherRoutes();
	void transformFromShipTypeToSpecificShips(int nbReplicates);
	void writeDaughterRoutes(const char *fileName);
	void writeOPLFormat(const char *directory, const char *fileName, bool printDemand = false);
	Label *searchDaughterLabelByIndex(int indexLabel);
	void testRoutes(vector<vector<int>> mother_routes, vector<vector<int>> daughter_routes);
	//double computeTransitTime();
};

typedef std::vector<Label> Label_Vector;
typedef Label_Vector::iterator LabelIterator;
typedef std::vector<LabelMother> LabelMother_Vector;
typedef LabelMother_Vector::iterator LabelMotherIterator;
typedef vector<int>::iterator IntIterator;
